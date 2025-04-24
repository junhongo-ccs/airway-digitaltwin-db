#include "Converter.h"
#include <memory>
#include <BS_thread_pool.hpp>
#include "Config.h"
#include "GeoUtil.h"
#include "GeoDataset.h"
#include "../../SpatialId/src/spatial_api.h"
#include "../../SpatialId/src/common/spatial_point.h"
#include "../../SpatialId/src/common/spatial_polygons.h"
#include "../ComLib/Voxel.h"
#include "DataAccessAdapter.h"
#include "DebugUtil.h"
#include "GeoPolygon.h"

Converter::Converter() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());

	//-----------------------------------------
	//座標変換関連
	//-----------------------------------------
	projContext = proj_context_create();
	if (projContext == nullptr) {
		throw std::runtime_error("PROJコンテキストが作成できません");
	}

	//mapEpsg -> pointOutLasEpsg
	if (config.getMapEpsg() != config.getPointOutLasEpsg()) {
		std::string fromCrs = "EPSG:" + std::to_string(config.getMapEpsg());
		std::string toCrs = "EPSG:" + std::to_string(config.getPointOutLasEpsg());

		logger->trace("map2Las：{0} -> {1}", fromCrs, toCrs);

		PJ* pjTemp = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
		if (pjTemp == nullptr) {
			throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
		}

		projMap2Las = proj_normalize_for_visualization(projContext, pjTemp);
		proj_destroy(pjTemp);
		if (projMap2Las == nullptr) {
			throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
		}

	}

	//pointOutLasEpsg → World
	if (config.getPointOutLasEpsg() != static_cast<int>(spatialid::CRS::kWGS84)) {
		std::string fromCrs = "EPSG:" +std::to_string(config.getPointOutLasEpsg());
		std::string toCrs = "EPSG:" + std::to_string(static_cast<int>(spatialid::CRS::kWGS84));

		logger->trace("Las2World：{0} -> {1}", fromCrs, toCrs);

		PJ* pjTemp = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
		if (pjTemp == nullptr) {
			throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
		}

		projLas2World = proj_normalize_for_visualization(projContext, pjTemp);
		proj_destroy(pjTemp);
		if (projLas2World == nullptr) {
			throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
		}

	}

}

