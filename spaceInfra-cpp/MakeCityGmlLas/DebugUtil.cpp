#pragma once

#include "DebugUtil.h"
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include "liblas/liblas.hpp"
#include "CityObject.h"
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <processthreadsapi.h>
#endif


void DebugUtil::dumpPoints(int nPoints, const GeoPoint** ppPoints, bool toLonlat) {
    std::string coords;
    for (int i = 0; i < nPoints; i++) {
        if (i > 0) coords.append(",");
        if (toLonlat)
            coords.append("[").append(std::to_string(ppPoints[i]->x / (double)DMS_SCALE)).append(",").append(std::to_string(ppPoints[i]->y / (double)DMS_SCALE)).append("]");
        else
            coords.append("[").append(std::to_string(ppPoints[i]->x)).append(",").append(std::to_string(ppPoints[i]->y)).append("]");
    }

    std::cout << "subj = [" << coords << "]" << std::endl;
    
}

//void DebugUtil::dumpBbox(const BBox& bbox) {
//    std::string coords;
//    coords.append("[");
//    coords.append("[").append(std::to_string(bbox.x1)).append(",").append(std::to_string(bbox.y1)).append("]");
//    coords.append(",[").append(std::to_string(bbox.x1)).append(",").append(std::to_string(bbox.y2)).append("]");
//    coords.append(",[").append(std::to_string(bbox.x2)).append(",").append(std::to_string(bbox.y2)).append("]");
//    coords.append(",[").append(std::to_string(bbox.x2)).append(",").append(std::to_string(bbox.y1)).append("]");
//    coords.append(",[").append(std::to_string(bbox.x1)).append(",").append(std::to_string(bbox.y1)).append("]");
//    coords.append("]");
//
//    std::cout << "bbox = " << coords << std::endl;
//}

void DebugUtil::transform(int numPoints, double* pLon, double* pLat, double *pAlt, PJ* proj) {

    if (proj != nullptr) {
        PJ_COORD input_coords, output_coords;

        for (int i = 0; i < numPoints; i++) {
            input_coords = proj_coord(pLon[i], pLat[i], pAlt[i], 0);
            output_coords = proj_trans(proj, PJ_FWD, input_coords);
            pLon[i] = output_coords.xyz.x;
            pLat[i] = output_coords.xyz.y;
            pAlt[i] = output_coords.xyz.z;
        }
    }
}

void DebugUtil::getColorMap(std::vector<int>& colorMap, int colorNum) {
    int step = 360 / colorNum;
    int maxVal = 254; //V
    double minVal = maxVal - 127 / 255.0 * maxVal;//S=127
    int r = 0, g = 0, b = 0;

    //一部の色を固定
    int fixedColorNum = 7;
    //１番目のラベル：00ffff(cyan)
    //２番目のラベル：ffff00(yellow)
    //３番目のラベル：ff00ff(magenta)
    //４番目のラベル：ff0000(red)
    //５番目のラベル：00ff00(green)
    //６番目のラベル：0000ff(blue)
    //最後のラベル：black //ffa500(orange)
    std::vector colorInd(fixedColorNum, -1);
    enum ColorInd {
        cyan,
        yellow,
        magenta,
        red,
        green,
        blue,
        black,
    };
    int fixedColors[] = { 255 * 256 + 255, 255 * 65536 + 255 * 256, 255 * 65536 + 255,
        255 * 65536, 255 * 256, 255, 0 };


    if (step == 0) step = 1;
    int num = 0;
    for (int h = 0; h <= 360; h += step) {
        if (h >= 0 && h < 60) {
            r = maxVal;
            g = static_cast<int>(h / 60.0 * (maxVal - minVal) + minVal);
            b = static_cast<int>(minVal);
        }
        else if (h >= 60 && h < 120) {
            r = static_cast<int>((120.0 - h) / 60.0 * (maxVal - minVal) + minVal);
            g = maxVal;
            b = static_cast<int>(minVal);
        }
        else if (h >= 120 && h < 180) {
            r = static_cast<int>(minVal);
            g = maxVal;
            b = static_cast<int>((h - 120.0) / 60.0 * (maxVal - minVal) + minVal);
        }
        else if (h >= 180 && h < 240) {
            r = static_cast<int>(minVal);
            g = static_cast<int>((240.0 - h) / 60.0 * (maxVal - minVal) + minVal);
            b = maxVal;
        }
        else if (h >= 240 && h < 300) {
            r = static_cast<int>((h - 240.0) / 60.0 * (maxVal - minVal) + minVal);
            g = static_cast<int>(minVal);
            b = maxVal;
        }
        else {
            r = maxVal;
            g = static_cast<int>(minVal);
            b = static_cast<int>((360.0 - h) / 60.0 * (maxVal - minVal) + minVal);
        }

        colorMap.push_back(r * 65536 + g * 256 + b);

        //固定色なら位置を覚えておく
        if (r == 0 && g == 0 && b == 0) {
            colorInd[static_cast<int>(ColorInd::black)] = num;
        }
        else if (r == 0 && g == 255 && b == 255) {
            colorInd[static_cast<int>(ColorInd::cyan)] = num;
        }
        else if (r == 255 && g == 255 && b == 0) {
            colorInd[static_cast<int>(ColorInd::yellow)] = num;
        }
        else if (r == 255 && g == 0 && b == 255) {
            colorInd[static_cast<int>(ColorInd::magenta)] = num;
        }
        else if (r == 255 && g == 0 && b == 0) {
            colorInd[static_cast<int>(ColorInd::red)] = num;
        }
        else if (r == 0 && g == 255 && b == 0) {
            colorInd[static_cast<int>(ColorInd::green)] = num;
        }
        else if (r == 0 && g == 0 && blue == 255) {
            colorInd[static_cast<int>(ColorInd::green)] = num;
        }

        num++;
    }

    for (int i = 0; i < fixedColorNum - 1; i++) {
        if (i >= colorNum - 1) break;
        if (colorInd[i] >= 0) {
            colorMap[colorInd[i]] = colorMap[i];
        }
        colorMap[i] = fixedColors[i];
    }
    if (colorInd[fixedColorNum - 1] >= 0) {
        //std::cout << "## ind:" << colorInd[fixedColorNum - 1] << ", color:" << colorMap[colorInd[fixedColorNum - 1]] << std::endl;
        colorMap[colorInd[fixedColorNum - 1]] = colorMap[colorNum - 1];
    }

    colorMap[colorNum - 1] = fixedColors[fixedColorNum - 1];

    //std::cout << "last:" << colorMap[colorNum - 1] << ", colorNum:" << colorNum << std::endl;
}


