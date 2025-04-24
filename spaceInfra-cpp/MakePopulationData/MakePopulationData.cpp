#pragma warning(disable : 4996)
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <regex>
#include "CommonConfig.h"
#include "../commonLib/SolutionCommonDefs.h"

#include "Arguments.h"

#include <fstream>
#include <string>
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"

#include "MysqlController.hpp"


#define APP_ID "MakePopulationData"
#define LOGGER_NAME "MakePopulationData_applog"
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
	//std::string argEspg = argument.getEspg();
	//std::string argEspgLas = argument.getEspgLas();
	//std::string argEspgCsv = argument.getEspgCsv();
	//std::string argInput = argument.getInput();


	//起動引数をチェックする
	if (argc < 2) {
		std::cout << "argument error, not enough!";
		return 9;
	}
	if (argc > 4) {
		std::cout << "argument error, too many!";
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

	//入出力フォルダを取得する
	std::string	inputPath = commonConfig.getDir(APP_ID, "input", argBatchId);
	/*if (argInput == "inputProcessingResults") {
		inputPath = commonConfig.getDir(APP_ID, "inputProcessingResults", argBatchId);
	}*/
	//std::string	inputHashPath = commonConfig.getDir(APP_ID, "inputHash", argBatchId) + "/" + argBatchId;
	//std::string	confirmPath = commonConfig.getDir(APP_ID, "confirmRoot", argBatchId) + "/" + argBatchId;
	//std::string	lasEpsg = commonConfig.getPointInLasEpsg();
	//std::string	outputLevel = commonConfig.getMakeVoxelOutputLevel();


	logger->info("<<<<<<<<<<<<< MakePopulationData app start >>>>>>>>>>>>");

	logger->info("argBatchId :  {:s}", argBatchId);
	logger->info("argBatchFile :  {:s}", argBatchFile);
	logger->info("argConfig :  {:s}", argConfig);
	logger->info("configFile  : {:s}", configPath);
	logger->info("InputPath  : {:s}", inputPath);
	logger->info("----------------------");

	/*
	if (std::filesystem::exists(confirmPath)) {
		std::filesystem::remove_all(confirmPath);
	}
	bool endStatus = CommonUtil::forceDirectories(confirmPath);
	if (endStatus = false) {
		logger->error(" {:s}　が作成出来ません！", confirmPath);
		std::cout << "Can not make directory " << confirmPath;
		return 9;
	}*/
	//入出力のEPSG指定
	//std::string espgLas = commonConfig.getPointInLasEpsg();

	//DB接続パラメータの取得
	std::string btDbHost = commonConfig.getBtDbHost();
	std::string btDbName = commonConfig.getBtDbName();
	std::string btDbUser = commonConfig.getBtDbUser();
	std::string btDbPass = commonConfig.getBtDbPass();

	//sendHashPathの取得
	//std::string sendHashPath = commonConfig.getSendHashPath();

	//Mysqlの前処理
	MysqlController Msc(btDbHost, btDbName, btDbUser, btDbPass, logger);

	int wsize_cnt = 0;
	int file_cnt = 0;
	int detail_cnt = 0;
	std::vector<std::string> csvFileList ;
	if (argBatchFile == "") {
		std::string lasFilePtn = ".*\.csv$";
		csvFileList = CommonUtil::getFileList(inputPath, lasFilePtn);
	}
	else {
		csvFileList.push_back(argBatchFile);
	}

	try {
		for (std::string inputFileId : csvFileList) {
			//inputファイル毎の処理を実施
			int in_cnt = 0;
			int sum_cnt_in = 0;
			int sum_cnt_up = 0;
			bool ins_flg;
			logger->info("file : {:s}   start", inputFileId);
			file_cnt++;
			std::string inputFileIdFull = inputPath + "/" + inputFileId;
			std::ifstream csvFile(inputFileIdFull);
			if (!csvFile) {
				logger->error("csvFile can not open! ");
				return 9;
			}
			std::string line = "";
			bool head_flg = true;
			bool first_flg = true;
			std::string break_key = "";
			std::string new_key = "";
			double wk_stay_sum = 0.0;
			double wk_move_sum = 0.0;
			std::vector<std::string> csvDetail_wk;
			while (getline(csvFile, line)) {
				if (head_flg == true) {
					head_flg = false;
					continue;
				}
				detail_cnt++;
				in_cnt++;
				//csv人流明細毎処理開始
				std::vector<std::string> csvDetail = CommonUtil::split(line, ',');
				//人数を数値化
				double wk_stay = 0.0;
				double wk_move = 0.0;
				try {
					wk_stay = stod(csvDetail[8]);
				}
				catch (const std::invalid_argument& e) {
					wk_stay = 0.0;
				}
				catch (const std::out_of_range& e) {
					wk_stay = 0.0;
				}
				try {
					wk_move = stod(csvDetail[9]);
				}
				catch (const std::invalid_argument& e) {
					wk_move = 0.0;
				}
				catch (const std::out_of_range& e) {
					wk_move = 0.0;
				}
				//keyの設定
				new_key = csvDetail[0] + csvDetail[1] + csvDetail[2] + csvDetail[3] + csvDetail[4];
				if (first_flg == true) {
					first_flg = false;
					break_key = new_key;
				}
				if (break_key == new_key) {
					wk_stay_sum += wk_stay;
					wk_move_sum += wk_move;
					csvDetail_wk = csvDetail;
				}
				else {
					//Mysqlテーブルへの書き込み
					ins_flg = Msc.MysqlInsert(csvDetail_wk, wk_stay_sum, wk_move_sum);
					if (ins_flg) {
						sum_cnt_in++;
					}
					else {
						sum_cnt_up++;
					}
					break_key = new_key;
					wk_stay_sum = wk_stay;
					wk_move_sum = wk_move;
					csvDetail_wk = csvDetail;
				}				
			}
			ins_flg = Msc.MysqlInsert(csvDetail_wk, wk_stay_sum, wk_move_sum);
			if (ins_flg) {
				sum_cnt_in++;
			}
			else {
				sum_cnt_up++;
			}
			csvFile.close();
			logger->info("    input count = {:d} ", in_cnt);
			logger->info("    sum insert  = {:d} ", sum_cnt_in);
			logger->info("    sum update  = {:d} ", sum_cnt_up);
			logger->info("file : {:s}   end", inputFileId);
		}
	}
	catch (std::bad_alloc& ex) {
		logger->error("bad_alloc");
		std::cout << "bad_alloc";
		return 9;
	}
	catch (std::runtime_error& ex) {
		logger->error(ex.what());
		std::cout << ex.what();
		return 9;
	}
	catch (...) {
		logger->error("unexpected error");
		std::cout << "unexpected error";
		return 9;
	}
	logger->info("file_cnt = {:d}", file_cnt);
	logger->info("all_input_cnt = {:d}", detail_cnt);
	logger->info("<<<<<<<<<<<<< MakePopulationData app end >>>>>>>>>>>>");
	return 0;

}