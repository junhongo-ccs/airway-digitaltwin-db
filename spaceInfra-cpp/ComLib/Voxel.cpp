#include "Voxel.h"
#include <vector>
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include "../../SpatialId/src/spatial_api.h"
//#include "../../SpatialId/src/common/spatial_point.h"
//#include "../../SpatialId/src/common/spatial_polygons.h"
#include "CommonUtil.h"
//#include "Config.h"
#include <liblas/liblas.hpp>
#include <laszip/laszip_api.h>

namespace ComLib {

    // コンストラクタ
    Voxel::Voxel(const std::string& spatialId, int localZoom, int lasEpsg, std::string& voxelFilePath, 
        const std::string& loggerName, bool needCheckInner)
        : spatialId(spatialId), localZoom(localZoom),lasEpsg(lasEpsg), voxelFilePath(voxelFilePath),
        needCheckInner(needCheckInner) {

        logger = spdlog::get(loggerName);

        if (lasEpsg == 3857) {
            defaultLasScale = 0.01;
        }

        spatialid::Zfxy zfxy(spatialId);
        globalZoom = zfxy.zoom;
        //localZoom = spatialid::kLocalZoom;

        int diffZoom = localZoom - globalZoom;
        int num = 1 << diffZoom;
        int baseF = (zfxy.alt_index < 0 ? zfxy.alt_index * num : zfxy.alt_index << diffZoom);
        int baseX = zfxy.lon_index << diffZoom;
        int baseY = zfxy.lat_index << diffZoom;
        baseLocalZfxy = spatialid::Zfxy(localZoom, baseF, baseX, baseY);

        int newF = baseF + num;
        int newX = baseX + num;
        int newY = baseY + num;
        spatialid::Zfxy southEastTop(localZoom, newF, newX, newY);

        ////空間IDの各頂点の座標を取得
        //std::vector<spatialid::Point> vertex;
        //spatialid::get_vertex_points_on_spatial_id(spatialId, vertex);

        //// 北西下をローカル空間ID
        //spatialid::Zfxy northWestBottom = spatialid::get_spatial_zfxy_on_point(vertex[0].x, vertex[0].y,
        //    vertex[0].z, localZoom);
        //baseLocalZfxy = northWestBottom;//基準になるzfxy

        //// 南東上のローカル空間ID
        //spatialid::Zfxy southEastTop(spatialid::get_spatial_zfxy_on_point(
        //    vertex[6].x, vertex[6].y, vertex[6].z, localZoom, false));

        // ローカル空間ID内の経度方向、緯度方向、高さの数
        lonIndexSize = southEastTop.lon_index - baseLocalZfxy.lon_index;
        latIndexSize = southEastTop.lat_index - baseLocalZfxy.lat_index;
        altIndexSize = southEastTop.alt_index - baseLocalZfxy.alt_index;

        // 経度方向は 1ID を 1bit、8ID を 1byte で表現 ※APIに合わせて最小64ビット
        int widthUnits = lonIndexSize / spatialid::kBoundaryBits;
        if ((lonIndexSize % spatialid::kBoundaryBits) > 0) {
            widthUnits++;
        }
        lonIndexByte = widthUnits * (spatialid::kBoundaryBits / 8);

        // 1平面（経度x緯度）のボクセルデータサイズ[bytes]
        planeSize = lonIndexByte * latIndexSize;
        //std::cout << "### planeSize:" << planeSize << std::endl;

        //std::cout << "voxelFilePath:" << voxelFilePath << std::endl;

        //ファイルをロードする
        if (!voxelFilePath.empty()) {
            //std::string fileName = spatialId + "_" + featureId + "." + config.getInputVoxelExtension();
            //std::replace(fileName.begin(), fileName.end(), '/', '-');
            //std::string gzFileName = CommonUtil::pathJoin(config.getInputFolder(), fileName);

            //std::cout << gzFileName << std::endl;
            if (std::filesystem::exists(voxelFilePath)) {
                loadExistedData();
            }
        }


        //LOG_DEBUG(
        //    "lon_index_size=%d (lon_bytes=%d) lat_index_size=%d alt_index_size=%d "
        //    "spatial_id=%s plane_size=%d",
        //    lon_index_size_, lon_index_byte_, lat_index_size_, alt_index_size_,
        //    spatial_id.c_str(), plane_size_);
    }

