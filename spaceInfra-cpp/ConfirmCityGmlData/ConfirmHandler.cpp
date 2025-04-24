#include "ConfirmHandler.h"

#include "CommonUtil.h"
#include <map>
#include <unordered_set>
#include <liblas/liblas.hpp>
#include "DataAccessAdapter.h"
#include <BS_thread_pool.hpp>
#include "../ComLib/CommonUtil.h"

ConfirmHandler::ConfirmHandler() {
    Config& config = Config::getInstance();
    logger = spdlog::get(config.getLoggerName());

    //-----------------------------------------
    //座標変換関連
    //-----------------------------------------
    projContext = proj_context_create();
    if (projContext == nullptr) {
        throw std::runtime_error("PROJコンテキストが作成できません");
    }

    //pointOutLasEpsg → confirmLasEpsg
    if (config.getPointOutLasEpsg() != config.getConfirmLasEpsg()) {
        std::string fromCrs = "EPSG:" + std::to_string(config.getPointOutLasEpsg());
        std::string toCrs = "EPSG:" + std::to_string(config.getConfirmLasEpsg());

        logger->trace("ALLと地物別：{0} -> {1}", fromCrs, toCrs);

        projPointOut2ConfirmLas = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
        if (projPointOut2ConfirmLas == nullptr) {
            throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
        }
    }

    ////pointInLasEpsg → confirmLasEpsg
    //if (config.getPointInLasEpsg() != config.getConfirmLasEpsg()) {
    //    std::string fromCrs = "EPSG:" + config.getPointInLasEpsg();
    //    std::string toCrs = "EPSG:" + config.getConfirmLasEpsg();

    //    logger->trace("INPUT: {0} -> {1}", fromCrs, toCrs);
    //    projPointIn2ConfirmLas = proj_create_crs_to_crs(projContext, fromCrs.c_str(), toCrs.c_str(), nullptr);
    //    if (projPointIn2ConfirmLas == nullptr) {
    //        throw std::runtime_error("PROJの変換用オブジェクトが作成できません");
    //    }
    //}

}

