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

namespace ComLib{

    //! @biref 設定をロードする
    //! 
    //! @param argc コマンド引数の数
    //! @param argv コマンド引数へのポイント配列
    void Config::loadConfig(int argc, char** argv) {

        const std::vector<std::string> args(argv, argv + argc);
        std::vector<std::string> options;
        std::string jsonFile;
        bool defaultJsonFile = true;

        std::string appName = CommonUtil::getAppName(argv[0]);

        int i = -1;
        for (const std::string& arg : args) {
            i++;
            //変換対象(argparseのデフォルト実装では「-v 3」のような指定に対応していないため、「--verbose 3」に変換してから処理させる
            std::map<std::string, std::string> convertMap = {
                {"-c", "--config"},
                {"-v", "--verbose" }
            };

            //[-] --> [--]変換用
            auto replaceFunc = [&convertMap](const std::string& arg, const std::string& from) {
                if (convertMap.find(from) == convertMap.end()) {//ない場合
                    return arg;
                }
                std::string str(convertMap[from]);
                str += arg.substr(from.length());

                return str;
            };

            //[--]のものはそのまま追加
            if (arg.length() > 1 && arg[0] == '-' && arg[1] == '-') {
                options.push_back(arg);
            }
            else if (arg[0] == '-') {
                //[=]がなければそのまま追加
                if (arg.find("=") < 0) {
                    if (convertMap.find(arg) == convertMap.end()) {
                        options.push_back(arg);
                    }
                    else {
                        options.push_back(convertMap[arg]);
                    }
                }
                else {
                    //[-] --> [--]
                    options.push_back(replaceFunc(arg, arg.substr(0, 2)));
                }
            }
            else {
                options.push_back(arg);
            }

        }


        //コマンド引数
        loadFromArguments(options, appName);



        //config.jsonをロード
        if (configJsonFile.length() == 0) {
            configJsonFile = CommonUtil::pathJoin(configDir, defConfigJsonFile);
        }


        loadConfig(configJsonFile);

        if (argVerbose > 0) {
            setVerbose(argVerbose);
        }

        if (!argFeatureId.empty()) {
            setTargetTypeList(argFeatureId);
        }


        //ロガーを用意
        prepareLogger(appName);
    }

    //! @brief コマンド引数から設定情報をロードする
//! 
//! @param options コマンド引数
//! @param appName アプリID
    void Config::loadFromArguments(const std::vector<std::string>& options, const std::string& appName) {
        //コマンド引数(>configファイル)
        argparse::ArgumentParser argParser(appName, "1.0", argparse::default_arguments::none);

        argParser.add_argument("batchid") //ただヘルプを表示させるために。
            .default_value("")
            .required()
            .help("バッチID [ファイルID]")
            .nargs(1, 2)
            .append();

        argParser.add_argument("--config")
            .help("設定ファイル。デフォルト：commonConfig.json");

        argParser.add_argument("--featureId")
            .default_value("")
            .help("対象地物");

        argParser.add_argument("--verbose")
            .help("ログ出力レベル(1:ERROR, 2:WARN, 3:INFO, 4:DEBUG, 5:TRACE)。デフォルト：3")
            .scan<'i', int>();

        //argParser.add_argument("--epsg")
        //    .default_value("")
        //    .help("入力と出力のEPSG");

        //argParser.add_argument("--epsgLas")
        //    .default_value("")
        //    .help("LASファイルのEPSG");

        //argParser.add_argument("--epsgCsv")
        //    .default_value("")
        //    .help("CSVファイルのEPSG");

        argParser.add_argument("-h", "--help")
            .action([=](const std::string& s) {
            std::cout << argParser.help().str();
            exit(1);
                })
            .default_value(false)
                    .help("shows help message")
                    .implicit_value(true)
                    .nargs(0)
                    .help("ヘルプ");



                if (!options.empty()) {
                    argParser.parse_args(options);
                }



                //必要に応じてconfigを更新
                if (argParser.is_used("batchid")) {
                    const auto& arg = argParser.get<std::vector<std::string>>("batchid");

                    if (arg.size() > 0) {
                        batchFolder = arg[0];
                    }
                    if (arg.size() > 1) {
                        batchFile = arg[1];
                        if (!batchFile.empty() && batchFile.find_last_of('.') == std::string::npos) {
                            batchFile += ".las";
                        }
                    }
                }

                if (batchFolder.empty()) {
                    std::cout << argParser.help().str() << std::endl;
                    exit(1);
                }


                if (argParser.is_used("--config")) {
                    configJsonFile = argParser.get("--config");
                }

                //if (argParser.is_used("--epsg")) {
                //    argEpsg = argParser.get("--epsg");
                //}

                //if (argParser.is_used("--epsgLas")) {
                //    argEpsgLas = argParser.get("--epsgLas");
                //}

                //if (argParser.is_used("--epsgCsv")) {
                //    argEpsgCsv = argParser.get("--epsgCsv");
                //}

                if (argParser.is_used("--verbose")) {
                    argVerbose = argParser.get<int>("--verbose");
                }

                if (argParser.is_used("--featureId")) {
                    argFeatureId = argParser.get("--featureId");
                }

    }



//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void Config::loadConfig(std::string & configFileName) {
    std::ifstream ifs(configFileName.data());
    rapidjson::IStreamWrapper iws(ifs);

