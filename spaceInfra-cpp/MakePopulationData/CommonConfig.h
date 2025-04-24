#pragma once

//! @file Config.h
//! @brief 共通config管理クラス　ヘッダー

#include <string>
#include <vector>
#include "spdlog/spdlog.h"
#include "../commonLib/SolutionCommonDefs.h"

//! @brief 共通config管理クラス
class CommonConfig {

	//common 環境依存定義
	std::string ownerId = "";
	std::string projectRoot = "";
	std::string execRoot = "";
	std::string dataRoot = "";
	std::string pointRoot = "";
	std::string mapRoot = "";
	std::string logRoot = "";
	std::string logsink = "console";
	std::string loglevel = "info";
	int colorByte = 2;
	int decimalPoint = 99;

	std::string pointInLasEpsg = "";
	std::string pointCsvEpsg = "";

	std::string btDbHost = "";
	std::string btDbName = "";
	std::string btDbUser = "";
	std::string btDbPass = "";


	//makePopulationData
	std::string input = "";
	std::string inputDrive = "";
	std::string inputRoot = "";
	std::string output = "";
	std::string outputDrive = "";
	std::string outputRoot = "";
	std::string outputIndex = "";
	std::string indexDrive = "";
	std::string indexRoot = "";
	std::string outputFormat = "";

private:



public:
	CommonConfig() {
	
	}

	void loadConfig(std::string configFileName);

	void prepareLogger(std::string appId , std::string loggerName);

	//IOディレクトリ取得
	std::string getDir(std::string appId, std::string ioId, std::string batId);

	//app>common アプリ共通定義

	std::string getOwnerId() {
		return ownerId;
	}
	std::string getProjectRoot() {
		return projectRoot;
	}
	std::string getExecRoot() {
		return execRoot;
	}
	std::string getDataRoot() {
		return dataRoot;
	}
	std::string getPointRoot() {
		return pointRoot;
	}
	std::string getMapRoot() {
		return mapRoot;
	}
	std::string getLogRoot() {
		return logRoot;
	}
	std::string getLogsink() {
		return logsink;
	}
	std::string getLoglevel() {
		return loglevel;
	}
	int getColorByte() {
		if (colorByte == 1 || colorByte == 2) {
			return colorByte;
		}
		else {
			return 2;
		}
	}
	int getDecimalPoint() {
		if (decimalPoint == 1 || decimalPoint == 2 || decimalPoint == 3 || decimalPoint == 4) {
			return decimalPoint;
		}
		else {
			return 99;
		}
	}
	void setPointInLasEpsg(std::string espg) {
		pointInLasEpsg = espg;
	}
	void setPointCsvEpsg(std::string espg) {
		pointCsvEpsg = espg;
	}
	std::string getPointInLasEpsg() {
		return pointInLasEpsg;
	}
	std::string getPointCsvEpsg() {
		return pointCsvEpsg;
	}
	std::string getBtDbHost() {
		return btDbHost;
	}
	std::string getBtDbName() {
		return btDbName;
	}
	std::string getBtDbUser() {
		return btDbUser;
	}
	std::string getBtDbPass() {
		return btDbPass;
	}

	//makePopulationData
	std::string getinput() {
		return input;
	}
	std::string getOutput() {
		return output;
	}
	std::string getOutputIndex() {
		return outputIndex;
	}
	std::string getOutputFormat() {
		return outputFormat;
	}

};

