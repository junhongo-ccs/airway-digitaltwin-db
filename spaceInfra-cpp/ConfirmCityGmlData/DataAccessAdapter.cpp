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
	con.reset(nullptr);
	conUserDb.reset(nullptr);
}

void DataAccessAdapter::connect(Config& config) {
	//std::cout << "in connect" << std::endl;
	if (conUserDb != nullptr) {
		return;
	}

	con.reset(driver->connect(config.getDbHost(), config.getDbUser(), config.getDbPass()));
	conUserDb.reset(driver->connect(config.getUserDbHost(), config.getUserDbUser(), config.getUserDbPass()));

	dbName = config.getDbName();
	userDbName = config.getUserDbName();

	//std::cout << "dbName:" << dbName << ", userDbName:" << userDbName << std::endl;
}


void DataAccessAdapter::createObjectIdMap(std::map<std::string, int>& featureId2ObjIdMap) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	const std::string sql = "SELECT object_cd, object_name FROM object_masters WHERE deleted_at IS NULL OR deleted_at = '' ";
	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	std::unique_ptr<sql::ResultSet> rs;
	rs.reset(pstmt->executeQuery());
	while (rs->next()) {
		int objCd = rs->getInt("object_cd");
		std::string objName = rs->getString("object_name");

		//std::cout << "objId:" << objId << std::endl;
		//std::cout << "objName:" << objName << std::endl;

		std::string featureId;
		if (objName == "Building") {
			featureId = "BUILDING";
		}
		else if (objName == "Water") {
			featureId = "WATER";
		}
		else if (objName == "Rail") {
			featureId = "RAILWAY";
		}
		else if (objName == "Road Surface") {
			featureId = "ROAD";
		}
		else if (objName == "Wire-Condactor") {
			featureId = "POWERLINE";
		}
		else if (objName == "Transmisson Tower") {
			featureId = "STEELTOWER";
		}
		else if (objName == "Ground" || objName == "GROUND") {
			featureId = "GROUND";
		}

		if (!featureId.empty()) {
			featureId2ObjIdMap.insert({ featureId, objCd });
		}
	}

	//100と200を追加
	featureId2ObjIdMap.insert({ "INPUT", OBJ_ID_INPUT });
	featureId2ObjIdMap.insert({ "ALL", OBJ_ID_ALL });
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

void DataAccessAdapter::updateSpaceDataManages(const std::string& ownerId,
	const std::string& batchId, const std::string& spatialId, std::string& recId) {

	//std::cout << "ownerId:" << ownerId << std::endl;
	//std::cout << "batchId:" << batchId << std::endl;
	//std::cout << "spatialId:" << spatialId << std::endl;
	//std::cout << "recId:" << recId << std::endl;
	//std::cout << "dbName:" << dbName << std::endl;

	std::unique_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("USE " + dbName);

	std::string sql = "UPDATE space_data_manages set rec_id=?, updated_at=now() ";
	sql.append(
		"WHERE owner_id=? AND batch_id=? and spatial_id=? AND (deleted_at IS NULL OR deleted_at= '') ");
	std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(sql));

	int ind = 1;
	pstmt->setString(ind++, recId);
	pstmt->setString(ind++, ownerId);
	pstmt->setString(ind++, batchId);
	pstmt->setString(ind++, spatialId);

	pstmt->execute();
}

