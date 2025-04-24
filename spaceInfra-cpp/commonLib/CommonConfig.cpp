//! @file CommonConfig.cpp
//! @brief 共通config管理クラス

#include <fstream>
#include <cassert>
#include <string>
#include <map>
#include "commonConfig.h"
#include "commonUtil.h"
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

        if (commonDict.HasMember("ownerId")) {
            ownerId = commonDict["ownerId"].GetString();
        }
        if (commonDict.HasMember("projectRoot")) {
            projectRoot = commonDict["projectRoot"].GetString();
        }
        if (commonDict.HasMember("execRoot")) {
            execRoot = commonDict["execRoot"].GetString();
        }
        if (commonDict.HasMember("dataRoot")) {
            dataRoot = commonDict["dataRoot"].GetString();
        }
        if (commonDict.HasMember("pointRoot")) {
            pointRoot = commonDict["pointRoot"].GetString();
        }
        if (commonDict.HasMember("mapRoot")) {
            mapRoot = commonDict["mapRoot"].GetString();
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
        if (commonDict.HasMember("colorByte")) {
            colorByte = commonDict["colorByte"].GetInt();
        }
        if (commonDict.HasMember("decimalPoint")) {
            decimalPoint = commonDict["decimalPoint"].GetInt();
        }
        if (commonDict.HasMember("btDbHost")) {
            btDbHost = commonDict["btDbHost"].GetString();
        }
        if (commonDict.HasMember("btDbName")) {
            btDbName = commonDict["btDbName"].GetString();
        }
        if (commonDict.HasMember("btDbUser")) {
            btDbUser = commonDict["btDbUser"].GetString();
        }
        if (commonDict.HasMember("btDbPass")) {
            btDbPass = commonDict["btDbPass"].GetString();
        }
    }
    else {
        printf("\r\ncommonConfig : common not exist\r\n");
    }

    //app
    if (doc.HasMember("app")) {
        auto& appDict = doc["app"];

        //app/common
        if (appDict.HasMember("common")) {
            auto& appCommonDict = appDict["common"];
            if (appCommonDict.HasMember("mapEpsg")) {
                mapEpsg = appCommonDict["mapEpsg"].GetString();
            }
            if (appCommonDict.HasMember("pointInLasEpsg")) {
                pointInLasEpsg = appCommonDict["pointInLasEpsg"].GetString();
            }
            if (appCommonDict.HasMember("pointCsvEpsg")) {
                pointCsvEpsg = appCommonDict["pointCsvEpsg"].GetString();
            }
            if (appCommonDict.HasMember("pointOutLasEpsg")) {
                pointOutLasEpsg = appCommonDict["pointOutLasEpsg"].GetString();
            }
            if (appCommonDict.HasMember("confirmLasEpsg")) {
                pointConfirmLasEpsg = appCommonDict["confirmLasEpsg"].GetString();
            }

        }
        else {
            printf("\r\ncommonConfig : app/common not exist\r\n");
        }

        //app/Las2Csvsp
        if (appDict.HasMember("Las2Csvsp")) {
            auto& las2CsvsphDict = appDict["Las2Csvsp"];

            //app/PoonitMatch/pointBatchPath
            if (las2CsvsphDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = las2CsvsphDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    las2csvspInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    las2csvspInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    las2csvspInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    las2csvspOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    las2csvspOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    las2csvspOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("outputIndex")) {
                    las2csvspOutputIndex = CommonUtil::getParam(pointBatchPathDict["outputIndex"].GetString(), "");
                    las2csvspOutputIndexDrive = CommonUtil::getParam(pointBatchPathDict["outputIndex"].GetString(), "drive");
                    las2csvspOutputIndexRoot = CommonUtil::getParam(pointBatchPathDict["outputIndex"].GetString(), "root");
                }
            }
            else {
                printf("\r\ncommonConfig : app/Las2csvsp/pointBatchPath not exist\r\n");
            }
            if (las2CsvsphDict.HasMember("isLandCutLas")) {
                isLandCutLas = las2CsvsphDict["isLandCutLas"].GetBool();
            }
            if (las2CsvsphDict.HasMember("outputFormat")) {
                las2csvspOutputFormat = las2CsvsphDict["outputFormat"].GetString();
            }
            if (las2CsvsphDict.HasMember("threadCount")) {
                las2csvspThreadCount = las2CsvsphDict["threadCount"].GetInt();
            }
            if (las2CsvsphDict.HasMember("threadCountForOutput")) {
                las2csvspThreadCountForOutput = las2CsvsphDict["threadCountForOutput"].GetInt();
            }
        }
        else {
            printf("\r\ncommonConfig : app/Las2Csvsp not exist\r\n");
        }

        //app/Csvsp2Space
        if (appDict.HasMember("Csvsp2Space")) {
            auto& csvsp2SpaceDict = appDict["Csvsp2Space"];

            //app/Csvsp2Space/pointBatchPath
            if (csvsp2SpaceDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = csvsp2SpaceDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    csvsp2spaceInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    csvsp2spaceInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    csvsp2spaceInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    csvsp2spaceOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    csvsp2spaceOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    csvsp2spaceOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("inputIndex")) {
                    csvsp2spaceInputIndex = CommonUtil::getParam(pointBatchPathDict["inputIndex"].GetString(), "");
                    csvsp2spaceInputIndexDrive = CommonUtil::getParam(pointBatchPathDict["inputIndex"].GetString(), "drive");
                    csvsp2spaceInputIndexRoot = CommonUtil::getParam(pointBatchPathDict["inputIndex"].GetString(), "root");
                }

            }

            if (csvsp2SpaceDict.HasMember("pointMatchOutType")) {
                pointMatchOutType = csvsp2SpaceDict["pointMatchOutType"].GetString();
            }
        }
        else {
            printf("\r\ncommonConfig : app/Csvsp2Space not exist\r\n");
        }

        //app/PointMatch
        if (appDict.HasMember("PointMatch")) {
            auto& pointMatchDict = appDict["PointMatch"];

            //app/PoonitMatch/pointBatchPath
            if (pointMatchDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = pointMatchDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    pointMatchInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    pointMatchInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    pointMatchInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    pointMatchOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    pointMatchOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    pointMatchOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("outputSteelTower")) {
                    pointMatchOutputSteelTower = CommonUtil::getParam(pointBatchPathDict["outputSteelTower"].GetString(), "");
                    pointMatchOutputSteelTowerDrive = CommonUtil::getParam(pointBatchPathDict["outputSteelTower"].GetString(), "drive");
                    pointMatchOutputSteelTowerRoot = CommonUtil::getParam(pointBatchPathDict["outputSteelTower"].GetString(), "root");
                }
            }

            if (pointMatchDict.HasMember("shapeFileSuffix")) {
                shapeFileSuffix = pointMatchDict["shapeFileSuffix"].GetString();
            }

            if (pointMatchDict.HasMember("noExpandMode")) {
                noExpandMode = pointMatchDict["noExpandMode"].GetBool();
            }

            if (pointMatchDict.HasMember("outPolygon")) {
                outPolygon = pointMatchDict["outPolygon"].GetInt();
            }

            if (pointMatchDict.HasMember("threadCount")) {
                threadCount = pointMatchDict["threadCount"].GetInt();
                if (threadCount <= 0) {
                    threadCount = 1;
                }
            }

        }
        else {
            printf("\r\ncommonConfig : app/PointMatch not exist\r\n");
        }

        //app/MakeVoxel
        if (appDict.HasMember("MakeVoxel")) {
            auto& makeVoxelDict = appDict["MakeVoxel"];

            //app/MakeVoxel/pointBatchPath
            if (makeVoxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = makeVoxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("inputOriginal")) {
                    makeVoxelInput = CommonUtil::getParam(pointBatchPathDict["inputOriginal"].GetString(), "");
                    makeVoxelInputDrive = CommonUtil::getParam(pointBatchPathDict["inputOriginal"].GetString(), "drive");
                    makeVoxelInputRoot = CommonUtil::getParam(pointBatchPathDict["inputOriginal"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("inputProcessingResults")) {
                    makeVoxelInputProcessingResults = CommonUtil::getParam(pointBatchPathDict["inputProcessingResults"].GetString(), "");
                    makeVoxelInputProcessingResultsDrive = CommonUtil::getParam(pointBatchPathDict["inputProcessingResults"].GetString(), "drive");
                    makeVoxelInputProcessingResultsRoot = CommonUtil::getParam(pointBatchPathDict["inputProcessingResults"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    makeVoxelOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    makeVoxelOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    makeVoxelOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
            }
            if (makeVoxelDict.HasMember("outputFormat")) {
                makeVoxelOutputFormat = CommonUtil::getParam(makeVoxelDict["outputFormat"].GetString(), "");
                makeVoxelOutputLevel = CommonUtil::getParam(makeVoxelDict["outputFormat"].GetString(), "level");

            }
        }
        else {
            printf("\r\ncommonConfig : app/MakeVoxel not exist\r\n");
        }

        //app/MergeVoxel
        if (appDict.HasMember("MergeVoxel")) {
            auto& mergeVoxelDict = appDict["MergeVoxel"];

            //app/MergeVoxel/pointBatchPath
            if (mergeVoxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = mergeVoxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    mergeVoxelInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    mergeVoxelInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    mergeVoxelInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    mergeVoxelOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    mergeVoxelOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    mergeVoxelOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
            }
            if (mergeVoxelDict.HasMember("outputFormat")) {
                mergeVoxelOutputFormat = CommonUtil::getParam(mergeVoxelDict["outputFormat"].GetString(), "");
                mergeVoxelOutputLevel = CommonUtil::getParam(mergeVoxelDict["outputFormat"].GetString(), "level");

            }
        }
        else {
           // printf("\r\ncommonConfig : app/MergeVoxel not exist\r\n");
        }

        //app/MakeHash
        if (appDict.HasMember("MakeHash")) {
            auto& makeHashDict = appDict["MakeHash"];

            //app/MakeHash/pointBatchPath
            if (makeHashDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = makeHashDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    makeHashInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    makeHashInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    makeHashInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    makeHashOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    makeHashOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    makeHashOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
            }
            if (makeHashDict.HasMember("outputFormat")) {
                makeHashOutputFormat = CommonUtil::getParam(makeHashDict["outputFormat"].GetString(), "");

            }
        }
        else {
            printf("\r\ncommonConfig : app/MakeHash not exist\r\n");
        }
    }
    else {
        printf("\r\ncommonConfig : app not exist\r\n");
    }



}

std::string CommonConfig::getDir(std::string appId, std::string ioId, std::string batId) {

    if (appId == "Las2Csvsp") {
        if (ioId == "input") {
            if (las2csvspInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, las2csvspInputDrive) + "/" + batId + "/" + las2csvspInput;
            }
            else if (las2csvspInputRoot != "") {
                return las2csvspInputRoot + "/" + batId + "/" + las2csvspInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + las2csvspInput;
            }
        }
        else if (ioId == "output") {
            if (las2csvspOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, las2csvspOutputDrive) + "/" + batId + "/" + las2csvspOutput;
            }
            else if (las2csvspOutputRoot != "") {
                return las2csvspOutputRoot + "/" + batId + "/" + las2csvspOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + las2csvspOutput;
            }
        }
        else if (ioId == "outputIndex") {
            if (las2csvspOutputIndexDrive != "") {
                return CommonUtil::changeDrive(pointRoot, las2csvspOutputIndexDrive) + "/" + batId + "/" + las2csvspOutputIndex;
            }
            else if (las2csvspOutputIndexRoot != "") {
                return las2csvspOutputIndexRoot + "/" + batId + "/" + las2csvspOutputIndex;
            }
            else {
                return pointRoot + "/" + batId + "/" + las2csvspOutputIndex;
            }
        }
    }
    else if (appId == "Csvsp2Space") {
        if (ioId == "input") {
            if (csvsp2spaceInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, csvsp2spaceInputDrive) + "/" + batId + "/" + csvsp2spaceInput;
            }
            else if (csvsp2spaceInputRoot != "") {
                return csvsp2spaceInputRoot + "/" + batId + "/" + csvsp2spaceInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + csvsp2spaceInput;
            }
        }
        else if (ioId == "output") {
            if (csvsp2spaceOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, csvsp2spaceOutputDrive) + "/" + batId + "/" + csvsp2spaceOutput;
            }
            else if (csvsp2spaceOutputRoot != "") {
                return csvsp2spaceOutputRoot + "/" + batId + "/" + csvsp2spaceOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + csvsp2spaceOutput;
            }
        }
        else if (ioId == "inputIndex") {
            if (csvsp2spaceInputIndexDrive != "") {
                return CommonUtil::changeDrive(pointRoot, csvsp2spaceInputIndexDrive) + "/" + batId + "/" + csvsp2spaceInputIndex;
            }
            else if (csvsp2spaceInputIndexRoot != "") {
                return csvsp2spaceInputIndexRoot + "/" + batId + "/" + csvsp2spaceInputIndex;
            }
            else {
                return pointRoot + "/" + batId + "/" + csvsp2spaceInputIndex;
            }
        }
    }
    else if (appId == "PointMatch") {
        if (ioId == "input") {
            if (pointMatchInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, pointMatchInputDrive) + "/" + batId + "/" + pointMatchInput;
            }
            else if (pointMatchInputRoot != "") {
                return pointMatchInputRoot + "/" + batId + "/" + pointMatchInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + pointMatchInput;
            }
        }
        else if (ioId == "output") {
            if (pointMatchOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, pointMatchOutputDrive) + "/" + batId + "/" + pointMatchOutput;
            }
            else if (pointMatchOutputRoot != "") {
                return pointMatchOutputRoot + "/" + batId + "/" + pointMatchOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + pointMatchOutput;
            }
        }
        else if (ioId == "outputSteelTower") {
            if (pointMatchOutputSteelTowerDrive != "") {
                return CommonUtil::changeDrive(pointRoot, pointMatchOutputSteelTowerDrive) + "/" + batId + "/" + pointMatchOutputSteelTower;
            }
            else if (pointMatchOutputSteelTowerRoot != "") {
                return pointMatchOutputSteelTowerRoot + "/" + batId + "/" + pointMatchOutputSteelTower;
            }
            else {
                return pointRoot + "/" + batId + "/" + pointMatchOutputSteelTower;
            }
        }

    }
    else if (appId == "MakeVoxel") {
        if (ioId == "input") {
            if (makeVoxelInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeVoxelInputDrive) + "/" + batId + "/" + makeVoxelInput;
            }
            else if (makeVoxelInputRoot != "") {
                return makeVoxelInputRoot + "/" + batId + "/" + makeVoxelInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeVoxelInput;
            }
        }
        else if (ioId == "inputProcessingResults") {
            if (makeVoxelInputProcessingResultsDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeVoxelInputProcessingResultsDrive) + "/" + batId + "/" + makeVoxelInputProcessingResults;
            }
            else if (makeVoxelInputProcessingResultsRoot != "") {
                return makeVoxelInputProcessingResultsRoot + "/" + batId + "/" + makeVoxelInputProcessingResults;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeVoxelInputProcessingResults;
            }
        }
        else if (ioId == "output") {
            if (makeVoxelOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeVoxelOutputDrive) + "/" + batId + "/" + makeVoxelOutput;
            }
            else if (makeVoxelOutputRoot != "") {
                return makeVoxelOutputRoot + "/" + batId + "/" + makeVoxelOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeVoxelOutput;
            }
        }
    }
    else if (appId == "MergeVoxel") {
        if (ioId == "input") {
            if (mergeVoxelInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, mergeVoxelInputDrive) + "/" + batId + "/" + mergeVoxelInput;
            }
            else if (mergeVoxelInputRoot != "") {
                return mergeVoxelInputRoot + "/" + batId + "/" + mergeVoxelInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + mergeVoxelInput;
            }
        }
        else if (ioId == "output") {
            if (mergeVoxelOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, mergeVoxelOutputDrive) + "/" + batId + "/" + mergeVoxelOutput;
            }
            else if (mergeVoxelOutputRoot != "") {
                return mergeVoxelOutputRoot + "/" + batId + "/" + mergeVoxelOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + mergeVoxelOutput;
            }
        }
    }

    else if (appId == "MakeHash") {
        if (ioId == "input") {
            if (makeHashInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeHashInputDrive) + "/" + batId + "/" + makeHashInput;
            }
            else if (makeHashInputRoot != "") {
                return makeHashInputRoot + "/" + batId + "/" + makeHashInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeHashInput;
            }
        }
        else if (ioId == "output") {
            if (makeHashOutputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeHashOutputDrive) + "/" + batId + "/" + makeHashOutput;
            }
            else if (makeHashOutputRoot != "") {
                return makeHashOutputRoot + "/" + batId + "/" + makeHashOutput;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeHashOutput;
            }
        }
    }
    printf("\r\nappId error %s", appId.c_str());
    return "";
}

//! @brief Loggerを準備する
void CommonConfig::prepareLogger(std::string appId , std::string loggerName) {
    this->appId = appId;

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
