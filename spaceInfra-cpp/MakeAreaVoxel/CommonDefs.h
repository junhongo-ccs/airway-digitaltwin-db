#pragma once


//! @file CommonDefs.h
//! @brief 共通の定義ファイル

#include <string>

#define STATUS_PROCESSING	2
#define STATUS_OK	9
#define STATUS_NG	99



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



