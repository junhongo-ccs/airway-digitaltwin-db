#pragma once

//! @file GeoUtil.h

#include "CommonDefs.h"
//#include <ogr_geometry.h>
//#include "GeoPolygon.h"
#include "Config.h"
//#include <proj.h>


//! @brief 地理関連ユーティリティクラス
class GeoUtil
{

	//static std::vector<int> getNearestMeshcds(int meshcd);
	//static int getLastDigit(int lat3, int lon3);
	//static std::array<int, 3> calcNewLatlon(int part1, int part2, int part3, bool plusFlag);
public:
	//static bool pointInPolygon(int px, int py, int numPolygonPoints, GeoPoint** ppPolygonPoints);

	//static bool pointInBBox(int px, int py, const BBox& bbox);

	//static bool bboxIntersected(const BBox& bbox1, const BBox& bbox2);

	//static bool bboxIntersected(int numPoints, const OGRRawPoint* pPoints, const BBox& bbox2, bool closed = true);
	//static bool bboxIntersected(int numPoints, const GeoPoint** ppPoints, const BBox& bbox2, bool closed=true);
	//static bool polygonIntersected(int nPointsOfPolygon, const GeoPoint** ppPointsOfPolygon, 
	//	int nPointsOfPolyline, const GeoPoint** ppPointsOfPolyline, bool closed = true);

	//static BBox getBBox(int numPoints, const OGRRawPoint* pPoints);

	//static BBox getBBox(int numPoints, const GeoPoint** ppPoints);

	//static BBox getBBox(std::string& meshCd);
	//static BBox getBBox(int meshCd);
	//static BBox getBBox(std::vector<GeoPoint>& points);

	//static std::string getSpatialIdFromFilename(const std::string& lasFileName);

	//static int calcZIndex(double z, int zoom);

	//static int getZenrinMapId(double lat, double lon, PJ* proj = nullptr);

	//static int calcScale(double refValue);

	//static int calcScale2(std::string& epsg);

	//static int getZenrinMapIdEx(double lat, double lon);

	//static void getAllCityGmlFileList(std::vector<std::string>& cityGmlFileList);

	//static void getCityGmlFileInfoFromFileName(const std::string& fileName, CityGmlFileInfo& cityGmlFileInfo);

	//static std::vector<std::string> getFileNameFromSpatialId(std::vector<std::string>& spatialIdList, bool checkExtraFlag=false);

	//static void prepareMapDefForFile(const std::string & lasFileName2, MapDef& mapDef, PJ* proj=nullptr);
};