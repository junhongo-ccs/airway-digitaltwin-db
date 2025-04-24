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
#include "CityObject.h"

//! マッチングデータを管理するためのクラス
class GeoDataset {
	//std::mutex _mutex;
	ObjectPool<GeoPoint> pointPool;
	ObjectPool<GeoPolygon> polygonPool;
	ObjectPool<GeoAttribute> attributePool;
	ObjectPool<CityObject> cityObjectPool;

	std::shared_ptr<spdlog::logger> logger;

	//CityObject
	ObjectRefContainer<CityObject> cityObjectContainer;

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

	CityObject* allocateCityObject(int num = 1) {
		return cityObjectPool.createObjectFromPool(num);
	}

	//! @brief Object数を取得
	//! 
	//! @return 取得結果
	int getCityObjectCount() {
		return cityObjectContainer.getObjectCount();
	}

	//! @brief i番目(0ベース）のオブジェクトデータを取得
	CityObject* getCityObject(int i) {
		return cityObjectContainer[i];
	}

	//! @brief 全てのオブジェクトデータにアクセスためのポインタを取得
	CityObject** getCityObjectRef() {
		return cityObjectContainer.getObjectRef();
	}


	//! @brief データセットにオブジェクトを追加する
	//! 
	//! @param pPolygon 追加対象のポリゴン
	void addCityObject(CityObject* cityObject) {
		cityObjectContainer.addObjects(1, cityObject);
	}


};

