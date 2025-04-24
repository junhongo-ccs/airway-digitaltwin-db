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
#include <liblas/liblas.hpp>
#include <laszip/laszip_api.h>

Converter::Converter() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());
}

void Converter::convert() {
	Config& config = Config::getInstance();

	//DBアクセス用
	DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
	adapter.connect(config);

	//dataSourceId
	std::string serverId = config.getServerId();
	std::string ownerId = config.getOwnerId();
	int dataSourceId = adapter.getDataSourceId(serverId, ownerId);
	logger->trace("## dataSourceId:{0}, serverId:{1}, ownerId:{2}", dataSourceId, serverId, ownerId);

	if (dataSourceId < 0) {
		throw std::runtime_error("データソースIDが取得できません");
	}

	//バッチ日付
	std::string curDatetime = ComLib::CommonUtil::getCurDatetime();


	//前回の出力ファイルを削除
	adapter.deleteVoxel("init", "", config.getVoxelDeleteDir());

	//lasファイルリストを取得する
	std::string folder = config.getLasInputDir();

	std::string ptn = "^.+\\." + config.getInputFileExt() + "$";

	std::vector<std::string> fileList = ComLib::CommonUtil::getFileList(folder, ptn, config.getBatchFile());

	logger->debug("対象ファイル数：{0}", fileList.size());

	//ファイルごとの処理
	int curCnt = 0;
	int fileCount = (int)fileList.size();
	std::unordered_set<std::string> processedSpatialIds;

	for (auto const& filePath : fileList) {
		clock_t ts, te;
		ts = clock();

		curCnt++;
		std::string baseFileName = std::filesystem::path(filePath).filename().string();

		logger->info("{0} START ({1}/{2})", baseFileName, curCnt, fileCount);


		//lasファイルをロード
		clock_t tmpS = clock();
		FileInfo fileInfo;
		fileInfo.fileName = filePath;
		fileInfo.epsg = config.getPointInLasEpsg();

		GeoDataset gds;

		//LASファイルの読み込み
		loadLasFile(fileInfo, gds);

		//グローバル空間IDリストを作成
		std::unordered_set<std::string> globalSpatialIds;
		std::string emptySpatialId;
		getSpatialIdList(emptySpatialId, spatialid::kGlobalZoom, fileInfo, gds, globalSpatialIds);
		if (globalSpatialIds.empty()) {
			logger->info("データがないのでスキップ");
			continue;
		}

		//logger->debug("+++ globalSpatialIds.size:{0}", globalSpatialIds.size());

		//初期の場合古いデータを削除
		for (auto& entry : globalSpatialIds) {
			std::string spatialIdWithSlash = entry;
			if (processedSpatialIds.find(spatialIdWithSlash) == processedSpatialIds.end()) {
				adapter.deleteVoxel("spatialInit", spatialIdWithSlash, config.getVoxelDeleteDir());
				processedSpatialIds.insert(spatialIdWithSlash);
			}
		}


		//ポリゴンから空間IDリストを求める
		tmpS = clock();
		createVoxels(curDatetime, dataSourceId, globalSpatialIds, gds, fileInfo, 
			adapter, baseFileName);

		logger->debug("createVoxels:{0:.2f}(秒)", (double)(clock() - tmpS) / CLOCKS_PER_SEC);

		te = clock();

		logger->info("{0} END 処理時間:{1}(秒)", baseFileName, (double)(te - ts) / CLOCKS_PER_SEC);

	}
}