void Converter::convert() {
	Config& config = Config::getInstance();

	//DBアクセス用
	DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
	adapter.connect(config);

	//前回の出力ファイルを削除
	if (config.getBatchFile().empty()) {
		for (PolygonType type : config.getTypeList()) {
			std::map<std::string, std::vector<CityObject*>> dummy;
			std::string dataType = getDataType(type);
			deleteOldFiles(adapter, dataType, dummy);


		//	logger->trace("前回のファイルを削除:{0}, dataType:{1}", config.getOutputFolder(), config.getFeatureId(type));

		//	std::string batchFile = config.getBatchFile();
		//	CommonUtil::deleteFiles(config.getOutputFolder(), batchFile, type);

		//	std::string tmpSpatialId = batchFile;
		//	if (!tmpSpatialId.empty()) {
		//		std::replace(tmpSpatialId.begin(), tmpSpatialId.end(), '-', '/');
		//	}

		//	std::string ownerId = config.getOwnerId();
		//	std::string batchId = config.getBatchId();
		//	std::string recId = FIXED_REC_ID;
		//	std::string dataType = getDataType(type);
		//	int rc = adapter.deleteSpaceDataManages(ownerId, batchId, tmpSpatialId, recId, dataType);

		//	logger->trace("削除されたレコード数：{0}, dataType:{1}", rc, config.getFeatureId(type));
		}
	}


	//gmlファイルリストを取得する
	//std::vector<std::string> featureIds;

	//std::vector<std::string> fileList;
	for (auto const& fileType : config.getTypeList()) {
		std::string featureId = config.getFeatureId(fileType);
		std::string dataType = getDataType(fileType);
		std::string folder = config.getInputDir(featureId);

		std::string ptn = "^.+\\." + config.getInputFileExtension() + "$";

		logger->info("---- 地物ID:{0} ----", featureId);
		if (!std::filesystem::exists(folder)) {
			logger->warn("フォルダ{0}が存在していないため、スキップ", folder);
			continue;
		}

		std::vector<std::string> fileList = ComLib::CommonUtil::getFileList(folder, ptn, config.getBatchFile());

		logger->debug("対象ファイル数：{0}", fileList.size());

		//ファイルごとの処理
		int curCnt = 0;
		int fileCount = (int)fileList.size();
		for (auto const& filePath : fileList) {
			clock_t ts, te;
			ts = clock();

			curCnt++;
			std::string baseFileName = std::filesystem::path(filePath).filename().string();

			logger->info("{0} START ({1}/{2})", baseFileName, curCnt, fileCount);

			//ファイル基本情報を取得
			CityGmlFileInfo fileInfo;
			fileInfo.featureId = featureId;
			fileInfo.dataType = dataType;
			fileInfo.originalFileId = baseFileName;
			fileInfo.crs = std::to_string(config.getMapEpsg());
			fileInfo.targetLod = config.getTargetLod();

			//GeoUtil::getCityGmlFileInfoFromFileName(filePath, fileInfo);

			logger->trace("meshcode:{0},type:{1},crs:{2},lod:{3}", fileInfo.meshcd,
				fileInfo.featureId, fileInfo.crs, fileInfo.targetLod);

			//gmlファイルをロード
			clock_t tmpS = clock();
			std::shared_ptr<const citygml::CityModel> city;

			citygml::ParserParams params;
			city = citygml::load(filePath, params, nullptr);

			if (city == nullptr) {
				throw std::runtime_error("ロードに失敗しました");
			}

			GeoDataset gds;

			//ファイルの読み込み
			loadCityGmlFile(gds, fileInfo, filePath);

			clock_t tmpE = clock();
			logger->debug("CityGmlの読み込み:{0:.2f}(秒)、オブジェクト数：{1}",
				(double)(tmpE - tmpS) / CLOCKS_PER_SEC, gds.getCityObjectCount());

			//ポリゴンから空間IDリストを求める
			tmpS = clock();
			createVoxels(gds, fileInfo, config, adapter);

			logger->debug("createVoxels:{0:.2f}(秒)", (double)(clock() - tmpS) / CLOCKS_PER_SEC);

			te = clock();

			logger->info("{0} END 処理時間:{1}(秒)", baseFileName, (double)(te - ts) / CLOCKS_PER_SEC);

#ifdef _WIN32
			if (logger->should_log(spdlog::level::debug) || logger->should_log(spdlog::level::trace)) {
				DebugUtil::printMemoryInfo();
			}
#endif
		}
	}
}