    void Voxel::loadExistedData() {
        addFile(voxelFilePath);
    }

    void Voxel::addFile(std::string& filePath) {
        //ファイルをロードする
        if (CommonUtil::endsWith(filePath, ".gz")) {//今のところ利用されていない
            //std::string fileName = spatialId + "_" + featureId + "." + config.getInputVoxelExtension();
            //std::replace(fileName.begin(), fileName.end(), '/', '-');
            //std::string gzFileName = CommonUtil::pathJoin(config.getInputFolder(), fileName);

            //std::cout << gzFileName << std::endl;
            if (std::filesystem::exists(filePath)) {
                //std::cout << "exists:" << filePath << std::endl;

                //voxelDataのない場合0を書き出すためのバッファ
                std::unique_ptr<unsigned char[]> zeroBuffer = std::make_unique<unsigned char[]>(planeSize);
                std::memset(zeroBuffer.get(), 0, planeSize);

                gzFile fp = gzopen(filePath.c_str(), "rb");
                if (fp == nullptr) {
                    logger->debug("filePath:{0}", filePath);
                    throw std::runtime_error("gzファイルがオープンできません");
                }

                for (int i = 0; i < altIndexSize; i++) {
                    unsigned char* pBuf = new unsigned char[planeSize]();
                    int ret = gzread(fp, pBuf, planeSize);
                    if (ret == 0 && gzeof(fp)) {
                        //ファイルの終わりに達している
                        delete[] pBuf;
                        break;
                    }
                    else if (ret != planeSize) {
                        gzclose(fp);
                        throw std::runtime_error("gzファイルサイズが正しくありません");
                    }

                    //全て０ならデータを捨てる
                    if (std::memcmp(zeroBuffer.get(), pBuf, planeSize) == 0) {
                        delete[] pBuf;
                        continue;
                    }

                    //全て０でなければ、保存しておく
                    if (voxelData.find(i) == voxelData.end()) {
                        voxelData[i].reset(pBuf);
                    }
                    else {
                        //既存とマージ
                        unsigned char* plane = voxelData[i].get();
                        for (int k = 0; k < planeSize; k++) {
                            plane[k] |= pBuf[k];
                        }

                        delete[] pBuf;
                    }
                }

                gzclose(fp);
            }
        }
        else if (CommonUtil::endsWith(filePath, ".las") || CommonUtil::endsWith(filePath, ".laz")) {
            bool lazMode = CommonUtil::endsWith(filePath, ".laz");

            //spatialid::Zfxy baseZfxy = baseLocalZfxy;

            //create reader
            laszip_POINTER laszip_reader;
            if (laszip_create(&laszip_reader)) {
                throw std::runtime_error("failed to create laszip reader");
            }

            //open reader
            laszip_BOOL is_compressed = 0;
            if (laszip_open_reader(laszip_reader, filePath.c_str(), &is_compressed))
            {
                throw std::runtime_error("failed to open laszip reader(" + filePath + ")");
            }
            //std::cout << "is_compressed:" << is_compressed << std::endl;

            //get a ponter to the header
            laszip_header* header;

            if (laszip_get_header_pointer(laszip_reader, &header))
            {
                throw std::runtime_error("failed to get the header pointer from laszip reader");
            }

            // how many points does the file have
            laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

            //logger->trace("polygon:{0}, points in las: {1}", polygonInd, npoints);

            // get a pointer to the points that will be read
            laszip_point* point;

            if (laszip_get_point_pointer(laszip_reader, &point))
            {
                throw std::runtime_error("failed to get point pointer from laszip reader");
            }

            double scaleX = header->x_scale_factor;
            double scaleY = header->y_scale_factor;
            double scaleZ = header->z_scale_factor;

            // read the points
            laszip_I64 p_count = 0;
            while (p_count < npoints)
            {
                // read a point
                if (laszip_read_point(laszip_reader))
                {
                    throw std::runtime_error("failed to read point");
                }

                double x = point->X * scaleX;
                double y = point->Y * scaleY;
                double z = point->Z * scaleZ;
                
                //std::cout << "x:" << x << ",y:" << y << ",z:" << z << ", epsg:" << lasEpsg << std::endl;

                //空間IDを取得
                std::string curSpatialId = spatialid::get_spatial_id_on_point(x, y, z, localZoom,
                    static_cast<spatialid::CRS>(lasEpsg));

                //std::cout << "curSpatialId:" << curSpatialId << std::endl;
                
                int curZoom = 0, curAltIndex = 0, curLonIndex = 0, curLatIndex = 0;
                CommonUtil::spatialIdToIndex(curSpatialId.c_str(), curZoom, curAltIndex, curLonIndex, curLatIndex);

                if (curLonIndex < baseLocalZfxy.lon_index || curLatIndex < baseLocalZfxy.lat_index) {
                    logger->debug("x:{0}, y:{1}, z:{2}, epsg:{3}, curId:{4}, base:{5}", 
                        x, y, z, lasEpsg, curSpatialId, baseLocalZfxy.getString());
                    //throw std::runtime_error("negative lon_index or lat_index");
                    p_count++;
                    continue;
                }

                setBit(curAltIndex - baseLocalZfxy.alt_index, 
                    curLonIndex - baseLocalZfxy.lon_index,
                    curLatIndex - baseLocalZfxy.lat_index);


                p_count++;
            }

            // close the reader
            if (laszip_close_reader(laszip_reader))
            {
                throw std::runtime_error("failed to close laszip reader");
            }

            // destroy the reader

            if (laszip_destroy(laszip_reader))
            {
                throw std::runtime_error("failed to destroy laszip reader");
            }
        }
    }