void DebugUtil::toLas(std::string& outputFileName, GeoDataset& gds, 
        std::vector<CityObject*>& objList, ComLib::Voxel& voxel) {
    clock_t t1 = clock();

    if (objList.size() <= 0) {
        std::cout << "出力対象のオブジェクトがありません" << std::endl;
        return;
    }

    std::cout << "--- outputFile:" << outputFileName << ",size:" << objList.size() << std::endl;

    //カラーを用意
    std::vector<int> colorMap;
    getColorMap(colorMap, (int)objList.size() + 1); // (labelCount < 8 ? 8 : labelCount));

    //std::cout << "colorMap creatd: size=" << colorMap.size() << std::endl;

    PJ_CONTEXT* projContext = proj_context_create();
    int tokyoCrs = 6677;
    std::string toCrs = "EPSG:" + std::to_string(tokyoCrs);//テストでは東京に固定
    std::string fromCrs = "EPSG:6697";
    PJ* projMap2Las = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
    if (projMap2Las == nullptr) {
        throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
    }



    std::ofstream ofs;
    ofs.open(outputFileName, std::ios::out | std::ios::binary);
    liblas::Header header;
    header.SetDataFormatId(liblas::ePointFormat3); // Time only

    // Set coordinate system using GDAL support
    liblas::SpatialReference srs;
    //std::string epsg("EPSG:6676");
    //srs.SetFromUserInput(epsg.c_str());

    header.SetSRS(srs);
    header.SetScale(0.001, 0.001, 0.001);

    liblas::Writer writer(ofs, header);


    int outputPointCount = 0;
    double minX = DBL_MAX, minY = DBL_MAX, minZ = DBL_MAX;
    double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

    //---- objectを書き出す
    int objCount = 0;
    for (auto const pCityObject : objList) {
        std::vector<spatialid::Triangle>& triangles = pCityObject->getTriangles();
        //std::cout << "+++ triangles:" << triangles.size() << std::endl;

        for (auto const& triangle : triangles) {
            double x[3], y[3], z[3];
            for (int i = 0; i < 3; i++) {
                x[i] = triangle.points[i].x;
                y[i] = triangle.points[i].y;
                z[i] = triangle.points[i].z;
            }
            transform(3, x, y, z, projMap2Las);

            for (int i = 0; i < 3; i++) {
                if (x[i] < minX) minX = x[i];
                if (x[i] > maxX) maxX = x[i];
                if (y[i] < minY) minY = y[i];
                if (y[i] > minY) maxY = y[i];
                if (z[i] < minZ) minZ = z[i];
                if (z[i] > minZ) maxZ = z[i];

                liblas::Point point(&header);

                //std::cout << "triangle:" << x[i] << "," << y[i] << "," << z[i] << std::endl;

                point.SetCoordinates(x[i], y[i], z[i]);

                int label = objCount % (colorMap.size() - 1);
                int intColor = colorMap[label];
                int red = (intColor >> 16) << 8;
                int green = (intColor & 0xff00);
                int blue = (intColor & 0xff) << 8;
                liblas::Color color(red, green, blue);

                point.SetColor(color);

                //TODO
                //point.SetClassification(6);

                //writer.WritePoint(point);
                //outputPointCount++;

            }
        }

        objCount++;
    }

    //ビットが立っているローカル空間の中心を書き出す]
    int altIndexSize = voxel.getAltIndexSize();
    int latIndexSize = voxel.getLatIndexSize();
    int lonIndexSize = voxel.getLonIndexSize();
    int localZoom = voxel.getLocalZoom();
    spatialid::Zfxy& baseZfxy = voxel.getBaseLocalZfxy();
    for (int i = 0; i < altIndexSize; i++) {
        if (!voxel.testPlane(i)) {
            continue;
        }
        for (int j = 0; j < latIndexSize; j++) {
            for (int k = 0; k < lonIndexSize; k++) {
                if (voxel.testBit(k, j, i)) {
                    spatialid::Zfxy zfxy(localZoom, i+baseZfxy.alt_index, k+baseZfxy.lon_index, j+baseZfxy.lat_index);
                    std::string localSpatialId = zfxy.getString();

                    //std::cout << "+++ localId:" << localSpatialId << std::endl;

                    spatialid::Point pt = spatialid::get_center_point_on_spatial_id(localSpatialId,
                        static_cast<spatialid::CRS>(tokyoCrs));
                    double x = pt.x, y = pt.y, z = pt.z;

                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > minY) maxY = y;
                    if (z < minZ) minZ = z;
                    if (z > minZ) maxZ = z;

                    liblas::Point point(&header);

                    //std::cout << "center:" << x << "," << y << "," << z << std::endl;
                    point.SetCoordinates(x, y, z);

                    int label = (int)colorMap.size() - 1;
                    int intColor = colorMap[label];
                    int red = (intColor >> 16) << 8;
                    int green = (intColor & 0xff00);
                    int blue = (intColor & 0xff) << 8;
                    liblas::Color color(red, green, blue);

                    point.SetColor(color);

                    //TODO
                    //point.SetClassification(6);

                    writer.WritePoint(point);
                    outputPointCount++;


                }
            }
        }
    }

    //std::cout << minX << "," << minY << "," << minZ << std::endl;
    //std::cout << maxX << "," << maxY << "," << maxZ << std::endl;
    header.SetMin(minX, minY, minZ);
    header.SetMax(maxX, maxY, maxZ);
    header.SetPointRecordsCount(outputPointCount);
    header.SetPointRecordsByReturnCount(0, outputPointCount);
    writer.SetHeader(header);
    writer.WriteHeader();

    ofs.close();


    std::cout << "outputPointCount:" << outputPointCount << std::endl;

    proj_context_destroy(projContext);

}