bool Converter::loadCityGmlFile(GeoDataset& gds, CityGmlFileInfo& fileInfo, const std::string& filePath) {
	//gmlファイルをロード
	clock_t ts = clock();

	Config& config = Config::getInstance();

	std::string featureId = fileInfo.featureId;

	std::shared_ptr<const citygml::CityModel> city;

	citygml::ParserParams params;
	city = citygml::load(filePath, params, nullptr);

	if (city == nullptr) {
		throw std::runtime_error("ロードに失敗しました");
	}

	if (config.needDebugOut()) {
		DebugUtil::dumpCityModel(city, logger);
	}

	if (featureId == FEATURE_BUILDING) { //下のelseのロジックでもよさそうだけど、テストも進んでいるのでこのまま
		std::string lonlatOrder = config.getLonlatOrder();

		const auto& rootCityObjects = city->getRootCityObjects();

		for (auto const& rootCityObject : rootCityObjects) {
			CityObject* pCityObject = nullptr;

			int geometryCount = rootCityObject->getGeometriesCount();

			for (int i = 0; i < geometryCount; i++) {
				auto const& geometry = rootCityObject->getGeometry(i);
				int subGeometryCount = geometry.getGeometriesCount();

				//logger->info(" i={0}, lod={1}, geoType:{2}, subGeometryCount:{3}, polygonCount:{4}",
				//	 i, geometry.getLOD(),
				//	geometry.getTypeAsString(), subGeometryCount, geometry.getPolygonsCount());

				//if (featureId == FEATURE_BUILDING) {
					//対象LODかをチェック
					if (geometry.getLOD() != fileInfo.targetLod) {
						continue;
					}

					//LOD1の場合subgeometryをチェックする必要はある
					for (int s = 0; s < subGeometryCount; s++) {
						auto const& subGeometry = geometry.getGeometry(s);
						int polygonCount = subGeometry.getPolygonsCount();

						//std::cout << "geometry:" << i << ",subGemotry:" << s << ",polygonCount:"
						//	<< polygonCount << ",type:" << subGeometry.getTypeAsString() << std::endl;


						for (int j = 0; j < polygonCount; j++) {
							auto polygon = subGeometry.getPolygon(j);
							auto& vertices = polygon->getVertices();

							//logger->trace("polygon:{0}, vertex:{1}", j, vertices.size());

							GeoPoint* pPoints = gds.allocatePoint((int)vertices.size());
							GeoPolygon* pPolygon = gds.allocatePolygon();
							int ind = 0;
							for (auto& vertex : vertices) {
								if (lonlatOrder == "auto") {
									lonlatOrder = (vertex.x > 100 ? "lonlat" : "latlon");
								}

								pPoints[ind].x = (lonlatOrder == "lonlat" ? vertex.x : vertex.y);
								pPoints[ind].y = (lonlatOrder == "lonlat" ? vertex.y : vertex.x);
								pPoints[ind].z = vertex.z;
								ind++;
							}


							pPolygon->addPoints((int)vertices.size(), pPoints);

							//座標変換
							transform(pPolygon, projMap2Las);

							if (pCityObject == nullptr) {
								pCityObject = gds.allocateCityObject();
								pCityObject->setObjectId(rootCityObject->getId());
								//std::cout << "path1" << std::endl;
								gds.addCityObject(pCityObject);
							}
							pCityObject->addPolygon(pPolygon);

						}
					}
				//}
			}
		}
		//std::cout << "type:" << rootCityObject->getTypeAsString() << ",geometryCount:" << rootCityObject->getGeometriesCount() <<
		//	",children:" << rootCityObject->getChildCityObjectsCount() << std::endl;
	}
	else {
		//データサンプル:
		//geometryCount=2 ->  LOD1, LOD2
		//subgeometryCount = 0 -> polygonCountをチェック
		//subgeometryCount > 0 -> subGeometryのpolygonCountをチェック
		//childCityObjectCount > 0のパターンもあるか
		const auto& rootCityObjects = city->getRootCityObjects();

		logger->debug("root count: {0}", rootCityObjects.size());
		for (auto const& rootCityObject : rootCityObjects) {

			CityObject* pCityObject = nullptr;

			int geometryCount = rootCityObject->getGeometriesCount();

			logger->debug("root-objId:{0},geometryCount:{1}, objType:{2}, childCount:{3}",
				rootCityObject->getId(), geometryCount,
				rootCityObject->getTypeAsString(), rootCityObject->getChildCityObjectsCount());


			if (featureId == FEATURE_GROUND) {
				for (int kk = 0; kk < rootCityObject->getChildCityObjectsCount(); kk++) {
					const auto& child = rootCityObject->getChildCityObject(kk);
					recursiveLoadCityObject(child, gds, fileInfo, config, pCityObject);
				}
			}


			for (int i = 0; i < geometryCount; i++) {
				auto const& geometry = rootCityObject->getGeometry(i);
				//std::cout << "++++ LOD:" << geometry.getLOD() << std::endl;

				if (geometry.getLOD() != fileInfo.targetLod) {
					logger->debug("++ LOD:{0}", geometry.getLOD());
					continue;
				}

				recursiveLoadGeometry(geometry, gds, fileInfo, config, pCityObject);
			}

			if (pCityObject != nullptr) {
				pCityObject->setObjectId(rootCityObject->getId());

				logger->debug("polygonCount:{0}", pCityObject->getPolyCount());
			}

			logger->debug("------ rootCityObject END ------");
		}


	}

	clock_t te = clock();
	logger->trace("ファイルのロード時間：{0:.2f}(秒)", (double)(te - ts) / CLOCKS_PER_SEC);

	return true;
}

void Converter::recursiveLoadCityObject(const citygml::CityObject& cityObject, GeoDataset& gds, 
		CityGmlFileInfo& fileInfo, Config& config, CityObject*& pCityObject) {
	for (int i = 0; i < cityObject.getChildCityObjectsCount(); i++) {
		const auto& subObj = cityObject.getChildCityObject(i);
		recursiveLoadCityObject(subObj, gds, fileInfo, config, pCityObject);
	}

	for (int ii = 0; ii < cityObject.getGeometriesCount(); ii++) {
		const auto& geometry = cityObject.getGeometry(ii);

		if (geometry.getLOD() != fileInfo.targetLod) {
			continue;
		}

		recursiveLoadGeometry(geometry, gds, fileInfo, config, pCityObject);
	}

}