    void ComLib::Voxel::addPolygon(std::vector<spatialid::Triangle>& triangles, spatialid::CRS crs) {
        std::vector<spatialid::Triangle> empty;

        // ローカル空間IDの精度レベルで表現した場合の空間IDを取得
        std::unordered_set<std::string> localZoomSpatialIds;

        clock_t t1 = clock();
        bool needsClosedChecking = false; //apiの閉塞チェックに問題がありそうなので、とりあえずfalseに固定
        spatialid::get_spatial_ids_on_polygons(triangles, empty,
            localZoomSpatialIds, localZoom, crs,
            needsClosedChecking, false);

        logger->trace("++ localZoomSpatialIds.size:{0}, localZoom:{1}, time:{2:.2f}", localZoomSpatialIds.size(),
            localZoom, (double)(clock() - t1) / CLOCKS_PER_SEC);

        addLocalZoomSpatialIds(localZoomSpatialIds);
    }

    void ComLib::Voxel::addLocalZoomSpatialIds(std::unordered_set<std::string>& localZoomSpatialIds) {
        clock_t t1 = clock();

        spatialid::Zfxy globalZfxy(spatialId);

        //std::cout << "++ voxel: alt=" << globalZfxy.alt_index << std::endl;

        // ローカル空間IDのビットを立てる
        for (auto& localZoomSpatialId : localZoomSpatialIds) {
            int localAltIndex, localLonIndex, localLatIndex, dummyZoom;
            CommonUtil::spatialIdToIndex(localZoomSpatialId.c_str(), dummyZoom, localAltIndex, localLonIndex, localLatIndex);

            //グローバルな空間ID
            int diffZoom = localZoom - globalZoom;
            int num = 1 << diffZoom;
            int chkAltIndex = (localAltIndex < 0 ? localAltIndex / num : localAltIndex >> diffZoom);
            int chkLongIndex = localLonIndex >> diffZoom;
            int chkLatIndex = localLatIndex >> diffZoom;


            //処理対象外のグローバル空間IDなら無視
            if (chkAltIndex != globalZfxy.alt_index || chkLongIndex != globalZfxy.lon_index ||
                chkLatIndex != globalZfxy.lat_index) {
                continue;
            }

            //std::cout << chkAltIndex << "/" << chkLongIndex << "/" << chkLatIndex << std::endl;

            //ビットバッファに設定する
            int lonIndex = localLonIndex - baseLocalZfxy.lon_index;
            int latIndex = localLatIndex - baseLocalZfxy.lat_index;
            int altIndex = localAltIndex - baseLocalZfxy.alt_index;

            //std::cout << "altIndex:" << altIndex << std::endl;

            setBit(altIndex, lonIndex, latIndex);
        }

        //std::cout << "### check-1:" << (double)(clock() - t1) / CLOCKS_PER_SEC << std::endl;

        //--------- 内部をチェック
        if (needCheckInner) {
            checkInnerVoxel(localZoomSpatialIds);
        }
    }