void DebugUtil::toGeoJSONBySpatialId(std::string& outputFileName, GeoDataset& gds, 
        std::vector<CityObject*>& objList, ComLib::Voxel& voxel) {
    clock_t t1 = clock();

    Config& config = Config::getInstance();

    std::cout << "<<<<< toGeoJSONBySpatialId(debug用) >>>>>" << std::endl;

    ////pointCsvEpst → mapEpsg
    //PJ_CONTEXT* projContext = proj_context_create();
    //if (projContext == nullptr) {
    //    throw std::runtime_error("PROJコンテキストが作成できません");
    //}

    //std::string toCrs = "EPSG:" + config.getMapEpsg();
    //std::string fromCrs = "EPSG:" + config.getPointCsvEpsg();
    //PJ* projCsv2Map = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
    //if (projCsv2Map == nullptr) {
    //    throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
    //}

    //preprocess
    //preprocessLasPointBeforeOutput(gds, mapDef, config);
    //logger->trace("+++ 出力前の前処理(座標変換など）:{0}", (double)(clock() - t1) / CLOCKS_PER_SEC);

    //get a registered driver
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GeoJSON");
    if (poDriver == nullptr)
    {
        throw std::runtime_error("driver not found");
    }

    //create datasource
    //日本語名が含まれている場合SET GDAL_FILENAME_IS_UTF8=NO
    //std::string baseOutFileName = std::regex_replace(mapDef.spatialId, std::regex("/"), "-");
    //std::string outputFileName = config.getOutputFileName(baseOutFileName);

    //logger->trace("-- outputFile:{0}", outputFileName.c_str());
    std::cout << "--++ outputFile:" << outputFileName << std::endl;

    GDALDataset* poDS = poDriver->Create(outputFileName.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    if (poDS == nullptr) {
        throw std::runtime_error("failed to create GDALDataset");
    }

    //create layer
    OGRLayer* poLayer = poDS->CreateLayer("geometry", nullptr, wkbLineString, nullptr);
    if (poLayer == nullptr) {
        throw std::runtime_error("failed to create OGRLayer");
    }

    //create fielddef
    std::vector<std::string> fieldList;
    fieldList.push_back("stroke");
    //fieldList.push_back("elcode");
    //fieldList.push_back("meshcd");
    //fieldList.push_back("strokeColor");
    //fieldList.push_back("objectid");

    for (auto& fieldName : fieldList) {
        //--color
        OGRFieldDefn oField(fieldName.c_str(), OFTString);

        //oField.SetWidth(32);

        //std::cout << "fieldName:" << fieldName << std::endl;

        if (poLayer->CreateField(&oField) != OGRERR_NONE) {
            throw std::runtime_error("failed to create [color] field");
        }
    }

    //-- create features
    //---- objectを書き出す
    int objCount = 0;
    for (auto const pCityObject : objList) {
        //polygon
        int numPolygon = pCityObject->getPolyCount();
        GeoPolygon** ppPolygon = pCityObject->getPolygonRef();

        //三角形
        std::vector<spatialid::Triangle>& triangles = pCityObject->getTriangles();
        for (auto const& triangle : triangles) {
            GeoPoint points[3];
            for (int i = 0; i < 3; i++) {
                points[i].x = triangle.points[i].x;
                points[i].y = triangle.points[i].y;
            }


            std::string color = "#808080";
            createOneFeatureForPolyline(poLayer, 3, (GeoPoint*)points, color);

        }


        //Polygon
        for (int k = 0; k < numPolygon; k++) {

            GeoPolygon* pPolygon = pCityObject->getPolygon(k);

            int nPoints = pPolygon->getNumOfPoints();
            if (nPoints == 0) {
                continue;
            }

            GeoPoint** ppPoints = pPolygon->getPointsRef();
            std::string color = "#ff0000";

            createOneFeatureForPolyline(poLayer, nPoints, ppPoints, color);
        }


        objCount++;
    }

    //ビットが立っているローカル空間の枠を書き出す]
    //まずメモリにあるビット配列がgzファイルと同じかを確認
    std::string spatialId = voxel.getSpatialId();
    std::string voxelFilePath = voxel.getVoxelFilePath();
    ComLib::Voxel gzVoxel(spatialId, voxel.getLocalZoom(), config.getPointOutLasEpsg(),
        voxelFilePath, config.getLoggerName());
    std::map<int, std::unique_ptr<unsigned char[]>>& voxelData = voxel.getVoxelData();
    std::map<int, std::unique_ptr<unsigned char[]>>& gzVoxelData = gzVoxel.getVoxelData();
    bool matched = true;
    if (voxelData.size() != gzVoxelData.size()) {
        std::cout << "### voxelData.size is unmatch" << std::endl;
        matched = false;
    }
    else {
        for (auto iter = voxelData.begin(); iter != voxelData.end(); iter++) {
            auto ind = iter->first;
            if (std::memcmp(voxelData[ind].get(),gzVoxelData[ind].get(),voxel.getPlaneSize()) != 0) {
                std::cout << "### unmatched bit " << std::endl;
                matched = false;
                break;
            }
        }
    }

    if (matched) {
        std::cout << "##### all bits matched ####" << std::endl;
    }


    int altIndexSize = voxel.getAltIndexSize();
    int latIndexSize = voxel.getLatIndexSize();
    int lonIndexSize = voxel.getLonIndexSize();
    int localZoom = voxel.getLocalZoom();
    spatialid::Zfxy& baseZfxy = voxel.getBaseLocalZfxy();
    for (int i = 0; i < altIndexSize; i++) {
        if (!voxel.testPlane(i)) {
            continue;
        }
        for (int j = 0; j < latIndexSize; j++) {
            for (int k = 0; k < lonIndexSize; k++) {
                if (voxel.testBit(k, j, i)) {
                    spatialid::Zfxy zfxy(localZoom, i + baseZfxy.alt_index, k + baseZfxy.lon_index, j + baseZfxy.lat_index);
                    std::string localSpatialId = zfxy.getString();

                    std::vector<spatialid::Point> pointList;
                    get_vertex_points_on_spatial_id(localSpatialId, pointList, static_cast<spatialid::CRS>(6668));
                    
                    GeoPoint points[5];
                    for (int p = 0; p < 4; p++) {
                        points[p].x = pointList[p].x;
                        points[p].y = pointList[p].y;
                    }
                    points[4].x = points[0].x;
                    points[4].y = points[0].y;

                    std::string color = "#808000";
                    createOneFeatureForPolyline(poLayer, 5, (GeoPoint*)points, color);

                }
            }
        }
    }







    ////polygon(EPSG:6668)
    //int numPolygon = gds.getPolyCount();
    //GeoPolygon** ppPolygon = gds.getPolygonRef();
    //std::vector<int> meshIdList;

    //int roadCount = 0; //for debug
    //std::set<std::string> processedObjectIds;
    //std::vector<GeoPolyline*> matchedList;

    //for (int k = 0; k < numPolygon; k++) {

    //    //出力ポリゴンを限定する
    //    std::vector<int>& targetPolygons = config.getTargetPolygonIds();
    //    if (targetPolygons.size() > 0 && std::find(targetPolygons.begin(), targetPolygons.end(), k) == targetPolygons.end()) {
    //        continue;
    //    }

    //    GeoPolygon* pPolygon = gds.getPolygon(k);
    //    PolygonType type = pPolygon->getPolygonType();


    //    int nPoints = pPolygon->getNumOfExPoints();
    //    if (nPoints == 0) {
    //        continue;
    //    }

    //    GeoPoint** ppPoints = pPolygon->getExPointsRef();

    //    std::string color;
    //    if (type == TYPE_ROAD) {
    //        color = "#00ff00";
    //    }
    //    else if (type == TYPE_WATER) {
    //        color = "#0000ff";
    //    }
    //    else if (type == TYPE_RAILWAY) {
    //        color = "#ff0000";
    //    }
    //    else if (type == TYPE_POWERTOWER) {
    //        color = "stroke", "#ff00cc";
    //    }
    //    else if (type == TYPE_POWERLINE) {
    //        color = "stroke", "#8000ff";
    //    }
    //    else {
    //        color = "#ff00ff";
    //    }

    //    createOneFeatureForPolyline(poLayer, nPoints, ppPoints, mapDef.xyScale, color,
    //        projCsv2Map, meshIdList);
    //    //-----------------------------------
    //    //std::cout << "polygonId:" << pPolygon->getPolygonId() << ",robjectid:" << pPolygon->getRobjectId() << std::endl;
    //    if (type == TYPE_ROAD) {//&& pPolygon->getRobjectId().size() > 0) {//道路の場合リンクも描画
    //        std::cout << "polygonId:" << k << ",meshcode:" << pPolygon->getAttribute("meshcode")->value <<
    //            ",objectid:" << pPolygon->getAttribute("objectid")->value <<
    //            ",robjectid:" << pPolygon->getAttribute("robjectid")->value <<
    //            ",numPoints:" << pPolygon->getNumOfPoints() <<
    //            ",numLinks:" << pPolygon->getRoadLinkList().size() << ",validLinkNum:" << pPolygon->getValidRoadLinkNum() << std::endl;

    //        RoadHeightManager& roadHeightManager = gds.getRoadHeightManager();
    //        std::map<std::string, std::vector<GeoPolyline*>>& polylineMap = roadHeightManager.getPolylineMap();
    //        std::map<std::string, RoadHeightData*>& heightMap = roadHeightManager.getHeightMap();

    //        std::vector<GeoPolyline*> targetList = pPolygon->getRoadLinkList();
    //        for (auto& pPolyline : targetList) {
    //            if (pPolyline->getAttribute("objectid")->value == pPolygon->getRobjectId()) {
    //                matchedList.push_back(pPolyline);
    //            }
    //        }
    //        //targetList.insert(targetList.end(), matchedList.begin(), matchedList.end());

    //        //道路リンク
    //        //int lineCount = 0;

    //        for (GeoPolyline* pPolyline : targetList) {
    //            bool matched = std::find(matchedList.begin(), matchedList.end(), pPolyline) != matchedList.end();

    //            std::cout << "meshcode:" << pPolyline->getAttribute("meshcode")->value <<
    //                ",linkno:" << pPolyline->getAttribute("linkno")->value <<
    //                ",objectid:" << pPolyline->getAttribute("objectid")->value <<
    //                ",numPoints:" << pPolyline->getNumOfPoints() << std::endl;


    //            OGRFeature* poFeature2 = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
    //            if (poFeature2 == nullptr) {
    //                throw std::runtime_error("failed to create OGRFeature");
    //            }

    //            //std::cout << "path1:" << std::endl;
    //            std::string linkNo = pPolyline->getAttribute("linkno")->value;

    //            if (heightMap.find(linkNo) != heightMap.end()) {
    //                if (matched /* && pPolyline->getAttribute("objectid")->value == pPolygon->getRobjectId()*/) {
    //                    poFeature2->SetField("stroke", "#fe0000");
    //                    processedObjectIds.insert(pPolyline->getAttribute("objectid")->value);
    //                }
    //                else {
    //                    if (processedObjectIds.find(pPolygon->getAttribute("objectid")->value) != processedObjectIds.end()) {
    //                        poFeature2->SetField("stroke", "#fe0000");
    //                    }
    //                    else {
    //                        poFeature2->SetField("stroke", "#0000ff");
    //                    }
    //                }
    //            }
    //            else {
    //                poFeature2->SetField("stroke", "#400000");
    //            }
    //            int num = pPolyline->getNumOfPoints();
    //            GeoPoint** ppTemp = pPolyline->getPointsRef();

    //            //std::cout << "path2: num = " << num << std::endl;
    //            //point
    //            OGRLineString lineString;
    //            OGRPoint ogrPoint;
    //            for (int m = 0; m < num; m++) {
    //                ogrPoint.setX(ppTemp[m]->x / (double)DMS_SCALE);
    //                ogrPoint.setY(ppTemp[m]->y / (double)DMS_SCALE);
    //                lineString.addPoint(&ogrPoint);
    //            }

    //            //std::cout << "path2-2: " << std::endl;
    //            poFeature2->SetGeometry(&lineString);

    //            //std::cout << "path3:" << std::endl;
    //            //--- add feature to layer
    //            if (poLayer->CreateFeature(poFeature2) != OGRERR_NONE) {
    //                OGRFeature::DestroyFeature(poFeature2);
    //                throw std::runtime_error("Featureの追加に失敗しました");
    //            }

    //            OGRFeature::DestroyFeature(poFeature2);
    //        }

    //        //std::cout << "==== lineCount:" << lineCount << std::endl;

    //        //height Data
    //        for (GeoPolyline* pPolyline : targetList) {
    //            //std::cout << "++++ getLinkNo:" << pPolyline->getLinkNo() << std::endl;

    //            RoadHeightData* pHeightData = heightMap.at(pPolyline->getLinkNo());

    //            if (config.getTargetRoadLinkNo().size() > 0 && pPolyline->getLinkNo() != config.getTargetRoadLinkNo()) {
    //                continue;
    //            }

    //            //標高データ関連のポリゴン
    //            //if (config.getOutputSubPolygon()) {
    //            int kk = 0;
    //            if (config.getOutputSubPolygon()) {
    //                for (GeoPolygon* pPolygon : pHeightData->subPolygonList) {
    //                    //std::cout << "##linkNo:" << pPolyline->getLinkNo() << ",slope:" <<
    //                    //    pHeightData->pointHeightList[kk].slope << ",curvature:" <<
    //                    //    pHeightData->pointHeightList[kk].curvature <<
    //                    //    ",accrucy:" << pHeightData->accuracy << std::endl;
    //                    kk++;

    //                    int nPoints = pPolygon->getNumOfExPoints();
    //                    if (nPoints == 0) {
    //                        continue;
    //                    }

    //                    GeoPoint** ppPoints = pPolygon->getExPointsRef();

    //                    std::string useColor = "#ff00cc";
    //                    createOneFeatureForPolyline(poLayer, nPoints, ppPoints, mapDef.xyScale, useColor, projCsv2Map, meshIdList);
    //                }
    //            }
    //            else {
    //                GeoPolygon* pPolygon = pHeightData->pPolygon;
    //                if (pPolygon != nullptr) {
    //                    //std::cout << "+++ polygon:" << std::hex << (int)pPolygon << ",pHeightData:" << std::hex << (int)pHeightData << std::endl;

    //                    int nPoints = pPolygon->getNumOfExPoints();
    //                    if (nPoints > 0) {

    //                        GeoPoint** ppPoints = pPolygon->getExPointsRef();

    //                        std::string useColor = "#ff00cc";
    //                        createOneFeatureForPolyline(poLayer, nPoints, ppPoints, mapDef.xyScale, useColor, projCsv2Map, meshIdList);
    //                    }
    //                }
    //                else {
    //                    std::cout << "no polygon" << std::endl;
    //                }
    //            }
    //        }
    //    }

    //}


    ////------------------------ サブメッシュ枠
    //for (auto& meshcd : meshIdList) {
    //    //BBox bboxDummy;

    //    //std::cout << "meshcd:" << meshcd << std::endl;

    //    BBox bbox = GeoUtil::getBBox(meshcd);

    //    std::cout << "sub mesh:" << meshcd << ", xmin = " << bbox.x1 << ", ymin:" << bbox.y1 <<
    //        ",xmax:" << bbox.x2 << ",ymax:" << bbox.y2 << std::endl;

    //    std::string color("#ffff00");
    //    createOneFeatureForBbox(poLayer, bbox, DMS_SCALE, color, nullptr);

    //}

    ////------------------------ メッシュ枠
    //std::vector<int> secondMeshList;
    ////std::cout << "++++ meshIdList:" << meshIdList.size() << std::endl;
    //for (auto& meshcd : meshIdList) {
    //    //std::cout << "meshcd:" << meshcd << std::endl;
    //    int cd = meshcd / 10;
    //    if (std::find(secondMeshList.begin(), secondMeshList.end(), cd) == secondMeshList.end()) {
    //        secondMeshList.push_back(cd);
    //    }
    //}
    //for (auto& meshcd : secondMeshList) {
    //    //BBox bboxDummy;

    //    BBox bbox = GeoUtil::getBBox(meshcd);

    //    std::cout << "mesh:" << meshcd << ", xmin = " << bbox.x1 << ", ymin:" << bbox.y1 <<
    //        ",xmax:" << bbox.x2 << ",ymax:" << bbox.y2 << std::endl;


    //    std::string color("#00ffff");
    //    createOneFeatureForBbox(poLayer, bbox, DMS_SCALE, color, nullptr);

    //}


    //{//空間IDの枠
    //    MapDef localMapDef;

    //    GeoUtil::prepareMapDefForFile(inLasFile, localMapDef);

    //    std::string color("#cc0000");
    //    createOneFeatureForBbox(poLayer, localMapDef.bbox, localMapDef.xyScale, color, projCsv2Map);
    //}

    //proj_context_destroy(projContext);

    GDALClose(poDS);
}


void DebugUtil::createOneFeatureForPolyline(OGRLayer* poLayer, int nPoints, GeoPoint** ppPoints, std::string& color) {
    if (nPoints == 0) {
        return;
    }

    //create feature
    OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
    if (poFeature == nullptr) {
        throw std::runtime_error("failed to create OGRFeature");
    }


    poFeature->SetField("stroke", color.c_str());
    //point
    { 
        //point
        OGRLineString lineString;
        OGRPoint ogrPoint;
        for (int i = 0; i < nPoints; i++) {
            ogrPoint.setX(ppPoints[i]->x);
            ogrPoint.setY(ppPoints[i]->y);
            lineString.addPoint(&ogrPoint);

            ////ついでにmeshcdを計算
            //int meshcd = GeoUtil::getZenrinMapIdEx(pLat[i], pLon[i]);
            //if (std::find(meshIdList.begin(), meshIdList.end(), meshcd) == meshIdList.end()) {
            //    meshIdList.push_back(meshcd);
            //}
        }

        poFeature->SetGeometry(&lineString);

        //--- add feature to layer
        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            OGRFeature::DestroyFeature(poFeature);
            throw std::runtime_error("Featureの追加に失敗しました");
        }
    }

    OGRFeature::DestroyFeature(poFeature);

}

