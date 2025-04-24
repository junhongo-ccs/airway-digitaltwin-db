#pragma once


#include <vector>
#include <string>
#include <unordered_set>
#include <spdlog/spdlog.h>
#include "ObjectRefContainer.h"
#include "../../SpatialId/src/spatial_api.h"

class GeoPolygon;

class AreaObject
{
	std::shared_ptr<spdlog::logger> logger;

	std::string objectId;
	std::unordered_set<std::string> spatialIds;

	//Polygon
	ObjectRefContainer<GeoPolygon> polygonContainer; // { 10 };

	std::vector<spatialid::Triangle> triangles;
	std::string objectType;
	double centerLon = 0;
	double centerLat = 0;
	double altitude = 0;
	double radius = 0;

public:
	AreaObject();
	virtual ~AreaObject() {}

	//! @brief ポリゴン数を取得
	//! 
	//! @return 取得結果
	int getPolyCount() {
		return polygonContainer.getObjectCount();
	}

	//! @brief i番目(0ベース）のポリゴンデータを取得
	GeoPolygon* getPolygon(int i) {
		return polygonContainer[i];
	}

	//! @brief 全てのポリゴンデータにアクセスためのポインタを取得
	GeoPolygon** getPolygonRef() {
		return polygonContainer.getObjectRef();
	}


	//! @brief データセットにポリゴンを追加する
	//! 
	//! @param pPolygon 追加対象のポリゴン
	void addPolygon(GeoPolygon* pPolygon);

	std::string& getObjectId() {
		return objectId;
	}

	void setObjectId(std::string val) {
		objectId = val;
	}

	std::unordered_set<std::string>& getSpatialIds() {
		return spatialIds;
	}

	std::vector<spatialid::Triangle>& getTriangles() {
		return triangles;
	}

	std::string getObjectType() {
		return objectType;
	}
	void setObjectType(std::string& type) {
		objectType = type;
	}

	double getAltitude() {
		return altitude;
	}

	void setAltitude(double val) {
		altitude = val;
	}

	void getCircleParam(double& lon, double& lat, double& radius) {
		lon = centerLon;
		lat = centerLat;
		radius = this->radius;
	}

	void setCircleParam(double lon, double lat, double radius) {
		centerLon = lon;
		centerLat = lat;
		this->radius = radius;
	}

	
};

