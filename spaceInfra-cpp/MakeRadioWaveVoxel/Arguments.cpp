//! @file Arguments.cpp
//! @brief 起動引数を管理するクラス
//! 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cassert>
#include <string>
#include <regex>
#include <map>
#include "Arguments.h"


//! @biref 設定をロードする
//! 
//! @param argc コマンド引数の数
//! @param argv コマンド引数へのポイント配列
void Arguments::loadArguments(int argc, char** argv) {

    const std::vector<std::string> args(argv, argv + argc);

    if (argc > 1) {
        batchId = args[1];
    }

    for (const std::string& arg : args) {
        if (arg.find("--config=") != std::string::npos) {
            argConfig = regex_replace(arg, std::regex("--config="), "");
        }
    }
}