void DebugUtil::createOneFeatureForPolyline(OGRLayer* poLayer, int nPoints, GeoPoint* pPoints, std::string& color) {
    if (nPoints == 0) {
        return;
    }

    //create feature
    OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
    if (poFeature == nullptr) {
        throw std::runtime_error("failed to create OGRFeature");
    }


    poFeature->SetField("stroke", color.c_str());
    //point
    {
        //point
        OGRLineString lineString;
        OGRPoint ogrPoint;
        for (int i = 0; i < nPoints; i++) {
            ogrPoint.setX(pPoints[i].x);
            ogrPoint.setY(pPoints[i].y);
            lineString.addPoint(&ogrPoint);

            ////ついでにmeshcdを計算
            //int meshcd = GeoUtil::getZenrinMapIdEx(pLat[i], pLon[i]);
            //if (std::find(meshIdList.begin(), meshIdList.end(), meshcd) == meshIdList.end()) {
            //    meshIdList.push_back(meshcd);
            //}
        }

        poFeature->SetGeometry(&lineString);

        //--- add feature to layer
        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            OGRFeature::DestroyFeature(poFeature);
            throw std::runtime_error("Featureの追加に失敗しました");
        }
    }

    OGRFeature::DestroyFeature(poFeature);

}

#if 0
void DebugUtil::createOneFeatureForBbox(OGRLayer* poLayer, BBox bbox, double xyScale, std::string& color, PJ* proj) {
    //create feature
    OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
    if (poFeature == nullptr) {
        throw std::runtime_error("failed to create OGRFeature");
    }
    poFeature->SetField("stroke", color.c_str());
    //poFeature->SetField("strokeColor", color.c_str());

    int nPoints = 5;
    double* pLon = new double[nPoints];
    double* pLat = new double[nPoints];

    //pLon[0] = bbox.x1 / xyScale;
    //pLat[0] = bbox.y1 / xyScale;
    //pLon[1] = bbox.x2 / xyScale;
    //pLat[1] = bbox.y1 / xyScale;
    //pLon[2] = bbox.x2 / xyScale;
    //pLat[2] = bbox.y2 / xyScale;
    //pLon[3] = bbox.x1 / xyScale;
    //pLat[3] = bbox.y2 / xyScale;
    //pLon[4] = bbox.x1 / xyScale;
    //pLat[4] = bbox.y1 / xyScale;

    //if (proj != nullptr) {
    //    transform(nPoints, pLon, pLat, proj);
    //}


    //point
    OGRLineString lineString;

    for (int i = 0; i < nPoints; i++) {
        OGRPoint ogrPoint;
        ogrPoint.setX(pLon[i]);
        ogrPoint.setY(pLat[i]);
        lineString.addPoint(&ogrPoint);
    }

    poFeature->SetGeometry(&lineString);

    //--- add feature to layer
    if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
        OGRFeature::DestroyFeature(poFeature);
        throw std::runtime_error("Featureの追加に失敗しました");
    }

    OGRFeature::DestroyFeature(poFeature);
    delete[] pLat;
    delete[] pLon;
}
#endif

