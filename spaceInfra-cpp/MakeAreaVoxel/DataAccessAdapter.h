#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"
#include "../../SpatialId/src/spatial_api.h"

#define FIXED_REC_ID	"MakeAreaVoxel"
#define FIXED_DATA_UNIT	"sp17"
#define FIXED_DATA_IO	"AreaVoxel_Out"

struct AreaObjectMastersInfo {
	uint64_t areaObjectId = 0; //auto increment
	std::string areaId;
	std::string fromDatetime;
	std::string toDatetime;
	//std::string areaName;
	int instrusionStatus;
	std::string coordinates;
	std::string traffics;
	int status;
};

struct AreaDetailObjectsInfo {
	uint64_t id;
	uint64_t areaObjectId;
	std::string spatialId;
	std::string voxelBitFilePath;
	int voxelBitSpatialZoomLevel;
	int pointCloudEpsg;
	std::string updateMemo;
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

	std::string SELECT_SQL = "SELECT * FROM area_object_masters "
		"WHERE area_object_id=?  ";

	std::string UPDATE_SQL = "UPDATE area_object_masters set status=?,updated_at=NOW() WHERE area_object_id IN (:id_list:)  ";

	std::string DELETE_SQL = "UPDATE  area_detail_objects set deleted_at=NOW()  "
		"WHERE area_object_id=? and (deleted_at is null || deleted_at = '') ";


	std::string INSERT_SQL = "INSERT INTO area_detail_objects("
		"area_object_id,spatial_id,voxel_bit_file_path,voxel_bit_spatial_zoom_level,"
		"point_cloud_epsg,update_memo,created_at,updated_at) "
		" VALUES(?, ?, ?, ?, ?, ?, NOW(),NOW()) ";

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

	int insertAreaDetailObjects(AreaDetailObjectsInfo& rec);
	int deleteAreaDetailObjects(uint64_t areaObjectId);
	int updateAreaObjectMasters(std::vector<std::string>& areaObjectIds, int status);
	std::vector<AreaObjectMastersInfo> getAreaObjectMasters(uint64_t areaObjectId);
};