bool Converter::loadLasFile(FileInfo& fileInfo, GeoDataset& gds) {
	std::string filePath = fileInfo.fileName;

	//gmlファイルをロード
	clock_t ts = clock();

	int lasEpsg = fileInfo.epsg;

	bool lazMode = ComLib::CommonUtil::endsWith(filePath, ".laz");

	//create reader
	laszip_POINTER laszip_reader;
	if (laszip_create(&laszip_reader)) {
		throw std::runtime_error("failed to create laszip reader");
	}

	//open reader
	laszip_BOOL is_compressed = 0;
	if (laszip_open_reader(laszip_reader, filePath.c_str(), &is_compressed))
	{
		throw std::runtime_error("failed to open laszip reader(" + filePath + ")");
	}
	//std::cout << "is_compressed:" << is_compressed << std::endl;

	//get a ponter to the header
	laszip_header* header;

	if (laszip_get_header_pointer(laszip_reader, &header))
	{
		throw std::runtime_error("failed to get the header pointer from laszip reader");
	}

	// how many points does the file have
	laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

	//logger->trace("polygon:{0}, points in las: {1}", polygonInd, npoints);

	// get a pointer to the points that will be read
	laszip_point* point;

	if (laszip_get_point_pointer(laszip_reader, &point))
	{
		throw std::runtime_error("failed to get point pointer from laszip reader");
	}

	double scaleX = header->x_scale_factor;
	double scaleY = header->y_scale_factor;
	double scaleZ = header->z_scale_factor;
	fileInfo.scaleX = scaleX;
	fileInfo.scaleY = scaleY;
	fileInfo.scaleZ = scaleZ;

	// read the points
	laszip_I64 p_count = 0;

	int blockSize = 200000;//サイズが小さいと、遅くなる原因になる
	GeoPoint* pLasPoint = nullptr;
	int curCount = 0;

	while (p_count < npoints)
	{
		// read a point
		if (laszip_read_point(laszip_reader))
		{
			throw std::runtime_error("failed to read point");
		}
		
		double x = point->X * scaleX;
		double y = point->Y * scaleY;
		double z = point->Z * scaleZ;

		if (curCount == 0 || curCount >= blockSize) {
			if (curCount > 0) {
				gds.addPoint(curCount, pLasPoint);
				curCount = 0;
			}

			pLasPoint = gds.allocatePoint(blockSize);
		}

		pLasPoint[curCount].x = point->X;
		pLasPoint[curCount].y = point->Y;
		pLasPoint[curCount].z = point->Z;
		curCount++;

		p_count++;
	}

	if (curCount > 0) {
		gds.addPoint(curCount, pLasPoint);
	}

	// close the reader
	if (laszip_close_reader(laszip_reader))
	{
		throw std::runtime_error("failed to close laszip reader");
	}

	// destroy the reader

	if (laszip_destroy(laszip_reader))
	{
		throw std::runtime_error("failed to destroy laszip reader");
	}

	logger->debug("loadLasFile:{0:.2f}(sec)", (double)(clock()-ts)/CLOCKS_PER_SEC);

}