////! @brief マッチング結果をCSVに書き出す(緯度経度のまま）
////! 
////! @param gds データセット
////! @param mapDef 地図定義情報
////! @param config 設定情報
//int DebugUtil::toCSV(GeoDataset& gds, const MapDef& mapDef, Config& config) {
//    std::shared_ptr<spdlog::logger> logger = spdlog::get(config.getLoggerName());
//    clock_t t1 = clock();
//
//    preprocessLasPointBeforeOutput(gds, mapDef, config);
//    logger->trace("+++ 出力前の前処理(座標変換など）:{0}", (double)(clock() - t1) / CLOCKS_PER_SEC);
//
//    //z-indexを計算
//    int numOfPolygons = gds.getPolyCount();
//    GeoPolygon** ppPolygon = gds.getPolygonRef();
//
//    std::string baseOutFileName = std::regex_replace(mapDef.spatialId, std::regex("/"), "-");
//    std::string outputFileName = config.getOutputFileName(baseOutFileName, TYPE_BUILDING);
//
//    logger->debug("-- outputFile:{0}", outputFileName.c_str());
//
//    CsvWriter writer(outputFileName);
//
//
////    if (config.needOutputCsvHeader()) {
//        writer.writeRow("X,Y,Z,RGB Color(R),RGB Color(G),RGB Color(B),オブジェクトID,地図種別\n");
////    }
//
//    int outputCount = 0;
//    for (int k = 0; k < numOfPolygons; k++) {
//        GeoPolygon* pPolygon = ppPolygon[k];
//
//        if (pPolygon->isOutputProcessed()) {
//            continue;
//        }
//
//        GeoAttribute* pElcode = pPolygon->getAttribute("elcode");
//        GeoAttribute* pObjectid = pPolygon->getAttribute("objectid");
//        if (pElcode == nullptr || pObjectid == nullptr) {
//            throw std::runtime_error(CommonUtil::format("---- cannot get attribute: elcode={0:p}, objectid=[:p]", (void*)pElcode, (void*)pObjectid));
//        }
//
//
//        //point
//        int nPoints = pPolygon->getNumOfLasPoints();
//
//        //---- PolygonのlasPointsをOGRPointにした場合再修正
//        GeoPoint** ppLasPoint = pPolygon->getLasPointsRef();
//        for (int i = 0; i < nPoints; i++) {
//            writer.writeRow("%.10lf,%.10lf,%.3lf,%d,%d,%d,%s,%s\n",
//                (double)ppLasPoint[i]->projX,
//                (double)ppLasPoint[i]->projY,
//                (double)ppLasPoint[i]->z / Z_SCALE,
//                ppLasPoint[i]->red, ppLasPoint[i]->green, ppLasPoint[i]->blue,
//                pObjectid->value.c_str(), pElcode->value.c_str());
//            outputCount++;
//        }
//
//    }
//
//    writer.close();
//
//
//    clock_t t2 = clock();
//    logger->trace("+++ toCsv:{:.3f}\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
//
//    return outputCount;
//}
//
//

