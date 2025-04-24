#include "DataAccessAdapter.h"
#include "Config.h"
#include <fstream>
#include "CommonUtil.h"
#include <regex>


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
    if (conUserDb != nullptr) {
        return;
    }
    //std::cout << "dbHost:" << config.getDbHost() << ", userDbName:" << config.getUserDbName() << std::endl;

    // ホスト名とポート番号をパース
    std::string dbHost = config.getDbHost();
    std::string hostName;
    int port = 3306; // デフォルトポート

    if (dbHost.find("tcp://") == 0) {
        dbHost = dbHost.substr(6);
    }
    size_t colonPos = dbHost.find(":");
    if (colonPos != std::string::npos) {
        hostName = dbHost.substr(0, colonPos);
        port = std::stoi(dbHost.substr(colonPos + 1));
    }
    else {
        hostName = dbHost;
    }

    // 接続オプションを設定
    sql::ConnectOptionsMap connection_properties;
    connection_properties["hostName"] = hostName;
    connection_properties["userName"] = config.getDbUser();
    connection_properties["password"] = config.getDbPass();
    connection_properties["port"] = port;
    connection_properties["OPT_RECONNECT"] = true;

    try {
        // データベースに接続
        con.reset(driver->connect(connection_properties));
    }
    catch (sql::SQLException& e) {
        std::cerr << "Failed to connect to dbHost: " << hostName << ", error: " << e.what() << std::endl;
        throw;
    }

    // ユーザーデータベース接続用にホスト名とポート番号を再設定
    dbHost = config.getUserDbHost();
    if (dbHost.find("tcp://") == 0) {
        dbHost = dbHost.substr(6);
    }
    colonPos = dbHost.find(":");
    if (colonPos != std::string::npos) {
        hostName = dbHost.substr(0, colonPos);
        port = std::stoi(dbHost.substr(colonPos + 1));
    }
    else {
        hostName = dbHost;
    }

    // 接続オプションを更新
    connection_properties["hostName"] = hostName;
    connection_properties["port"] = port;

    try {
        // ユーザーデータベースに接続
        conUserDb.reset(driver->connect(connection_properties));
    }
    catch (sql::SQLException& e) {
        std::cerr << "Failed to connect to userDbHost: " << hostName << ", error: " << e.what() << std::endl;
        throw;
    }

    userDbName = config.getUserDbName();
}


void DataAccessAdapter::insertRadioWaveObject(RadioWaveObjectRec& rec) {
    std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
    stmt->execute("USE " + userDbName);

    std::string sql = "INSERT INTO radio_wave_objects (spatial_id, from_datetime, to_datetime, voxel_bit_file_path, voxel_bit_spatial_zoom_level, point_cloud_epsg, update_memo, created_at, updated_at) ";
    sql.append("VALUES (?, ?, ?, ?, ?, ?, ?, NOW(), NOW())");

    std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

    int ind = 1;
    pstmt->setString(ind++, rec.spatialId);
    pstmt->setString(ind++, rec.fromDatetime);

    if (!rec.toDatetime.empty()) {
        pstmt->setString(ind++, rec.toDatetime);
    }
    else {
        pstmt->setNull(ind++, sql::DataType::VARCHAR);
    }

    pstmt->setString(ind++, rec.voxelBitFilePath);
    pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
    pstmt->setInt(ind++, rec.pointCloudEpsg);

    if (!rec.updateMemo.empty()) {
        pstmt->setString(ind++, rec.updateMemo);
    }
    else {
        pstmt->setNull(ind++, sql::DataType::VARCHAR);
    }

    pstmt->execute();

    std::unique_ptr<sql::Statement> stmt2(conUserDb->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt2->executeQuery("SELECT LAST_INSERT_ID()"));
    if (rs->next()) {
        rec.radioWaveObjectId = rs->getUInt64(1);
    }
}

void DataAccessAdapter::updateRadioWaveObject(RadioWaveObjectRec& rec) {
    std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
    stmt->execute("USE " + userDbName);

    std::string sql = "UPDATE radio_wave_objects SET "
        "spatial_id = ?, "
        "from_datetime = ?, "
        "to_datetime = ?, "
        "voxel_bit_file_path = ?, "
        "voxel_bit_spatial_zoom_level = ?, "
        "point_cloud_epsg = ?, "
        "update_memo = ?, "
        "updated_at = NOW() "
        "WHERE radio_wave_object_id = ?";

    std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

    int ind = 1;
    pstmt->setString(ind++, rec.spatialId);
    pstmt->setString(ind++, rec.fromDatetime);

    if (!rec.toDatetime.empty()) {
        pstmt->setString(ind++, rec.toDatetime);
    }
    else {
        pstmt->setNull(ind++, sql::DataType::VARCHAR);
    }

    pstmt->setString(ind++, rec.voxelBitFilePath);
    pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
    pstmt->setInt(ind++, rec.pointCloudEpsg);

    if (!rec.updateMemo.empty()) {
        pstmt->setString(ind++, rec.updateMemo);
    }
    else {
        pstmt->setNull(ind++, sql::DataType::VARCHAR);
    }

    pstmt->setUInt64(ind++, rec.radioWaveObjectId);

    pstmt->execute();
}

void DataAccessAdapter::upsertRadioWaveObject(const std::string& spatialId, const std::string& lasFilePath) {
	// Config インスタンスを取得
	Config& config = Config::getInstance();
	std::string fromDatetime = config.getFromDatetime();
	std::string toDatetime = config.getToDatetime();

    std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
    stmt->execute("USE " + userDbName);

    std::string sql = "SELECT radio_wave_object_id FROM radio_wave_objects "
        "WHERE spatial_id = ? AND from_datetime = ? AND deleted_at IS NULL";

    std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

    int ind = 1;
    pstmt->setString(ind++, spatialId);
    pstmt->setString(ind++, fromDatetime);

    std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());

    // ファイルパスの区切り文字を"/"からバックスラッシュに置き換え
	std::string lasFilePathEscaped = std::regex_replace(lasFilePath, std::regex("/"), "\\");

    RadioWaveObjectRec rec;
    rec.spatialId = spatialId;
    rec.voxelBitFilePath = lasFilePathEscaped;
	rec.fromDatetime = fromDatetime;
	rec.toDatetime = toDatetime;
    rec.voxelBitSpatialZoomLevel = 17;
    rec.pointCloudEpsg = std::stoi(config.getConfirmLasEpsg());
	rec.createdAt = CommonUtil::getCurrentDateTime();

    if (rs->next()) {
		// すでに空間IDが存在する場合は更新
        rec.radioWaveObjectId = rs->getUInt64("radio_wave_object_id");
        updateRadioWaveObject(rec);
    }
    else {
		// 空間IDが存在しない場合は新規登録
        insertRadioWaveObject(rec);
    }
}