void Converter::getSpatialIdList(std::string& spatialId, int zoomLevel, FileInfo& fileInfo,
		GeoDataset& gds, std::unordered_set<std::string>& spatialIds) {
	clock_t t1 = clock();

	Config& config = Config::getInstance();

	int threadCount = config.getThreadCount();
	int pointCount = gds.getPointCount();
	GeoPoint** ppPoint = gds.getPointRef();

	logger->trace("pointCount:{0}", pointCount);


	if (!spatialId.empty()) {
		int localZoom = zoomLevel;

		//空間IDの範囲を決める（性能アップのために）
		std::vector<spatialid::Point> pointList;
		spatialid::get_vertex_points_on_spatial_id(spatialId, pointList,
			static_cast<spatialid::CRS>(fileInfo.epsg));
		BBox bboxInner, bboxOuter;
		//四点が長方形ではないので、代わりに以下のような長方形を用意(点が空間IDの範囲内にあるかの判定用）
		//①：点１と点４の最大X座標、②：点２と点３の最小X座標
		//③：点１と点２の最小Y座標、④：点３と点４の最大Y座標
		bboxInner.xmin = std::max(pointList[0].x, pointList[3].x);
		bboxInner.xmax = std::min(pointList[1].x, pointList[2].x);
		bboxInner.ymax = std::min(pointList[0].y, pointList[1].y);
		bboxInner.ymin = std::max(pointList[2].y, pointList[3].y);

		bboxOuter.xmin = std::min(pointList[0].x, pointList[3].x);
		bboxOuter.xmax = std::max(pointList[1].x, pointList[2].x);
		bboxOuter.ymax = std::max(pointList[0].y, pointList[1].y);
		bboxOuter.ymin = std::min(pointList[2].y, pointList[3].y);

		bboxInner.zmin = std::min(pointList[0].z, pointList[4].z);
		bboxInner.zmax = std::max(pointList[0].z, pointList[4].z);
		bboxOuter.zmin = bboxInner.zmin;
		bboxOuter.zmax = bboxInner.zmax;

		auto loopFunc = [this, &spatialId, ppPoint, &fileInfo, &bboxInner, &bboxOuter,
			localZoom](const int a, const int b)
			{
				std::unordered_set<std::string> localSpatialIdList;

				for (int ind = a; ind < b; ind++) {
					GeoPoint* pPoint = ppPoint[ind];

					double x = pPoint->x * fileInfo.scaleX;
					double y = pPoint->y * fileInfo.scaleY;
					double z = pPoint->z * fileInfo.scaleZ;

					//outのbboxになければ捨てる
					if (x < bboxOuter.xmin || x > bboxOuter.xmax || y < bboxOuter.ymin ||
						y > bboxOuter.ymax || z < bboxOuter.zmin || z > bboxOuter.zmax) {
						continue;
					}
					//ローカル空間IDを算出
					std::string curSpatialId = spatialid::get_spatial_id_on_point(x, y, z, localZoom,
						static_cast<spatialid::CRS>(fileInfo.epsg));

					//inのbboxになければ、空間IDを算出して範囲内にあるかを判定
					if (x < bboxInner.xmin || x > bboxInner.xmax || y < bboxInner.ymin || y > bboxInner.ymax) {
						int diff = localZoom - spatialid::kGlobalZoom;
						spatialid::Zfxy localZfxy(curSpatialId);
						//ローカル空間IDからグローバル空間IDへ
						int newX = localZfxy.lon_index >> diff;
						int newY = localZfxy.lat_index >> diff;
						int newF = (localZfxy.alt_index > 0 ? localZfxy.alt_index >> diff : -((-localZfxy.alt_index) >> diff));
						spatialid::Zfxy newZfxy(spatialid::kGlobalZoom, newF, newX, newY);
						if (spatialId != newZfxy.getString()) {
							continue;
						}
					}

					localSpatialIdList.insert(curSpatialId);
				}

				return localSpatialIdList;
			};

		if (threadCount > 1) {
			BS::thread_pool pool(threadCount);
			BS::multi_future<std::unordered_set<std::string>> futures = pool.parallelize_loop(0, pointCount, loopFunc);

			futures.wait();
			std::vector<std::unordered_set<std::string>> results = futures.get();

			for (auto& list : results) {
				spatialIds.insert(list.begin(), list.end());
			}
		}
		else {
			std::unordered_set<std::string> list = loopFunc(0, pointCount);
			spatialIds.insert(list.begin(), list.end());
		}

	}
	else { //主にグローバル空間IDリスト取得用
		int globalZoom = zoomLevel;

		auto loopFunc = [this, &spatialId, ppPoint, &fileInfo, globalZoom](const int a, const int b)
			{
				std::unordered_set<std::string> localSpatialIdList;

				std::map<std::string, BBox> bboxMap;

				//int calcCount = 0;
				for (int ind = a; ind < b; ind++) {
					GeoPoint* pPoint = ppPoint[ind];

					double x = pPoint->x * fileInfo.scaleX;
					double y = pPoint->y * fileInfo.scaleY;
					double z = pPoint->z * fileInfo.scaleZ;

					//--- 空間IDを算出する
					//まず算出済みかをチェック
					std::string curSpatialId;
					for (auto& entry : bboxMap) {
						const std::string& tmpSpatialId = entry.first;
						BBox& bbox = entry.second;

						//outのbboxになければ捨てる
						if (x >= bbox.xmin && x <= bbox.xmax && y >= bbox.ymin &&
							y <= bbox.ymax && z >= bbox.zmin && z <= bbox.zmax) {
							curSpatialId = tmpSpatialId;
							break;
						}
					}
					//まだ見算出ならここで算出する
					if (curSpatialId.empty()) {
						//calcCount++;
						//空間IDを取得
						curSpatialId = spatialid::get_spatial_id_on_point(x, y, z, globalZoom,
							static_cast<spatialid::CRS>(fileInfo.epsg));

						//bboxを作成
						std::vector<spatialid::Point> pointList;
						spatialid::get_vertex_points_on_spatial_id(curSpatialId, pointList,
							static_cast<spatialid::CRS>(fileInfo.epsg));
						BBox bboxInner;
						//四点が長方形ではないので、代わりに以下のような長方形を用意(点が空間IDの範囲内にあるかの判定用）
						//①：点１と点４の最大X座標、②：点２と点３の最小X座標
						//③：点１と点２の最小Y座標、④：点３と点４の最大Y座標
						bboxInner.xmin = std::max(pointList[0].x, pointList[3].x);
						bboxInner.xmax = std::min(pointList[1].x, pointList[2].x);
						bboxInner.ymax = std::min(pointList[0].y, pointList[1].y);
						bboxInner.ymin = std::max(pointList[2].y, pointList[3].y);
						bboxInner.zmin = std::min(pointList[0].z, pointList[4].z);
						bboxInner.zmax = std::max(pointList[0].z, pointList[4].z);

						bboxMap.insert({ curSpatialId, bboxInner });
					}

					if (!curSpatialId.empty()) {
						localSpatialIdList.insert(curSpatialId);
					}
				}

				//logger->trace("## total:{0}, calcaluted:{1}", (b - a), calcCount);

				return localSpatialIdList;
			};

		if (threadCount > 1) {
			BS::thread_pool pool(threadCount);
			BS::multi_future<std::unordered_set<std::string>> futures = pool.parallelize_loop(0, pointCount, loopFunc);

			futures.wait();
			std::vector<std::unordered_set<std::string>> results = futures.get();

			for (auto& list : results) {
				spatialIds.insert(list.begin(), list.end());
			}
		}
		else {
			std::unordered_set<std::string> list = loopFunc(0, pointCount);
			spatialIds.insert(list.begin(), list.end());
		}

	}

	logger->debug("getSpatialIdList: id:{0}, zoom:{1}, size:{2} (経過時間：{3:.2f})", 
		spatialId, zoomLevel, spatialIds.size(),
		(double)(clock() - t1) / CLOCKS_PER_SEC);

}


