//! @file MakeRadioWaveVoxel.cpp
//! @brief 電波情報ボクセル変換
//!
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "voxelWrite.h"
#include "Config.h"
#include "../commonLib/CommonUtil.h"
#include "../commonLib/SpatialUtil.h"
#include "../commonLib/SolutionCommonDefs.h"
#include "../ComLib/CommonUtil.h"
#include "CoordSegmentation.h"
#include <filesystem>

#include "Arguments.h"

#include <fstream>
#include <string>
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"
#include <spdlog/spdlog.h>
#include "proj.h"

#define APP_ID "MakeRadioWaveVoxel"
#define LOGGER_NAME "MakeRadioWaveVoxel_applog"
#define COMMON_CONFIG_ID "commonConfig-rpcRadioWave.json"

//! @brief MakeRadioWaveVoxel メイン関数
int	main(int argc, char* argv[])
{
	Arguments argument;
	argument.loadArguments(argc, argv);

	std::string argBatchId = argument.getBatchId();
	std::string argConfig = argument.getConfig();

	if (argBatchId == "") {
		std::cout << "第一引数でバッチIDを指定してください" << std::endl;
		return 9;
	}

    Config& config = Config::getInstance();

	// バッチ処理日時
	std::string batchDateTime = ComLib::CommonUtil::getCurDatetime();

	// configパスの取得
	std::string configPath = COMMON_CONFIG_ID;
	if (argConfig != "") {
		configPath = argConfig;
	}
	config.loadConfig(configPath);

	// loggerの設定
	config.prepareLogger(APP_ID, LOGGER_NAME);
	auto logger = spdlog::get(LOGGER_NAME);

	// 入力パス、出力パスの設定
	std::string	inputPath = config.getDir(APP_ID, "input", argBatchId);
	std::string	outputPath = config.getDir(APP_ID, "output", argBatchId);


	logger->info("<<<<<<<<<<<<< MakeRadioWaveVoxel app start >>>>>>>>>>>>");

	logger->info("argBatchId :  {:s}", argBatchId);
	logger->info("argConfig :  {:s}", argConfig);
	logger->info("configFile  : {:s}", configPath);
	logger->info("InputPath  : {:s}", inputPath);
	logger->info("outputPath  : {:s}", outputPath);
	logger->info("----------------------");

	struct stat dirCheck;
	if (stat(outputPath.c_str(), &dirCheck) != 0) {
		bool endStatus = std::filesystem::create_directories(outputPath);
		if (!endStatus) {
			logger->error("Can't create path: {}", outputPath);
			return 9;
		}
	}

	try {
		// std::string lasFilePtn = "^[0-9]+-(-)?[0-9]+-[0-9]+-[0-9].+\\.las$";
        std::string txtFilePtn = ".*UTM[0-9]{1,2}[NS]_800_[0-9]+\\.txt$";
		std::vector<std::string> txtFileList = CommonUtil::getFileList(inputPath, txtFilePtn);

		logger->info("txtFileList size  :{:d}", txtFileList.size());
		logger->info("----------------------");

		for (std::string inputFileId : txtFileList) {
			int zone = 0;
			bool isNorthern = true;
			double height = 0.0;

			logger->info("{:s}  start", inputFileId);
			// 入力ファイルパス
			std::string inputFileIdFull = inputPath + "/" + inputFileId;
			int rtn_code = 0;

			// UTMファイル名から高さを取得
			std::regex re(".*UTM[0-9]{1,2}[NS]_800_[0-9]+\\.txt$");
			std::smatch match;
			if (std::regex_search(inputFileId, match, re)) {
				std::string heightStr = match.str();
				height = std::stod(heightStr.substr(heightStr.find_last_of('_') + 1, heightStr.find_last_of('.') - heightStr.find_last_of('_') - 1));
			}
			// UTMファイル名からUTMゾーンと北緯か南緯かを取得
			std::regex re2(".*UTM([0-9]{1,2})([NS])_800_[0-9]+\\.txt$");
			std::smatch match2;
			if (std::regex_search(inputFileId, match2, re2)) {
				std::string zoneStr = match2.str(1);
				std::string nsStr = match2.str(2);
				zone = std::stoi(zoneStr);
				isNorthern = (nsStr == "N");
			}

			std::string confirmLasEpsg = config.getConfirmLasEpsg();
			std::string makeRadioWaveVoxelColor = config.getMakeRadioWaveVoxelColor();
			VoxelWrite voxelWrite = VoxelWrite(zone, isNorthern, height, confirmLasEpsg, makeRadioWaveVoxelColor);
			
			// CoordSegmentationによる入力ファイルの指定エリアの裁断
			std::string clippedOutputFile = outputPath + "/clipped_" + inputFileId;
			// CoordSegmentationのインスタンス化
			CoordSegmentation coordSeg = CoordSegmentation(zone, isNorthern, confirmLasEpsg);
			try {
				coordSeg.run(inputFileIdFull, clippedOutputFile);
				logger->info("Clipping completed for {:s}", inputFileIdFull);
			}
			catch (const std::exception& ex) {
				logger->error("Clipping failed for {:s}: {}", inputFileIdFull, ex.what());
				// クリップでエラーが起きた場合は次のファイルへ
				continue;
			}

			// UTMファイルからボクセルデータを作成
            rtn_code = voxelWrite.writeVoxel(clippedOutputFile, outputPath, *logger, batchDateTime);
			
			switch (rtn_code) {
			case 1:
				logger->error("Can not open input file!! {:s}", inputFileIdFull);
				throw EXIT_FAILURE;
				break;
			case 2:
				logger->error(" Err!! SpatialID unmatch!! {:s}", inputFileIdFull);
				throw EXIT_FAILURE;
				break;
			case 3:
				logger->error("Can not open output file!! {:s}", outputPath);
				throw EXIT_FAILURE;
			}
			logger->info("{:s} output end :", outputPath);

			// 裁断データの削除
			std::remove(clippedOutputFile.c_str());
		}
	}
	catch (std::bad_alloc& ex) {
		logger->error("bad_alloc");
		return 9;
	}
	catch (std::runtime_error& ex) {
		logger->error(ex.what());
		return 9;
	}
	catch (...) {
		logger->error("unexpected error");
		return 9;
	}
	logger->info("<<<<<<<<<<<<< MakeRadioWaveVoxel app end >>>>>>>>>>>>");
	return 0;

}