//! @file Config.cpp
//! @brief 設定を管理するためのクラス


#include <fstream>
#include <sstream>
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

}
void Config::parseExtraArguments(argparse::ArgumentParser& argParser) {

}

//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void Config::loadExtraConfig(rapidjson::Document& doc) {
    //threadCount = 1;
    voxelOutputRoot = confirmRoot;

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/MakeFlightProhibitedArea(for debug only)
        if (appDict.HasMember("MakeFlightProhibitedArea")) {
            auto& voxelDict = appDict["MakeFlightProhibitedArea"];

            //app/MakeFlightProhibitedArea/pointBatchPath
            if (voxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = voxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("outputVoxel")) {
                    voxelOutputDir = pointBatchPathDict["outputVoxel"].GetString();
                    std::vector<std::string> arr = ComLib::CommonUtil::split(voxelOutputDir, ',');
                    if (arr.size() == 2) {
                        voxelOutputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            voxelOutputRoot = voxelOutputRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }

            }

            //voxelExt
            if (voxelDict.HasMember("voxelExt")) {
                //voxelExt
                voxelExt = voxelDict["voxelExt"].GetString();
            }

            if (voxelDict.HasMember("color")) {
                std::string strColor = voxelDict["color"].GetString();
                color = std::stol(strColor, nullptr, 16);
            }

            //localZoom
            if (voxelDict.HasMember("localZoom")) {
                //localZoom
                localZoom = voxelDict["localZoom"].GetInt();
            }

            if (voxelDict.HasMember("threadCount")) {
                threadCount = voxelDict["threadCount"].GetInt();
            }

            if (voxelDict.HasMember("edgeCount")) {
                edgeCount = voxelDict["edgeCount"].GetInt();
            }

            if (voxelDict.HasMember("useCylinderApi")) {
                useCylinderApi = voxelDict["useCylinderApi"].GetBool();
            }

            //---------- 以下はデバッグ用
            if (voxelDict.HasMember("debug")) {
                auto& debugDict = voxelDict["debug"];
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