int ConfirmHandler::execute() {
    Config& config = Config::getInstance();

    //バッチ日付
    std::string today = ComLib::CommonUtil::getCurDatetime();

    //DBアクセス用
    DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
    adapter.connect(config);

    //削除
    adapter.deleteVoxel("init", "", config.getVoxelDeleteDir());

    //地物ID変換マップ
    std::map<std::string, int> featureId2ObjIdMap;
    adapter.createObjectIdMap(featureId2ObjIdMap);
    //std::cout << "feature2ObjIdMap:" << featureId2ObjIdMap.size() << std::endl;

    //dataSourceId
    std::string serverId = config.getServerId();
    std::string ownerId = config.getOwnerId();
    int dataSourceId = adapter.getDataSourceId(serverId, ownerId);
    logger->trace("## dataSourceId:{0}, serverId:{1}, ownerId:{2}", dataSourceId, serverId, ownerId);

    if (dataSourceId < 0) {
        throw std::runtime_error("データソースIDが取得できません");
    }

    //元LASファイル一覧を取得
    //std::string lasFilePtn = "^[0-9]+-(-)?[0-9]+-[0-9]+-[0-9]+_ALL\\.las$";
    std::string lasFilePtn = "^[0-9]+-(-)?[0-9]+-[0-9]+-[0-9]+_ALL\\." + config.getInputVoxelExt() + "$";
    std::vector<std::string> lasFileList = ComLib::CommonUtil::getFileList(config.getVoxelInputFolder(), lasFilePtn, config.getBatchFile());

    int fileCount = 0;
    std::set<std::string> processedSpatialIds;

    if (lasFileList.size() == 0) {
        logger->info("対象{0}ファイルがありません", config.getInputVoxelExt());
    }

    for (std::string& lasFile : lasFileList) {
        clock_t ts = clock();

        fileCount++;

        //ファイル名から空間IDを求める
        std::string spatialIdWithSlash = CommonUtil::getSpatialIdFromFilename(lasFile);

        //初期の場合古いデータを削除
        if (processedSpatialIds.find(spatialIdWithSlash) == processedSpatialIds.end()) {
            adapter.deleteVoxel("spatialInit", spatialIdWithSlash, config.getVoxelDeleteDir());
            processedSpatialIds.insert(spatialIdWithSlash);
        }

        logger->info("{0} START ({1}/{2})", spatialIdWithSlash, fileCount, lasFileList.size());
        bool errorFlag = false;
        try {
            //空間IDのボクセル
            std::vector<spatialid::Point> vertexList;
            //spatialid::get_vertex_points_on_spatial_id(spatialIdWithSlash, vertexList, static_cast<spatialid::CRS>(config.getMapEpsg()));

            //featureIdのリスト
            std::vector<std::string> featureIdList;

            ////100番
            //featureIdList.push_back("INPUT");

            //地物別
            std::vector<std::string> subList = CommonUtil::getFeatureIdList(config.getVoxelInputFolder(), spatialIdWithSlash);
            if (subList.size() == 0) {
                logger->info("地物関連LASファイルがありません");
            }
            for (const auto& featureId : subList) {
                if (featureId == "ALL" || featureId == "INPUT") {
                    continue;
                }
                if (featureId2ObjIdMap.find(featureId) == featureId2ObjIdMap.end()) {
                    //対象外のfeatureId
                    continue;
                }

                featureIdList.push_back(featureId);
            }

            //-- 200番
            if (subList.size() > 0) {
                featureIdList.push_back("ALL");
            }

            //-- 地物別のデータを作成する
            //std::cout << "threadCount:" << config.getThreadCount() << std::endl;
            if (featureIdList.size() == 1 || config.getThreadCount() <= 1) {
                //シングルスレッド
                for (auto const featureId : featureIdList) {
                    int objId = featureId2ObjIdMap.at(featureId);

                    createDataCommon(spatialIdWithSlash, today, vertexList, objId, featureId, dataSourceId);
                }
            }
            else {
                //マルチスレッド
                int threadCount = config.getThreadCount();
                if (featureIdList.size() < threadCount) {
                    threadCount = featureIdList.size();
                }

                //std::cout << "threadCount:" << threadCount << std::endl;

                BS::thread_pool pool(threadCount);
                for (int i = 0; i < featureIdList.size(); i++) {
                    std::string featureId = featureIdList[i];
                    //std::cout << "featureId:" << featureId << std::endl;

                    pool.push_task(
                        [this, &featureId2ObjIdMap, &spatialIdWithSlash, &today, &vertexList, featureId, &dataSourceId] {
                                int objId = featureId2ObjIdMap.at(featureId);
                            //std::cout << "<<<featureId:" << featureId << ",objId:" << objId << ">>>" << std::endl;

                            createDataCommon(spatialIdWithSlash, today, vertexList, objId, featureId, dataSourceId);
                        });
                }

                pool.wait_for_tasks();
            }

            //space_data_managesを更新
            std::string recId = "confirmCityGmlData";
            adapter.updateSpaceDataManages(config.getOwnerId(), config.getBatchId(),
                spatialIdWithSlash, recId);
        }
        catch (std::runtime_error& ex) {
            logger->error(ex.what());
            errorFlag = true;
        }
        catch (...) {
            logger->error("予期せねエラーが発生しました");
            errorFlag = true;
        }

        if (errorFlag) {
            logger->warn("{0} ABORTED (elapsed time:{1:.2f})", spatialIdWithSlash, (double)(clock()-ts)/CLOCKS_PER_SEC );
            return 9; //直ちに終了
        }
        else {
            logger->info("{0} END (elapsed time:{1:.2f})", spatialIdWithSlash, (double)(clock() - ts) / CLOCKS_PER_SEC);
        }
    }

    return 0;
}


