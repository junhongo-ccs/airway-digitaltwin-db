#include "Converter.h"
#include <memory>
#include <cmath>
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

	if (config.getConfirmLasEpsg() != static_cast<int>(spatialid::CRS::kWebMercator)) {
		{//map2world
			std::string fromCrs = "EPSG:" + std::to_string(config.getMapEpsg());
			std::string toCrs = "EPSG:" + std::to_string(static_cast<int>(spatialid::CRS::kWebMercator));

			logger->trace("las2world：{0} -> {1}", fromCrs, toCrs);

			PJ* pjTemp = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
			if (pjTemp == nullptr) {
				throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
			}
			projMap2World = proj_normalize_for_visualization(projContext, pjTemp);
			proj_destroy(pjTemp);

		}

		{//world2las
			std::string fromCrs = "EPSG:" + std::to_string(static_cast<int>(spatialid::CRS::kWebMercator));
			std::string toCrs = "EPSG:" + std::to_string(config.getConfirmLasEpsg());

			logger->trace("world2las：{0} -> {1}", fromCrs, toCrs);

			PJ* pjTemp = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
			if (pjTemp == nullptr) {
				throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
			}
			projWorld2Las = proj_normalize_for_visualization(projContext, pjTemp);
			proj_destroy(pjTemp);
		}

	}


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
		adapter.updateFlightProhibitedAreaObjectMasters(batchIdList, STATUS_PROCESSING);

		for (auto& batchId : batchIdList) {
			//flightProhibitedAreaObjectId
			uint64_t flightProhibitedAreaObjectId = std::stoull(batchId);

			logger->info("##### 処理対象：flightProhibitedAreaObjectId={0}", flightProhibitedAreaObjectId);

			std::vector<FlightProhibitedAreaObjectMastersInfo> infos = adapter.getFlightProhibitedAreaObjectMasters(flightProhibitedAreaObjectId);
			if (infos.size() == 0) {
				logger->info("処理対象がありません(flightProhibitedAreaObjectId:{0})", flightProhibitedAreaObjectId);
				continue;
			}

			FlightProhibitedAreaObjectMastersInfo info = infos[0];

			////処理中のステータスにする
			//adapter.updateFlightProhibitedAreaObjectMasters(flightProhibitedAreaObjectId, STATUS_PROCESSING);

			//旧データを削除
			adapter.deleteFlightProhibitedAreaObjects(flightProhibitedAreaObjectId);

			//メッシュデータを読み込み、ボクセルを生成
			GeoDataset gds;

			loadAreaObjectData(gds, info.range, config.getEdgeCount());

			//voxelを作成
			createVoxels(gds, flightProhibitedAreaObjectId, info.flightProhibitedAreaId, config, adapter, today);

			////ステータスを更新
			//adapter.updateFlightProhibitedAreaObjectMasters(flightProhibitedAreaObjectId, STATUS_OK);
		}

		//ステータスを更新
		adapter.updateFlightProhibitedAreaObjectMasters(batchIdList, (hasError ? STATUS_NG : STATUS_OK));
	}
	catch (...) {
		//ステータスを更新
		adapter.updateFlightProhibitedAreaObjectMasters(batchIdList, STATUS_NG);

		throw;
	}
}

