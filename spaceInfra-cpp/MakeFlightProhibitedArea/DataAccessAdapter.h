#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"
#include "../../SpatialId/src/spatial_api.h"

#define FIXED_REC_ID	"MakeFlightProhibitedArea"
#define FIXED_DATA_UNIT	"sp17"
#define FIXED_DATA_IO	"FlightVoxel_Out"

struct FlightProhibitedAreaObjectMastersInfo {
	uint64_t flightProhibitedAreaObjectId = 0; //auto increment
	std::string flightProhibitedAreaId;
	std::string name;
	std::string fromDatetime;
	std::string toDatetime;
	std::string range;
	std::string detail;
	std::string url;
	int flightProhibitedAreaType;
	int status;
};

struct FlightProhibitedAreaObjectsInfo {
	uint64_t id;
	uint64_t flightProhibitedAreaObjectId;
	std::string spatialId;
	std::string voxelBitFilePath;
	int voxelBitSpatialZoomLevel;
	int pointCloudEpsg;
	//std::string updateMemo;
};



class DataAccessAdapter
{
	std::shared_ptr<spdlog::logger> logger;

	sql::mysql::MySQL_Driver* driver = nullptr;
	//std::unique_ptr<sql::Connection> con;
	std::unique_ptr<sql::Connection> conUserDb;
	//std::string dbName;
	std::string userDbName;

	DataAccessAdapter();
	virtual ~DataAccessAdapter();

	std::string SELECT_SQL = "SELECT * FROM flight_prohibited_area_object_masters "
		"WHERE flight_prohibited_area_object_id=?  ";

	std::string UPDATE_SQL = "UPDATE flight_prohibited_area_object_masters set status=?,updated_at = NOW() WHERE flight_prohibited_area_object_id IN (:id_list:) ";

	std::string DELETE_SQL = "UPDATE  flight_prohibited_area_objects set deleted_at=NOW()  "
		"WHERE flight_prohibited_area_object_id=? and (deleted_at is null || deleted_at = '') ";


	std::string INSERT_SQL = "INSERT INTO flight_prohibited_area_objects("
		"flight_prohibited_area_object_id,spatial_id,voxel_bit_file_path,voxel_bit_spatial_zoom_level,"
		"point_cloud_epsg,created_at,updated_at) "
		" VALUES(?, ?, ?, ?, ?, NOW(),NOW()) ";

public:
	DataAccessAdapter(DataAccessAdapter const&) = delete;
	DataAccessAdapter& operator=(DataAccessAdapter const&) = delete;

	//! @brief インスタンスを取得（singleton)
	//! 
	//! @return 取得できたインスタンスオブジェクト
	static DataAccessAdapter& getInstance() {
		static DataAccessAdapter _dataAccessAdapter;
		return _dataAccessAdapter;
	}
	void connect(Config& config);
	//void startTransaction();
	//void commit();
	//void rollback();

	int insertFlightProhibitedAreaObjects(FlightProhibitedAreaObjectsInfo& rec);
	int deleteFlightProhibitedAreaObjects(uint64_t flightProhibitedAreaObjectId);
	int updateFlightProhibitedAreaObjectMasters(std::vector<std::string>& flightProhibitedAreaObjectIds, int status);
	std::vector<FlightProhibitedAreaObjectMastersInfo> getFlightProhibitedAreaObjectMasters(uint64_t flightProhibitedAreaObjectId);
};

