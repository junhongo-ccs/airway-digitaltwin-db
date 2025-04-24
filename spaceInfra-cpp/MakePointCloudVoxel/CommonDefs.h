#pragma once


//! @file CommonDefs.h
//! @brief 共通の定義ファイル

#include <string>

//ポイント（ポリゴン)
struct GeoPoint {
	int x = 0; //経度方向
	int y = 0; //緯度方向
	int z = 0;
};

struct FileInfo {
	std::string fileName;
	double scaleX;
	double scaleY;
	double scaleZ;
	int epsg;
};

struct BBox {
	double xmin = DBL_MAX;
	double xmax = -DBL_MAX;
	double ymin = DBL_MAX;
	double ymax = -DBL_MAX;
	double zmin = DBL_MAX;
	double zmax = -DBL_MAX;
};