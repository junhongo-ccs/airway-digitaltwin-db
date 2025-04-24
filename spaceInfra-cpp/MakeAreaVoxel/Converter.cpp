#include "Converter.h"
#include <memory>
#include <BS_thread_pool.hpp>
#include "Config.h"
#include "GeoDataset.h"
#include "../../SpatialId/src/spatial_api.h"
#include "../../SpatialId/src/common/spatial_point.h"
#include "../../SpatialId/src/common/spatial_polygons.h"
#include "../ComLib/Voxel.h"
#include "DataAccessAdapter.h"
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

	//mapEpsg -> confirmLasEpsg
	if (config.getMapEpsg() != config.getConfirmLasEpsg()) {
		std::string fromCrs = "EPSG:" + std::to_string(config.getMapEpsg());
		std::string toCrs = "EPSG:" + std::to_string(config.getConfirmLasEpsg());

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

	////pointOutLasEpsg → World
	//if (config.getPointOutLasEpsg() != static_cast<int>(spatialid::CRS::kWGS84)) {
	//	std::string fromCrs = "EPSG:" +std::to_string(config.getPointOutLasEpsg());
	//	std::string toCrs = "EPSG:" + std::to_string(static_cast<int>(spatialid::CRS::kWGS84));

	//	logger->trace("Las2World：{0} -> {1}", fromCrs, toCrs);

	//	projLas2World = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
	//	if (projLas2World == nullptr) {
	//		throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
	//	}
	//}

}

void Converter::convert() {
	Config& config = Config::getInstance();

	//バッチ日付
	std::string today = ComLib::CommonUtil::getCurDatetime();

	//DBアクセス用
	DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
	adapter.connect(config);

	std::string strBatchId = config.getBatchId();
	std::vector<std::string> batchIdList = ComLib::CommonUtil::split(strBatchId, ',');

	try {
		bool hasError = false;

		//処理中のステータスにする
		adapter.updateAreaObjectMasters(batchIdList, STATUS_PROCESSING);

		for (auto& batchId : batchIdList) {
			//areaObjectId
			uint64_t areaObjectId = std::stoull(batchId);

			logger->info("##### 処理対象：areaObjectId={0}", areaObjectId);

			std::vector<AreaObjectMastersInfo> infos = adapter.getAreaObjectMasters(areaObjectId);
			if (infos.size() == 0) {
				logger->info("処理対象がありません(areaObjectId:{0})", areaObjectId);
				continue;
			}

			AreaObjectMastersInfo info = infos[0];

			////処理中のステータスにする
			//adapter.updateAreaObjectMasters(areaObjectId, STATUS_PROCESSING);

			//旧データを削除
			adapter.deleteAreaDetailObjects(areaObjectId);

			//メッシュデータを読み込み、ボクセルを生成
			GeoDataset gds;

			loadMeshData(gds, info.coordinates);

			//voxelを作成
			createVoxels(gds, areaObjectId, info.areaId, config, adapter, today);

		}
		//ステータスを更新
		adapter.updateAreaObjectMasters(batchIdList, (hasError ? STATUS_NG : STATUS_OK));
	}
	catch (...) {
		//ステータスを更新
		adapter.updateAreaObjectMasters(batchIdList, STATUS_NG);

		throw;
	}
}

