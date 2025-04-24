#pragma once

//! @file GeoDataset.h
//! @brief マッチングデータ管理用

#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include "ObjectPool.h"
#include "GeoPolygon.h"
#include "ObjectRefContainer.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "AreaObject.h"

//! マッチングデータを管理するためのクラス
class GeoDataset {
	//std::mutex _mutex;
	ObjectPool<GeoPoint> pointPool;
	ObjectPool<GeoPolygon> polygonPool;
	ObjectPool<GeoAttribute> attributePool;
	ObjectPool<AreaObject> areaObjectPool;

	std::shared_ptr<spdlog::logger> logger;

	//AreaObject
	ObjectRefContainer<AreaObject> areaObjectContainer;

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

	//! @brief 属性データ用のメモリを確保（排他制御なし）
	//! 
	//! @param num ポイント数（デフォルト：１）
	//! @return 確保したメモリの先頭へのポインタ
	GeoAttribute* allocateAttribute(int num = 1) {
		return attributePool.createObjectFromPool(num);
	}

	//! @brief ポリゴンデータ用のメモリを確保（排他制御なし）
	//! 
	//! @num ポリゴン数（デフォルト：１）
	//! @return 確保したメモリの先頭へのポインタ
	GeoPolygon* allocatePolygon(int num = 1) {
		return polygonPool.createObjectFromPool(num);
	}

	AreaObject* allocateAreaObject(int num = 1) {
		return areaObjectPool.createObjectFromPool(num);
	}

	int getAreaObjectCount() {
		return areaObjectContainer.getObjectCount();
	}

	//! @brief i番目(0ベース）のオブジェクトデータを取得
	AreaObject* getAreaObject(int i) {
		return areaObjectContainer[i];
	}

	//! @brief 全てのオブジェクトデータにアクセスためのポインタを取得
	AreaObject** getAreaObjectRef() {
		return areaObjectContainer.getObjectRef();
	}


	//! @brief データセットにオブジェクトを追加する
	//! 
	//! @param pPolygon 追加対象のポリゴン
	void addAreaObject(AreaObject* pObject) {
		areaObjectContainer.addObjects(1, pObject);
	}


};

