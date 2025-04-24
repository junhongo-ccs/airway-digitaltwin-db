#pragma once

//! @file GeoDataset.h
//! @brief マッチングデータ管理用

#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include "ObjectPool.h"
#include "ObjectRefContainer.h"
#include "spdlog/spdlog.h"
#include "Config.h"

//! マッチングデータを管理するためのクラス
class GeoDataset {
	//std::mutex _mutex;
	ObjectPool<GeoPoint> pointPool;
	ObjectRefContainer<GeoPoint> pointContainer;

	std::shared_ptr<spdlog::logger> logger;

public:
	//! @brief コンストラクタ
	GeoDataset() {
		Config& config = Config::getInstance();
		logger = spdlog::get(config.getLoggerName());
	}

	//! @brief デストラクタ
	virtual ~GeoDataset() {
		//logger->trace("--- in ~GeoDataset");
	}

	//! ポイント用のメモリを確保（排他制御なし)
	//! 
	//! @param num ポイント数（デフォルト：１）
	//! @return 確保したメモリの先頭へのポインタ
	GeoPoint* allocatePoint(int num = 1) {
		return pointPool.createObjectFromPool(num);
	}

	void addPoint(int num, GeoPoint* pPoint) {
		pointContainer.addObjects(num, pPoint);
	}

	int getPointCount() {
		return pointContainer.getObjectCount();
	}

	GeoPoint** getPointRef() {
		return  pointContainer.getObjectRef();
	}

	GeoPoint* getPoint(int i) {
		return pointContainer.getObject(i);
	}
};