void DataAccessAdapter::insertSpaceObject(SpaceObjectRec& rec) {
	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	//std::string sql = "INSERT INTO ground_feature_objects(spatial_id,north_latitude,south_latitude,";
	//sql.append("east_longitude, west_longitude,lower_altitude,upper_altitude,");
	//sql.append("object_cd, voxel_bit_file_path, "); // point_cloud_file_path, ");
	//sql.append("hash_18,hash_19,hash_20,hash_21,hash_22,hash_23,hash_24,hash_25,");
	//sql.append("hash_26, hash_27, hash_28, ");
	std::string sql = "INSERT INTO ground_feature_objects(spatial_id,from_datetime,to_datetime,";
	sql.append("object_cd, voxel_bit_file_path, voxel_bit_spatial_zoom_level,");
	sql.append("point_cloud_epsg, data_source_id, ");
	sql.append("update_memo, created_at, updated_at ) ");
	sql.append("VALUES(?,?,");
	if (!rec.toDatetime.empty()) {
		sql.append("?,");
	}
	else {
		sql.append("NULL,");
	}
	sql.append("  ? , ? , ? , ? , ? , ");
	sql.append(" ? , now(), now() )");

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	int ind = 1;
	pstmt->setString(ind++, rec.spatialId); 

	sql::SQLString fromDatetime(rec.fromDatetime);
	pstmt->setDateTime(ind++, fromDatetime);
	if (!rec.toDatetime.empty()) {
		sql::SQLString toDatetime(rec.toDatetime);
		pstmt->setDateTime(ind++, toDatetime);
	}
	//pstmt->setDouble(ind++, rec.northLat);
	//pstmt->setDouble(ind++, rec.southLat);
	//pstmt->setDouble(ind++, rec.eastLon);
	//pstmt->setDouble(ind++, rec.westLon);
	//pstmt->setDouble(ind++, rec.lowerAltitude);
	//pstmt->setDouble(ind++, rec.upperAltitude);
	pstmt->setInt(ind++, rec.objectCd);
	pstmt->setString(ind++, rec.voxelBitFilePath);
	pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
	//pstmt->setString(ind++, rec.pointCloudFilePath);

	//for (const auto hash : rec.hashList) {
	//	pstmt->setString(ind++, hash);
	//}
	pstmt->setInt(ind++, rec.pointCloudEpsg);
	pstmt->setInt(ind++, rec.dataSourceId);
	pstmt->setString(ind++, rec.updateMemo);

	pstmt->execute();
}

//void DataAccessAdapter::updateMemo(SpaceObjectRec& rec) {
//	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
//	stmt->execute("USE " + userDbName);
//
//	std::string sql = "UPDATE ground_feature_objects SET update_memo = CONCAT(CONCAT(update_memo,', '), ?), updated_at=now() ";
//	sql.append("WHERE spatial_id=? AND object_cd=? ");
//
//	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));
//
//	int ind = 1;
//	pstmt->setString(ind++, rec.updateMemo);
//	pstmt->setString(ind++, rec.spatialId);
//	pstmt->setInt(ind++, rec.objectCd);
//
//	pstmt->execute();
//}


void DataAccessAdapter::deleteVoxel(std::string init_flg, std::string spatial_id, std::string dirPath) {
	logger->trace("++ init_flg:{0}, spatial_id:{1}, dirPath:{2}", init_flg, spatial_id, dirPath);

	//念のためにフォルダ名をチェック
	if (dirPath.empty() || dirPath.length() <= 3 ||
		ComLib::CommonUtil::startsWithIgnoreCase(dirPath, "C:/Windows") ||
		ComLib::CommonUtil::startsWithIgnoreCase(dirPath, "C:/Program")
		) {
		return;
	}

	//第１引数が"init"の場合、第３引数のフォルダを空にする
	if (init_flg == "init") {
		std::filesystem::remove_all(dirPath);
		ComLib::CommonUtil::forceDirectories(dirPath);
	}
	else if (init_flg == "spatialInit") {

		//第１引数が"spatialInit"の場合、削除ファイル作成とＤＢ更新を行う

		std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
		stmt->execute("USE " + userDbName);

		//削除フォルダ内に削除ファイルを空で作成
		std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sqlSELECT));
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

		//space_objectsの更新
		pstmt.reset(conUserDb->prepareStatement(sqlUPDATE));
		pstmt->setString(1, spatial_id);
		pstmt->execute();

	}

	//第１引数が"init""spatialInit"以外の場合、何もしない
}
