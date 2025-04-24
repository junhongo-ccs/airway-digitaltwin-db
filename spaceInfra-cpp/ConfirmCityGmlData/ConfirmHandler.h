#pragma once
#include "CommonDefs.h"
#include "Config.h"
#include <spdlog/spdlog.h>
#include <proj.h>
#include "..\..\SpatialId\src\\spatial_api.h"
#include <mutex>
#include <vector>

class ConfirmHandler
{
	std::mutex mtx;
	std::shared_ptr<spdlog::logger> logger;
	//std::mutex _mtx;
	PJ_CONTEXT* projContext = nullptr;
	PJ* projPointOut2ConfirmLas = nullptr;
	//PJ* projPointIn2ConfirmLas = nullptr;

	void createDataCommon(const std::string& spatialIdWithSlash, const std::string& today,
		const std::vector<spatialid::Point>& vertexList, int objectId, const std::string& featureId,
		int dataSourceId);
	//void createDetailData(const std::string& spatialIdWithSlash, const std::string& today,
	//	int objectId, const std::string& featureId,int dataSourceId);
	void copyLasFile(const std::string& lasFileIn, const std::string& lasFileOut,
		int confirmLasEpsg, PJ* proj);
	//std::string getHash(std::string& fileName);
	//void sendHash(const std::string& spatialId, int objectCd, const std::vector<std::string>& hashList);
	//std::map<std::string, std::vector<std::string>> readObjectAttributes(std::string& fileName);
	//std::string sjis2Utf8(std::string& str) {
	//	Config& config = Config::getInstance();
	//	if (config.needSjis2Utf8()) {
	//		return CommonUtil::sjisToUtf8(str);
	//	}
	//	else {
	//		return str;
	//	}
	//}

public:
	ConfirmHandler();
	virtual ~ConfirmHandler() {
		if (projPointOut2ConfirmLas != nullptr) {
			proj_destroy(projPointOut2ConfirmLas);
		}
		//if (projPointIn2ConfirmLas != nullptr) {
		//	proj_destroy(projPointIn2ConfirmLas);
		//}

		if (projContext != nullptr) {
			proj_context_destroy(projContext);
		}

	}

	int execute();
};

