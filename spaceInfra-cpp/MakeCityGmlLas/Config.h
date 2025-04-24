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


#define FEATURE_BUILDING	"BUILDING"
#define FEATURE_ROAD		"ROAD"
#define FEATURE_WATER		"WATER"
#define FEATURE_RAILWAY		"RAILWAY"
#define FEATURE_GROUND		"GROUND"
#define FEATURE_STEELTOWER	"STEELTOWER"
#define FEATURE_POWERLINE	"POWERLINE"

//! @brief 設定情報を管理するためのクラス
class Config : public ComLib::Config<Config> {
	friend class ComLib::Config<Config>;

	//app:CityGml2Voxel
	std::string inputDir = "input";
	std::string outputDir = "output/cvoxel";
	std::string inputFileExtension = "xml";
	std::string lonlatOrder = "auto";
	int targetLod = 1;

	//処理対象の地物
	std::vector<PolygonType> typeList = { TYPE_BUILDING, TYPE_ROAD, TYPE_WATER, TYPE_RAILWAY, TYPE_GROUND };

	int localZoom = spatialid::kLocalZoom;//APIの定義値を利用
	int colorBuilding = 0xb5e6a2;
	int colorRoad = 0xbfbfbf;
	int colorWater = 0x61cbf3;
	int colorRailway = 0xffccff;
	int colorGround = 0xf7c7ac;

	//double bldgHeightThreshold = 50; //単位：m

	//debug
	bool debugOut = false; //デバッグ用
	std::vector<std::string> targetSpatialIds; //出力したいポリゴン

	//コマンド引数関連
	std::string argFeatureId;

	Config() {}
	void setTargetTypeList(std::string& typeStr);
public:
	Config(Config const&) = delete;
	Config& operator=(Config const&) = delete;

	void addExtraArguments(argparse::ArgumentParser& argParser) override;
	void parseExtraArguments(argparse::ArgumentParser& argParser) override;
	void loadExtraConfig(rapidjson::Document& doc) override;
	std::string getCityGmlFileFolder();
	std::string getCityGmlFileFolder(std::string& exBatchFolder);
	std::string getOutputFolder();


	bool useMultithread() {
		return (threadCount > 1);
	}


	bool needDebugOut() {
		return debugOut;
	}

	std::vector<PolygonType>& getTypeList() {
		return typeList;
	}


	std::string getFeatureId(const PolygonType& type) {
		std::string featureId;
		switch (type) {
		case TYPE_BUILDING:
			featureId = FEATURE_BUILDING;
			break;
		case TYPE_ROAD:
			featureId = FEATURE_ROAD;
			break;
		case TYPE_WATER:
			featureId = FEATURE_WATER;
			break;
		case TYPE_RAILWAY:
			featureId = FEATURE_RAILWAY;
			break;
		case TYPE_GROUND:
			featureId = FEATURE_GROUND;
			break;
		case TYPE_POWERLINE:
			featureId = FEATURE_POWERLINE;
			break;
		case TYPE_STEELTOWER:
			featureId = FEATURE_STEELTOWER;
			break;
		default:
			throw std::runtime_error("unsupported featureId");
		}

		return featureId;
	}

	std::string getInputDir(const std::string& featureId) {
		std::string path(pointInRoot);

		std::string featureFolder = featureId;

		std::string result = ComLib::CommonUtil::pathJoin(path, batchId);
		result = ComLib::CommonUtil::pathJoin(result, inputDir);
		result = ComLib::CommonUtil::pathJoin(result, featureFolder);

		return result;
	}

	int getLocalZoom() {
		return localZoom;
	}

	const std::vector<std::string>& getTargetSpatialIds() {
		return targetSpatialIds;
	}

	//double getBldgHeightThreshold() {
	//	return bldgHeightThreshold;
	//}

	std::string getInputFileExtension() {
		return inputFileExtension;
	}

	std::string getLonlatOrder() {
		return lonlatOrder;
	}

	int getTargetLod() {
		return targetLod;
	}

	int getColor(std::string& featureId) {
		if (featureId == FEATURE_BUILDING) {
			return colorBuilding;
		}
		else if (featureId == FEATURE_ROAD) {
			return colorRoad;
		}
		else if (featureId == FEATURE_WATER) {
			return colorWater;
		}
		else if (featureId == FEATURE_RAILWAY) {
			return colorRailway;
		}
		else if (featureId == FEATURE_GROUND) {
			return colorGround;
		}
		else {
			return 0xff2000;
		}
	}
};

