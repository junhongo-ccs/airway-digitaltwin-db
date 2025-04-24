//! @file CommonConfig.cpp
//! @brief 共通config管理クラス

#include <fstream>
#include <cassert>
#include <string>
#include <map>
#include "commonConfig.h"
#include "../commonLib/CommonUtil.h"
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "argparse/argparse.hpp"


//! @brief 設定を指定ファイルからロードする
//! 
//! @param configFileName 設定ファイル
void CommonConfig::loadConfig(std::string configFileName) {

    std::ifstream ifs(configFileName);
    rapidjson::IStreamWrapper iws(ifs);

    rapidjson::Document doc;
    doc.ParseStream(iws);

    //common
    if (doc.HasMember("common")) {
        auto& commonDict = doc["common"];

        if (commonDict.HasMember("pointRoot")) {
            pointRoot = commonDict["pointRoot"].GetString();
        }
        if (commonDict.HasMember("logRoot")) {
            logRoot = commonDict["logRoot"].GetString();
        }
        if (commonDict.HasMember("logsink")) {
            logsink = commonDict["logsink"].GetString();
            if (commonDict.HasMember("loglevel")) {
                loglevel = commonDict["loglevel"].GetString();
            }
        }
        if (commonDict.HasMember("btUserDbHost")) {
            btDbHost = commonDict["btUserDbHost"].GetString();
        }
        if (commonDict.HasMember("btUserDbName")) {
            btDbName = commonDict["btUserDbName"].GetString();
        }
        if (commonDict.HasMember("btUserDbUser")) {
            btDbUser = commonDict["btUserDbUser"].GetString();
        }
        if (commonDict.HasMember("btUserDbPass")) {
            btDbPass = commonDict["btUserDbPass"].GetString();
        }
    }
    else {
        printf("\r\ncommonConfig : common not exist\r\n");
    }

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];
        //app/makePopulationData
        if (appDict.HasMember("MakePopulationData")) {
            auto& detailDict = appDict["MakePopulationData"];

            //app/makePopulationData/pointBatchPath
            if (detailDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = detailDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    input = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    inputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    inputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    output = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    outputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    outputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
            }
            else {
                printf("\r\ncommonConfig : app/MakePopulationData/pointBatchPath not exist\r\n");
            }
        }
        else {
            printf("\r\ncommonConfig : app/MakePopulationData not exist\r\n");
        }

    }
    else {
        printf("\r\ncommonConfig : app not exist\r\n");
    }

}

std::string CommonConfig::getDir(std::string appId, std::string ioId, std::string batId) {

    if (appId == "MakePopulationData") {
        if (ioId == "input") {
            if (inputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, inputDrive) + "/" + batId + "/"  + input;
            }
            else if (inputRoot != "") {
                return inputRoot + "/" + batId + "/" + input;
            }
            else {
                return pointRoot + "/" + batId + "/" + input;
            }
        }
        else if (ioId == "output") {
            if (outputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, outputDrive) + "/" + output;
            }
            else if (outputRoot != "") {
                return outputRoot  + "/" + batId + "/" + output;
            }
            else {
                return pointRoot + "/" + batId + "/" + output;
            }
        }
    }
    else {

        printf("\r\nappId error %s", appId.c_str());
        return "";
    }
}

//! @brief Loggerを準備する
void CommonConfig::prepareLogger(std::string appId , std::string loggerName) {
    //----------------------------------------------
    //■ログについてはとりあえず以下のように実装（spdlogger利用）
    //・3パターンのsinkを用意
    //     ①console →　画面に出力
    //     ②basic  →　日付なしのファイルを作成。例（pointmatch.log)
    //     ③daily  →　日付ありのファイルを作成。例（pointmatch-2023-04-06.log)。
    //・フォルダ名は固定。ただsinkがconsoleの場合、ファイル出力を行わない。※リダイレクトでファイルに落とせる
    //・古いログファイルを移動するためのbatファイルを別途作成。たとえば
    //     、{logroot}\pointmatch\pointmatch-2023-04-06.logを{logroot}\history\pointmatch\の下に移動させるとか。
    //     例：PointMatfchの場合、{logroot}\pointmatch\の下にログを作成する
    //
    // ※動的な情報についてはフォルダ名やファイル名に入れるのではなく、ログの中に書き出すようにする
    // ※ログファイルに時刻を入れない
    // ※古いログファイルを退避させるbatの作成は優先順位が少し低いと思われるので、あとでもいい（？）
    // ※アプリを平行に複数個実行させる必要がある場合、出力される各行にプロセスIDもしくはスレッドIDも出力したほうがいい
    //   PointMatch.exeについては内部マルチスレッドにしているので、exeを複数実行させると、逆に遅くなるパターンもあるかもしれない

    std::shared_ptr<spdlog::logger> logger;


    if (logsink != "console" && logsink != "stdout") {
        std::string dest = logRoot;
        dest = dest + "/" + appId;
        CommonUtil::forceDirectories(dest);

        if (logsink == "basic") {
            dest = dest + "/" + appId + ".log";
            logger = spdlog::basic_logger_mt(loggerName, dest);
        }
        else { //daily
            dest = dest + "/" + appId + "-%Y-%m-%d.log";
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
