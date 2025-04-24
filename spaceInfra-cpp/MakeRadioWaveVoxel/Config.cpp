//! @file Config.cpp
//! @brief 電波情報のボクセル化に関する設定クラス


#include "Config.h"
#include <fstream>
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <stdexcept>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include "CommonUtil.h"

namespace fs = std::filesystem;

//! @brief 現在の日時を YYYYMMDDHHMMSS 形式で取得
std::string Config::getCurrentDateTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &time);

    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y%m%d%H%M%S");
    return ss.str();
}


//! @brief パス中のバッチ日時プレースホルダを現在の日時に置換
std::string Config::replaceBatchDateTime(const std::string& path) const {
    std::string result = path;
    std::string placeholder = "<batchDatetime>";
    size_t pos = result.find(placeholder);

    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), getCurrentDateTime());
    }

    return result;
}


void Config::loadConfig(std::string configFileName) {

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
        if (commonDict.HasMember("confirmRoot")) {
            confirmRoot = commonDict["confirmRoot"].GetString();
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
        if (commonDict.HasMember("btUserDbName")) {
            btUserDbName = commonDict["btUserDbName"].GetString();
        }
        if (commonDict.HasMember("btUserDbUser")) {
            btUserDbUser = commonDict["btUserDbUser"].GetString();
        }
        if (commonDict.HasMember("btUserDbPass")) {
            btUserDbPass = commonDict["btUserDbPass"].GetString();
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
				confirmLasEpsg = appCommonDict["confirmLasEpsg"].GetString();
			}
            if (appCommonDict.HasMember("fromDatetime")) {
				fromDatetime = appCommonDict["fromDatetime"].GetString();
            }
            if (appCommonDict.HasMember("toDatetime")) {
                toDatetime = appCommonDict["toDatetime"].GetString();
            }
        }
        else {
            printf("\r\ncommonConfig : app/common not exist\r\n");
        }

		//app/MakeRadioWaveVoxel
        if (appDict.HasMember("MakeRadioWaveVoxel")) {
            auto& makeRadioWaveVoxelDict = appDict["MakeRadioWaveVoxel"];

			if (makeRadioWaveVoxelDict.HasMember("color")) {
				makeRadioWaveVoxelColor = makeRadioWaveVoxelDict["color"].GetString();
			}

            if (makeRadioWaveVoxelDict.HasMember("mapSegPointStartLng")) {
				mapSegPointStartLng = makeRadioWaveVoxelDict["mapSegPointStartLng"].GetDouble();
            }
			if (makeRadioWaveVoxelDict.HasMember("mapSegPointStartLat")) {
				mapSegPointStartLat = makeRadioWaveVoxelDict["mapSegPointStartLat"].GetDouble();
			}
			if (makeRadioWaveVoxelDict.HasMember("mapSegPointEndLng")) {
				mapSegPointEndLng = makeRadioWaveVoxelDict["mapSegPointEndLng"].GetDouble();
			}
			if (makeRadioWaveVoxelDict.HasMember("mapSegPointEndLat")) {
				mapSegPointEndLat = makeRadioWaveVoxelDict["mapSegPointEndLat"].GetDouble();
			}

            //app/MakeRadioWaveVoxel/pointBatchPath
            if (makeRadioWaveVoxelDict.HasMember("pointBatchPath")) {
                auto& pointBatchPathDict = makeRadioWaveVoxelDict["pointBatchPath"];
                if (pointBatchPathDict.HasMember("input")) {
                    makeRadioWaveVoxelInput = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "");
                    makeRadioWaveVoxelInputDrive = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "drive");
                    makeRadioWaveVoxelInputRoot = CommonUtil::getParam(pointBatchPathDict["input"].GetString(), "root");
                }
                if (pointBatchPathDict.HasMember("output")) {
                    makeRadioWaveVoxelOutput = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "");
                    makeRadioWaveVoxelOutputDrive = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "drive");
                    makeRadioWaveVoxelOutputRoot = CommonUtil::getParam(pointBatchPathDict["output"].GetString(), "root");
                }
            }
        }
        else {
            printf("\r\ncommonConfig : app/MakeRadioWaveVoxel not exist\r\n");
        }
    }
    else {
        printf("\r\ncommonConfig : app not exist\r\n");
    }
}

std::string Config::getLoggerName() const {
    return appId;
}

std::string Config::getDbHost() const {
	return btDbHost;
}

std::string Config::getUserDbHost() const {
    return btUserDbHost;
}

std::string Config::getUserDbName() const {
	return btUserDbName;
}

std::string Config::getDbUser() const {
	return btUserDbUser;
}

