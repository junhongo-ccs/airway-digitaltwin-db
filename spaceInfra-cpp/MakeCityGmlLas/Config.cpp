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
    argParser.add_argument("--featureId")
        .default_value("")
        .help("対象地物");

}
void Config::parseExtraArguments(argparse::ArgumentParser& argParser) {
    if (argParser.is_used("--featureId")) {
        argFeatureId = argParser.get("--featureId");
    }

}

//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void Config::loadExtraConfig(rapidjson::Document& doc) {
    //threadCount = 1;

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/MakeCityGmlLas
        if (appDict.HasMember("MakeCityGmlLas")) {
            auto& cityGml2VoxelDict = appDict["MakeCityGmlLas"];

            //app/CityGml2Voxel/pointBatchPath
            if (cityGml2VoxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = cityGml2VoxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    inputDir = pointBatchPathDict["input"].GetString();

                    std::vector<std::string> arr = ComLib::CommonUtil::split(inputDir, ',');
                    if (arr.size() == 2) {
                        inputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            pointInRoot = pointInRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }
                if (pointBatchPathDict.HasMember("output")) {
                    outputDir = pointBatchPathDict["output"].GetString();
                    std::vector<std::string> arr = ComLib::CommonUtil::split(outputDir, ',');
                    if (arr.size() == 2) {
                        outputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = ComLib::CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            pointOutRoot = pointOutRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }

            }

            //inputFileExtension
            if (cityGml2VoxelDict.HasMember("inputFileExtension")) {
                //inputFileExtension
                inputFileExtension = cityGml2VoxelDict["inputFileExtension"].GetString();
            }

            //lonlatOrder
            if (cityGml2VoxelDict.HasMember("lonlatOrder")) {
                //lonlatOrder
                std::string s1 = cityGml2VoxelDict["lonlatOrder"].GetString();
                std::transform(s1.begin(),s1.end(),s1.begin(),[](char c) { return std::tolower(c); });
                if (s1 == "lonlat" || s1 == "latlon") {
                    lonlatOrder = s1;
                }
                else {
                    lonlatOrder = "auto";
                }
            }

            //localZoom
            if (cityGml2VoxelDict.HasMember("localZoom")) {
                //localZoom
                localZoom = cityGml2VoxelDict["localZoom"].GetInt();
            }

            //処理対象
            if (cityGml2VoxelDict.HasMember("targetType")) {

                std::string targetTypeStr = cityGml2VoxelDict["targetType"].GetString();
                setTargetTypeList(targetTypeStr);

            }

            ////建物高さの閾値（超えた場合個別処理）
            //if (cityGml2VoxelDict.HasMember("bldgHeightThreshold")) {
            //    bldgHeightThreshold = cityGml2VoxelDict["bldgHeightThreshold"].GetDouble();
            //}

            if (cityGml2VoxelDict.HasMember("threadCount")) {
                threadCount = cityGml2VoxelDict["threadCount"].GetInt();
            }

            if (cityGml2VoxelDict.HasMember("targetLod")) {
                targetLod = cityGml2VoxelDict["targetLod"].GetInt();
            }

            //色関係
            if (cityGml2VoxelDict.HasMember("colorBuilding")) {
                std::string strColor = cityGml2VoxelDict["colorBuilding"].GetString();
                colorBuilding = std::stol(strColor, nullptr, 16);
            }
            if (cityGml2VoxelDict.HasMember("colorRoad")) {
                std::string strColor = cityGml2VoxelDict["colorRoad"].GetString();
                colorRoad = std::stol(strColor, nullptr, 16);
            }
            if (cityGml2VoxelDict.HasMember("colorWater")) {
                std::string strColor = cityGml2VoxelDict["colorWater"].GetString();
                colorWater = std::stol(strColor, nullptr, 16);
            }
            if (cityGml2VoxelDict.HasMember("colorRailway")) {
                std::string strColor = cityGml2VoxelDict["colorRailway"].GetString();
                colorRailway = std::stol(strColor, nullptr, 16);
            }
            if (cityGml2VoxelDict.HasMember("colorGround")) {
                std::string strColor = cityGml2VoxelDict["colorGround"].GetString();
                colorGround = std::stol(strColor, nullptr, 16);
            }

            //---------- 以下はデバッグ用
            if (cityGml2VoxelDict.HasMember("debug")) {
                auto& debugDict = cityGml2VoxelDict["debug"];
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

            //if (debugOut) {//デバッグでは個別処理を行わない
            //    bldgHeightThreshold = 99999;
            //}
        }

        if (!argFeatureId.empty()) {
            setTargetTypeList(argFeatureId);
        }

    }

}

void Config::setTargetTypeList(std::string& targetTypeStr) {
    //std::cout << "targetTypeStr::" << targetTypeStr << std::endl;

    std::vector<PolygonType> list;

    std::transform(targetTypeStr.begin(), targetTypeStr.end(), targetTypeStr.begin(), ::tolower);
    if (targetTypeStr == "all") {
        typeList.push_back(TYPE_BUILDING);
        typeList.push_back(TYPE_ROAD);
        typeList.push_back(TYPE_WATER);
        typeList.push_back(TYPE_RAILWAY);
        typeList.push_back(TYPE_GROUND);
        typeList.push_back(TYPE_POWERLINE);
        typeList.push_back(TYPE_STEELTOWER);
    }
    else {
        std::vector<std::string> arr = ComLib::CommonUtil::split(targetTypeStr, ',');
        for (std::string str : arr) {
            if (str == "building") {
                list.push_back(TYPE_BUILDING);
            }
            else if (str == "road") {
                list.push_back(TYPE_ROAD);
            }
            else if (str == "water") {
                list.push_back(TYPE_WATER);
            }
            else if (str == "railway") {
                list.push_back(TYPE_RAILWAY);
            }
            else if (str == "ground") {
                list.push_back(TYPE_GROUND);
            }
            else if (str == "powerline") {
                list.push_back(TYPE_POWERLINE);
            }
            else if (str == "steeltower") {
                list.push_back(TYPE_STEELTOWER);
            }
            else {
                //std::cout << "★wrong type:" << str << std::endl;
            }
        }

        if (list.size() > 0) {
            typeList = list;
        }
    }
}


//! @brief cityGmlファイル名を取得
//! 
//! @return 取得結果
std::string Config::getCityGmlFileFolder() {
    return getCityGmlFileFolder(batchId);
}

std::string Config::getCityGmlFileFolder(std::string& exBatchFolder) {
    std::string result = ComLib::CommonUtil::pathJoin(pointInRoot, exBatchFolder);

    result = ComLib::CommonUtil::pathJoin(result, inputDir);

    return result;
}


//! @brief 出力フォルダ(output)のフルパスを取得
//! 
//! @return 取得結果
std::string Config::getOutputFolder() {
    std::string result = ComLib::CommonUtil::pathJoin(pointOutRoot, batchId);

    result = ComLib::CommonUtil::pathJoin(result, outputDir);

    return result;
}