void Converter::recursiveLoadGeometry(const citygml::Geometry& geometry, GeoDataset& gds, 
	CityGmlFileInfo& fileInfo, Config& config, CityObject*& pCityObject) {

	logger->debug("# lod:{0}, polygonCount:{1}, lineCount:{2}, geometryCount:{3}, type;{4}",
		geometry.getLOD(),
		geometry.getPolygonsCount(), geometry.getLineStringCount(), geometry.getGeometriesCount(),
		geometry.getTypeAsString());

	for (int i = 0; i < geometry.getGeometriesCount(); i++) {
		const auto& sub = geometry.getGeometry(i);
		recursiveLoadGeometry(sub, gds, fileInfo, config, pCityObject);
	}

	std::string lonlatOrder = config.getLonlatOrder();

	int polygonCount = geometry.getPolygonsCount();


	for (int j = 0; j < polygonCount; j++) {
		auto polygon = geometry.getPolygon(j);
		auto& vertices = polygon->getVertices();

		//logger->trace("polygon:{0}, vertex:{1}", j, vertices.size());

		GeoPoint* pPoints = gds.allocatePoint((int)vertices.size());
		GeoPolygon* pPolygon = gds.allocatePolygon();
		int ind = 0;
		for (auto& vertex : vertices) {
			if (lonlatOrder == "auto") {
				lonlatOrder = (vertex.x > 100 ? "lonlat" : "latlon");
			}

			pPoints[ind].x = (lonlatOrder == "lonlat" ? vertex.x : vertex.y);
			pPoints[ind].y = (lonlatOrder == "lonlat" ? vertex.y : vertex.x);
			pPoints[ind].z = vertex.z;
			ind++;
		}


		pPolygon->addPoints((int)vertices.size(), pPoints);

		//座標変換
		transform(pPolygon, projMap2Las);

		if (pCityObject == nullptr) {
			pCityObject = gds.allocateCityObject();
			//pCityObject->setObjectId(rootCityObject->getId());
			//std::cout << "path1" << std::endl;
			gds.addCityObject(pCityObject);
		}
		pCityObject->addPolygon(pPolygon);

	}

}


