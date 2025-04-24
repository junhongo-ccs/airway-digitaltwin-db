#pragma once

//! @file Config.h
//! @brief 設定関連
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>
#include "spdlog/spdlog.h"
#include "CommonDefs.h"
#include "CommonUtil.h"
#include "../ComLib/Config.h"
#include "../ComLib/CommonUtil.h"
#include "../../SpatialId/src/spatial_api.h"
#include <rapidjson/rapidjson.h>


//! @brief 設定情報を管理するためのクラス
class Config : public ComLib::Config<Config> {
	friend class ComLib::Config<Config>;

	std::string voxelOutputRoot = "";
	//std::string inputDir = "input";
	std::string voxelOutputDir = "flightProhibitedAreaObjects/<batchDatetime>/voxel";
	std::string voxelExt = "laz";
	int color = 0xff0000;


	int localZoom = spatialid::kLocalZoom;//APIの定義値を利用
	int edgeCount = 0;
	bool useCylinderApi = false;//false -> ポリゴン近似

	//debug
	bool debugOut = false; //デバッグ用
	std::vector<std::string> targetSpatialIds; //出力したいポリゴン

	Config() {}
public:
	Config(Config const&) = delete;
	Config& operator=(Config const&) = delete;

	void addExtraArguments(argparse::ArgumentParser& argParser) override;
	void parseExtraArguments(argparse::ArgumentParser& argParser) override;
	void loadExtraConfig(rapidjson::Document& doc) override;

	std::vector<std::string> getBatchIdDefs() {
		std::vector<std::string> resultList;
		resultList.push_back("flight_prohibited_area_object_id");
		resultList.push_back("飛行禁止エリア情報ID");
		resultList.push_back("1");
		return resultList;
	}


	bool useMultithread() {
		return (threadCount > 1);
	}


	bool needDebugOut() {
		return debugOut;
	}


	int getLocalZoom() {
		return localZoom;
	}

	const std::vector<std::string>& getTargetSpatialIds() {
		return targetSpatialIds;
	}

	std::string getVoxelBitFilePath(const std::string& spatialIdForFolder, const std::string& curDate) {
		std::string result = spatialIdForFolder;

		std::string voxelOutputDirTemp = voxelOutputDir;
		result = ComLib::CommonUtil::pathJoin(result,
			std::regex_replace(voxelOutputDirTemp, std::regex("<batchDatetime>"), curDate));

		result = regex_replace(result, std::regex("\\\\"), "/");

		return result;
	}

	std::string getVoxelOutputFolder(const std::string& spatialIdForFolder, const std::string& curDate) {
		std::string result = ComLib::CommonUtil::pathJoin(voxelOutputRoot, spatialIdForFolder);

		std::string voxelOutputDirTemp = voxelOutputDir;
		result = ComLib::CommonUtil::pathJoin(result,
			std::regex_replace(voxelOutputDirTemp, std::regex("<batchDatetime>"), curDate));

		return result;
	}

	std::string getVoxelExt() {
		return voxelExt;
	}

	int getColor() {
		return color;
	}

	int getEdgeCount() {
		return edgeCount;
	}

	bool needUseCylinderApi() {
		return useCylinderApi;
	}


};