void ConfirmHandler::createDataCommon(const std::string & spatialIdWithSlash, const std::string & today,
        const std::vector<spatialid::Point>&vertexList, int objectId, const std::string & featureId,
        int dataSourceId) {
    clock_t ts = clock();

    logger->debug("create data START: spatialId:{0}, featureId:{1}", spatialIdWithSlash, featureId);

    Config& config = Config::getInstance();

    //[/] -> [_]
    std::string spatialId = spatialIdWithSlash;
    std::replace(spatialId.begin(), spatialId.end(), '/', '_');

    //[/] -> [-]
    std::string spatialIdInFile = spatialIdWithSlash;
    std::replace(spatialIdInFile.begin(), spatialIdInFile.end(), '/', '-');

    //入力フォルダ
    std::string lasInputFolder;
    std::string voxelInputFolder;

    //if (objectId == OBJ_ID_INPUT) {
    //    lasInputFolder = config.getLas100InputFolder();
    //}
    //else {
        lasInputFolder = config.getLasInputFolder();
    //}

    voxelInputFolder = config.getVoxelInputFolder();

    //logger->trace("input(las):{0}", lasInputFolder);
    //logger->trace("input(voxel,hash):{0}", voxelInputFolder);

    //出力フォルダ
    std::string lasOutputFolder = config.getLasOutputFolder(spatialId, today);
    std::string voxelOutputFolder = config.getVoxelOutputFolder(spatialId, today);
    std::string voxelOutputFolder2 = config.getVoxelBitFilePath(spatialId, today);

    //logger->trace("output(las):{0}", lasOutputFolder);
    //logger->trace("output(voxel):{0}", voxelOutputFolder);

    if (!lasOutputFolder.empty()) {
        ComLib::CommonUtil::forceDirectories(lasOutputFolder);
    }
    ComLib::CommonUtil::forceDirectories(voxelOutputFolder);

    //Lasファイルをコピー
    std::string lasFileNameIn;
    std::string lasFrom, lasTo;
    if (!lasInputFolder.empty() && !lasOutputFolder.empty()) {
        lasFileNameIn = spatialIdInFile + "_" + featureId + ".las";
        PJ* proj = nullptr;
        //if (objectId == OBJ_ID_INPUT) {
        //    lasFileNameIn = spatialIdInFile + ".las";
        //    if (config.getPointInLasEpsg() != config.getConfirmLasEpsg()) {
        //        proj = projPointIn2ConfirmLas;
        //    }
        //}
        //else 
        if (projPointOut2ConfirmLas != nullptr) {
            proj = projPointOut2ConfirmLas;
        }
        std::string lasFileNameOut = spatialId + "_" + std::to_string(objectId) + ".las";
        lasFrom = ComLib::CommonUtil::pathJoin(lasInputFolder, lasFileNameIn);
        lasTo = ComLib::CommonUtil::pathJoin(lasOutputFolder, lasFileNameOut);

        logger->trace("{0} -> {1}", lasFileNameIn, lasFileNameOut);

        copyLasFile(lasFrom, lasTo, config.getConfirmLasEpsg(), proj);
    }
    //voxelをコピー
    std::string voxelFileNameIn = spatialIdInFile + "_" + featureId + "." + config.getInputVoxelExt();
    if (objectId == OBJ_ID_INPUT) {
        voxelFileNameIn = spatialIdInFile + "." + config.getInputVoxelExt();
    }
    std::string voxelFileNameOut = spatialId + "_" + std::to_string(objectId) + "." + config.getInputVoxelExt();
    std::string voxelFrom = ComLib::CommonUtil::pathJoin(voxelInputFolder, voxelFileNameIn);
    std::string voxelTo = ComLib::CommonUtil::pathJoin(voxelOutputFolder, voxelFileNameOut);
    std::string voxelBitFilePath = ComLib::CommonUtil::pathJoin(voxelOutputFolder2, voxelFileNameOut);

    logger->trace("{0} -> {1}", voxelFileNameIn, voxelFileNameOut);
    if (!std::filesystem::exists(voxelFrom)) {
        throw std::runtime_error(voxelFileNameIn + "がありません");
    }

    std::filesystem::copy_file(voxelFrom, voxelTo, std::filesystem::copy_options::overwrite_existing);

    ////hashを取得
    //std::string hashFileNameIn = spatialIdInFile + "_" + featureId + ".hash";
    //if (objectId == OBJ_ID_INPUT) {
    //    hashFileNameIn = spatialIdInFile + ".hash";
    //}
    //std::string hashFrom = CommonUtil::pathJoin(config.getVoxelInputFolder(), hashFileNameIn);
    //if (!std::filesystem::exists(hashFrom)) {
    //    throw std::runtime_error(hashFileNameIn + "がありません");
    //}

    //logger->trace("hash file:{0}", hashFileNameIn);

    //std::string hash = getHash(hashFrom);
    //if (hash.empty()) {
    //    std::string msg("ハッシュデータがありません");
    //    msg.append("(" + hashFrom + ")");
    //    throw std::runtime_error(msg);
    //    //logger->warn(msg);
    //}

    //std::vector<std::string> hashList = CommonUtil::split(hash, ',');
    //if (hashList.size() != 11) {
    //    throw std::runtime_error("ハッシュの長さが正しくありません");
    //}


    ////sendHash
    //if (config.getThreadCount() > 1) {//マルチスレッドはまだ未完成
    //    std::lock_guard<std::mutex> lock(mtx); // mtxを使ってロックする
    //    sendHash(spatialIdWithSlash, objectId, rec.hashList);
    //}
    //else {
    //    sendHash(spatialIdWithSlash, objectId, rec.hashList);
    //}

    //std::string hashFileNameIn = "";
    //std::vector<std::string> hashList;
    //for (int i = 0; i < 11; i++) {
    //    hashList.push_back("");
    //}

    SpaceObjectRec rec;
    rec.spatialId = spatialIdWithSlash;
    rec.fromDatetime = config.getFromDatetime();
    rec.toDatetime = config.getToDatetime();
        
    rec.objectCd = objectId;
    rec.voxelBitFilePath = voxelBitFilePath;
    rec.voxelBitSpatialZoomLevel = config.getLocalZoom();
    rec.pointCloudEpsg = config.getConfirmLasEpsg();

    rec.dataSourceId = dataSourceId;
    //rec.updateMemo = lasFileNameIn + "," + voxelFileNameIn + "," + hashFileNameIn;
    rec.updateMemo = lasFileNameIn + "," + voxelFileNameIn;

    //rec.northLat = vertexList[0].y;
    //rec.southLat = vertexList[2].y;
    //rec.eastLon = vertexList[1].x;
    //rec.westLon = vertexList[0].x;
    //rec.lowerAltitude = vertexList[0].z;
    //rec.upperAltitude = vertexList[6].z;
    //rec.pointCloudFilePath = lasTo;
    //rec.hashList = hashList;

    //space_objectsへ登録
    DataAccessAdapter& adapter = DataAccessAdapter::getInstance();

    adapter.insertSpaceObject(rec);


    ////detailを出力
    //if (featureId == "BUILDING" || featureId == "ROAD") {
    //    createDetailData(spatialIdWithSlash, today, objectId, featureId, dataSourceId);
    //}

    logger->debug("create data END (elapsed time:{0:.2f})", (double)(clock() - ts)/CLOCKS_PER_SEC);

}