    rapidjson::Document doc;
    rapidjson::ParseResult parseOk = doc.ParseStream(iws);
    if (!parseOk) {
        std::cout << CommonUtil::format("config parse error(%s,%d)",
            rapidjson::GetParseError_En(parseOk.Code()), parseOk.Offset()) << std::endl;
        //throw std::runtime_error("Configファイルには誤りがあります");
        std::exit(9);
    }

    //common
    if (doc.HasMember("common")) {
        auto& commonDict = doc["common"];

        if (commonDict.HasMember("projectRoot")) {
            projectRoot = commonDict["projectRoot"].GetString();
        }
        if (commonDict.HasMember("dataRoot")) {
            dataRoot = commonDict["dataRoot"].GetString();
        }
        if (commonDict.HasMember("citygmlRoot")) {
            citygmlInRoot = commonDict["citygmlRoot"].GetString();
            citygmlOutRoot = citygmlInRoot;
        }
        if (commonDict.HasMember("logRoot")) {
            logRoot = commonDict["logRoot"].GetString();
        }
        //ログ関連
        if (commonDict.HasMember("logsink")) {
            logsink = commonDict["logsink"].GetString();

            if (commonDict.HasMember("loglevel")) {
                loglevel = commonDict["loglevel"].GetString();
            }
        }

    }

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/common
        if (appDict.HasMember("common")) {
            auto& appCommonDict = appDict["common"];

        }

