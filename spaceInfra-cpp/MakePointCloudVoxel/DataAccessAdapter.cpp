#include "DataAccessAdapter.h"
#include "Config.h"
#include <fstream>
#include "CommonUtil.h"


DataAccessAdapter::DataAccessAdapter() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());

	driver = sql::mysql::get_mysql_driver_instance();
}

DataAccessAdapter::~DataAccessAdapter() {
	//.reset(nullptr);
	conUserDb.reset(nullptr);
}

void DataAccessAdapter::connect(Config& config) {
	//std::cout << "in connect" << std::endl;
	if (conUserDb != nullptr) {
		return;
	}

	//con.reset(driver->connect(config.getDbHost(), config.getDbUser(), config.getDbPass()));
	conUserDb.reset(driver->connect(config.getUserDbHost(), config.getUserDbUser(), config.getUserDbPass()));

	//dbName = config.getDbName();
	userDbName = config.getUserDbName();

	//std::cout << "dbName:" << dbName << ", userDbName:" << userDbName << std::endl;
}

int DataAccessAdapter::getDataSourceId(std::string& serverId, std::string& projectId) {
	logger->trace("getDataSourceId:: dbName:{0}, serverId:{1}, projectId:{2}", userDbName, serverId, projectId);

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	const std::string sql = "SELECT data_source_id FROM data_sources WHERE data_create_server_id = ? AND data_create_project_id = ? AND (deleted_at IS NULL OR deleted_at='') ";
	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	int ind = 1;
	pstmt->setString(ind++, serverId);
	pstmt->setString(ind++, projectId);

	std::unique_ptr<sql::ResultSet> rs;
	rs.reset(pstmt->executeQuery());

	if (rs->next()) {
		return rs->getInt("data_source_id");
	}

	return -1;
}


PointCloudFeatureObjectsRec DataAccessAdapter::getPointCloudFeatureObjects(std::string& spatialId) {
	logger->trace("getPointCloudFeatureObjects: spatialId={0}", spatialId);

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(SELECT_SQL));

	int ind = 1;
	pstmt->setString(ind++, spatialId);

	std::unique_ptr<sql::ResultSet> rs;
	PointCloudFeatureObjectsRec rec;
	rs.reset(pstmt->executeQuery());

	if (rs->next()) {
		rec.pointCloudFeatureObjectId = rs->getUInt64("point_cloud_feature_object_id");
		rec.spatialId = rs->getString("spatial_id");
		rec.fromDatetime = rs->getString("from_datetime");
		rec.toDatetime = rs->getString("to_datetime");
		rec.voxelBitFilePath = rs->getString("voxel_bit_file_path");
		rec.voxelBitSpatialZoomLevel = rs->getInt("voxel_bit_spatial_zoom_level");
		rec.pointCloudEpsg = rs->getInt("point_cloud_epsg");
		rec.dataSourceId = rs->getInt("data_source_id");
		rec.updateMemo = rs->getString("update_memo");
	}

	return rec;
}


int DataAccessAdapter::updatePointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);
	
	//std::cout << "path2:" << std::endl;

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(UPDATE_SQL));

	int ind = 1;
	pstmt->setString(ind++, rec.updateMemo);
	pstmt->setInt64(ind++, rec.pointCloudFeatureObjectId);

	int rc = pstmt->executeUpdate();
	//std::cout << "rc:" << rc << std::endl;

	return rc;
}

int DataAccessAdapter::insertPointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec) {
	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::string sql = INSERT_SQL;
	std::string fromStr = ":to_datetime:";
	const unsigned int pos = sql.find(fromStr);
	if (rec.toDatetime.empty()) {
		sql.replace(pos, fromStr.length(), "NULL");
	}
	else {
		sql.replace(pos, fromStr.length(), "?");
	}

	//std::cout << "sql:" << sql << std::endl;

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	int ind = 1;
	pstmt->setString(ind++, rec.spatialId);
	pstmt->setString(ind++, rec.fromDatetime);
	if (!rec.toDatetime.empty()) {
		pstmt->setString(ind++, rec.toDatetime);
	}
	pstmt->setString(ind++, rec.voxelBitFilePath);
	pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
	pstmt->setInt(ind++, rec.pointCloudEpsg);
	pstmt->setInt(ind++, rec.dataSourceId);
	pstmt->setString(ind++, rec.updateMemo);

	//std::cout << "++ ind:" << ind << ", clsPoints.size:" << rec.clsPoints.size() << std::endl;

	int rc = pstmt->executeUpdate();

	return rc;
}

