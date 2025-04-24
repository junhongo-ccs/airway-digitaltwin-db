#pragma once

//! @file Arguments.h
//! @brief 起動引数を管理するクラス　ヘッダ
//! 
#include <string>
#include <vector>

//! @brief 起動引数を管理するクラス
class Arguments {
	std::string defConfigJsonFile = "";
	std::string configJsonFile = defConfigJsonFile;

	// 第一引数
	std::string batchId = "";

	// 第二引数
	// --config=で指定された設定ファイル
	std::string argConfig = "";

public:
	Arguments() {
	}
	void loadArguments(int argc, char** argv);
	std::string getBatchId() {
		return batchId;
	}
	std::string getConfig() {
		return argConfig;
	}
};