    void ComLib::Voxel::checkInnerVoxel(const std::unordered_set<std::string>& spatialIdList) {

        int altMaxIndex = std::numeric_limits<int>::min();
        for (const auto& spatialId : spatialIdList) {
            spatialid::Zfxy zfxy(spatialId);
            altMaxIndex = std::max(altMaxIndex, zfxy.alt_index);
        }

        //std::cout << "++ spatialIdList:" << spatialIdList.size() << std::endl;

        for (auto const& localSpatialId : spatialIdList) {
            //---------------- 処理対象を絞る ----------
            // 中心座標を取得
            spatialid::Point point =
                spatialid::get_center_point_on_spatial_id(localSpatialId, spatialid::CRS::kWGS84);

            // 中心座標のグローバル空間IDを取得
            std::string tmpSpatialId =
                spatialid::get_spatial_id_on_point(point.x, point.y, point.z,
                    globalZoom, spatialid::CRS::kWGS84);

            spatialid::Zfxy tmpGlobalZfxy(tmpSpatialId);
            spatialid::Zfxy globalZfxy(spatialId);
            //std::cout << "+++ localZoomSpatialId:" << localZoomSpatialId << ",tmpSpatialId:" << tmpSpatialId << std::endl;

            //処理中の空間IDと同じ経度緯度でなければスキップ
            if (tmpGlobalZfxy.lon_index != globalZfxy.lon_index || tmpGlobalZfxy.lat_index != globalZfxy.lat_index) {
                continue;
            }

            //Z方向に一つずつ積み上げていく
            spatialid::Zfxy zfxy(localSpatialId);
            int newF = zfxy.alt_index + 1;

            //次の候補ID
            std::string checkSpatialId = spatialid::Zfxy(localZoom, newF, zfxy.lon_index, zfxy.lat_index).getString();

            // すでに囲まれている面にある場合無視
            if (spatialIdList.find(checkSpatialId) != spatialIdList.end()) {
                continue;
            }

            // 表面にないボクセルからスタートして表面に衝突するためのものを内部のボクセルとする
            std::unordered_set<std::string> checkSpatialIds;

            while (true) {
                // 空間IDの高さ成分を取得
                spatialid::Zfxy checkZfxy(checkSpatialId);

                if (checkZfxy.alt_index > altMaxIndex) {
                    //表面に衝突しないままで最大高さに達した場合、外部のものとする
                    checkSpatialIds.clear();
                    break;
                }
                else if (spatialIdList.find(checkSpatialId) != spatialIdList.end()) {
                    // 表目に衝突した場合ビットを立てる
                    for (auto const& tempId : checkSpatialIds) {
                        // 中心座標を取得
                        spatialid::Point point =
                            spatialid::get_center_point_on_spatial_id(tempId, spatialid::CRS::kWGS84);

                        // 中心座標のグローバル空間IDを取得
                        std::string tmpSpatialId =
                            spatialid::get_spatial_id_on_point(point.x, point.y, point.z,
                                globalZoom, spatialid::CRS::kWGS84);

                        //std::cout << "+++ localZoomSpatialId:" << localZoomSpatialId << ",tmpSpatialId:" << tmpSpatialId << std::endl;

                        //処理中の空間IDでなければスキップ
                        if (spatialId != tmpSpatialId) {
                            continue;
                        }

                        spatialid::Zfxy localZfxy(tempId);
                        int lonIndex = localZfxy.lon_index - baseLocalZfxy.lon_index;
                        int latIndex = localZfxy.lat_index - baseLocalZfxy.lat_index;
                        int altIndex = localZfxy.alt_index - baseLocalZfxy.alt_index;

                        setBit(altIndex, lonIndex, latIndex);

                    }

                    checkSpatialIds.clear();
                    break;
                }

                //判定済みリストに追加
                checkSpatialIds.insert(checkSpatialId);

                //次の候補
                checkSpatialId = spatialid::Zfxy(localZoom, checkZfxy.alt_index + 1,
                    checkZfxy.lon_index, checkZfxy.lat_index).getString();
            }
        }
    }