void Converter::createVoxels(GeoDataset& gds, CityGmlFileInfo& fileInfo, Config& config,
		DataAccessAdapter& adapter) {
	int cityObjectCount = gds.getCityObjectCount();
	CityObject** ppCityObject = gds.getCityObjectRef();

	//空間ＩＤに紐づいているオブジェクト
	std::map<std::string, std::vector<CityObject *>> spatialIdToObjectMap;

	//std::cout << "##cityObjectCount:" << cityObjectCount << std::endl;

	////複数空間ＩＤにまたがっているオブジェクトリスト
	std::vector<CityObject *> objWithMultiIds;

	//SpaceDataManagesInfo用
	std::map<std::string, std::shared_ptr<SpaceDataManagesInfo>> spaceDataManagesInfoMap;

	//ローカルズーム
	int localZoom = config.getLocalZoom();
	if (localZoom < spatialid::kGlobalZoom) {
		localZoom = spatialid::kLocalZoom;
	}

	//入力ファイルのCRS
	int crs = std::atoi(fileInfo.crs.c_str());
	if (projMap2Las != nullptr) {
		crs = config.getPointOutLasEpsg();
	}

	//std::cout << "############# crs:" << crs << std::endl;

	//処理対象の空間ＩＤ。基本的にデバッグ用
	const std::vector<std::string>& targetSpatialIds = config.getTargetSpatialIds();
	logger->trace("targetSpatialIds.size:{0}", targetSpatialIds.size());

	int exceptionCount = 0; //for debug
	clock_t ts = clock();
	for (int i = 0; i < cityObjectCount; i++) {
		clock_t t1 = clock();
		CityObject* pCityObject = ppCityObject[i];

		logger->trace("空間ID算出。オブジェクト：{0} START ({1}/{2})", pCityObject->getObjectId(), (i + 1), cityObjectCount);

		std::vector<spatialid::Triangle>& triangles = pCityObject->getTriangles();

		int polygonCount = pCityObject->getPolyCount();
		GeoPolygon** ppPolygon = pCityObject->getPolygonRef();

		double maxZ = 0;
		for (int j = 0; j < polygonCount; j++) {
			GeoPolygon* pPolygon = ppPolygon[j];
			GeoPoint** ppPoints = pPolygon->getPointRef();

			//平面の向きを調べる
			if (fileInfo.featureId == FEATURE_ROAD) {
				pPolygon->setPlaneType(PlaneType::PLANE_XY);
			}
			else {
				pPolygon->decidePlaneType();
			}

			//三角形分割
			pPolygon->doTriangulation();

			//三角形リストに追加
			for (int k = 0; k < pPolygon->getTriangleIndices().size(); k += 3) {
				if (k + 3 > pPolygon->getTriangleIndices().size()) {
					logger->warn("+++ size of index:{0}", pPolygon->getTriangleIndices().size());
					break;
				}

				spatialid::Point point[3];

				for (int p = 0; p < 3; p++) {
					int ind = pPolygon->getTriangleIndices()[k + p];

					point[p].x = ppPoints[ind]->x;
					point[p].y = ppPoints[ind]->y;
					point[p].z = ppPoints[ind]->z;

					if (point[p].z > maxZ) {
						maxZ = point[p].z;
					}
				}

				try {
					spatialid::Triangle triangle(point[0], point[1], point[2]);

					triangles.push_back(triangle);
				}
				catch (std::invalid_argument& ex) {
					logger->debug("error occured. cause:{0}", ex.what());

					exceptionCount++;

					int ind = pPolygon->getTriangleIndices()[k + 0];
					logger->trace("k={6}, p[0]=({0},{1}), p[1]=({2},{3}), p[2]=({4},{5})", ppPoints[ind + 0]->x,
						ppPoints[ind + 0]->y, ppPoints[ind + 1]->x, ppPoints[ind + 1]->y,
						ppPoints[ind + 2]->x, ppPoints[ind + 2]->y, k);
					continue;
				}
			}
		}
		pCityObject->setMaxZ(maxZ);

		//空間ID(globalZoom)を求める
		std::vector<spatialid::Triangle> empty;

		spatialid::get_global_spatial_ids_on_polygons(triangles, empty,
			pCityObject->getSpatialIds(),
			static_cast<spatialid::CRS>(crs), false, false);


		for (auto const& spatialId : pCityObject->getSpatialIds()) {
			////複数IDにまたがっているオブジェクトの高さが閾値を超えていたら、あとで処理するようにリストに入れておく
			//if (pCityObject->getSpatialIds().size() > 1 &&
			//		pCityObject->getMaxZ() > config.getBldgHeightThreshold()) {
			//	if (std::find(objWithMultiIds.begin(), objWithMultiIds.end(), pCityObject) == objWithMultiIds.end()) {
			//		objWithMultiIds.push_back(pCityObject);
			//	}
			//	continue;
			//}
			if (logger->should_log(spdlog::level::trace)) {
				if (pCityObject->getSpatialIds().size() > 1 ) {
					if (std::find(objWithMultiIds.begin(), objWithMultiIds.end(), pCityObject) == objWithMultiIds.end()) {
						objWithMultiIds.push_back(pCityObject);
					}
				}
			}

			//空間ＩＤ単位でまとめて処理するオブジェクト
			if (spatialIdToObjectMap.find(spatialId) == spatialIdToObjectMap.end()) {
				std::vector<CityObject*> list;
				list.push_back(pCityObject);
				spatialIdToObjectMap.insert(std::make_pair(spatialId, list));
			}
			else {
				std::vector<CityObject*>& list = spatialIdToObjectMap.at(spatialId);
				list.push_back(pCityObject);
			}
		}
		logger->trace("空間ID算出。オブジェクト：{0} END 処理時間：{1:.3f}", pCityObject->getObjectId(),
			(double)(clock()-t1)/CLOCKS_PER_SEC);
	}

	if (exceptionCount > 0) {
		logger->warn("++ exceptionCount:{0}", exceptionCount);
	}

	if (logger->should_log(spdlog::level::trace)) {
		for (auto& pObj : objWithMultiIds) {
			std::string str;
			for (auto& tmpId : pObj->getSpatialIds()) {
				if (!str.empty()) {
					str += ",";
				}
				str += tmpId;
			}
			logger->trace("obj:{0}, ids:{1}", pObj->getObjectId(), str);
		}
	}

	logger->trace("三角形分割とグロバール空間IDリスト作成：{0:.2f}", (double)(clock() - ts) / CLOCKS_PER_SEC);

	//batchFile指定の場合このタイミングで古いファイルを削除
	if (!config.getBatchFile().empty()) {
		deleteOldFiles(adapter, fileInfo.dataType, spatialIdToObjectMap);
	}

	//-----------------------------------------------------
	//２パターンのロジックが用意されている
	//　①空間ＩＤ単位での処理
	//　②オブジェクト単位での処理
	//-----------------------------------------------------

	//①空間ＩＤ単位での処理
	logger->debug("空間ＩＤ単位での処理　START、オブジェクト数:{0}", spatialIdToObjectMap.size());
	ts = clock();
	
	int numOfSpatialId = spatialIdToObjectMap.size();
	int spatialIdCount = 0;
	for (auto iter = spatialIdToObjectMap.begin(); iter != spatialIdToObjectMap.end(); iter++) {
		clock_t startTime = clock();
		std::string spatialId = iter->first;
		std::vector<CityObject*> objList = iter->second;
		spatialIdCount++;
		logger->info("spatialId:{0}　START、オブジェクト数：{1}  ({2}/{3})", spatialId,
			objList.size(), spatialIdCount, spatialIdToObjectMap.size());


		if (objList.size() == 0) {
			logger->info("空間ＩＤ:{0}　END", spatialId);
			continue;
		}

		if (targetSpatialIds.size() > 0 && 
				std::find(targetSpatialIds.begin(), targetSpatialIds.end(), spatialId) == targetSpatialIds.end()) {
			logger->info("空間ＩＤ:{0}　SKIP", spatialId);
			continue;
		}
#ifdef _WIN32
		if (logger->should_log(spdlog::level::debug) || logger->should_log(spdlog::level::trace)) {
			DebugUtil::printMemoryInfo();
		}
#endif

		clock_t t1 = clock();

		std::shared_ptr<SpaceDataManagesInfo> ptr;
		if (spaceDataManagesInfoMap.find(spatialId) == spaceDataManagesInfoMap.end()) {
			ptr = initiaizeSpaceDataManagesInfo(config, spatialId, fileInfo);
			spaceDataManagesInfoMap.insert(std::make_pair(spatialId, ptr));
		}
		else {
			ptr = spaceDataManagesInfoMap[spatialId];
		}


		//ボクセルを作成	
		std::string fileName = ptr->fileId; 
		std::string gzFileName = ptr->filePath; // ComLib::CommonUtil::pathJoin(config.getOutputFolder(), fileName);
		ComLib::Voxel voxel(spatialId, localZoom, config.getPointOutLasEpsg(), gzFileName, config.getLoggerName());
		voxel.setScaleZ(config.getLasScaleZ());

		if (objList.size() == 1 || config.getThreadCount() <= 1) {
			//シングルスレッド
			for (auto const pCityObject : objList) {
				std::vector<spatialid::Triangle>& triangles = pCityObject->getTriangles();
				voxel.addPolygon(triangles, static_cast<spatialid::CRS>(crs));
			}
		}
		else {
			//マルチスレッド
			int threadCount = config.getThreadCount();
			if (objList.size() < threadCount) {
				threadCount = objList.size();
			}

			BS::thread_pool pool(threadCount);
			for (int i = 0; i < objList.size(); i++) {
				pool.push_task(
					[&voxel, &objList, crs, i] {
						try {
							CityObject* pCityObject = objList[i];
							std::vector<spatialid::Triangle>& triangles = pCityObject->getTriangles();
							voxel.addPolygon(triangles, static_cast<spatialid::CRS>(crs));
						}
						catch (std::exception& ex) {
							std::cout << ex.what() << std::endl;
							throw ex;
						}
					});
			}

			pool.wait_for_tasks();
		}

		int altCount = voxel.getVoxelData().size();
		logger->trace("create voxel。spaticlId:{0}, elapsed time:{1:.2f}(sec)", 
			spatialId,(double)(clock() - t1) / CLOCKS_PER_SEC);
		if (altCount <= 0) {
			logger->info("spatialId:{0} END (データなし） (elapsed time:{1:.2f}(sec))", spatialId,
				(double)(clock() - startTime) / CLOCKS_PER_SEC);
			continue;
		}

		//ファイルへ出力
		t1 = clock();
		logger->trace("++ color:{0:x}, featrueId:{1}", config.getColor(fileInfo.featureId), fileInfo.featureId);
		voxel.toFile(config.getLasScale(), config.getColor(fileInfo.featureId));

		//DB登録
		logger->trace("---- DB登録開始");
		voxel.getLasMinMax(ptr->xmin, ptr->xmax, ptr->ymin, ptr->ymax, ptr->zmin, ptr->zmax);
		int cls = getLasClassfication(fileInfo.featureId);
		int pointCount = voxel.getLasPointCount();
		if (cls <= 20) {
			ptr->clsPoints[cls] = pointCount;
		}
		else {
			ptr->clsPoints[21] = pointCount;
		}
		ptr->points = pointCount;
		calcWorldMinMax(ptr->xmin, ptr->xmax, ptr->ymin, ptr->ymax, ptr->xminW, ptr->xmaxW, ptr->yminW, ptr->ymaxW);
		adapter.registSpaceDataManages(*ptr);
		logger->trace("output to file.spaticlId:{0}, elapsed time:{1:.2f}(sec)", spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);

		logger->info("spatialId:{0} END (elapsed time:{1:.2f}(sec))", spatialId,
			(double)(clock()-startTime)/CLOCKS_PER_SEC);

	}
	logger->debug("空間ＩＤ単位での処理　END (経過時間：{0:.2f})", (double)(clock() - ts) / CLOCKS_PER_SEC);

	
}

