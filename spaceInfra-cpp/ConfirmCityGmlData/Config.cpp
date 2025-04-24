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
    //argParser.add_argument("--clear")
    //    .help("削除フラグ(y: 削除,その他：削除しない。デフォルト:n");
}

void Config::parseExtraArguments(argparse::ArgumentParser& argParser) {
    //if (argParser.is_used("--clear")) {
    //    configClear = argParser.get("--clear");
    //    if (!configClear.empty()) {
    //        std::transform(configClear.begin(), configClear.end(), configClear.begin(), [](char const& c) {
    //            return std::tolower(c);
    //            });
    //    }
    //}
}



//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void Config::loadExtraConfig(rapidjson::Document& doc) {
    lasInputRoot = pointOutRoot;
    voxelInputRoot = pointOutRoot;
    lasOutputRoot = confirmRoot;
    voxelOutputRoot = confirmRoot;
    threadCount = 1;

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/ConfirmCityGmlData
        if (appDict.HasMember("ConfirmCityGmlData")) {
            auto& confirmDataDict = appDict["ConfirmCityGmlData"];

            //app/ConfirmData/pointBatchPath
            if (confirmDataDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = confirmDataDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("inputProcessingResultsLas")) {
                    lasInputDir = pointBatchPathDict["inputProcessingResultsLas"].GetString();

                    std::vector<std::string> arr = ComLib::CommonUtil::split(lasInputDir, ',');
                    if (arr.size() == 2) {
                        lasInputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            lasInputRoot = lasInputRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }
                if (pointBatchPathDict.HasMember("inputProcessingResultsVoxel")) {
                    voxelInputDir = pointBatchPathDict["inputProcessingResultsVoxel"].GetString();

                    std::vector<std::string> arr = ComLib::CommonUtil::split(voxelInputDir, ',');
                    if (arr.size() == 2) {
                        voxelInputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            voxelInputRoot = voxelInputRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }
                if (pointBatchPathDict.HasMember("outputLas")) {
                    lasOutputDir = pointBatchPathDict["outputLas"].GetString();
                    std::vector<std::string> arr = ComLib::CommonUtil::split(lasOutputDir, ',');
                    if (arr.size() == 2) {
                        lasOutputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            lasOutputRoot = lasOutputRoot.replace(0, 1, arr2[1]);
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
                            voxelOutputRoot = voxelOutputRoot.replace(0, 1, arr2[1]);
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


            if (confirmDataDict.HasMember("inputVoxelExtension")) {
                inputVoxelExt = confirmDataDict["inputVoxelExtension"].GetString();
            }

            //localZoom
            if (confirmDataDict.HasMember("localZoom")) {
                //localZoom
                localZoom = confirmDataDict["localZoom"].GetInt();
            }


            //---------- 以下はデバッグ用
            if (confirmDataDict.HasMember("debug")) {
                auto& debugDict = confirmDataDict["debug"];
                if (debugDict.HasMember("debugOut")) {
                    debugOut = debugDict["debugOut"].GetBool();
                }

            }

        }
    }

}