    void ComLib::Voxel::toFile(double lasScale, int color) {
        logger->trace("+++toFile: voxelFilePath={0}", voxelFilePath);

        if (!CommonUtil::endsWith(voxelFilePath, ".gz")) {
            toLas(lasScale, color);
            return;
        }
        //出力ファイル
        //std::string fileName = spatialId + "_" + featureId + ".gz";
        //std::replace(fileName.begin(), fileName.end(), '/', '-');

        //std::string outFileName = CommonUtil::pathJoin(outputDir, fileName);

        std::string outFileName = voxelFilePath;
        if (outFileName.empty()) {
            throw std::runtime_error("ボクセルファイルパスが指定されていません");
        }

        //voxelDataのない場合0を書き出すためのバッファ
        std::unique_ptr<unsigned char[]> zeroBuffer = std::make_unique<unsigned char[]>(planeSize);
        std::memset(zeroBuffer.get(), 0, planeSize);

        //zip圧縮用のバッファ
        //const int BUFFER_SIZE = 128000;
        //std::unique_ptr<unsigned char[]> zipBuf = std::make_unique<unsigned char[]>(BUFFER_SIZE);

        //z_stream zStream{ 0 };
        //auto ret = deflateInit(&zStream, Z_DEFAULT_COMPRESSION);

        gzFile fp = gzopen(outFileName.c_str(), "w6");
        if (fp == nullptr) {
            logger->debug("outFileName:{0}", outFileName);
            throw std::runtime_error("gz出力ファイルがオープンできません");
        }

        for (int i = 0; i < altIndexSize; i++) {
            if (voxelData.find(i) != voxelData.end()) {
                gzwrite(fp, voxelData[i].get(), planeSize);

            }
            else {
                gzwrite(fp, zeroBuffer.get(), planeSize);
            }
        }

        gzclose(fp);

    }