std::string Converter::getDataType(PolygonType type) {
	std::string dataType = "";

#if 1
	Config& config = Config::getInstance();

	dataType = config.getFeatureId(type);
	
#else
	switch (type) {
	case TYPE_BUILDING:
		dataType = "1";
		break;
	case TYPE_ROAD:
		dataType = "4";
		break;
	case TYPE_WATER:
		dataType = "5";
		break;
	case TYPE_RAILWAY:
		dataType = "6";
		break;
	case TYPE_STEELTOWER:
		dataType = "2";
		break;
	case TYPE_POWERLINE:
		dataType = "3";
		break;
	}
#endif

	return dataType;
}

std::shared_ptr<SpaceDataManagesInfo> Converter::initiaizeSpaceDataManagesInfo(Config& config, 
		std::string& spatialId, CityGmlFileInfo& fileInfo) {
	std::shared_ptr<SpaceDataManagesInfo> ptr(new SpaceDataManagesInfo());
	ptr->ownerId = config.getOwnerId();
	ptr->batchId = config.getBatchId();
	ptr->spatialId = spatialId;
	ptr->recId = FIXED_REC_ID;
	ptr->dataType = fileInfo.dataType;

	std::vector<spatialid::Point> vertex;
	spatialid::get_vertex_points_on_spatial_id(spatialId, vertex, static_cast<spatialid::CRS>(config.getPointOutLasEpsg()));

	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
	double xminW = DBL_MAX, xmaxW = -DBL_MAX, yminW = DBL_MAX, ymaxW = -DBL_MAX;
	for (auto& pt : vertex) {
		xmin = std::min(xmin, pt.x);
		xmax = std::max(xmax, pt.x);
		ymin = std::min(ymin, pt.y);
		ymax = std::max(ymax, pt.y);
		zmin = std::min(zmin, pt.z);
		zmax = std::max(zmax, pt.z);
	}

	if (projLas2World != nullptr) {
		double lon[2], lat[2];
		lon[0] = xmin;
		lat[0] = ymin;
		lon[1] = xmax;
		lat[1] = ymax;

		CommonUtil::transform(2, &lon[0], &lat[0], projLas2World);

		xminW = lon[0];
		yminW = lat[0];
		xmaxW = lon[1];
		ymaxW = lat[1];

	}
	else {
		xminW = xmin;
		xmaxW = xmax;
		yminW = ymin;
		ymaxW = ymax;
	}

	ptr->spXmin = xmin;
	ptr->spXmax = xmax;
	ptr->spYmin = ymin;
	ptr->spYmax = ymax;
	ptr->spZmin = zmin;
	ptr->spZmax = zmax;
	ptr->spXminW = xminW;
	ptr->spXmaxW = xmaxW;
	ptr->spYminW = yminW;
	ptr->spYmaxW = ymaxW;

	std::string fileName = spatialId + "_" + fileInfo.featureId + ".las";
	std::replace(fileName.begin(), fileName.end(), '/', '-');
	std::string gzFileName = ComLib::CommonUtil::pathJoin(config.getOutputFolder(), fileName);
	ptr->fileId = fileName;
	ptr->filePath = gzFileName;
	ptr->epsgLas = std::to_string(config.getMapEpsg());
	ptr->epsgCsv = std::to_string(config.getPointOutLasEpsg());
	ptr->dataUnit = FIXED_DATA_UNIT;
	ptr->dataIo = FIXED_DATA_IO;
	ptr->dataStatus = "";
	ptr->dataAbcd = "ABCD";
	ptr->originalFileIds.push_back(fileInfo.originalFileId);
	ptr->points = 0;

	//xmin,xmax,ymin,ymax,zmin,zmax,xminW,xmaxW,yminW,ymaxWはあとで決定

	for (int i = 0; i < 21; i++) {
		ptr->clsPoints.push_back(0);
	}
	ptr->clsPoints.push_back(0); //99
	ptr->comment = "";

	return ptr;
}