//! @brief LASポイントデータ(LAS)を読み込む
//! 
//! @param lasFile LASファイル名
//! @mapDef 地図関連定義
//! @param config 設定情報
void ConfirmHandler::copyLasFile(const std::string& lasFileIn, const std::string& lasFileOut, 
        int confirmLasEpsg, PJ* proj) {
    if (proj == nullptr) {
        //EPSGが同じなのでコピーのみ
        logger->trace("copyLasFile:: same EPSG:{0}", confirmLasEpsg);
        std::filesystem::copy_file(lasFileIn, lasFileOut, std::filesystem::copy_options::overwrite_existing);
        return;
    }

    Config& config = Config::getInstance();

    std::ifstream ifs;
    ifs.open(lasFileIn, std::ios::in | std::ios::binary);

    liblas::ReaderFactory f;
    liblas::Reader reader = f.CreateWithStream(ifs);

    //read header
    liblas::Header const& headerIn = reader.GetHeader();
    liblas::SpatialReference srsIn = headerIn.GetSRS();

    int pointCount = headerIn.GetPointRecordsCount();

    //-------- output
    std::ofstream ofs;
    ofs.open(lasFileOut, std::ios::out | std::ios::binary);
    liblas::Header headerOut;
    //headerOut.SetDataFormatId(liblas::ePointFormat3); // Time only
    headerOut.SetDataFormatId(headerIn.GetDataFormatId());

    liblas::SpatialReference srs;
    std::string strConfirmLasEpsg = std::to_string(confirmLasEpsg);
    std::string epsg("EPSG:");
    epsg.append(strConfirmLasEpsg);
    //srs.SetFromUserInput(epsg.c_str());

    double xyScale = 1.0/CommonUtil::calcScale(strConfirmLasEpsg, proj);
    double zScale = headerIn.GetScaleZ();

    logger->trace("copyLasFile:: xyScale:{0},zScale:{1}", xyScale, zScale);

    headerOut.SetSRS(srs);
    headerOut.SetScale(xyScale, xyScale, zScale);

    liblas::Writer writer(ofs, headerOut);


    int outputPointCount = 0;
    double minX = DBL_MAX, minY = DBL_MAX, minZ = DBL_MAX;
    double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

    while (reader.ReadNextPoint())
    {
        liblas::Point const& p = reader.GetPoint();

        double x = p.GetX(), y = p.GetY(), z = p.GetZ();

        //座標変換
        CommonUtil::transform(1, &x, &y, proj);
        x = static_cast<long>(x / xyScale) * xyScale;
        y = static_cast<long>(y / xyScale) * xyScale;
        z = static_cast<long>(z / zScale) * zScale;

        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (z < minZ) minZ = z;

        if (x > maxX) maxX = x;
        if (x > maxY) maxY = y;
        if (z > maxZ) maxZ = z;


        //書き出し
        liblas::Point pointOut(&headerOut);
        pointOut.SetCoordinates(x, y, z);
        pointOut.SetColor(p.GetColor());

        writer.WritePoint(pointOut);
        outputPointCount++;

    }

    ifs.close();

    //std::cout << std::setprecision(15) << "minX:" << minX << ",maxX:" << maxX << std::endl;

    headerOut.SetMin(minX, minY, minZ);
    headerOut.SetMax(maxX, maxY, maxZ);
    headerOut.SetPointRecordsCount(outputPointCount);
    headerOut.SetPointRecordsByReturnCount(0, outputPointCount);
    writer.SetHeader(headerOut);
    writer.WriteHeader();

    ofs.close();

}


