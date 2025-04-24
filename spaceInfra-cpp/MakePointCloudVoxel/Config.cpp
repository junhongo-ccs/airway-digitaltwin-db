//! @file Config.cpp
//! @brief 設定を管理するためのクラス


#include <fstream>
#include <cassert>
#include <string>
#include <map>
#include "Config.h"
#include "CommonUtil.h"
#include <rapidjson/Document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <argparse/argparse.hpp>



void Config::addExtraArguments(argparse::ArgumentParser& argParser) {
    //
}
void Config::parseExtraArguments(argparse::ArgumentParser& argParser) {
       //
}

//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void Config::loadExtraConfig(rapidjson::Document& doc) {
    lasInputRoot = pointOutRoot;
    voxelOutputRoot = confirmRoot;

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/MakePointCloudVoxel
        if (appDict.HasMember("MakePointCloudVoxel")) {
            auto& moduleDict = appDict["MakePointCloudVoxel"];

            //app/MakePointCloudVoxel/pointBatchPath
            if (moduleDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = moduleDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    lasInputDir = pointBatchPathDict["input"].GetString();

                    std::vector<std::string> arr = ComLib::CommonUtil::split(lasInputDir, ',');
                    if (arr.size() == 2) {
                        lasInputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            lasInputRoot = pointInRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }

                if (pointBatchPathDict.HasMember("outputVoxel")) {
                    voxelOutputDir = pointBatchPathDict["outputVoxel"].GetString();

                    std::vector<std::string> arr = ComLib::CommonUtil::split(voxelOutputDir, ',');
                    if (arr.size() == 2) {
                        voxelOutputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            voxelOutputRoot = confirmRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }

                //voxelDelete
                if (pointBatchPathDict.HasMember("voxelDeleteDir")) {
                    std::string dir = pointBatchPathDict["voxelDeleteDir"].GetString();
                    voxelDeleteDir = regex_replace(dir, std::regex("<ownerId>"), ownerId);
                    voxelDeleteDir = regex_replace(voxelDeleteDir, std::regex("\\\\"), "/");
                    std::vector<std::string> arr = ComLib::CommonUtil::split(voxelDeleteDir, ',');
                    if (arr.size() == 2) {
                        voxelDeleteDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            voxelDeleteDir = voxelDeleteDir.replace(0, 1, arr2[1]);
                        }
                    }
                }

            }


            //localZoom
            if (moduleDict.HasMember("localZoom")) {
                //localZoom
                localZoom = moduleDict["localZoom"].GetInt();
            }

            if (moduleDict.HasMember("threadCount")) {
                threadCount = moduleDict["threadCount"].GetInt();
            }

            //色関係
            if (moduleDict.HasMember("color")) {
                std::string strColor = moduleDict["color"].GetString();
                color = std::stol(strColor, nullptr, 16);
            }

            //ext
            if (moduleDict.HasMember("inputFileExt")) {
                inputFileExt = moduleDict["inputFileExt"].GetString();
            }
            if (moduleDict.HasMember("voxelExt")) {
                outputFileExt = moduleDict["voxelExt"].GetString();
            }

            //---------- 以下はデバッグ用
            if (moduleDict.HasMember("debug")) {
                auto& debugDict = moduleDict["debug"];
                if (debugDict.HasMember("debugOut")) {
                    debugOut = debugDict["debugOut"].GetBool();
                }

                if (debugDict.HasMember("spatialId")) {
                    std::string str = debugDict["spatialId"].GetString();
                    if (!str.empty()) {
                        std::vector tmpList = ComLib::CommonUtil::split(str, ',');
                        for (auto const& val : tmpList) {
                            targetSpatialIds.push_back(val);
                        }
                    }
                }

            }

        }

    }

}


//! @brief cityGmlファイル名を取得
//! 
//! @return 取得結果
std::string Config::getLasInputFolder() {
    return getLasInputFolder(batchId);
}

std::string Config::getLasInputFolder(std::string& exBatchFolder) {
    std::string result = ComLib::CommonUtil::pathJoin(lasInputRoot, exBatchFolder);

    result = ComLib::CommonUtil::pathJoin(result, lasInputDir);

    return result;
}