int DataAccessAdapter::registPointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec) {
	PointCloudFeatureObjectsRec curInfo;

	//すでにあるかをチェック
	curInfo = getPointCloudFeatureObjects(rec.spatialId);

	int rc = 0;
	if (curInfo.pointCloudFeatureObjectId > 0) {//すでにある
		//std::cout << "++ uniqueId:" << curInfo.uniqueId << std::endl;

		PointCloudFeatureObjectsRec newInfo = rec;
		newInfo.pointCloudFeatureObjectId = curInfo.pointCloudFeatureObjectId;

		//データをマージ
		std::string curUpdateMemo = curInfo.updateMemo;
		std::string newUpdateMemo = rec.updateMemo;
		if (curUpdateMemo.find(rec.updateMemo) == std::string::npos) {
			//長さをチェック
			int finalLen = curUpdateMemo.length() + 1 + rec.updateMemo.length();
			while (finalLen >= 1000) {
				unsigned int pos = curUpdateMemo.find(",");
				if (pos >= 0) {
					curUpdateMemo = curUpdateMemo.substr(pos + 1);
				}
				else {
					curUpdateMemo = "";
				}
				finalLen = curUpdateMemo.length();
			}
			if (!curUpdateMemo.empty()) {
				newUpdateMemo = curUpdateMemo + "," + newUpdateMemo;
			}
		}
		else {
			newUpdateMemo = curInfo.updateMemo;
		}

		//std::cout << "originalFileIds(after):" << originalFileIds << std::endl;

		newInfo.updateMemo = newUpdateMemo;

		rc = updatePointCloudFeatureObjects(newInfo);
	}
	else {
		insertPointCloudFeatureObjects(rec);
	}

	return rc;
}

int DataAccessAdapter::deletePointCloudFeatureObjects(std::string& spatialId) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt;
	
	pstmt.reset(conUserDb->prepareStatement(DELETE_SQL));
	int ind = 1;
	pstmt->setString(ind++, spatialId);

	std::unique_ptr<sql::ResultSet> rs;
	int rc = pstmt->executeUpdate();

	return rc;
}

void DataAccessAdapter::deleteVoxel(std::string init_flg, std::string spatial_id, std::string dirPath) {
	//logger->trace("++ init_flg:{0}, spatial_id:{1}, dirPath:{2}", init_flg, spatial_id, dirPath);

	//念のためにフォルダ名をチェック
	if (/*dirPath.empty() || dirPath.length() <= 3 ||*/
		ComLib::CommonUtil::startsWithIgnoreCase(dirPath, "C:/Windows") ||
		ComLib::CommonUtil::startsWithIgnoreCase(dirPath, "C:/Program")
		) {
		return;
	}

	//第１引数が"init"の場合、第３引数のフォルダを空にする
	if (init_flg == "init") {
		if (!dirPath.empty()) {
			std::filesystem::remove_all(dirPath);
			ComLib::CommonUtil::forceDirectories(dirPath);
		}
	}
	else if (init_flg == "spatialInit") {

		//第１引数が"spatialInit"の場合、削除ファイル作成とＤＢ更新を行う

		std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
		stmt->execute("USE " + userDbName);

		//削除フォルダ内に削除ファイルを空で作成
		if (!dirPath.empty()) {
			std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(SELECT_SQL));
			pstmt->setString(1, spatial_id);
			std::unique_ptr<sql::ResultSet> result;
			result.reset(pstmt->executeQuery());
			while (result->next()) {
				int object_cd = result->getInt(1);
				std::string filePath = dirPath + "/" + regex_replace(spatial_id, std::regex("/"), "_") + "_" +
					std::to_string(object_cd) + ".txt";
				std::ofstream strm;
				strm.open(filePath);
				strm.close();
			}
		}

		//space_objectsの更新
		{
			std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(SELECT_SQL));
			pstmt.reset(conUserDb->prepareStatement(DELETE_SQL));
			pstmt->setString(1, spatial_id);
			pstmt->execute();
		}
	}

	//第１引数が"init""spatialInit"以外の場合、何もしない
}