//std::string ConfirmHandler::getHash(std::string& fileName) {
//    std::ifstream infile(fileName);
//    std::string line;
//
//    if (!std::getline(infile, line)) {
//        line = "";
//    }
//
//    infile.close();
//
//    return line;
//}
//
//
//void ConfirmHandler::sendHash(const std::string & spatialId, int objectCd, const std::vector<std::string>& hashList) {
//    Config& config = Config::getInstance();
//    if (config.getSendHashPath().empty()) {
//        return;
//    }
//
//#ifdef _WIN32
//#define popen _popen
//#define pclose _pclose
//#define WEXITSTATUS
//#endif
//
//    char buffer[512];
//
//    bool errFlag = false;
//    try {
//        std::string params = " -spId " + spatialId + " -ojCd " + std::to_string(objectCd);
//
//        int curId = 18;
//        for (const auto hash : hashList) {
//            params += " -hash" + std::to_string(curId) + " " + hash;
//            curId++;
//        }
//        std::string program = config.getSendHashPath() + params;
//        std::string command = "powershell.exe -ExecutionPolicy Bypass \"" + program + "\" ";
//
//
//        FILE* pipe = popen(command.c_str(), "r");
//        int rc = 9;
//        //bool alreadySet = false;
//        if (pipe != nullptr) {
//            try {
//                int bufLen = sizeof(buffer) / sizeof(buffer[0]);
//                //std::vector<std::string> strList;
//                std::string lastLine;
//                while (fgets(buffer, bufLen-1, pipe) != nullptr) {
//                    //buffer[bufLen - 1] = '\0';
//                    //strList.push_back(buffer);
//                    if (buffer[std::strlen(buffer) - 1] == '\n') {
//                        buffer[std::strlen(buffer) - 1] = '\0';
//                    }
//
//                    logger->debug("#PowerShell:{0}", buffer);
//
//                    lastLine = buffer;
//                }
//
//                //最終行にreturn値がある前提
//                if (!lastLine.empty()) {
//                    std::regex re(R"(^\d+$)");
//                    if (std::regex_match(lastLine, re)) {
//                        rc = std::stoi(lastLine);
//                        //alreadySet = true;
//                    }
//                }
//
//                
//            }
//            catch (...) {
//                pclose(pipe);
//                throw;
//            }
//
//            //rc = WEXITSTATUS(pclose(pipe));
//            pclose(pipe);
//        }
//        else {
//            rc = -1;
//        }
//
//
//        //rc = std::system(command.c_str());
//        logger->trace("sendHash exitcode:{0}", rc);
//        errFlag = (rc != 0);
//    }
//    catch (...) {
//        logger->warn("ハッシュデータ送信処理に失敗しました");
//        errFlag = true;
//    }
//   
//    if (errFlag) {
//        logger->trace("updateMemo");
//        SpaceObjectRec rec;
//        rec.spatialId = spatialId;
//        rec.objectCd = objectCd;
//        rec.hashList = hashList;
//        rec.updateMemo = "sendHash error";
//
//        DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
//        adapter.updateMemo(rec);
//
//        throw std::runtime_error("sendHashでエラーが起きました");
//    }
//}