void Converter::calcWorldMinMax(double xmin, double xmax, double ymin, double ymax,
	double& xminW, double& xmaxW, double& yminW, double& ymaxW) {
	double lon[2], lat[2];
	lon[0] = xmin;
	lat[0] = ymin;
	lon[1] = xmax;
	lat[1] = ymax;

	CommonUtil::transform(2, &lon[0], &lat[0], projLas2World);

	xminW = lon[0];
	yminW = lat[0];
	xmaxW = lon[1];
	ymaxW = lat[1];

}

int Converter::getLasClassfication(std::string& featureId) {
	int cls = 99;
	
	if (featureId == FEATURE_BUILDING) {
		cls = 6;
	}
	else if (featureId == FEATURE_ROAD) {
		cls = 11;
	}
	else if (featureId == FEATURE_RAILWAY) {
		cls = 10;
	}
	else if (featureId == FEATURE_WATER) {
		cls = 9;
	}
	else if (featureId == FEATURE_STEELTOWER) {
		cls = 15;
	}
	else if (featureId == FEATURE_POWERLINE) {
		cls = 14;
	}
	else if (featureId == FEATURE_GROUND) {
		cls = 2;
	}
	else {
		cls = 99;
	}

	return cls;
}

void Converter::transform(GeoPolygon* pPolygon, PJ* proj) {
	if (proj == nullptr) {
		return;
	}

	int num = pPolygon->getNumOfPoints();

	GeoPoint** ppPoint = pPolygon->getPointsRef();

	PJ_COORD input_coords, output_coords;

	for (int i = 0; i < num; i++) {
		input_coords = proj_coord(ppPoint[i]->x, ppPoint[i]->y, 0, 0);
		output_coords = proj_trans(proj, PJ_FWD, input_coords);
		ppPoint[i]->orgX = ppPoint[i]->x;
		ppPoint[i]->orgY = ppPoint[i]->y;
		ppPoint[i]->x = output_coords.xy.x;
		ppPoint[i]->y = output_coords.xy.y;

	}
}

