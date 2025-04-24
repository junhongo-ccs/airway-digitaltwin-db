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
#include "../../SpatialId/src/spatial_api.h"
#include "../ComLib/Config.h"

//! @brief 設定情報を管理するためのクラス
class Config : public ComLib::Config<Config> {
	friend class ComLib::Config<Config>;


	//app:comfirmCityGmlData
	//las(地物別と200)
	std::string lasInputRoot = "";
	std::string lasInputDir = "output/all";

	////las(100)
	//std::string las100InputRoot = dataRoot + "\\point";
	//std::string las100InputDir = "w02";

	//voxel
	std::string voxelInputRoot = "";
	std::string voxelInputDir = "output/zvoxel";

	//voxelDelete
	std::string voxelDeleteDir = "";


	//output
	std::string lasOutputRoot = ""; // dataRoot + "\\userPublic";
	std::string lasOutputDir = "";
	std::string voxelOutputRoot = ""; // dataRoot + "\\userPublic";
	std::string voxelOutputDir = "voxel";
	std::string inputVoxelExt = "laz";

	//std::string configClear = "";
	int localZoom = spatialid::kLocalZoom;//APIの定義値を利用

	bool debugOut = false; //デバッグ用
	//bool usePipeForSendHash = true;
	//bool sjis2Utf8 = true;

	Config() {}
public:
	Config(Config const&) = delete;
	Config& operator=(Config const&) = delete;

	void addExtraArguments(argparse::ArgumentParser& argParser) override;
	void parseExtraArguments(argparse::ArgumentParser& argParser) override;
	void loadExtraConfig(rapidjson::Document& doc) override;

	int getLocalZoom() {
		return localZoom;
	}

	//bool needBatchId() override {
	//	return true;
	//}
	virtual std::vector<std::string> getBatchIdDefs() {
		std::vector<std::string> resultList;
		resultList.push_back("batchid");
		resultList.push_back("バッチID [ファイルID]");
		resultList.push_back("2");
		return resultList;
	}

	bool needDebugOut() {
		return debugOut;
	}


	//std::string getLas100InputFolder() {
	//	std::string result = CommonUtil::pathJoin(las100InputRoot, batchFolder);

	//	result = CommonUtil::pathJoin(result, las100InputDir);

	//	return result;
	//}

	std::string getLasOutputRoot() {
		return lasOutputRoot;
	}

	std::string getVoxelOutputRoot() {
		return voxelOutputRoot;
	}

	std::string getLasInputFolder() {
		if (lasInputDir.empty()) {
			return "";
		}
		std::string result = ComLib::CommonUtil::pathJoin(lasInputRoot, batchId);

		result = ComLib::CommonUtil::pathJoin(result, lasInputDir);

		return result;
	}

	std::string getVoxelInputFolder() {
		std::string result = ComLib::CommonUtil::pathJoin(voxelInputRoot, batchId);

		result = ComLib::CommonUtil::pathJoin(result, voxelInputDir);

		return result;
	}

	std::string getLasOutputFolder(const std::string& spatialIdForFolder, const std::string& curDate) {
		if (lasOutputDir.empty()) {
			return "";
		}
		std::string result = ComLib::CommonUtil::pathJoin(lasOutputRoot, spatialIdForFolder);

		std::string lasOutputDirTemp = lasOutputDir;
		result = ComLib::CommonUtil::pathJoin(result, 
				std::regex_replace(lasOutputDirTemp, std::regex("<batchDatetime>"), curDate));


		return result;
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


	std::string getInputVoxelExt() {
		return inputVoxelExt;
	}

	std::string getVoxelDeleteDir() {
		return voxelDeleteDir;
	}

	//bool needClear() {
	//	return (configClear == "y" || configClear == "yes");
	//}


};

