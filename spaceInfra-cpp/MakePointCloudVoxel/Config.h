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

	//app:MakePointCloudVoxel
	std::string lasInputRoot = "";
	std::string lasInputDir = "input";
	std::string inputFileExt = "las";

	std::string voxelOutputRoot = "";
	std::string voxelOutputDir = "voxel";
	std::string outputFileExt = "laz";

	int localZoom = spatialid::kLocalZoom;//APIの定義値を利用
	int color = 0xffffff;

	std::string voxelDeleteDir = "";

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
	std::string getLasInputFolder();
	std::string getLasInputFolder(std::string& exBatchFolder);


	bool useMultithread() {
		return (threadCount > 1);
	}


	bool needDebugOut() {
		return debugOut;
	}

	std::string getLasInputDir() {
		std::string path(lasInputRoot);

		std::string result = ComLib::CommonUtil::pathJoin(path, batchId);
		result = ComLib::CommonUtil::pathJoin(result, lasInputDir);

		return result;
	}

	int getLocalZoom() {
		return localZoom;
	}

	const std::vector<std::string>& getTargetSpatialIds() {
		return targetSpatialIds;
	}


	int getColor() {
		return color;
	}

	std::string getInputFileExt() {
		return inputFileExt;
	}

	std::string getOutputFileExt() {
		return outputFileExt;
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

	std::string getVoxelDeleteDir() {
		return voxelDeleteDir;
	}
};

