//! @file ClearVoxel.cpp
//! @brief メイン
//!
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cassert>

#include <fstream>

#include "../commonLib/CommonConfig.h"
#include "../commonLib/CommonUtil.h"
#include "../commonLib/SpatialUtil.h"
#include "../commonLib/SolutionCommonDefs.h"

#include "Arguments.h"


#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"

#define APP_ID "MakeVoxel"
#define APP_ID_LOG "ClearVoxel"
#define LOGGER_NAME "ClearVoxel_applog"
#define COMMON_CONFIG_ID "commonConfig.json"


//! @brief メイン関数
//! 
//! @param argc 引数の数
//! @param **argv 引数が格納されている配列
int	main(int argc, char* argv[])
{
	//argumetsを取得する
	Arguments argument;
	argument.loadArguments(argc, argv);

	std::string argBatchId = argument.getBatchId();
	std::string argConfig = argument.getConfig();

	//起動引数をチェックする
	if (argBatchId == "") {
		std::cout << "第一引数でバッチIDを指定してください" << std::endl;
		return 9;
	}

	CommonConfig commonConfig;

	//configを取得する
	std::string configPath = COMMON_CONFIG_ID;
	if (argConfig != "") {
		configPath = argConfig;
	}
	commonConfig.loadConfig(configPath);

	//loggerを準備する
	commonConfig.prepareLogger(APP_ID_LOG, LOGGER_NAME);
	auto logger = spdlog::get(LOGGER_NAME);

	//出力フォルダを取得する
	
	std::string	outputPath = commonConfig.getDir(APP_ID, "output", argBatchId);

	logger->info("<<<<<<<<<<<<< ClearVoxel app start >>>>>>>>>>>>");

	logger->info("argBatchId :  {:s}", argBatchId);
	logger->info("argConfig :  {:s}", argConfig);
	logger->info("configFile  : {:s}", configPath);
	logger->info("deletePath  : {:s}", outputPath);
	logger->info("----------------------");

	try {		
		std::filesystem::remove_all(outputPath);
	}
	catch (std::filesystem::filesystem_error& ex) {
		logger->error(" {:s}　が削除出来ません！ {:s}" , outputPath, ex.what());
		return 9;
	}
	catch (...) {
		logger->error("unexpected error");
		return 9;
	}

	logger->info("<<<<<<<<<<<<< ClearVoxel app end >>>>>>>>>>>>");
	return 0;

}