void Converter::loadAreaObjectData(GeoDataset& gds, std::string& coordinates, int n) {
	//gmlファイルをロード
	clock_t ts = clock();

	std::string jsonText = "{\"range\":" + coordinates + "}";

	rapidjson::Document doc;
	doc.Parse(jsonText.c_str());
	if (doc.HasParseError()) {
		throw std::runtime_error(rapidjson::GetParseError_En(doc.GetParseError()));
	}

	auto& rangeDict = doc["range"];
	std::string type = rangeDict["type"].GetString();
	ComLib::CommonUtil::toLower(type);

	double altitude = 0; // rangeDict["altitude"].GetDouble();
	GeoPolygon* pPolygon = nullptr;
	AreaObject* pAreaObject = nullptr;

	if (type == "polygon") {
		auto arrPoints = rangeDict["coordinates"].GetArray();
		int size = arrPoints.Size();

		GeoPoint* pPoints = gds.allocatePoint(size);
		pPolygon = gds.allocatePolygon();

		int ind = 0;
		for (const auto& pt : arrPoints) {
			pPoints[ind].x = pt[0].GetDouble();
			pPoints[ind].y = pt[1].GetDouble();
			pPoints[ind].z = altitude;

			ind++;
		}

		pPolygon->addPoints(size, pPoints);

		//座標変換
		transform(pPolygon, projMap2Las);

	}
	else if (type == "circle") {
		double radius = rangeDict["radius"].GetDouble();
		auto& centerDict = rangeDict["center"];
		double lon = centerDict[0].GetDouble();
		double lat = centerDict[1].GetDouble();

		double centerX = lon;
		double centerY = lat;

		if (projMap2Las != nullptr) {
			PJ_COORD input_coords, output_coords;
			input_coords = proj_coord(lon, lat, 0, 0);
			output_coords = proj_trans(projMap2Las, PJ_FWD, input_coords);

			centerX = output_coords.xy.x;
			centerY = output_coords.xy.y;
		}
		pAreaObject = gds.allocateAreaObject();
		pAreaObject->setObjectType(type);

		//円の部分
		pAreaObject->setCircleParam(centerX, centerY, radius);
		pAreaObject->setAltitude(altitude);


		Config& config = Config::getInstance();
		if (!config.needUseCylinderApi()) {
			double cxTemp = centerX;
			double cyTemp = centerY;

			//必要に応じて緯度経度を投影座標系に変換
			if (projMap2World != nullptr) {
				PJ_COORD inTemp, outTemp;
				inTemp = proj_coord(lon, lat, 0, 0);
				outTemp = proj_trans(projMap2World, PJ_FWD, inTemp);

				cxTemp = outTemp.xy.x;
				cyTemp = outTemp.xy.y;
			}


			int n = config.getEdgeCount();
			if (n <= 0) {
				double angle = std::atan2(0.05, radius);
				n = std::ceil(M_PI / angle);
			}

			double radiusNew = radius / std::cos(M_PI / n);

			logger->debug("ポリゴン近似： edgeCount={0}, radiusNew:{1}", n, radiusNew);

			GeoPoint* pPoints = gds.allocatePoint(n);
			pPolygon = gds.allocatePolygon();
			for (int i = 0; i < n; i++) {
				double angle = 2 * M_PI * i / n;

				pPoints[i].x = cxTemp + radiusNew * std::cos(angle);
				pPoints[i].y = cyTemp + radiusNew * std::sin(angle);
				pPoints[i].z = altitude;

				if (projWorld2Las != nullptr) {
					PJ_COORD inTemp, outTemp;
					inTemp = proj_coord(pPoints[i].x, pPoints[i].y, 0, 0);
					outTemp = proj_trans(projWorld2Las, PJ_FWD, inTemp);

					pPoints[i].x = outTemp.xy.x;
					pPoints[i].y = outTemp.xy.y;
				}
			}

			pPolygon->addPoints(n, pPoints);
		}

	}
	else {
		throw std::runtime_error("unsupported geometry type");
	}

	if (pPolygon != nullptr) {
		int size = pPolygon->getPointCount();
		GeoPoint** ppPoints = pPolygon->getPointsRef();

		//１番上の面
		pAreaObject = gds.allocateAreaObject();
		pAreaObject->setObjectType(type);
		
		int polygonId = 0;
		pPolygon->setPolygonId(polygonId);
		pAreaObject->addPolygon(pPolygon);
#if 0
		{//1番下の面
			GeoPolygon* pLower = gds.allocatePolygon();
			GeoPoint* pTempPoints = gds.allocatePoint(size);
			for (int i = 0; i < size; i++) {
				pTempPoints[i].x = ppPoints[i]->x;
				pTempPoints[i].y = ppPoints[i]->y;
				pTempPoints[i].z = 0.0;
			}
			pLower->addPoints(size, pTempPoints);
			polygonId++;
			pLower->setPolygonId(polygonId);
			pAreaObject->addPolygon(pLower);
		}

		{ //側面
			double eps = 1.0e-10;
			for (int i = 0; i < size; i++) {
				//最後の点と最初の点が一致している場合
				if (i == size - 1 && std::fabs(ppPoints[0]->x - ppPoints[i]->x) < eps &&
					std::fabs(ppPoints[0]->y - ppPoints[i]->y) < eps &&
					std::fabs(ppPoints[0]->z - ppPoints[i]->z) < eps) {
					continue;
				}

				double zStep = 100;//小さい四角形に分割
				double curZ = 0;
				while (curZ - altitude < eps) {
					double nextZ = curZ + zStep;
					if (nextZ > altitude) {
						nextZ = altitude;
					}
					GeoPolygon* pSide = gds.allocatePolygon();
					GeoPoint* pTempPoints = gds.allocatePoint(5);
					pTempPoints[0].x = ppPoints[i]->x;
					pTempPoints[0].y = ppPoints[i]->y;
					pTempPoints[0].z = nextZ;

					int ind = (i + 1 == size ? 0 : i + 1);
					pTempPoints[1].x = ppPoints[ind]->x;
					pTempPoints[1].y = ppPoints[ind]->y;
					pTempPoints[1].z = nextZ;

					pTempPoints[2].x = pTempPoints[1].x;
					pTempPoints[2].y = pTempPoints[1].y;
					pTempPoints[2].z = curZ;

					pTempPoints[3].x = pTempPoints[0].x;
					pTempPoints[3].y = pTempPoints[0].y;
					pTempPoints[3].z = curZ;

					pTempPoints[4].x = pTempPoints[0].x;
					pTempPoints[4].y = pTempPoints[0].y;
					pTempPoints[4].z = pTempPoints[0].z;

					pSide->addPoints(5, pTempPoints);
					polygonId++;
					pSide->setPolygonId(polygonId);

					pAreaObject->addPolygon(pSide);

					curZ += zStep;
				}
			}
		}
#endif
	}

	if (pAreaObject != nullptr) {
		gds.addAreaObject(pAreaObject);
	}

	clock_t te = clock();
	logger->trace("メッシュデータのロード時間：{0:.2f}(秒)", (double)(te - ts) / CLOCKS_PER_SEC);

}