    void ComLib::Voxel::toLas(/*std::string& outputLazFileName, int epsg,*/ double scale, int color) {
        if (!CommonUtil::endsWith(voxelFilePath, ".las") && !CommonUtil::endsWith(voxelFilePath, ".laz")) {
            throw std::runtime_error("unsupported file format");
        }

        bool lazMode = CommonUtil::endsWith(voxelFilePath, ".laz");

        std::string outputLasFileName = voxelFilePath;
        if (lazMode) {
            outputLasFileName = std::regex_replace(outputLasFileName, std::regex("\\.laz"), ".las");
        }
        //clock_t t1 = clock();

        spatialid::Zfxy baseZfxy = baseLocalZfxy;


        std::ofstream ofs;
        ofs.open(outputLasFileName, std::ios::out | std::ios::binary);
        liblas::Header header;
        header.SetDataFormatId(liblas::ePointFormat3); // Time only

        // Set coordinate system using GDAL support
        liblas::SpatialReference srs;
        //std::string epsg("EPSG:6676");
        //srs.SetFromUserInput(epsg.c_str());

        header.SetSRS(srs);
        double lasScaleX = scale;
        double lasScaleY = scale;
        double lasScaleZ = scaleZ;
        logger->trace("----- lasScaleXY:{0}, lasScaleZ:{1}, color:{2:x}, lasEpsg:{3}", 
            lasScaleX, lasScaleZ, color, lasEpsg);
        header.SetScale(lasScaleX, lasScaleY, lasScaleZ);

        liblas::Writer writer(ofs, header);

        int outputPointCount = 0;
        double minX = DBL_MAX, minY = DBL_MAX, minZ = DBL_MAX;
        double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

        //std::cout << "altIndex:" << altIndexSize << ",lonIndex:" << lonIndexSize << ",latIndex:" << latIndexSize << std::endl;
        //std::cout << "voxelData.size:" << voxelData.size() << std::endl;
        //std::cout << "lasEpsg:" << lasEpsg << std::endl;

        //ビットが立っているローカル空間の中心を書き出す]
        for (int i = 0; i < altIndexSize; i++) {
            if (voxelData.find(i) == voxelData.end()) {
                continue;
            }
            //std::cout << "++ altIndex=" << i << std::endl;

            for (int j = 0; j < latIndexSize; j++) {
                for (int k = 0; k < lonIndexSize; k++) {
                    //bitテスト
                    unsigned char* plane = voxelData[i].get();
                    uint64_t offset = (uint64_t)lonIndexByte * j + (k / 8);
                    int bit_shift = k % 8;
                    if ((plane[offset] & (1 << bit_shift)) != 0) {
                        spatialid::Zfxy zfxy(localZoom, i + baseZfxy.alt_index, k + baseZfxy.lon_index, j + baseZfxy.lat_index);
                        std::string localSpatialId = zfxy.getString();

                        //std::cout << "+++ localId:" << localSpatialId << std::endl;

                        spatialid::Point pt = spatialid::get_center_point_on_spatial_id(localSpatialId,
                            static_cast<spatialid::CRS>(lasEpsg));
                        double x = pt.x, y = pt.y, z = pt.z;
                        //std::cout << "#-2:x:" << x << ",y:" << y << ",z:" << z << std::endl;

                        if (x < minX) minX = x;
                        if (x > maxX) maxX = x;
                        if (y < minY) minY = y;
                        if (y > maxY) maxY = y;
                        if (z < minZ) minZ = z;
                        if (z > maxZ) maxZ = z;

                        liblas::Point point(&header);

                        //std::cout << "center:" << x << "," << y << "," << z << std::endl;
                        point.SetCoordinates(x, y, z);

                        //int label = (int)colorMap.size() - 1;
                        //int intColor = colorMap[label];
                        int intColor = color;
                        //if (config.needDebugOut()) {
                        //    intColor = colorMap[polygonInd];
                        //}
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
        //std::cout << "pointCount:" << outputPointCount << std::endl;

        header.SetMin(minX, minY, minZ);
        header.SetMax(maxX, maxY, maxZ);
        header.SetPointRecordsCount(outputPointCount);
        header.SetPointRecordsByReturnCount(0, outputPointCount);
        writer.SetHeader(header);
        writer.WriteHeader();
        ofs.close();

        lasMinX = minX;
        lasMaxX = maxX;
        lasMinY = minY;
        lasMaxY = maxY;
        lasMinZ = minZ;
        lasMaxZ = maxZ;
        lasPointCount = outputPointCount;

        logger->trace("##outputPointCount:{0}", outputPointCount);

        //std::cout << outputLasFileName << "(outputPointCount:" << outputPointCount << ")" << std::endl;

        //lazへ
        if (lazMode) {
            las2laz(outputLasFileName, voxelFilePath);
        }

        //std::cout << "end of toLas" << std::endl;
    }

    void ComLib::Voxel::las2laz(std::string& lasFileName, std::string& lazFileName) {
        clock_t ts = clock();

        //std::string lazFileName = std::regex_replace(lasFileName, std::regex("\\.las"), ".laz");
        if (lasFileName == lazFileName) {
            throw std::runtime_error("LASファイルの形式が間違っています");
        }
        //std::cout << "lazFile:" << lazFileName << std::endl;

        //create reader
        laszip_POINTER laszip_reader;
        if (laszip_create(&laszip_reader)) {
            throw std::runtime_error("failed to create laszip reader");
        }

        //open reader
        laszip_BOOL is_compressed = 0;
        if (laszip_open_reader(laszip_reader, lasFileName.c_str(), &is_compressed))
        {
            throw std::runtime_error("failed to open laszip reader(" + lasFileName + ")");
        }

        //get a ponter to the header
        laszip_header* header;

        if (laszip_get_header_pointer(laszip_reader, &header))
        {
            throw std::runtime_error("failed to get the header pointer from laszip reader");
        }

        // how many points does the file have
        laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

        // get a pointer to the points that will be read
        laszip_point* point;

        if (laszip_get_point_pointer(laszip_reader, &point))
        {
            throw std::runtime_error("failed to get point pointer from laszip reader");
        }

        // create the writer
        laszip_POINTER laszip_writer;
        if (laszip_create(&laszip_writer))
        {
            throw std::runtime_error("failed to create laszip writer");
        }

        // initialize the header for the writer using the header of the reader
        if (laszip_set_header(laszip_writer, header))
        {
            throw std::runtime_error("failed to set header for laszip writer");
        }

        // open the writer
        laszip_BOOL compress = true;

        if (laszip_open_writer(laszip_writer, lazFileName.c_str(), compress))
        {
            throw std::runtime_error("failed to open laszip writer");
        }

        // read the points

        laszip_I64 p_count = 0;
        while (p_count < npoints)
        {
            // read a point
            if (laszip_read_point(laszip_reader))
            {
                throw std::runtime_error("failed to read point");
            }

            // copy the point
            if (laszip_set_point(laszip_writer, point))
            {
                throw std::runtime_error("failed to set point");
            }

            // write the point

            if (laszip_write_point(laszip_writer))
            {
                throw std::runtime_error("failed to write point");
            }

            p_count++;
        }

        // close the writer
        if (laszip_close_writer(laszip_writer))
        {
            throw std::runtime_error("failed to close laszip writer");
        }

        // destroy the writer
        if (laszip_destroy(laszip_writer))
        {
            throw std::runtime_error("failed to destroy laszip writer");
        }

        // close the reader
        if (laszip_close_reader(laszip_reader))
        {
            throw std::runtime_error("failed to close laszip reader");
        }

        // destroy the reader

        if (laszip_destroy(laszip_reader))
        {
            throw std::runtime_error("failed to destroy laszip reader");
        }

        ////LASファイルの削除
        //if (!keepLas && std::filesystem::exists(lasFileName)) {
        //    bool rc = false;
        //    for (int i = 0; i < 3; i++) {
        //        try {
        //            std::filesystem::remove(lasFileName);
        //            rc = true;
        //            break;
        //        }
        //        catch (...) {
        //            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //        }
        //    }
        //    if (!rc) {
        //        logger->warn("削除エラー：{1}", lasFileName);
        //    }
        //}

    }
};