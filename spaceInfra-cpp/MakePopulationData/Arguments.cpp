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
    std::vector<std::string> options;

    int i = -1;
    for (const std::string& arg : args) {
        i++;
        //プログラム名をそのまま追加
        if (i == 0) {
            options.push_back(arg);
            continue;
        }

        // 第一 コマンド引数設定
        if (i == 1) {
            batchId = arg;
            continue;
        }
        // 第二 コマンド引数設定
        if (i == 2 && arg[0] != '-') {
            batchFile = arg;
        }
        //変換対象
        std::map<std::string, std::string> convertMap = {
            {"-c", "--config"}
        };

        //アーギュメント整形
        // 
        //[-] --> [--]変換用
        auto replaceFunc = [&convertMap](const std::string& arg, const std::string& from) {
            if (convertMap.find(from) == convertMap.end()) {//ない場合
                return arg;
            }
            std::string str(convertMap[from]);
            str += arg.substr(from.length());

            return str;
        };
        //[--]のものはそのまま追加
        if (arg.length() > 1 && arg[0] == '-' && arg[1] == '-') {
            options.push_back(arg);
        }
        else if (arg[0] == '-') {
            //[=]がなければそのまま追加
            if (arg.find("=") < 0) {
                if (convertMap.find(arg) == convertMap.end()) {
                    options.push_back(arg);
                }
                else {
                    options.push_back(convertMap[arg]);
                }
            }
            else {
                //[-] --> [--]
                options.push_back(replaceFunc(arg, arg.substr(0, 2)));
            }
        }

    }

    // --コマンド引数設定
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].find("--config=") != std::string::npos) {
            argConfig = regex_replace(options[i], std::regex("--config="), "");
        }
        else if (options[i].find("--espg=") != std::string::npos) {
            argEspg = regex_replace(options[i], std::regex("--espg="), "");
        }
        else if (options[i].find("--espgLas=") != std::string::npos) {
            argEspgLas = regex_replace(options[i], std::regex("--espgLas="), "");
        }
        else if (options[i].find("--espgCsv=") != std::string::npos) {
            argEspgCsv = regex_replace(options[i], std::regex("--espgCsv="), "");
        }
        else if (options[i].find("--input=") != std::string::npos) {
            argInput = regex_replace(options[i], std::regex("--input="), "");
        }

    }

}
