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
	con.reset(nullptr);
}

void DataAccessAdapter::connect(Config& config) {
	//std::cout << "in connect" << std::endl;
	if (con != nullptr) {
		return;
	}

	con.reset(driver->connect(config.getDbHost(), config.getDbUser(), config.getDbPass()));
	//conUserDb.reset(driver->connect(config.getUserDbHost(), config.getUserDbUser(), config.getUserDbPass()));

	dbName = config.getDbName();
	//userDbName = config.getUserDbName();

	//std::cout << "dbName:" << dbName << ", userDbName:" << userDbName << std::endl;
}


SpaceDataManagesInfo DataAccessAdapter::getSpaceDataManages(std::string& ownerId, std::string& batchId,
		std::string& spatialId, std::string& recId, std::string& dataType) {
	SpaceDataManagesInfo info;

	//std::cout << "++ info.id:" << info.uniqueId << std::endl;

	std::unique_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("USE " + dbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(SELECT_SQL));
	int ind = 1;
	pstmt->setString(ind++, ownerId);
	pstmt->setString(ind++, batchId);
	pstmt->setString(ind++, spatialId);
	pstmt->setString(ind++, recId);
	pstmt->setString(ind++, dataType);

	std::unique_ptr<sql::ResultSet> rs;
	rs.reset(pstmt->executeQuery());
	if (rs->next()) {
		info.uniqueId = rs->getUInt64("id");
		info.ownerId = ownerId;
		info.batchId = batchId;
		info.spatialId = spatialId;
		info.recId = recId;
		info.dataType = dataType;

		info.fileId = rs->getString("file_id");
		info.filePath = rs->getString("file_path");
		info.epsgLas = rs->getString("epsg_las");
		info.epsgCsv = rs->getString("epsg_csv");
		info.dataUnit = rs->getString("data_unit");
		info.dataIo = rs->getString("data_io");
		info.dataStatus = rs->getString("data_status");
		info.dataAbcd = rs->getString("data_abcd");

		info.spXmin = rs->getDouble("sp_xmin");
		info.spXmax = rs->getDouble("sp_xmax");
		info.spYmin = rs->getDouble("sp_ymin");
		info.spYmax = rs->getDouble("sp_ymax");
		info.spZmin = rs->getDouble("sp_zmin");
		info.spZmax = rs->getDouble("sp_zmax");
		info.spXminW = rs->getDouble("sp_xmin_w");
		info.spXmaxW = rs->getDouble("sp_xmax_w");
		info.spYminW = rs->getDouble("sp_ymin_w");
		info.spYmaxW = rs->getDouble("sp_ymax_w");

		std::string tmpFileIds = rs->getString("originalFileIds");
		info.originalFileIds = ComLib::CommonUtil::split(tmpFileIds, ',');

		info.points = rs->getInt("points");
		info.xmin = rs->getDouble("xmin");
		info.xmax = rs->getDouble("xmax");
		info.ymin = rs->getDouble("ymin");
		info.ymax = rs->getDouble("ymax");
		info.zmin = rs->getDouble("zmin");
		info.zmax = rs->getDouble("zmax");

		info.xminW = rs->getDouble("xmin_w");
		info.xmaxW = rs->getDouble("xmax_w");
		info.yminW = rs->getDouble("ymin_w");
		info.ymaxW = rs->getDouble("ymax_w");

		for (int i = 0; i < 21; i++) {
			info.clsPoints.push_back(rs->getInt("cls" + std::to_string(i)));
		}
		info.clsPoints.push_back(rs->getInt("cls99"));

		info.comment = rs->getString("comment");

	}

	return info;
}


int DataAccessAdapter::updateSpaceDataManages(SpaceDataManagesInfo& rec) {

	std::unique_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("USE " + dbName);
	
	//std::cout << "path2:" << std::endl;

	std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(UPDATE_SQL));

	int ind = 1;
	pstmt->setString(ind++, ComLib::CommonUtil::join(rec.originalFileIds, ":"));
	pstmt->setInt(ind++, rec.points);
	pstmt->setDouble(ind++, rec.xmin);
	pstmt->setDouble(ind++, rec.xmax);
	pstmt->setDouble(ind++, rec.ymin);
	pstmt->setDouble(ind++, rec.ymax);
	pstmt->setDouble(ind++, rec.zmin);
	pstmt->setDouble(ind++, rec.zmax);
	pstmt->setDouble(ind++, rec.xminW);
	pstmt->setDouble(ind++, rec.xmaxW);
	pstmt->setDouble(ind++, rec.yminW);
	pstmt->setDouble(ind++, rec.ymaxW);

	for (auto& clsPoint : rec.clsPoints) {
		pstmt->setInt(ind++, clsPoint);
	}

	//condition
	//pstmt->setString(ind++, rec.ownerId);
	//pstmt->setString(ind++, rec.batchId);
	//pstmt->setString(ind++, rec.spatialId);
	//pstmt->setString(ind++, rec.recId);
	//pstmt->setString(ind++, rec.dataType);
	pstmt->setUInt64(ind++, rec.uniqueId);

	int rc = pstmt->executeUpdate();
	//std::cout << "rc:" << rc << std::endl;

	return rc;
}

