#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <fstream>
#include <time.h>
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include <gdal_priv.h>
#include <proj.h>
#include <spdlog/spdlog.h>
#include <liblas/liblas.hpp>
#include "GeoDataset.h"
#include "CommonUtil.h"
#include "GeoUtil.h"
#include "../../SpatialId\src\\spatial_api.h"
//#include "CsvWriter.h"
#include "../ComLib/Voxel.h"
#include <citygml/citygml.h>
#include <citygml/citymodel.h>
#include <citygml/cityobject.h>
#include <citygml/geometry.h>
#include <citygml/polygon.h>
#include <citygml/linestring.h>
#include <citygml/vecs.hpp>


class Voxel;
class CityObject;

class DebugUtil {
    PJ* projMap2Las = nullptr;

public:
    static void dumpPoints(int nPoints, const GeoPoint** ppPoints, bool toLonlat = false);

    //static void dumpBbox(const BBox& bbox);

    static void transform(int numPoints, double* pLon, double* pLat, double *pAlt, PJ* proj);

    static void getColorMap(std::vector<int>& colorMap, int colorNum);

    static void toLas(std::string& outputFileName, GeoDataset& gds,
        std::vector<CityObject*>& objList, ComLib::Voxel& voxel);

    static void toGeoJSONBySpatialId(std::string& outputFileName, GeoDataset& gds,
        std::vector<CityObject*>& objList, ComLib::Voxel& voxel);
    //static void createOneFeatureForBbox(OGRLayer* poLayer, BBox bbox, double xyScale, std::string& color, PJ* proj);
    static void createOneFeatureForPolyline(OGRLayer* poLayer, int numPoints, GeoPoint** ppPoints, std::string& color);

    static void createOneFeatureForPolyline(OGRLayer* poLayer, int numPoints, GeoPoint* pPoints, std::string& color);
    static void dumpCityModel(std::shared_ptr<const citygml::CityModel>& cityModel, 
        std::shared_ptr<spdlog::logger>& logger);
    static void dumpCityObject(const citygml::CityObject& cityObject,std::shared_ptr<spdlog::logger>& logger);
    static void dumpGeometry(const citygml::Geometry& geometry, std::shared_ptr<spdlog::logger>& logger);
        
#ifdef _WIN32
    static void printMemoryInfo();
#endif
    //! @brief マッチング結果をCSVに書き出す(緯度経度のまま）
    //! 
    //! @param gds データセット
    //! @param mapDef 地図定義情報
    //! @param config 設定情報
    //int toCSV(GeoDataset& gds, const MapDef& mapDef, Config& config);

};