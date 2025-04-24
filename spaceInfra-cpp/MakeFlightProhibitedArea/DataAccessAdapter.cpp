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


std::vector<FlightProhibitedAreaObjectMastersInfo> DataAccessAdapter::getFlightProhibitedAreaObjectMasters(uint64_t flightProhibitedAreaObjectId) {
	std::vector<FlightProhibitedAreaObjectMastersInfo> infoList;

	//std::cout << "++ info.id:" << info.uniqueId << std::endl;

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(SELECT_SQL));
	int ind = 1;
	pstmt->setUInt64(ind++, flightProhibitedAreaObjectId);

	std::unique_ptr<sql::ResultSet> rs;
	rs.reset(pstmt->executeQuery());
	while (rs->next()) {
		FlightProhibitedAreaObjectMastersInfo info;

		info.flightProhibitedAreaObjectId = rs->getUInt64("flight_prohibited_area_object_id");
		info.flightProhibitedAreaId = rs->getString("flight_prohibited_area_id");
		info.name = rs->getString("name");
		info.fromDatetime = rs->getString("from_datetime");
		info.toDatetime = rs->getString("to_datetime");
		info.range = rs->getString("range");
		info.detail = rs->getString("detail");
		info.url = rs->getString("url");
		info.flightProhibitedAreaType = rs->getInt("flight_prohibited_area_type_id");
		info.status = rs->getInt("status");

		infoList.push_back(info);
	}

	return infoList;
}


int DataAccessAdapter::updateFlightProhibitedAreaObjectMasters(std::vector<std::string>& flightProhibitedAreaObjectIds, int status) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);
	
	std::string str;
	for (auto& infoId : flightProhibitedAreaObjectIds) {
		if (str.length() > 0) {
			str += ",";
		}
		str += "?";
	}

	std::string sql = std::regex_replace(UPDATE_SQL, std::regex(":id_list:"), str);

	//std::cout << "update: flightProhibitedAreaObjectId=" << flightProhibitedAreaObjectId << ",status=" << status << std::endl;

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	int ind = 1;
	pstmt->setInt(ind++, status);

	for (auto& infoId : flightProhibitedAreaObjectIds) {
		uint64_t val = std::stoull(infoId);

		pstmt->setUInt64(ind++, val);
	}

	int rc = pstmt->executeUpdate();
	//std::cout << "rc:" << rc << std::endl;

	return rc;
}

int DataAccessAdapter::insertFlightProhibitedAreaObjects(FlightProhibitedAreaObjectsInfo& rec) {
	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(INSERT_SQL));

	int ind = 1;
	pstmt->setUInt64(ind++, rec.flightProhibitedAreaObjectId);
	pstmt->setString(ind++, rec.spatialId);
	pstmt->setString(ind++, rec.voxelBitFilePath);
	pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
	pstmt->setInt(ind++, rec.pointCloudEpsg);
	//pstmt->setString(ind++, rec.updateMemo);

	//std::cout << "++ ind:" << ind << ", clsPoints.size:" << rec.clsPoints.size() << std::endl;

	int rc = pstmt->executeUpdate();

	return rc;
}

int DataAccessAdapter::deleteFlightProhibitedAreaObjects(uint64_t flightProhibitedAreaObjectId) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt;
	
	pstmt.reset(conUserDb->prepareStatement(DELETE_SQL));
	
	int ind = 1;
	pstmt->setUInt64(ind++, flightProhibitedAreaObjectId);
	int rc = pstmt->executeUpdate();

	return rc;
}