        //app/CityGml2Voxel
        if (appDict.HasMember("CityGml2Voxel")) {
            auto& cityGml2VoxelDict = appDict["CityGml2Voxel"];

            //app/CityGml2Voxel/pointBatchPath
            if (cityGml2VoxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = cityGml2VoxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    inputDir = pointBatchPathDict["input"].GetString();

                    std::vector<std::string> arr = CommonUtil::split(inputDir, ',');
                    if (arr.size() == 2) {
                        inputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            citygmlInRoot = citygmlInRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }
                if (pointBatchPathDict.HasMember("output")) {
                    outputDir = pointBatchPathDict["output"].GetString();
                    std::vector<std::string> arr = CommonUtil::split(outputDir, ',');
                    if (arr.size() == 2) {
                        outputDir = arr[0];
                        //drive=cのようなものなので、再度スプリット
                        std::vector<std::string> arr2 = CommonUtil::split(arr[1], '=');
                        if (arr2.size() == 2) {
                            citygmlOutRoot = citygmlOutRoot.replace(0, 1, arr2[1]);
                        }
                    }
                }

            }

            //threadCount
            if (cityGml2VoxelDict.HasMember("threadCount")) {
                threadCount = cityGml2VoxelDict["threadCount"].GetInt();
                if (threadCount <= 0) {
                    threadCount = 1;
                }

                //localZoom
                localZoom = cityGml2VoxelDict["localZoom"].GetInt();
            }

            //処理対象
            if (cityGml2VoxelDict.HasMember("targetType")) {

                std::string targetTypeStr = cityGml2VoxelDict["targetType"].GetString();
                setTargetTypeList(targetTypeStr);

            }

            //建物高さの閾値（超えた場合個別処理）
            if (cityGml2VoxelDict.HasMember("bldgHeightThreshold")) {
                bldgHeightThreshold = cityGml2VoxelDict["bldgHeightThreshold"].GetDouble();
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
                        std::vector tmpList = CommonUtil::split(str, ',');
                        for (auto const& val : tmpList) {
                            targetSpatialIds.push_back(val);
                        }
                    }
                }

            }

            if (debugOut) {//デバッグでは個別処理を行わない
                bldgHeightThreshold = 99999;
            }
        }
    }

}

