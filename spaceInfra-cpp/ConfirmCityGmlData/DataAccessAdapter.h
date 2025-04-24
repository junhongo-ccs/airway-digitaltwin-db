#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"

struct SpaceObjectRec {
	std::string spatialId;
	std::string fromDatetime;
	std::string toDatetime;
	int objectCd;
	std::string voxelBitFilePath;
	int voxelBitSpatialZoomLevel;
	int pointCloudEpsg;
	int dataSourceId;
	std::string updateMemo;
	//double northLat;
	//double southLat;
	//double eastLon;
	//double westLon;
	//double lowerAltitude;
	//double upperAltitude;
	//std::string pointCloudFilePath;
	//std::vector<std::string> hashList;
};

//struct DetailObjectMasterRec {
//	uint64_t detailObjectId;
//	int objectCd;
//	std::string detailObjectCd;
//	std::string detailObjectName;
//	int dataSourceId=0;
//	int fileCnt=0;
//	double xmin=0;
//	double xmax=0;
//	double ymin=0;
//	double ymax=0;
//	double zmin=0;
//	double zmax=0;
//	std::string info;
//	std::string updateMemo;
//};
//
//struct SpatialDetailObjectRec {
//	uint64_t recId;
//	uint64_t detailObjectId;
//	std::string spatialId;//level:17
//	std::string voxelBitFileId;
//	std::string voxelBitFilePath;
//	int voxelBitSpatialZoomLevel;
//	std::string updateMemo;
//	int pointCloudEpsg;
//};

#define INFO_LENGTH	1000

class DataAccessAdapter
{
	std::shared_ptr<spdlog::logger> logger;

	sql::mysql::MySQL_Driver* driver = nullptr;
	std::unique_ptr<sql::Connection> con;
	std::unique_ptr<sql::Connection> conUserDb;
	std::string dbName;
	std::string userDbName;

	DataAccessAdapter();
	virtual ~DataAccessAdapter();

	std::string sqlSELECT = "SELECT object_cd FROM ground_feature_objects WHERE spatial_id = ? and deleted_at is NULL order by object_cd, created_at";

	std::string sqlUPDATE = "UPDATE ground_feature_objects SET deleted_at = NOW()  WHERE spatial_id = ? and deleted_at is NULL ";

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

	void createObjectIdMap(std::map<std::string, int>& featureId2ObjIdMap);

	int getDataSourceId(std::string& serverId, std::string& projectId);
	void updateSpaceDataManages(const std::string& ownerId,
		const std::string& batchId, const std::string& spatialId, std::string& recId);
	void insertSpaceObject(SpaceObjectRec& rec);
	//void updateMemo(SpaceObjectRec& rec);
	void deleteVoxel(std::string initFlg, std::string spatialId, std::string deleteDirFullPath);

};

