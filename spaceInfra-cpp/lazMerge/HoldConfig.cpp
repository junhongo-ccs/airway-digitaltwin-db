#include <string>
#include <iostream>
#include <fstream>
#include "rapidjson/Document.h"
#include "rapidjson/istreamwrapper.h"
#include "../commonLib/CommonUtil.h"
#include "HoldConfig.hpp"

HoldConfig::HoldConfig()
{
    CommonConfig commonConfig;
    commonConfig.prepareLogger(APP_ID, LOGGER_NAME);
    loggerPointer = spdlog::get(LOGGER_NAME);
    /*if (configExist) {
        std::ifstream ifs(configPath);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);

        //app/common
        if (doc.HasMember("common")) {
            auto& appCommonDict = doc["common"];
            if (appCommonDict.HasMember("btDbHost")) {
                btDbHost = appCommonDict["btDbHost"].GetString();
            }
            if (appCommonDict.HasMember("btDbName")) {
                btDbName = appCommonDict["btDbName"].GetString();
            }
            if (appCommonDict.HasMember("btDbUser")) {
                btDbUser = appCommonDict["btDbUser"].GetString();
            }
            if (appCommonDict.HasMember("btDbPass")) {
                btDbPass = appCommonDict["btDbPass"].GetString();
            }

        }
        else {
            loggerPointer->error("commonConfig : common not exist");
            exit(9);
        }
    }*/
}

/*
std::string HoldConfig::getBtDbHost() {
    return btDbHost;
}

std::string HoldConfig::getBtDbName() {
    return btDbName;
}

std::string HoldConfig::getBtDbUser() {
    return btDbUser;
}

std::string HoldConfig::getBtDbPass() {
    return btDbPass;
}
*/
std::shared_ptr<spdlog::logger> HoldConfig::getLoggerPointer() {
    return loggerPointer;
}