#ifdef _WIN32
void DebugUtil::printMemoryInfo() {
    DWORD processID = GetCurrentProcessId();
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);
    if (NULL == hProcess)
        return;

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        DWORD memory = pmc.WorkingSetSize/1024;
        DWORD peak = pmc.PeakWorkingSetSize / 1024;
        printf("memory usage: current=%.2lf(MB), peak:=%.2lf(MB)\n", memory / 1024.0, peak / 1024.0);

        //printf("\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount);
        //printf("\tPeakWorkingSetSize: 0x%08X\n",
        //    pmc.PeakWorkingSetSize);
        //printf("\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize);
        //printf("\tQuotaPeakPagedPoolUsage: 0x%08X\n",
        //    pmc.QuotaPeakPagedPoolUsage);
        //printf("\tQuotaPagedPoolUsage: 0x%08X\n",
        //    pmc.QuotaPagedPoolUsage);
        //printf("\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
        //    pmc.QuotaPeakNonPagedPoolUsage);
        //printf("\tQuotaNonPagedPoolUsage: 0x%08X\n",
        //    pmc.QuotaNonPagedPoolUsage);
        //printf("\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage);
        //printf("\tPeakPagefileUsage: 0x%08X\n",
        //    pmc.PeakPagefileUsage);
    }

    CloseHandle(hProcess);

}
#endif