int DataAccessAdapter::insertSpaceDataManages(SpaceDataManagesInfo& rec) {
	std::unique_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("USE " + dbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(INSERT_SQL));

	int ind = 1;
	pstmt->setString(ind++, rec.ownerId);
	pstmt->setString(ind++, rec.batchId);
	pstmt->setString(ind++, rec.spatialId);
	pstmt->setString(ind++, rec.recId);
	pstmt->setString(ind++, rec.fileId);
	pstmt->setString(ind++, rec.filePath);
	pstmt->setString(ind++, rec.epsgLas);
	pstmt->setString(ind++, rec.epsgCsv);
	pstmt->setString(ind++, rec.dataUnit);
	pstmt->setString(ind++, rec.dataIo);
	pstmt->setString(ind++, rec.dataType);
	pstmt->setString(ind++, rec.dataStatus);
	pstmt->setString(ind++, rec.dataAbcd);

	pstmt->setDouble(ind++, rec.spXmin);
	pstmt->setDouble(ind++, rec.spXmax);
	pstmt->setDouble(ind++, rec.spYmin);
	pstmt->setDouble(ind++, rec.spYmax);
	pstmt->setDouble(ind++, rec.spZmin);
	pstmt->setDouble(ind++, rec.spZmax);
	pstmt->setDouble(ind++, rec.spXminW);
	pstmt->setDouble(ind++, rec.spXmaxW);
	pstmt->setDouble(ind++, rec.spYminW);
	pstmt->setDouble(ind++, rec.spYmaxW);

	pstmt->setString(ind++, ComLib::CommonUtil::join(rec.originalFileIds, ":"));
	pstmt->setInt(ind++, rec.points);
	pstmt->setDouble(ind++, rec.xmin);
	pstmt->setDouble(ind++, rec.xmax);
	pstmt->setDouble(ind++, rec.ymin);
	pstmt->setDouble(ind++, rec.ymax);
	pstmt->setDouble(ind++, rec.zmin);
	pstmt->setDouble(ind++, rec.zmax);
	pstmt->setDouble(ind++, rec.xminW);
	pstmt->setDouble(ind++, rec.xmaxW);
	pstmt->setDouble(ind++, rec.yminW);
	pstmt->setDouble(ind++, rec.ymaxW);

	for (auto& clsPoint : rec.clsPoints) {
		pstmt->setInt(ind++, clsPoint);
	}

	//std::cout << "++ ind:" << ind << ", clsPoints.size:" << rec.clsPoints.size() << std::endl;

	int rc = pstmt->executeUpdate();

	return rc;
}

int DataAccessAdapter::registSpaceDataManages(SpaceDataManagesInfo& rec) {
	SpaceDataManagesInfo curInfo;

	//std::cout << "ownerId:" << rec.ownerId << std::endl;
	//std::cout << "batchId:" << rec.batchId << std::endl;
	//std::cout << "spatialId:" << rec.spatialId << std::endl;
	//std::cout << "recId:" << rec.recId << std::endl;
	//std::cout << "dataType:" << rec.dataType << std::endl;


	//すでにあるかをチェック
	curInfo = getSpaceDataManages(rec.ownerId, rec.batchId, rec.spatialId, rec.recId, rec.dataType);

	int rc = 0;
	if (curInfo.uniqueId > 0) {//すでにある
		//std::cout << "++ uniqueId:" << curInfo.uniqueId << std::endl;

		SpaceDataManagesInfo newInfo = rec;
		newInfo.uniqueId = curInfo.uniqueId;

		//データをマージ
		std::string originalFileIds = ComLib::CommonUtil::join(curInfo.originalFileIds, ":");
		//std::cout << "originalFileIds(before):" << originalFileIds << std::endl;

		for (auto& fileId : rec.originalFileIds) {
			if (originalFileIds.find(fileId) == std::string::npos) {
				originalFileIds += ":" + fileId;
			}
		}
		//std::cout << "originalFileIds(after):" << originalFileIds << std::endl;

		newInfo.originalFileIds = ComLib::CommonUtil::split(originalFileIds, ':');
#if 0 //既存のLASがすでにマージされているので、ここでminmaxの再計算は不要
		newInfo.xmin = std::min(curInfo.xmin, newInfo.xmin);
		newInfo.xmax = std::max(curInfo.xmax, newInfo.xmax);
		newInfo.ymin = std::min(curInfo.ymin, newInfo.ymin);
		newInfo.ymax = std::max(curInfo.ymax, newInfo.ymax);
		newInfo.zmin = std::min(curInfo.zmin, newInfo.zmin);
		newInfo.zmax = std::max(curInfo.zmax, newInfo.zmax);
		newInfo.xminW = std::min(curInfo.xminW, newInfo.xminW);
		newInfo.xmaxW = std::max(curInfo.xmaxW, newInfo.xmaxW);
		newInfo.yminW = std::min(curInfo.yminW, newInfo.yminW);
		newInfo.ymaxW = std::max(curInfo.ymaxW, newInfo.ymaxW);
		newInfo.points += curInfo.points;
		for (int i = 0; i < newInfo.clsPoints.size(); i++) {
			newInfo.clsPoints[i] += curInfo.clsPoints[i];
		}
#endif
		rc = updateSpaceDataManages(newInfo);
	}
	else {
		insertSpaceDataManages(rec);
	}

	return rc;
}

int DataAccessAdapter::deleteSpaceDataManages(std::string& ownerId, std::string& batchId,
	std::string& spatialId, std::string& recId, std::string& dataType) {

	std::unique_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("USE " + dbName);

	std::unique_ptr<sql::PreparedStatement> pstmt;
	
	if (spatialId.empty()) {
		pstmt.reset(con->prepareStatement(DELETE_SQL_WITHOUT_SPATIALID));
	}
	else {
		pstmt.reset(con->prepareStatement(DELETE_SQL));
	}
	int ind = 1;
	pstmt->setString(ind++, ownerId);
	pstmt->setString(ind++, batchId);
	if (!spatialId.empty()) {
		pstmt->setString(ind++, spatialId);
	}
	pstmt->setString(ind++, recId);
	pstmt->setString(ind++, dataType);

	std::unique_ptr<sql::ResultSet> rs;
	int rc = pstmt->executeUpdate();

	return rc;
}
