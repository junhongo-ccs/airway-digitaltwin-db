//! @file MakeVoxel.cpp
//! @brief メイン
//!
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "voxelWrite.h"
#include "../commonLib/CommonConfig.h"
#include "../commonLib/CommonUtil.h"
#include "../commonLib/SpatialUtil.h"
#include "../commonLib/SolutionCommonDefs.h"

#include "Arguments.h"

#include <fstream>
#include <string>
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"

#define APP_ID "MakeVoxel"
#define LOGGER_NAME "MakeVoxel_applog"
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
	std::string argBatchFile = argument.getBatchFile();
	std::string argConfig = argument.getConfig();
	std::string argEspg = argument.getEspg();
	std::string argEspgLas = argument.getEspgLas();
	std::string argEspgCsv = argument.getEspgCsv();
	std::string argInput = argument.getInput();


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
	commonConfig.prepareLogger(APP_ID, LOGGER_NAME);
	auto logger = spdlog::get(LOGGER_NAME);

	//アーギュメント指定でのconfigの置き換え
	if (argEspg != "") {
		commonConfig.setPointInLasEpsg(argEspg);
		commonConfig.setPointCsvEpsg(argEspg);
	}
	if (argEspgLas != "") {
		commonConfig.setPointInLasEpsg(argEspgLas);
	}
	if (argEspgCsv != "") {
		commonConfig.setPointCsvEpsg(argEspgCsv);
	}

	//入出力フォルダを取得する
	std::string	inputPath = commonConfig.getDir(APP_ID, "input", argBatchId);
	if (argInput == "inputProcessingResults") {
		inputPath = commonConfig.getDir(APP_ID, "inputProcessingResults", argBatchId);
	}
	std::string	outputPath = commonConfig.getDir(APP_ID, "output", argBatchId);
	std::string	lasEpsg = commonConfig.getPointConfirmLasEpsg();
	std::string	outputLevel = commonConfig.getMakeVoxelOutputLevel();


	logger->info("<<<<<<<<<<<<< MakeVoxel app start >>>>>>>>>>>>");

	logger->info("argBatchId :  {:s}", argBatchId);
	logger->info("argBatchFile :  {:s}", argBatchFile);
	logger->info("argConfig :  {:s}", argConfig);
	logger->info("argInput :  {:s}", argInput);
	logger->info("configFile  : {:s}", configPath);
	logger->info("InputPath  : {:s}", inputPath);
	logger->info("LasEpsg  : {:s}", lasEpsg);
	logger->info("outputPath  : {:s}", outputPath);
	logger->info("outputLevel  : {:s}", outputLevel);
	logger->info("----------------------");

	//outputLevelが"0"～"9"以外の時は強制的に"1"にする
	try {
		if (stoi(outputLevel) < 0 || 9 < stoi(outputLevel)) {
			outputLevel = "1";
		}
	}
	catch (const std::invalid_argument& e) {
		logger->info("invalid outputLevel outputLevel <- 1");
		outputLevel = "1";
	}
	catch (const std::out_of_range& e) {
		logger->info("out of range outputLevel outputLevel <- 1");
		outputLevel = "1";
	}

	struct stat dirCheck;
	if (stat(outputPath.c_str(), &dirCheck) != 0) {
		bool endStatus = CommonUtil::forceDirectories(outputPath);
		if (endStatus = false) {
			logger->error(" {:s}　が作成出来ません！", outputPath);
			return 9;
		}
	}

	//入出力のEPSG指定
	std::string espgLas = commonConfig.getPointConfirmLasEpsg();

	int wsize_cnt = 0;
	
	try {
		if (argBatchFile == "") {
			//対象LASファイル一覧を取得する
			//std::string lasFilePtn = "^[^.]+\\.las$";
			std::string lasFilePtn = "^[0-9]+-(-)?[0-9]+-[0-9]+-[0-9].+\\.las$";
			std::vector<std::string> lasFileList = CommonUtil::getFileList(inputPath, lasFilePtn);

			logger->info("lasFileList size  :{:d}", lasFileList.size());
			logger->info("----------------------");

			for (std::string inputFileId : lasFileList) {
				//１ファイルずつファイル変換処理を行う
				logger->info("{:s}  start", inputFileId);
				std::string inputFileIdFull = inputPath + "/" + inputFileId;
				size_t	pos = inputFileId.find_last_of('.');
				std::string outputFileId = inputFileId.substr(0, pos) + commonConfig.getMakeVoxelFileExt();
				std::string outputFileIdFull = outputPath + "/" + outputFileId;
				int rtn_code = 0;
				int wsize_cnt = 0;
				//出力形式の指定により処理を振り分け
				if (commonConfig.getMakeVoxelFileExt() == ".las") {
					rtn_code = voxelWrite::lasReadLasWrite(inputFileIdFull, outputFileIdFull, lasEpsg, logger);
				} else {
					rtn_code = voxelWrite::lasReadVxlWrite(inputFileIdFull, outputFileIdFull, lasEpsg, outputLevel, logger, &wsize_cnt);
				}
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
					logger->error("Can not open output file!! {:s}", outputFileIdFull);
					throw EXIT_FAILURE;
				}

				logger->info("{:s} output end :", outputFileId);
			}
		}
		else {
			//指定ファイルのみを処理対象にする
			std::string inputFileId = argBatchFile;
			logger->info("{:s}  start", inputFileId);
			std::string inputFileIdFull = inputPath + "/" + inputFileId;
			size_t	pos = inputFileId.find_last_of('.');
			std::string outputFileId = inputFileId.substr(0, pos) + commonConfig.getMakeVoxelFileExt();
			std::string outputFileIdFull = outputPath + "/" + outputFileId;
			int rtn_code = 0;
			//出力形式の指定により処理を振り分け
			if (commonConfig.getMakeVoxelFileExt() == ".las") {
				rtn_code = voxelWrite::lasReadLasWrite(inputFileIdFull, outputFileIdFull, lasEpsg, logger);
			}
			else {
				rtn_code = voxelWrite::lasReadVxlWrite(inputFileIdFull, outputFileIdFull, lasEpsg, outputLevel, logger, &wsize_cnt);
			}
			switch (rtn_code) {
			case 1:
				logger->error("Can not open input file!! {:s}", inputFileIdFull);
				throw EXIT_FAILURE;
				break;
			case 2:
				logger->error("Multi SpatialID Err!!");
				throw EXIT_FAILURE;
				break;
			case 3:
				logger->error("Can not open output file!! {:s}", outputFileIdFull);
				throw EXIT_FAILURE;
			}

			logger->info("{:s} output end ", outputFileId);
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

	if (wsize_cnt != 0) {
		logger->info("wrong size count = {:d} ", wsize_cnt);
	}

	logger->info("<<<<<<<<<<<<< MakeVoxel app end >>>>>>>>>>>>");
	return 0;

}