void Converter::deleteOldFiles(DataAccessAdapter& adapter, std::string& dataType,
		std::map<std::string, std::vector<CityObject*>>& spatialIdMap) {
	Config& config = Config::getInstance();

	if (config.getBatchFile().empty()) {
		logger->info("前回のファイルを削除:{0}, dataType:{1}", config.getOutputFolder(), dataType);

		std::string tmpSpatialId;
		CommonUtil::deleteFiles(config.getOutputFolder(), tmpSpatialId, dataType);

		std::string ownerId = config.getOwnerId();
		std::string batchId = config.getBatchId();
		std::string recId = FIXED_REC_ID;
		int rc = adapter.deleteSpaceDataManages(ownerId, batchId, tmpSpatialId, recId, dataType);

		logger->info("削除されたレコード数：{0}, dataType:{1}", rc, dataType);
	}
	else {
		logger->info("前回のファイルを削除:{0}, dataType:{1}", config.getOutputFolder(), dataType);
		//同一空間IDが複数ファイルに含まれることがあるので、このモードは基本的デバッグ用。
		int delCount = 0;
		for (auto& pair : spatialIdMap) {
			std::string tmpSpatialId = pair.first;
			std::string tmpSpatialIdWithoutSlash = tmpSpatialId;
			std::replace(tmpSpatialIdWithoutSlash.begin(), tmpSpatialIdWithoutSlash.end(), '/', '-');

			CommonUtil::deleteFiles(config.getOutputFolder(), tmpSpatialIdWithoutSlash, dataType);

			std::string ownerId = config.getOwnerId();
			std::string batchId = config.getBatchId();
			std::string recId = FIXED_REC_ID;
			int rc = adapter.deleteSpaceDataManages(ownerId, batchId, tmpSpatialId, recId, dataType);
			delCount += rc;
		}
		logger->info("削除されたレコード数：{0}, dataType:{1}", delCount, dataType);
	}
}