bool Converter::loadMeshData(GeoDataset& gds, std::string& coordinates) {
	//gmlファイルをロード
	clock_t ts = clock();

	std::string jsonText = "{\"coordinates\":" + coordinates + "}";

	rapidjson::Document doc;
	doc.Parse(jsonText.c_str());
	if (doc.HasParseError()) {
		throw std::runtime_error(rapidjson::GetParseError_En(doc.GetParseError()));
	}

#if 0
	const rapidjson::Value& arrPoint = doc["coordinates"];

	const rapidjson::Value& leftUp = arrPoint[0];
	const rapidjson::Value& rightDown = arrPoint[1];

	double lon1 = leftUp[0].GetDouble();
	double lat1 = leftUp[1].GetDouble();
	double alt1 = leftUp[2].GetDouble();
	double lon2 = rightDown[0].GetDouble();
	double lat2 = rightDown[1].GetDouble();
	double alt2 = rightDown[2].GetDouble();

	//長方形を三角形に分割
	{//一つ目
		GeoPoint* pPoints = gds.allocatePoint(3);
		GeoPolygon* pPolygon = gds.allocatePolygon();

		pPoints[0].x = lon1;
		pPoints[0].y = lat1;
		pPoints[0].z = alt1;
		pPoints[1].x = lon2;
		pPoints[1].y = lat1;
		pPoints[1].z = alt1;
		pPoints[2].x = lon2;
		pPoints[2].y = lat2;
		pPoints[2].z = alt2;

		pPolygon->addPoints(3, pPoints);

		//座標変換
		transform(pPolygon, projMap2Las);

		gds.addMesh(pPolygon);

	}
	{//二つ目
		GeoPoint* pPoints = gds.allocatePoint(3);
		GeoPolygon* pPolygon = gds.allocatePolygon();

		pPoints[0].x = lon2;
		pPoints[0].y = lat2;
		pPoints[0].z = alt2;
		pPoints[1].x = lon1;
		pPoints[1].y = lat2;
		pPoints[1].z = alt2;
		pPoints[2].x = lon1;
		pPoints[2].y = lat1;
		pPoints[2].z = alt1;

		pPolygon->addPoints(3, pPoints);

		//座標変換
		transform(pPolygon, projMap2Las);

		gds.addMesh(pPolygon);

	}


#else
	const rapidjson::Value& arrMesh = doc["coordinates"];
	int size = arrMesh.Size();

	GeoDataset gdsTemp;
	bool needTriangulation = true; //仕様によってfalseにすることもある
	GeoDataset& gdsActive = (needTriangulation ? gdsTemp : gds);
	
	for (const auto& mesh : arrMesh.GetArray()) {
		GeoPoint* pPoints = gdsActive.allocatePoint(mesh.Size());
		GeoPolygon* pPolygon = gdsActive.allocatePolygon();

		int ind = 0;
		for (const auto& pt : mesh.GetArray()) {
			//pPoints[ind].x = std::atof(pt["longitude"].GetString());
			//pPoints[ind].y = std::atof(pt["latitude"].GetString());
			//pPoints[ind].z = std::atof(pt["altitude"].GetString());
			pPoints[ind].x = pt[0].GetDouble();
			pPoints[ind].y = pt[1].GetDouble();
			pPoints[ind].z = pt[2].GetDouble();

			ind++;
		}

		pPolygon->addPoints(ind, pPoints);

		//座標変換
		transform(pPolygon, projMap2Las);

		gdsActive.addMesh(pPolygon);
	}

	//三角形分割
	if (needTriangulation) {
		int polygonCount = gdsActive.getMeshCount();
		GeoPolygon** ppPolygon = gdsActive.getMeshRef();

		logger->debug("Triangulation: polygonCount:{0}", polygonCount);

		for (int j = 0; j < polygonCount; j++) {
			GeoPolygon* pPolygon = ppPolygon[j];
			GeoPoint** ppPoints = pPolygon->getPointsRef();

			//平面の向きを調べる
			pPolygon->decidePlaneType();

			//三角形分割
			pPolygon->doTriangulation();

			//三角形リストに追加
			for (int k = 0; k < pPolygon->getTriangleIndices().size(); k += 3) {
				if (k + 3 > pPolygon->getTriangleIndices().size()) {
					logger->warn("+++ size of index:{0}", pPolygon->getTriangleIndices().size());
					break;
				}

				GeoPoint* pNewPoints = gds.allocatePoint(3);
				GeoPolygon* pNewPolygon = gds.allocatePolygon();

				for (int p = 0; p < 3; p++) {
					int ind = pPolygon->getTriangleIndices()[k + p];

					pNewPoints[p].x = ppPoints[ind]->x;
					pNewPoints[p].y = ppPoints[ind]->y;
					pNewPoints[p].z = ppPoints[ind]->z;
				}

				pNewPolygon->addPoints(3, pNewPoints);
				gds.addMesh(pNewPolygon);

			}

		}
	}
#endif

	clock_t te = clock();
	logger->trace("メッシュデータのロード時間：{0:.2f}(秒)", (double)(te - ts) / CLOCKS_PER_SEC);

	return true;
}