void Config::setTargetTypeList(std::string & targetTypeStr) {
    //std::cout << "targetTypeStr::" << targetTypeStr << std::endl;

    std::vector<PolygonType> list;

    std::transform(targetTypeStr.begin(), targetTypeStr.end(), targetTypeStr.begin(), ::tolower);
    if (targetTypeStr == "all") {
        typeList.push_back(TYPE_BUILDING);
        typeList.push_back(TYPE_ROAD);
    }
    else {
        std::vector<std::string> arr = CommonUtil::split(targetTypeStr, ',');
        for (std::string str : arr) {
            if (str == "bldg") {
                list.push_back(TYPE_BUILDING);
            }
            else if (str == "tran") {
                list.push_back(TYPE_ROAD);
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

//! @brief Loggerを準備する
//! 
//! @param appName アプリID
void Config::prepareLogger(const std::string & appName) {
    //----------------------------------------------
    //■ログについてはとりあえず以下のように実装（spdlogger利用）
    //・3パターンのsinkを用意
    //     ①console →　画面に出力
    //     ②basic  →　日付なしのファイルを作成。例（pointmatch.log)
    //     ③daily  →　日付ありのファイルを作成。例（pointmatch-2023-04-06.log)。
    //・フォルダ名は固定。ただsinkがconsoleの場合、ファイル出力を行わない。※リダイレクトでファイルに落とせる
    //・古いログファイルを移動するためのbatファイルは必要なら別途作成。たとえば
    //     、{logroot}\pointmatch\pointmatch-2023-04-06.logを{logroot}\history\pointmatch\の下に移動させるとか。
    //
    // ※アプリを平行に複数個実行させる必要がある場合、PointMatchをシングルスレッドモード(threadCount=1)にしたほうがいい
    std::shared_ptr<spdlog::logger> logger;


    if (logsink != "console" && logsink != "stdout") {
        std::string dest = logRoot;
        dest = CommonUtil::pathJoin(dest, appName);
        CommonUtil::forceDirectories(dest);

        if (logsink == "basic") {
            dest = CommonUtil::pathJoin(dest, appName).append(".log");
            logger = spdlog::basic_logger_mt(loggerName, dest);
        }
        else { //daily
            dest = CommonUtil::pathJoin(dest, appName).append("-%Y-%m-%d.log");
            logger = spdlog::daily_logger_format_mt(loggerName, dest, 0, 0); //0時0分
        }
    }
    else {
        logger = spdlog::stdout_color_mt(loggerName);
    }

    if (loglevel == "trace") {
        logger->set_level(spdlog::level::trace);
    }
    else if (loglevel == "debug") {
        logger->set_level(spdlog::level::debug);
    }
    else if (loglevel == "info") {
        logger->set_level(spdlog::level::info);
    }
    else if (loglevel == "warn") {
        logger->set_level(spdlog::level::warn);
    }
    else {
        logger->set_level(spdlog::level::err);
    }

}


////! @brief シェープファイル名を取得
////! 
////! @param zfid メッシュID
////! @param polygonType タイプ
////! 
////! @return 取得結果
//std::string Config::getShapeFileName(const std::string_view& zfid, PolygonType polygonType) {
//    std::string shapeFileName(zfid);
//    shapeFileName += std::filesystem::path::preferred_separator;
//    shapeFileName += zfid;
//    if (polygonType == TYPE_POWERTOWER) {
//        shapeFileName += "_L_TOWN_BUILDING.shp"; // std::regex_replace(shapeFileSuffix, std::regex("_P"), "_L") + ".shp";
//    }
//    else if (polygonType == TYPE_WATER) {
//        shapeFileName += "_P_AREA_WATER.shp";
//    }
//    else if (polygonType == TYPE_ROAD) {
//        shapeFileName += "_P_TOWN_ROAD.shp";
//    }
//    else if (polygonType == TYPE_RAILWAY) {
//        shapeFileName += "_L_RAIL_ROAD.shp";
//    }
//    else {
//        shapeFileName += "_P_TOWN_BUILDING.shp"; //"shapeFileSuffix + ".shp";
//    }
//    return CommonUtil::pathJoin(CommonUtil::pathJoin(mapRoot, map10kDir), shapeFileName);
//}
//
//! @brief cityGmlファイル名を取得
//! 
//! @return 取得結果
std::string Config::getCityGmlFileFolder() {
    //std::string result = CommonUtil::pathJoin(pointInRoot, batchFolder);

    //result = CommonUtil::pathJoin(result, inputDir);

    //return result;
    return getCityGmlFileFolder(batchFolder);
}

std::string Config::getCityGmlFileFolder(std::string & exBatchFolder) {
    std::string result = CommonUtil::pathJoin(citygmlInRoot, exBatchFolder);

    result = CommonUtil::pathJoin(result, inputDir);

    return result;
}


//! @brief 出力フォルダ(output)のフルパスを取得
//! 
//! @return 取得結果
std::string Config::getOutputFolder() {
    std::string result = CommonUtil::pathJoin(citygmlOutRoot, batchFolder);

    result = CommonUtil::pathJoin(result, outputDir);

    return result;
}


////! @brief マッチング結果を出力するためのファイル名を取得
////! 
////! @param baseOutputFileName 出力ファイル名のベース（空間ID）
////! @param isTower trueなら鉄塔
////! @return 取得結果
//std::string Config::getOutputFileName(const std::string_view& baseOutputFileName, PolygonType polygonType) {
//    std::string outputFileName(baseOutputFileName);
//    
//   
//    if (polygonType == TYPE_ROAD) {
//        outputFileName += "_ROAD";
//    }
//    else {
//        outputFileName += "_BUILDING";
//    }
//
//    //if (outputFormat == FileFormat::FMT_CSV)
//    //    outputFileName += ".csv";
//    //else if (outputFormat == FileFormat::FMT_GEOJSON)
//    //    outputFileName += ".geojson";
//    //else
//        outputFileName += ".csv";
//
//    //std::string result = CommonUtil::pathJoin(pointRoot, batchFolder);
//
//    //if (isTower) {
//    //    result = CommonUtil::pathJoin(result, outputSteelTowerDir);
//    //}
//    //else {
//    //    result = CommonUtil::pathJoin(result, outputDir);
//    //}
//    std::string result = getOutputFolder();
//
//    if (!std::filesystem::exists(result)) {
//        bool rc = CommonUtil::forceDirectories(result);
//        if (!rc) {
//            return "";
//        }
//    }
//
//    return CommonUtil::pathJoin(result, outputFileName);
//    
//}
//

};