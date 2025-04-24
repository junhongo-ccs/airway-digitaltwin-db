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


std::vector<AreaObjectMastersInfo> DataAccessAdapter::getAreaObjectMasters(uint64_t areaObjectId) {
	std::vector<AreaObjectMastersInfo> infoList;

	//std::cout << "++ info.id:" << info.uniqueId << std::endl;

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(SELECT_SQL));
	int ind = 1;
	pstmt->setUInt64(ind++, areaObjectId);

	std::unique_ptr<sql::ResultSet> rs;
	rs.reset(pstmt->executeQuery());
	while (rs->next()) {
		AreaObjectMastersInfo info;

		info.areaObjectId = rs->getUInt64("area_object_id");
		info.areaId = rs->getString("area_id");
		info.fromDatetime = rs->getString("from_datetime");
		info.toDatetime = rs->getString("to_datetime");
		//info.instrusionStatus = rs->getInt("instrusion_status");
		info.coordinates = rs->getString("coordinates");
		info.traffics = rs->getString("traffics");
		info.status = rs->getInt("status");

		infoList.push_back(info);
	}

	return infoList;
}


int DataAccessAdapter::updateAreaObjectMasters(std::vector<std::string>& areaObjectIds, int status) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);
	
	std::string str;
	for (auto& infoId : areaObjectIds) {
		if (str.length() > 0) {
			str += ",";
		}
		str += "?";
	}

	std::string sql = std::regex_replace(UPDATE_SQL, std::regex(":id_list:"), str);
	//std::cout << "update: areaObjectId=" << areaObjectId << ",status=" << status << std::endl;

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(sql));

	int ind = 1;
	pstmt->setInt(ind++, status);

	for (auto& infoId : areaObjectIds) {
		uint64_t val = std::stoull(infoId);

		pstmt->setUInt64(ind++, val);
	}

	int rc = pstmt->executeUpdate();
	//std::cout << "rc:" << rc << std::endl;

	return rc;
}

int DataAccessAdapter::insertAreaDetailObjects(AreaDetailObjectsInfo& rec) {
	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt(conUserDb->prepareStatement(INSERT_SQL));

	int ind = 1;
	pstmt->setUInt64(ind++, rec.areaObjectId);
	pstmt->setString(ind++, rec.spatialId);
	pstmt->setString(ind++, rec.voxelBitFilePath);
	pstmt->setInt(ind++, rec.voxelBitSpatialZoomLevel);
	pstmt->setInt(ind++, rec.pointCloudEpsg);
	pstmt->setString(ind++, rec.updateMemo);

	//std::cout << "++ ind:" << ind << ", clsPoints.size:" << rec.clsPoints.size() << std::endl;

	int rc = pstmt->executeUpdate();

	return rc;
}

int DataAccessAdapter::deleteAreaDetailObjects(uint64_t areaObjectId) {

	std::unique_ptr<sql::Statement> stmt(conUserDb->createStatement());
	stmt->execute("USE " + userDbName);

	std::unique_ptr<sql::PreparedStatement> pstmt;
	
	pstmt.reset(conUserDb->prepareStatement(DELETE_SQL));
	
	int ind = 1;
	pstmt->setUInt64(ind++, areaObjectId);
	int rc = pstmt->executeUpdate();

	return rc;
}