void Converter::createVoxels(GeoDataset& gds, uint64_t flightProhibitedAreaObjectId, std::string& areaId, Config& config,
	DataAccessAdapter& adapter, std::string& today) {
	clock_t ts = clock();

	int areaObjectCount = gds.getAreaObjectCount();

	if (areaObjectCount == 0) {
		logger->info("areaObjectCount:{0}", areaObjectCount);
		return;
	}

	AreaObject* pAreaObject = gds.getAreaObject(0);
	std::string areaType = pAreaObject->getObjectType();
	if (areaType == "polygon" || pAreaObject->getPolyCount() > 0) {
		createPolygonVoxels(pAreaObject,
			gds, flightProhibitedAreaObjectId, areaId, config, adapter, today);
	}
	else if (areaType == "circle") {//ポリゴン近似にしたため、この分岐は今利用されていない
		createCircleVoxels(pAreaObject,
			gds, flightProhibitedAreaObjectId, areaId, config, adapter, today);
	}

	logger->trace("createVoxels：areaType={0}, 経過時間={1:.2f}", areaType, (double)(clock() - ts) / CLOCKS_PER_SEC);

}


void Converter::createPolygonVoxels(AreaObject* pAreaObject,
		GeoDataset& gds, uint64_t flightProhibitedAreaObjectId, 
		std::string& areaId, Config& config,
	DataAccessAdapter& adapter, std::string& today) {

	//空間ＩＤに紐づいているオブジェクト
	std::map<std::string, std::vector<GeoPolygon* >> spatialIdToMeshMap;

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

	clock_t t1 = clock();

	//std::vector<spatialid::Triangle>& triangles = pAreaObject->getTriangles();
	int polygonCount = pAreaObject->getPolyCount();
	GeoPolygon** ppPolygon = pAreaObject->getPolygonRef();

	logger->trace("polygonCount:{0}", polygonCount);

	for (int j = 0; j < polygonCount; j++) {
		GeoPolygon* pPolygon = ppPolygon[j];
		GeoPoint** ppPoints = pPolygon->getPointsRef();
		std::vector<spatialid::Triangle>& triangles = pPolygon->getTriangles();

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

			spatialid::Point point[3];

			for (int p = 0; p < 3; p++) {
				int ind = pPolygon->getTriangleIndices()[k + p];

				point[p].x = ppPoints[ind]->x;
				point[p].y = ppPoints[ind]->y;
				point[p].z = ppPoints[ind]->z;

				//logger->debug("x:{0}, y:{1}, z:{2}", point[p].x, point[p].y, point[p].z);
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

		//空間ID(globalZoom)を求める
		std::vector<spatialid::Triangle> empty;

		spatialid::get_global_spatial_ids_on_polygons(triangles, empty,
			pPolygon->getSpatialIds(),
			static_cast<spatialid::CRS>(crs), false, false);
	}

	for (int j = 0; j < polygonCount; j++) {
		GeoPolygon* pPolygon = ppPolygon[j];

		for (auto& spatialId : pPolygon->getSpatialIds()) {

			//空間ＩＤ単位でまとめて処理するオブジェクト
			if (spatialIdToMeshMap.find(spatialId) == spatialIdToMeshMap.end()) {
				std::vector<GeoPolygon*> list;
				list.push_back(pPolygon);
				spatialIdToMeshMap.insert(std::make_pair(spatialId, list));
			}
			else {
				std::vector<GeoPolygon*>& list = spatialIdToMeshMap.at(spatialId);
				list.push_back(pPolygon);
			}
		}

	}

	logger->trace("空間ID算出。オブジェクト：{0} END 処理時間：{1:.3f}", pAreaObject->getObjectId(),
		(double)(clock() - t1) / CLOCKS_PER_SEC);

	if (exceptionCount > 0) {
		logger->warn("++ exceptionCount:{0}", exceptionCount);
	}


	//-----------------------------------------------------
	//２パターンのロジックが用意されている
	//　①空間ＩＤ単位での処理
	//　②オブジェクト単位での処理
	//-----------------------------------------------------

	//①空間ＩＤ単位での処理
	logger->debug("空間ＩＤ単位での処理　START、空間ID数:{0}", spatialIdToMeshMap.size());
	clock_t ts = clock();
	
	int numOfSpatialId = spatialIdToMeshMap.size();
	int spatialIdCount = 0;
	for (auto iter = spatialIdToMeshMap.begin(); iter != spatialIdToMeshMap.end(); iter++) {
		clock_t startTime = clock();
		std::string spatialId = iter->first;
		std::vector<GeoPolygon* > objList = iter->second;
		spatialIdCount++;
		logger->info("spatialId:{0}　START、ポリゴン数：{1}  ({2}/{3})", spatialId,
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


		std::string voxelFileNameOut = spatialIdOut + "_flightProhibitedArea_" + areaId + "." + config.getVoxelExt();
		std::string gzFileName = ComLib::CommonUtil::pathJoin(voxelOutputFolder, voxelFileNameOut);
		std::string voxelBitFilePath = ComLib::CommonUtil::pathJoin(voxelOutputFolder2, voxelFileNameOut);

		//std::cout << "gzFileName:" << gzFileName << std::endl;
		//std::cout << "voxelBitFilePath:" << voxelBitFilePath << std::endl;
		//std::cout << "voxelOutputFolder:" << voxelOutputFolder << std::endl;

		ComLib::Voxel voxel(spatialId, localZoom, config.getConfirmLasEpsg(), gzFileName, config.getLoggerName());
		voxel.setScaleZ(config.getLasScaleZ());

		if (objList.size() == 1 || config.getThreadCount() <= 1) {
			//シングルスレッド
			for (auto const& pMesh : objList) {
				//std::cout << "++ spatialId:" << spatialId << ", polygonId:" << pMesh->getPolygonId() << std::endl;

				voxel.addPolygon(pMesh->getTriangles(), static_cast<spatialid::CRS>(crs));
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
				GeoPolygon* pMesh = objList[i];
				pool.push_task(
					[&voxel, &objList, pMesh, crs] {
						try {
							GeoPoint** ppPoint = pMesh->getPointsRef();

							voxel.addPolygon(pMesh->getTriangles(), static_cast<spatialid::CRS>(crs));
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
		FlightProhibitedAreaObjectsInfo flightProhibitedAreaObjectsInfo;
		flightProhibitedAreaObjectsInfo.flightProhibitedAreaObjectId = flightProhibitedAreaObjectId;
		flightProhibitedAreaObjectsInfo.spatialId = spatialId;
		flightProhibitedAreaObjectsInfo.voxelBitFilePath = voxelBitFilePath;
		flightProhibitedAreaObjectsInfo.voxelBitSpatialZoomLevel = localZoom;
		flightProhibitedAreaObjectsInfo.pointCloudEpsg = crs;

		adapter.insertFlightProhibitedAreaObjects(flightProhibitedAreaObjectsInfo);
		logger->trace("output to file.spatialId:{0}, elapsed time:{1:.2f}(sec)", spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);


		logger->info("spatialId:{0} END (elapsed time:{1:.2f}(sec))", spatialId,
			(double)(clock()-startTime)/CLOCKS_PER_SEC);

	}
	logger->debug("空間ＩＤ単位での処理　END (経過時間：{0:.2f})", (double)(clock() - ts) / CLOCKS_PER_SEC);

	
}

void Converter::createCircleVoxels(AreaObject* pAreaObject,
	GeoDataset& gds, uint64_t flightProhibitedAreaObjectId, std::string& areaId, Config& config,
	DataAccessAdapter& adapter, std::string& today) {

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

	clock_t t1 = clock();

	double centerX = 0, centerY = 0, radius = 0;
	double altitude = pAreaObject->getAltitude();

	pAreaObject->getCircleParam(centerX, centerY, radius);

	//グロバール空間IDリストを取得
	int maxF = std::floor(pAreaObject->getAltitude() / std::pow(2.0, 25-spatialid::kGlobalZoom));
	logger->trace("+++ altitude:{0}, maxF:{1}", pAreaObject->getAltitude(), maxF);

	std::unordered_set<std::string> circleAllSpatialIds;
	std::unordered_set<std::string> circleBoundarySpatialIds;

	getCircleSpatialIds(centerX, centerY, radius, spatialid::kGlobalZoom, crs,
		circleAllSpatialIds, circleBoundarySpatialIds);
	
	//エリアオブジェクトIDリストへ追加
	std::unordered_set<std::string>& areaSpatialIds = pAreaObject->getSpatialIds();

	//底面
	areaSpatialIds.insert(circleAllSpatialIds.begin(), circleAllSpatialIds.end());

	//天面
	for (auto& tmpId : circleAllSpatialIds) {
		int tmpZ, tmpF, tmpX, tmpY;
		ComLib::CommonUtil::spatialIdToIndex(tmpId.c_str(), tmpZ, tmpF, tmpX, tmpY);
		std::string newId = ComLib::CommonUtil::indexToSpatialId(tmpZ, maxF, tmpX, tmpY);
		areaSpatialIds.insert(newId);
	}

	//積み上げる
	for (auto& tmpId : circleBoundarySpatialIds) {
		int tmpZ, tmpF, tmpX, tmpY;
		ComLib::CommonUtil::spatialIdToIndex(tmpId.c_str(), tmpZ, tmpF, tmpX, tmpY);

		for (int tmpF = 0; tmpF <= maxF; tmpF++) {
			std::string newId = ComLib::CommonUtil::indexToSpatialId(tmpZ, tmpF, tmpX, tmpY);
			areaSpatialIds.insert(newId);
		}
	}

	logger->trace("グロバール空間IDリスト作成。 END 処理時間：{0:.3f}", (double)(clock() - t1) / CLOCKS_PER_SEC);


	//-----------------------------------------------------
	//２パターンのロジックが用意されている
	//　①空間ＩＤ単位での処理
	//　②オブジェクト単位での処理
	//-----------------------------------------------------

	//①空間ＩＤ単位での処理
	logger->debug("空間ＩＤ単位での処理　START、空間ID数:{0}", areaSpatialIds.size());
	clock_t ts = clock();

	int spatialIdCount = 0;
	int diffZ = localZoom - spatialid::kGlobalZoom;
	for (auto& spatialId : areaSpatialIds) {
		clock_t startTime = clock();
		spatialIdCount++;
		logger->info("spatialId:{0}　START ({1}/{2})", spatialId, spatialIdCount, areaSpatialIds.size());


		if (targetSpatialIds.size() > 0 &&
			std::find(targetSpatialIds.begin(), targetSpatialIds.end(), spatialId) == targetSpatialIds.end()) {
			logger->info("空間ＩＤ:{0}　SKIP", spatialId);
			continue;
		}

		//ローカルの空間IDリストを作成
		std::unordered_set<std::string> circleAllLocalSpatialIds;
		std::unordered_set<std::string> circleBoundaryLocalSpatialIds;

		getCircleSpatialIds(centerX, centerY, radius, localZoom, crs,
			circleAllLocalSpatialIds, circleBoundaryLocalSpatialIds);

		//データの最大F（ローカルズーム）
		int maxLocalF = std::floor(pAreaObject->getAltitude() / std::pow(2.0, 25 - localZoom));

		//対象のブローバル空間ID
		int curZ, curF, curX, curY;
		ComLib::CommonUtil::spatialIdToIndex(spatialId.c_str(), curZ, curF, curX, curY);

		//対象グローバルボクセルのFの範囲（ローカルズーム）
		int curMinF = curF << diffZ;
		int curMaxF = curMinF + (1 << diffZ);
		//std::cout << "++ curMinF:" << curMinF << ",curMaxF:" << curMaxF << ", maxLocalF : " << maxLocalF << std::endl

		//ボクセルを作成	
		std::string spatialIdOut = spatialId;
		std::replace(spatialIdOut.begin(), spatialIdOut.end(), '/', '_');

		std::string voxelOutputFolder = config.getVoxelOutputFolder(spatialIdOut, today);
		std::string voxelOutputFolder2 = config.getVoxelBitFilePath(spatialIdOut, today); //without confirmRoot


		std::string voxelFileNameOut = spatialIdOut + "_flightProhibitedArea_" + areaId + "." + config.getVoxelExt();
		std::string gzFileName = ComLib::CommonUtil::pathJoin(voxelOutputFolder, voxelFileNameOut);
		std::string voxelBitFilePath = ComLib::CommonUtil::pathJoin(voxelOutputFolder2, voxelFileNameOut);

		//std::cout << "gzFileName:" << gzFileName << std::endl;
		//std::cout << "voxelBitFilePath:" << voxelBitFilePath << std::endl;
		//std::cout << "voxelOutputFolder:" << voxelOutputFolder << std::endl;

		ComLib::Voxel voxel(spatialId, localZoom, config.getConfirmLasEpsg(), gzFileName, config.getLoggerName());

		//底面の場合
		if (curMinF == 0) {
			voxel.addLocalZoomSpatialIds(circleAllLocalSpatialIds);
		}
		//天面の場合
		if (curMinF <= maxLocalF && maxLocalF <= curMaxF) {
			std::unordered_set<std::string> upperSpatialIds;
			for (auto& tmpId : circleAllLocalSpatialIds) {
				int tmpZ, tmpF, tmpX, tmpY;
				ComLib::CommonUtil::spatialIdToIndex(tmpId.c_str(), tmpZ, tmpF, tmpX, tmpY);
				std::string newId = ComLib::CommonUtil::indexToSpatialId(tmpZ, maxLocalF, tmpX, tmpY);
				upperSpatialIds.insert(newId);
			}
			voxel.addLocalZoomSpatialIds(upperSpatialIds);
		}
		//積み上げる
		{
			std::unordered_set<std::string> middleSpatialIds;
			for (auto& tmpId : circleBoundaryLocalSpatialIds) {
				int tmpZ, tmpF, tmpX, tmpY;
				ComLib::CommonUtil::spatialIdToIndex(tmpId.c_str(), tmpZ, tmpF, tmpX, tmpY);

				for (int tmpF = curMinF; tmpF <= curMaxF; tmpF++) {
					if (tmpF > maxLocalF) {
						break;
					}
					std::string newId = ComLib::CommonUtil::indexToSpatialId(tmpZ, tmpF, tmpX, tmpY);
					middleSpatialIds.insert(newId);
				}
			}
			voxel.addLocalZoomSpatialIds(middleSpatialIds);
		}


		int altCount = voxel.getVoxelData().size();
		logger->trace("create voxel。spaticlId:{0}, elapsed time:{1:.2f}(sec)",
			spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);
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
		FlightProhibitedAreaObjectsInfo flightProhibitedAreaObjectsInfo;
		flightProhibitedAreaObjectsInfo.flightProhibitedAreaObjectId = flightProhibitedAreaObjectId;
		flightProhibitedAreaObjectsInfo.spatialId = spatialId;
		flightProhibitedAreaObjectsInfo.voxelBitFilePath = voxelBitFilePath;
		flightProhibitedAreaObjectsInfo.voxelBitSpatialZoomLevel = localZoom;
		flightProhibitedAreaObjectsInfo.pointCloudEpsg = crs;

		adapter.insertFlightProhibitedAreaObjects(flightProhibitedAreaObjectsInfo);
		logger->trace("output to file.spaticlId:{0}, elapsed time:{1:.2f}(sec)", spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);


		logger->info("spatialId:{0} END (elapsed time:{1:.2f}(sec))", spatialId,
			(double)(clock() - startTime) / CLOCKS_PER_SEC);

	}
	logger->debug("空間ＩＤ単位での処理　END (経過時間：{0:.2f})", (double)(clock() - ts) / CLOCKS_PER_SEC);


}

void Converter::getCircleSpatialIds(double centerX, double centerY, double radius, 
	int zoomLevel, int crs,
	std::unordered_set<std::string>& allSpatialIds, std::unordered_set<std::string>& hollowSpatialIds) {

	//std::cout << "++ getCircleSpatialIds start" << std::endl;
	//spatialid::Logger::SetLogLevel(spatialid::LogLevel::kDebug);

	spatialid::Point tmpPoint[2];
	tmpPoint[0].x = centerX;
	tmpPoint[0].y = centerY;
	tmpPoint[0].z = 0;
	tmpPoint[1].x = centerX;
	tmpPoint[1].y = centerY;
	tmpPoint[1].z = 0.1; //平面のものでいいので小さい値でいい
	std::vector<spatialid::Point> pointList;
	pointList.push_back(tmpPoint[0]);
	pointList.push_back(tmpPoint[1]);

	std::unordered_set<std::string> tmpSpatialIds;
	spatialid::get_spatial_ids_on_cylinders(pointList, radius, tmpSpatialIds,
		zoomLevel, static_cast<spatialid::CRS>(crs), false);
	logger->trace("tmpSpatialIds.size:{0}", tmpSpatialIds.size());

	//中空のものを求める	
	int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

	for (auto& tmpId : tmpSpatialIds) {
		int tmpZ, tmpF, tmpX, tmpY;
		ComLib::CommonUtil::spatialIdToIndex(tmpId.c_str(), tmpZ, tmpF, tmpX, tmpY);
		if (tmpF != 0) continue;
		if (minX > tmpX) minX = tmpX;
		if (maxX < tmpX) maxX = tmpX;
		if (minY > tmpY) minY = tmpY;
		if (maxY < tmpY) maxY = tmpY;

		//底面の空間IDを追加
		allSpatialIds.insert(tmpId);
	}
	logger->trace("center:({4},{5}),radius:{6}, minX:{0}, maxX:{1}, minY:{2}, maxY:{3}", 
		minX, maxX, minY, maxY, centerX, centerY, radius);

	//円の境界線の空間ID
	for (int curY = minY; curY <= maxY; curY++) {
		int left = -1, right = -1;
		//左から
		for (int curX = minX; curX <= maxX; curX++) {
			std::string tmpId = ComLib::CommonUtil::indexToSpatialId(zoomLevel, 0, curX, curY);
			if (allSpatialIds.find(tmpId) != allSpatialIds.end()) {
				left = curX;
				break;
			}
		}
		//右から
		for (int curX = maxX; curX >= minX; curX--) {
			std::string tmpId = ComLib::CommonUtil::indexToSpatialId(zoomLevel, 0, curX, curY);
			if (allSpatialIds.find(tmpId) != allSpatialIds.end()) {
				right = curX;
				break;
			}
		}

		if (left < 0 || right < 0) {
			continue;//ありえないと思われるが
		}
		//std::cout << "++ curY:" << curY << ",left:" << left << ",right:" << right << std::endl;

		//空間IDを保存
		{//左
			std::string tmpId = ComLib::CommonUtil::indexToSpatialId(zoomLevel, 0,
				left, curY);
			hollowSpatialIds.insert(tmpId);
		}
		{//右
			std::string tmpId = ComLib::CommonUtil::indexToSpatialId(zoomLevel, 0,
				right, curY);
			hollowSpatialIds.insert(tmpId);
		}

		//一番上と一番下
		if (curY == minY || curY == maxY) {
			for (int tmpX = left + 1; tmpX <= right - 1; tmpX++) {
				std::string tmpId = ComLib::CommonUtil::indexToSpatialId(zoomLevel, 0, tmpX, curY);
				hollowSpatialIds.insert(tmpId);
			}
		}
	}

	logger->trace("zoomLevle:{0}, crs:{1}, circleAllSpatialIds:{2}, circleBoundarySpatialIds:{3}",
		zoomLevel, crs,
		allSpatialIds.size(), hollowSpatialIds.size());
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