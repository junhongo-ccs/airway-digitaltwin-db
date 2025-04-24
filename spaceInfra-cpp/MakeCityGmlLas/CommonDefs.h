#pragma once


//! @file CommonDefs.h
//! @brief 共通の定義ファイル

#include <string>

#define DMS_SCALE 3600000

#define Z_SCALE 1000.0
#define W_SCALE 100.0

struct CityGmlFileInfo {
	std::string meshcd;
	std::string crs;
	std::string featureId;
	std::string dataType;
	int targetLod = 1;
	std::string originalFileId;
};



//ポイント（ポリゴン)
struct GeoPoint {
	double x = 0; //経度方向
	double y = 0; //緯度方向
	double z = 0;
	double orgX = 0.0; //経度方向
	double orgY = 0.0; //緯度方向
};


//属性
struct GeoAttribute {
	std::string name;
	std::string value;
};

enum PolygonType {
	TYPE_BUILDING=1,
	TYPE_ROAD,
	TYPE_WATER,
	TYPE_RAILWAY,
	TYPE_GROUND,
	TYPE_POWERLINE,
	TYPE_STEELTOWER
};