void Converter::createVoxels(std::string& curDatetime, int dataSourceId, 
		std::unordered_set<std::string>& spatialIds, GeoDataset& gds, FileInfo& fileInfo,
		DataAccessAdapter& adapter, std::string& originalFileName) {
	clock_t ts = clock();

	Config& config = Config::getInstance();
	const std::vector<std::string>& targetSpatialIds = config.getTargetSpatialIds();
	int localZoom = config.getLocalZoom();
	int threadCount = config.getThreadCount();

	for (auto& spatialId : spatialIds) {
		clock_t startTime = clock();

		if (targetSpatialIds.size() > 0 &&
			std::find(targetSpatialIds.begin(), targetSpatialIds.end(), spatialId) == targetSpatialIds.end()) {
			//主にデバッグ用
			logger->info("spatialId:{0}　SKIP", spatialId);
			continue;
		}

		clock_t t1 = clock();

		//ローカル空間IDリストを作成
		std::unordered_set<std::string> localSpatialIds;
		std::string globalSpatialId = spatialId;
		getSpatialIdList(globalSpatialId, localZoom, fileInfo, gds, localSpatialIds);

		std::string spatialIdFile = spatialId;
		std::replace(spatialIdFile.begin(), spatialIdFile.end(), '/', '_');

		std::string voxelOutputFolder = config.getVoxelOutputFolder(spatialIdFile, curDatetime);
		std::string voxelOutputFolder2 = config.getVoxelBitFilePath(spatialIdFile, curDatetime);
		std::string outputFileName = spatialIdFile + "_pointcloud." + config.getOutputFileExt();

		//logger->trace("output(las):{0}", lasOutputFolder);
		//logger->trace("output(voxel):{0}", voxelOutputFolder);

		ComLib::CommonUtil::forceDirectories(voxelOutputFolder);


		PointCloudFeatureObjectsRec rec;
		rec.spatialId = spatialId;
		rec.fromDatetime = config.getFromDatetime();
		rec.toDatetime = config.getToDatetime();
		rec.voxelBitFilePath = ComLib::CommonUtil::pathJoin(voxelOutputFolder2, outputFileName);
		rec.voxelBitSpatialZoomLevel = config.getLocalZoom();
		rec.pointCloudEpsg = config.getConfirmLasEpsg();
		rec.dataSourceId = dataSourceId;
		rec.updateMemo = originalFileName;

		std::string voxelFileName = ComLib::CommonUtil::pathJoin(voxelOutputFolder, outputFileName);

		//ボクセルを作成	
		ComLib::Voxel voxel(spatialId, config.getLocalZoom(), config.getConfirmLasEpsg(), voxelFileName, config.getLoggerName());
		voxel.setScaleZ(config.getLasScaleZ());

		voxel.addLocalZoomSpatialIds(localSpatialIds);

		int altCount = voxel.getVoxelData().size();
		if (altCount <= 0) {
			logger->info("spatialId:{0} END (データなし） (elapsed time:{1:.2f}(sec))", spatialId,
				(double)(clock() - t1) / CLOCKS_PER_SEC);
			continue;
		}
		logger->trace("create voxel。spatialId:{0}, elapsed time:{1:.2f}(sec)",
			spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);

		//ファイルへ出力
		t1 = clock();
		logger->trace("++ color:{0:x}", config.getColor());
		voxel.toFile(config.getLasScale(), config.getColor());

		if (config.getOutputFileExt() == "laz") {
			std::string lasFileName = std::regex_replace(voxelFileName, std::regex("\\.laz$"), ".las");
			//std::cout << "lasFile:" << lasFileName << std::endl;
			std::filesystem::remove(lasFileName);
		}

		//DB登録
		logger->trace("---- DB登録開始");
		adapter.registPointCloudFeatureObjects(rec);
		logger->trace("output to file.spatialId:{0}, elapsed time:{1:.2f}(sec)", spatialId, (double)(clock() - t1) / CLOCKS_PER_SEC);

		logger->info("spatialId:{0} (elapsed time:{1:.2f}(sec))", spatialId,
			(double)(clock() - startTime) / CLOCKS_PER_SEC);

	}
	logger->debug("空間ＩＤ単位での処理　END (経過時間：{0:.2f})", (double)(clock() - ts) / CLOCKS_PER_SEC);
	
}



//void Converter::deleteOldFiles(DataAccessAdapter& adapter, std::string& dataType,
//		std::map<std::string, std::unordered_set<std::string>>& spatialIds) {
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
//
//		for (auto& entry : spatialIds) {
//			std::string tmpSpatialId = entry.first;
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