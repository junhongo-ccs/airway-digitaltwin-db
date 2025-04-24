#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"
#include "../../SpatialId/src/spatial_api.h"

#define FIXED_REC_ID	"MakeCityGmlLas"
#define FIXED_DATA_UNIT	"sp17"
#define FIXED_DATA_IO	"MakeCityGml_Out"

struct SpaceDataManagesInfo {
	uint64_t uniqueId = 0; //auto increment
	std::string ownerId;
	std::string batchId;
	std::string spatialId;
	std::string recId; //MakeCityGmlLas
	std::string fileId;
	std::string filePath;
	std::string epsgLas;
	std::string epsgCsv;
	std::string dataUnit; //sp17
	std::string dataIo; //MakeCityGmlLas_Output
	std::string dataType; //BUILDING,ROAD,RAILWAY,WATER,GROUND 
	std::string dataStatus;
	std::string dataAbcd; //ABCD
	double spXmin;
	double spXmax;
	double spYmin;
	double spYmax;
	double spZmin;
	double spZmax;
	double spXminW; //ワールド座標
	double spXmaxW;
	double spYminW;
	double spYmaxW;
	std::vector<std::string> originalFileIds;
	int points = 0;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double zmin;
	double zmax;
	double xminW;
	double xmaxW;
	double yminW;
	double ymaxW;

	std::vector<int> clsPoints; //0-20, 99
	std::string comment;
};




class DataAccessAdapter
{
	std::shared_ptr<spdlog::logger> logger;

	sql::mysql::MySQL_Driver* driver = nullptr;
	std::unique_ptr<sql::Connection> con;
	//std::unique_ptr<sql::Connection> conUserDb;
	std::string dbName;
	//std::string userDbName;

	DataAccessAdapter();
	virtual ~DataAccessAdapter();

	std::string SELECT_SQL = "SELECT * FROM space_data_manages "
		"WHERE owner_id=? and batch_id=? and spatial_id=? and rec_id=? and data_type=? "
		"and (deleted_at is null or deleted_at = '') ";

	std::string DELETE_SQL = "DELETE FROM space_data_manages  "
		"WHERE owner_id=? and batch_id=? and spatial_id=? and rec_id=? and data_type=? ";

	std::string DELETE_SQL_WITHOUT_SPATIALID = "DELETE FROM space_data_manages "
		"WHERE owner_id=? and batch_id=? and rec_id=? and data_type=?  ";

	//std::string DELETE_SQL = "UPDATE space_data_manages set deleted_at=NOW() "
	//	"WHERE owner_id=? and batch_id=? and spatial_id=? and rec_id=? and data_type=? "
	//	"AND (deleted_at is null || deleted_at = '') ";

	//std::string DELETE_SQL_WITHOUT_SPATIALID = "UPDATE space_data_manages set deleted_at=NOW() "
	//	"WHERE owner_id=? and batch_id=? and rec_id=? and data_type=?  "
	//	"AND (deleted_at is null || deleted_at = '') ";

	std::string INSERT_SQL = "INSERT INTO space_data_manages("
		"owner_id,batch_id,spatial_id,rec_id,file_id,file_path,epsg_las,epsg_csv,"
		"data_unit,data_io,data_type,data_status,data_abcd,"
		"sp_xmin,sp_xmax,sp_ymin,sp_ymax,sp_zmin,sp_zmax,sp_xmin_w,sp_xmax_w,sp_ymin_w,sp_ymax_w,"
		"originalFileIds,points,xmin,xmax,ymin,ymax,zmin,zmax,xmin_w,xmax_w,ymin_w,ymax_w,"
		"cls0,cls1,cls2,cls3,cls4,cls5,cls6,cls7,cls8,cls9,cls10,"
		"cls11,cls12,cls13,cls14,cls15,cls16,cls17,cls18,cls19,cls20,cls99,created_at,updated_at)"
		" VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
		"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";

	std::string UPDATE_SQL = "UPDATE space_data_manages SET "
		//"file_id = ?,file_path = ?,epsg_las = ?,epsg_csv = ?,data_unit = ?, data_io = ?, data_type = ?,data_status = ?,"
		//"data_abcd = ?,sp_xmin = ?,sp_xmax = ?,sp_ymin = ?,sp_ymax = ?,sp_zmin = ?,sp_zmax = ?,sp_xmin_w = ?,sp_xmax_w = ?,sp_ymin_w = ?,sp_ymax_w = ?,"
		"originalFileIds = ?, points = ?,xmin = ?,xmax = ?,ymin = ?,ymax = ?,zmin = ?,zmax = ?,xmin_w = ?,xmax_w = ?,ymin_w = ?,ymax_w = ?,"
		"cls0 = ? ,cls1 = ? ,cls2 = ? ,cls3 = ? ,cls4 = ? ,cls5 = ? ,cls6 = ? ,cls7 = ? ,cls8 = ? ,cls9 = ? ,"
		"cls10 = ? ,cls11 = ?,cls12 = ?,cls13 = ?,cls14 = ?,cls15 = ?,cls16 = ?,cls17 = ?,cls18 = ?,cls19 = ?,"
		"cls20 = ?,cls99 = ?,updated_at  = NOW() "
		"WHERE id = ? ";
		//"WHERE owner_id = ? and batch_id = ? and spatial_id =? and rec_id =? and data_type=? ";


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

	int insertSpaceDataManages(SpaceDataManagesInfo& rec);
	int updateSpaceDataManages(SpaceDataManagesInfo& rec);
	SpaceDataManagesInfo getSpaceDataManages(std::string& ownerId, std::string& batchId, std::string& spatialId,
		std::string& recId, std::string& dataType);
	int registSpaceDataManages(SpaceDataManagesInfo& rec);
	int deleteSpaceDataManages(std::string& ownerId, std::string& batchId, std::string& spatialId,
		std::string& recId, std::string& dataType);
};

