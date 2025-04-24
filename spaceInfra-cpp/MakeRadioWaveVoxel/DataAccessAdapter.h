#pragma once

#include <memory>
#include <mysql/jdbc.h>
#include "Config.h"
#include <spdlog/spdlog.h>

struct RadioWaveObjectRec {
	unsigned long long radioWaveObjectId; // 電波情報空間ID
	std::string spatialId;                // 空間ID（Lv17）
	std::string fromDatetime;             // 開始日時
	std::string toDatetime;               // 終了日時（nullable）
	std::string voxelBitFilePath;         // ボクセル地物ビット
	int voxelBitSpatialZoomLevel;         // ボクセル地物ローカル空間ズームレベル
	int pointCloudEpsg;                   // 空間座標系
	std::string updateMemo;               // 更新情報メモ（nullable）
	std::string createdAt;                // 登録日
	std::string updatedAt;                // 更新日
	std::string deletedAt;                // 削除日（nullable）
};

#define INFO_LENGTH	1000

class DataAccessAdapter
{
	std::shared_ptr<spdlog::logger> logger;

	sql::mysql::MySQL_Driver* driver = nullptr;
	std::unique_ptr<sql::Connection> con;
	std::unique_ptr<sql::Connection> conUserDb;
	std::string userDbName;

	DataAccessAdapter();
	virtual ~DataAccessAdapter();

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

	void insertRadioWaveObject(RadioWaveObjectRec& rec);
	void updateRadioWaveObject(RadioWaveObjectRec& rec);
	void upsertRadioWaveObject(const std::string& spatialId, const std::string& lasFilePath);
};

