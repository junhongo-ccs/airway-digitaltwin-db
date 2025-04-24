#pragma once
#pragma once

//! @file Arguments.h
//! @brief 起動引数を管理するクラス　ヘッダー
//! 
#include <string>
#include <vector>

//! @brief 起動引数を管理するクラス
class Arguments {
	std::string defConfigJsonFile = "";
	std::string configJsonFile = defConfigJsonFile;

	//第一引数
	std::string batchId = "";

	//第二引数
	std::string batchFile = ""; //指定されていない場合全て

	//-- 指定された引数
    std::string argConfig = "";

    std::string argEspg = "";

    std::string argEspgLas = "";

    std::string argEspgCsv = "";

	std::string argInput = "";

public:
	Arguments() {
	}
	void loadArguments(int argc, char** argv);
	std::string getBatchId() {
		return batchId;
	}
	std::string getBatchFile() {
		return batchFile;
	}
	std::string getConfig() {
		return argConfig;
	}
	std::string getEspg() {
		return argEspg;
	}
	std::string getEspgLas() {
		return argEspgLas;
	}
	std::string getEspgCsv() {
		return argEspgCsv;
	}
	std::string getInput() {
		return argInput;
	}

};