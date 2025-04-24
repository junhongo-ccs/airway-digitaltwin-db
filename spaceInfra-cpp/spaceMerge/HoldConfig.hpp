#pragma once
#ifndef __HOLDCONFIG_HPP_INCLUDED__
#define __HOLDCONFIG_HPP_INCLUDED__

#define APP_ID "spaceMerge"
#define LOGGER_NAME "spaceMerge_applog"
#define COMMON_CONFIG_ID "./commonConfig.json"

#include "../commonLib/CommonConfig.h"

//! @CommonConfigから取得したパラメータを保存するクラス
class HoldConfig
{
private:
    std::string btDbHost = "";
    std::string btDbName = "";
    std::string btDbUser = "";
    std::string btDbPass = "";

    bool fillOnly;
    std::shared_ptr<spdlog::logger> loggerPointer;


public:
    HoldConfig(std::string, bool);  // コンストラクタ


    std::string getBtDbHost();
    std::string getBtDbName();
    std::string getBtDbUser();
    std::string getBtDbPass();

    std::shared_ptr<spdlog::logger> getLoggerPointer();

};
#endif