void Converter::createVoxels(GeoDataset& gds, uint64_t areaObjectId, std::string& areaId, Config& config,
		DataAccessAdapter& adapter, std::string& today) {
	int meshCount = gds.getMeshCount();
	GeoPolygon** ppMesh = gds.getMeshRef();

	//空間ＩＤに紐づいているオブジェクト
	std::map<std::string, std::vector<int>> spatialIdToMeshMap;

	//std::cout << "##cityObjectCount:" << cityObjectCount << std::endl;

	////複数空間ＩＤにまたがっているオブジェクトリスト
	//std::vector<CityObject *> objWithMultiIds;

	////AreaDetailObjectsInfo用
	//std::map<std::string, std::shared_ptr<AreaDetailObjectsInfo>> areaDetailObjectsInfoMap;

	//ローカルズーム
	int localZoom = config.getLocalZoom();
	if (localZoom < spatialid::kGlobalZoom) {
		localZoom = spatialid::kLocalZoom;
	}

	//入力ファイルのCRS
	int crs = config.getMapEpsg();
	if (projMap2Las != nullptr) {
		crs = config.getConfirmLasEpsg();
	}

	//処理対象の空間ＩＤ。基本的にデバッグ用
	const std::vector<std::string>& targetSpatialIds = config.getTargetSpatialIds();
	logger->trace("targetSpatialIds.size:{0}", targetSpatialIds.size());

	int exceptionCount = 0; //for debug
	clock_t ts = clock();
	for (int i = 0; i < meshCount; i++) {
		//clock_t t1 = clock();

		GeoPolygon* pMesh = ppMesh[i];
		GeoPoint** ppPoint = pMesh->getPointRef();

		spatialid::Point point[3];
		for (int j = 0; j < 3; j++) {
			point[j].x = ppPoint[j]->x;
			point[j].y = ppPoint[j]->y;
			point[j].z = ppPoint[j]->z;
		}

		std::vector<spatialid::Triangle> triangles;

		try {
			spatialid::Triangle triangle(point[0], point[1], point[2]);

			triangles.push_back(triangle);
		}
		catch (std::invalid_argument& ex) {
			logger->trace("error occured. cause:{0}", ex.what());

			exceptionCount++;
			continue;
		}


		//空間ID(globalZoom)を求める
		std::vector<spatialid::Triangle> empty;
		std::unordered_set<std::string> spatialIds;

		spatialid::get_global_spatial_ids_on_polygons(triangles, empty,
			spatialIds, static_cast<spatialid::CRS>(crs), false, false);

		for (const auto& spatialId : spatialIds) {
			//空間ＩＤ単位でまとめて処理するオブジェクト
			if (spatialIdToMeshMap.find(spatialId) == spatialIdToMeshMap.end()) {
				std::vector<int> list;
				list.push_back(i);
				spatialIdToMeshMap.insert(std::make_pair(spatialId, list));
			}
			else {
				std::vector<int>& list = spatialIdToMeshMap.at(spatialId);
				list.push_back(i);
			}

		}

		logger->trace("空間ID算出 ({0}/{1})、空間ID数={2}", (i + 1), meshCount, spatialIds.size());

	}

	if (exceptionCount > 0) {
		logger->warn("exceptionCount:{0}", exceptionCount);
	}


	logger->trace("三角形分割とグロバール空間IDリスト作成：{0:.2f}", (double)(clock() - ts) / CLOCKS_PER_SEC);

	//-----------------------------------------------------
	//２パターンのロジックが用意されている
	//　①空間ＩＤ単位での処理
	//　②オブジェクト単位での処理
	//-----------------------------------------------------

	//①空間ＩＤ単位での処理
	logger->debug("空間ＩＤ単位での処理　START、空間ID数:{0}", spatialIdToMeshMap.size());
	ts = clock();
	
	int numOfSpatialId = spatialIdToMeshMap.size();
	int spatialIdCount = 0;
	for (auto iter = spatialIdToMeshMap.begin(); iter != spatialIdToMeshMap.end(); iter++) {
		clock_t startTime = clock();
		std::string spatialId = iter->first;
		std::vector<int> objList = iter->second;
		spatialIdCount++;
		logger->info("spatialId:{0}　START、メッシュ数：{1}  ({2}/{3})", spatialId,
			objList.size(), spatialIdCount, spatialIdToMeshMap.size());


		if (targetSpatialIds.size() > 0 && 
				std::find(targetSpatialIds.begin(), targetSpatialIds.end(), spatialId) == targetSpatialIds.end()) {
			logger->info("空間ＩＤ:{0}　SKIP", spatialId);
			continue;
		}

		//std::cout << "##spatialId:" << spatialId << std::endl;
		//logger->debug("spatialId:{0} START objList.size:{1} ({2}/{3})", spatialId, objList.size(),
		//	spatialIdCount, numOfSpatialId);

		clock_t t1 = clock();

		//std::shared_ptr<AreaDetailObjectsInfo> ptr;
		//if (spatialIdToMeshMap.find(spatialId) == spatialIdToMeshMap.end()) {
		//	ptr = initiaizeAreaDetailObjectsInfo(config, spatialId, areaObjectId);
		//	spatialIdToMeshMap.insert(std::make_pair(spatialId, ptr));
		//}
		//else {
		//	ptr = spatialIdToMeshMap[spatialId];
		//}


		//ボクセルを作成	
		std::string spatialIdOut = spatialId;
		std::replace(spatialIdOut.begin(), spatialIdOut.end(), '/', '_');

		std::string voxelOutputFolder = config.getVoxelOutputFolder(spatialIdOut, today);
		std::string voxelOutputFolder2 = config.getVoxelBitFilePath(spatialIdOut, today); //without confirmRoot


		std::string voxelFileNameOut = spatialIdOut + "_area_" + areaId + "." + config.getVoxelExt();
		std::string gzFileName = ComLib::CommonUtil::pathJoin(voxelOutputFolder, voxelFileNameOut);
		std::string voxelBitFilePath = ComLib::CommonUtil::pathJoin(voxelOutputFolder2, voxelFileNameOut);

		//std::cout << "gzFileName:" << gzFileName << std::endl;
		//std::cout << "voxelBitFilePath:" << voxelBitFilePath << std::endl;
		//std::cout << "voxelOutputFolder:" << voxelOutputFolder << std::endl;

		ComLib::Voxel voxel(spatialId, localZoom, config.getConfirmLasEpsg(), gzFileName, config.getLoggerName());
		voxel.setScaleZ(config.getLasScaleZ());

		if (objList.size() == 1 || config.getThreadCount() <= 1) {
			//シングルスレッド
			for (auto const ind : objList) {
				GeoPolygon* pMesh = ppMesh[ind];
				GeoPoint** ppPoint = pMesh->getPointRef();

				spatialid::Point point[3];
				for (int j = 0; j < 3; j++) {
					point[j].x = ppPoint[j]->x;
					point[j].y = ppPoint[j]->y;
					point[j].z = ppPoint[j]->z;
				}

				std::vector<spatialid::Triangle> triangles;
				spatialid::Triangle triangle(point[0], point[1], point[2]);

				triangles.push_back(triangle);
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
					[&voxel, &objList, ppMesh, crs, i] {
						try {
							GeoPolygon* pMesh = ppMesh[objList[i]];
							GeoPoint** ppPoint = pMesh->getPointRef();

							spatialid::Point point[3];
							for (int j = 0; j < 3; j++) {
								point[j].x = ppPoint[j]->x;
								point[j].y = ppPoint[j]->y;
								point[j].z = ppPoint[j]->z;
							}

							std::vector<spatialid::Triangle> triangles;
							spatialid::Triangle triangle(point[0], point[1], point[2]);

							triangles.push_back(triangle);

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
		//出力フォルダがなければ作成
		if (!std::filesystem::exists(voxelOutputFolder)) {
			ComLib::CommonUtil::forceDirectories(voxelOutputFolder);
		}
		voxel.toFile(config.getLasScale(), config.getColor());
		if (config.getVoxelExt() == "laz") {
			std::string lasFileName = std::regex_replace(gzFileName, std::regex("\\.laz$"), ".las");
			//std::cout << "lasFile:" << lasFileName << std::endl;
			std::filesystem::remove(lasFileName);
		}

		//DB登録
		AreaDetailObjectsInfo areaDetailObjectsInfo;
		areaDetailObjectsInfo.areaObjectId = areaObjectId;
		areaDetailObjectsInfo.spatialId = spatialId;
		areaDetailObjectsInfo.voxelBitFilePath = voxelBitFilePath;
		areaDetailObjectsInfo.voxelBitSpatialZoomLevel = localZoom;
		areaDetailObjectsInfo.pointCloudEpsg = crs;

		adapter.insertAreaDetailObjects(areaDetailObjectsInfo);
		logger->trace("output to file.spaticlId:{0}, elapsed time:{1:.2f}(sec)", spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);


		logger->info("spatialId:{0} END (elapsed time:{1:.2f}(sec))", spatialId,
			(double)(clock()-startTime)/CLOCKS_PER_SEC);

	}
	logger->debug("空間ＩＤ単位での処理　END (経過時間：{0:.2f})", (double)(clock() - ts) / CLOCKS_PER_SEC);

	
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

//void Converter::deleteOldFiles(DataAccessAdapter& adapter, std::string& dataType,
//		std::map<std::string, std::vector<CityObject*>>& spatialIdMap) {
//	Config& config = Config::getInstance();
//
//	if (config.getBatchFile().empty()) {
//		logger->info("前回のファイルを削除:{0}, dataType:{1}", config.getOutputFolder(), dataType);
//
//		std::string tmpSpatialId;
//		CommonUtil::deleteFiles(config.getOutputFolder(), tmpSpatialId, dataType);
//
//		std::string ownerId = config.getOwnerId();
//		std::string batchId = config.getBatchId();
//		std::string recId = FIXED_REC_ID;
//		int rc = adapter.deleteSpaceDataManages(ownerId, batchId, tmpSpatialId, recId, dataType);
//
//		logger->info("削除されたレコード数：{0}, dataType:{1}", rc, dataType);
//	}
//	else {
//		logger->info("前回のファイルを削除:{0}, dataType:{1}", config.getOutputFolder(), dataType);
//		//同一空間IDが複数ファイルに含まれることがあるので、このモードは基本的デバッグ用。
//		int delCount = 0;
//		for (auto& pair : spatialIdMap) {
//			std::string tmpSpatialId = pair.first;
//			std::string tmpSpatialIdWithoutSlash = tmpSpatialId;
//			std::replace(tmpSpatialIdWithoutSlash.begin(), tmpSpatialIdWithoutSlash.end(), '/', '-');
//
//			CommonUtil::deleteFiles(config.getOutputFolder(), tmpSpatialIdWithoutSlash, dataType);
//
//			std::string ownerId = config.getOwnerId();
//			std::string batchId = config.getBatchId();
//			std::string recId = FIXED_REC_ID;
//			int rc = adapter.deleteSpaceDataManages(ownerId, batchId, tmpSpatialId, recId, dataType);
//			delCount += rc;
//		}
//		logger->info("削除されたレコード数：{0}, dataType:{1}", delCount, dataType);
//	}
//}