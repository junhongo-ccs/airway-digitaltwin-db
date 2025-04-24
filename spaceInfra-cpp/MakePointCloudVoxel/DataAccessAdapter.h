#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"


struct PointCloudFeatureObjectsRec {
	uint64_t pointCloudFeatureObjectId;
	std::string spatialId;
	std::string fromDatetime;
	std::string toDatetime;
	std::string voxelBitFilePath;
	int voxelBitSpatialZoomLevel;
	int pointCloudEpsg;
	int dataSourceId;
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

	std::string DELETE_SQL = "UPDATE point_cloud_feature_objects  "
		"SET deleted_at=NOW() WHERE spatial_id=? AND (deleted_at is NULL OR deleted_at='') ";

	std::string UPDATE_SQL = "UPDATE  point_cloud_feature_objects set update_memo=?, updated_at=NOW() "
		"WHERE point_cloud_feature_object_id=? ";

	std::string INSERT_SQL = "INSERT INTO point_cloud_feature_objects("
		"spatial_id,from_datetime,to_datetime,voxel_bit_file_path,voxel_bit_spatial_zoom_level,point_cloud_epsg,"
		"data_source_id,update_memo,created_at,updated_at) "
		" VALUES(?, ?, :to_datetime:, ?, ?, ?,"
		"?, ?, NOW(), NOW())";

	std::string SELECT_SQL = "SELECT point_cloud_feature_object_id, spatial_id, from_datetime, to_datetime,"
		"voxel_bit_file_path, voxel_bit_spatial_zoom_level, point_cloud_epsg, data_source_id, update_memo "
		"FROM point_cloud_feature_objects WHERE spatial_id=? AND (deleted_at is  NULL OR deleted_at = '') ";

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
	
	int getDataSourceId(std::string& serverId, std::string& projectId);
	int insertPointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec);
	int updatePointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec);
	int registPointCloudFeatureObjects(PointCloudFeatureObjectsRec& rec);
	int deletePointCloudFeatureObjects(std::string& spatialId);
	PointCloudFeatureObjectsRec getPointCloudFeatureObjects(std::string& spatialId);
	void deleteVoxel(std::string init_flg, std::string spatial_id, std::string dirPath);

};