std::string Config::getDbPass() const {
	return btUserDbPass;
}


//! @brief パスに含まれる<batchDatetime>を実行時日時に置換する
std::string replaceBatchDatetime(std::string input) {
	// 現在の日時を取得
    std::time_t now = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &now);

    // 日次文字列を生成
    char buffer[15];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &tm);
    std::string datetime(buffer);

    // 文字列置換
    size_t pos = input.find("<batchDatetime>");
    if (pos != std::string::npos) {
        input.replace(pos, 15, datetime);
    }

    return input;
}


std::string Config::getConfirmLasEpsg() const {
	return confirmLasEpsg;
}


std::string Config::getFromDatetime() const {
	return fromDatetime;
}


std::string Config::getToDatetime() const {
	return toDatetime;
}


std::string Config::getMakeRadioWaveVoxelColor() const {
	return makeRadioWaveVoxelColor;
}

std::string Config::getDir(std::string app_id, std::string ioId, std::string batId) {
    if (appId == "MakeRadioWaveVoxel") {
        if (ioId == "input") {
            if (makeRadioWaveVoxelInputDrive != "") {
                return CommonUtil::changeDrive(pointRoot, makeRadioWaveVoxelInputDrive) + "/" + batId + "/" + makeRadioWaveVoxelInput;
            }
            else if (makeRadioWaveVoxelInputRoot != "") {
                return makeRadioWaveVoxelInputRoot + "/" + batId + "/" + makeRadioWaveVoxelInput;
            }
            else {
                return pointRoot + "/" + batId + "/" + makeRadioWaveVoxelInput;
            }
        }
        else if (ioId == "output") {
            // makeRadioWaveVoxelOutputの"<batchDatetime>"の部分を実行時の日時"yyyyMMddHHmmss"に置換
            makeRadioWaveVoxelOutput = replaceBatchDatetime(makeRadioWaveVoxelOutput);
            if (makeRadioWaveVoxelOutputDrive != "") {				
                // TODO MakeRadioWaveVoxel.pointBatchPath.output driveの指定がある場合の処理
                return CommonUtil::changeDrive(confirmRoot, makeRadioWaveVoxelOutputDrive);
            }
            else {
                return confirmRoot;
            }
        }
    }
}


//! @brief 出力パスの設定
void Config::processOutputPath(const std::string& output_relative_path) {
	// 出力パスの設定
    std::string processed_path = replaceBatchDateTime(output_relative_path);

    size_t pos = processed_path.find(",drive=");
    if (pos != std::string::npos) {
        std::string drive = processed_path.substr(pos + 7, 1) + ":/";
        std::string relative_path = processed_path.substr(0, pos);
        makeRadioWaveVoxelOutput = drive + relative_path;
    }
    else {
        makeRadioWaveVoxelOutput = confirmRoot + "/" + processed_path;
    }

	// 出力ディレクトリが存在しない場合は作成
    try {
		std::cout << "Output directory: " << makeRadioWaveVoxelOutput << std::endl;

        if (!fs::exists(makeRadioWaveVoxelOutput)) {
            fs::create_directories(makeRadioWaveVoxelOutput);
        }
    }
    catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to create output directory: " + std::string(e.what()));
    }
}


std::string Config::getRadioWaveVoxelOutputFormat() {
	return makeRadioWaveVoxelOutputFormat;
}


//! @brief Loggerの設定
void Config::prepareLogger(std::string appId, std::string loggerName) {
    this->appId = appId;

    std::shared_ptr<spdlog::logger> logger;


    if (logsink != "console" && logsink != "stdout") {
        std::string dest = logRoot;
        dest = dest + "/" + appId;
        std::filesystem::create_directories(dest);

        if (logsink == "basic") {
            dest = dest + "/" + appId + ".log";
            logger = spdlog::basic_logger_mt(loggerName, dest);
        }
        else { //daily
            dest = dest + "/" + appId + "-%Y-%m-%d.log";
            logger = spdlog::daily_logger_format_mt(loggerName, dest, 0, 0); //0��0��
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

std::string Config::getInputFolder() const {
    return makeRadioWaveVoxelInput;
}

std::string Config::getOutputFilePath() const {
    return makeRadioWaveVoxelOutput;
}

double Config::getMapSegPointStartLng() const {
	return mapSegPointStartLng;
}

double Config::getMapSegPointStartLat() const {
	return mapSegPointStartLat;
}

double Config::getMapSegPointEndLng() const {
	return mapSegPointEndLng;
}

double Config::getMapSegPointEndLat() const {
	return mapSegPointEndLat;
}