void DebugUtil::dumpCityModel(std::shared_ptr<const citygml::CityModel>& cityModel,
    std::shared_ptr<spdlog::logger>& logger) {
    const auto& rootCityObjects = cityModel->getRootCityObjects();

    logger->info("root count: {0}", rootCityObjects.size());
    for (auto const& rootCityObject : rootCityObjects) {
        logger->info("------ rootCityObject START ------");

        CityObject* pCityObject = nullptr;

        int geometryCount = rootCityObject->getGeometriesCount();

        logger->info("root-objId:{0},geometryCount:{1}, objType:{2}, childCount:{3}",
            rootCityObject->getId(), geometryCount,
            rootCityObject->getTypeAsString(), rootCityObject->getChildCityObjectsCount());


        for (int kk = 0; kk < rootCityObject->getChildCityObjectsCount(); kk++) {
            const auto& child = rootCityObject->getChildCityObject(kk);
            dumpCityObject(child, logger);
        }


        for (int i = 0; i < geometryCount; i++) {
            auto const& geometry = rootCityObject->getGeometry(i);
            dumpGeometry(geometry, logger);
        }

        logger->info("------ rootCityObject END ------");
    }

}

void DebugUtil::dumpCityObject(const citygml::CityObject& cityObject, std::shared_ptr<spdlog::logger>& logger) {
    logger->info("------ dumpCityObject START ------");
    logger->info("++child-id:{0}, type:{1}, geometryCount:{2}, grandChild:{3}",
        cityObject.getId(), cityObject.getTypeAsString(), cityObject.getGeometriesCount(),
        cityObject.getChildCityObjectsCount());

    for (int i = 0; i < cityObject.getChildCityObjectsCount(); i++) {
        const auto& subObj = cityObject.getChildCityObject(i);
        dumpCityObject(subObj, logger);
    }

    for (int ii = 0; ii < cityObject.getGeometriesCount(); ii++) {
        const auto& geometry = cityObject.getGeometry(ii);
        dumpGeometry(geometry, logger);
    }

    logger->info("------ dumpCityObject END ------");
}

void DebugUtil::dumpGeometry(const citygml::Geometry& geometry, std::shared_ptr<spdlog::logger>& logger) {
    logger->info("------ dumpGeometry START ------");

    logger->info("# lod:{0}, polygonCount:{1}, lineCount:{2}, geometryCount:{3}, type;{4}",
        geometry.getLOD(),
        geometry.getPolygonsCount(), geometry.getLineStringCount(), geometry.getGeometriesCount(),
        geometry.getTypeAsString());

    for (int i = 0; i < geometry.getGeometriesCount(); i++) {
        const auto& sub = geometry.getGeometry(i);
        dumpGeometry(sub, logger);
    }

    for (int i = 0; i < geometry.getPolygonsCount(); i++) {
        const auto& polygon = geometry.getPolygon(i);
        logger->info("polygon: pointCount={0}", polygon->getVertices().size());
    }

    for (int i = 0; i < geometry.getLineStringCount(); i++) {
        const auto& lineString = geometry.getLineString(i);
        logger->info("lineString: pointCount:{0},{1}", lineString->getVertices2D().size(), lineString->getVertices3D().size());
    }

    logger->info("------ dumpGeometry END ------");

}
