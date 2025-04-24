#pragma once

//! @file Config.h
//! @brief 共通config管理クラス　ヘッダー

#include <string>
#include <vector>
#include "spdlog/spdlog.h"
#include "solutionCommonDefs.h"

//! @brief 共通config管理クラス
class CommonConfig {
	std::string appId = "";

	//common 環境依存定義
	std::string ownerId = "";
	std::string projectRoot = "";
	std::string execRoot = "";
	std::string dataRoot = "";
	std::string pointRoot = "";
	std::string mapRoot = "";
	std::string logRoot = "";
	std::string logsink = "console";
	std::string loglevel = "info";
	int colorByte = 2;
	int decimalPoint = 99;

	std::string btDbHost = "";
	std::string btDbName = "";
	std::string btDbUser = "";
	std::string btDbPass = "";

	//app>common アプリ共通定義
	std::string mapEpsg = "";
	std::string pointInLasEpsg = "";
	std::string pointCsvEpsg;
	std::string pointOutLasEpsg = "";
	std::string pointConfirmLasEpsg = "";

	//Las2Csvsp
	std::string las2csvspInput = "";
	std::string las2csvspInputDrive = "";
	std::string las2csvspInputRoot = "";
	std::string las2csvspOutput = "";
	std::string las2csvspOutputDrive = "";
	std::string las2csvspOutputRoot = "";
	std::string las2csvspOutputIndex = "";
	std::string las2csvspOutputIndexDrive = "";
	std::string las2csvspOutputIndexRoot = "";
	std::string las2csvspOutputFormat = "csv";

	bool isLandCutLas = false;
	int las2csvspThreadCount = 6;
	int las2csvspThreadCountForOutput = 6;

	//Csvsp2Space
	std::string csvsp2spaceInput = "";
	std::string csvsp2spaceInputDrive = "";
	std::string csvsp2spaceInputRoot = "";
	std::string csvsp2spaceOutput = "";
	std::string csvsp2spaceOutputDrive = "";
	std::string csvsp2spaceOutputRoot = "";
	std::string csvsp2spaceInputIndex = "";
	std::string csvsp2spaceInputIndexDrive = "";
	std::string csvsp2spaceInputIndexRoot = "";

	std::string pointMatchOutType;

	//PointMatch
	std::string pointMatchInput = "";
	std::string pointMatchInputDrive = "";
	std::string pointMatchInputRoot = "";
	std::string pointMatchOutput = "";
	std::string pointMatchOutputDrive = "";
	std::string pointMatchOutputRoot = "";
	std::string pointMatchOutputSteelTower = "";
	std::string pointMatchOutputSteelTowerDrive = "";
	std::string pointMatchOutputSteelTowerRoot = "";
	std::string shapeFileSuffix = "";
	bool noExpandMode= false;
	int outPolygon=0;
	int threadCount=0;

	//MakeVoxel
	std::string makeVoxelInput = "";
	std::string makeVoxelInputDrive = "";
	std::string makeVoxelInputRoot = "";
	std::string makeVoxelInputProcessingResults = "";
	std::string makeVoxelInputProcessingResultsDrive = "";
	std::string makeVoxelInputProcessingResultsRoot = "";
	std::string makeVoxelOutput = "";
	std::string makeVoxelOutputDrive = "";
	std::string makeVoxelOutputRoot = "";
	std::string makeVoxelOutputFormat = "vxl";
	std::string makeVoxelOutputLevel = "1";

	//MergeVoxel
	std::string mergeVoxelInput = "";
	std::string mergeVoxelInputDrive = "";
	std::string mergeVoxelInputRoot = "";
	std::string mergeVoxelOutput = "";
	std::string mergeVoxelOutputDrive = "";
	std::string mergeVoxelOutputRoot = "";
	std::string mergeVoxelOutputFormat = "vxl";
	std::string mergeVoxelOutputLevel = "1";

	//MakeHash
	std::string makeHashInput = "";
	std::string makeHashInputDrive = "";
	std::string makeHashInputRoot = "";
	std::string makeHashOutput = "";
	std::string makeHashOutputDrive = "";
	std::string makeHashOutputRoot = "";
	std::string makeHashOutputFormat = "SHA-256";

private:



public:
	CommonConfig() {
	
	}

	static CommonConfig& getInstance() {
		//singletonにする。上のconstructorをprivateにしたかったけど、ほかでも利用されているのでいったん断念
		static CommonConfig _config;
		return _config;
	}
	void loadConfig(std::string configFileName);

	void prepareLogger(std::string appId , std::string loggerName);

	//IOディレクトリ取得
	std::string getDir(std::string appId, std::string ioId, std::string batId);

	//app>common アプリ共通定義

	std::string getOwnerId() {
		return ownerId;
	}
	std::string getProjectRoot() {
		return projectRoot;
	}
	std::string getExecRoot() {
		return execRoot;
	}
	std::string getDataRoot() {
		return dataRoot;
	}
	std::string getPointRoot() {
		return pointRoot;
	}
	std::string getMapRoot() {
		return mapRoot;
	}
	std::string getLogRoot() {
		return logRoot;
	}
	std::string getLogsink() {
		return logsink;
	}
	std::string getLoglevel() {
		return loglevel;
	}
	int getColorByte() {
		if (colorByte == 1 || colorByte == 2) {
			return colorByte;
		}
		else {
			return 2;
		}
	}
	int getDecimalPoint() {
		if (decimalPoint == 1 || decimalPoint == 2 || decimalPoint == 3 || decimalPoint == 4) {
			return decimalPoint;
		}
		else {
			return 99;
		}
	}
	std::string getBtDbHost() {
		return btDbHost;
	}
	std::string getBtDbName() {
		return btDbName;
	}
	std::string getBtDbUser() {
		return btDbUser;
	}
	std::string getBtDbPass() {
		return btDbPass;
	}

	std::string getMapEpsg() {
		return mapEpsg;
	}
	std::string getPointInLasEpsg() {
		return pointInLasEpsg;
	}
	void setPointInLasEpsg(std::string espg) {
		pointInLasEpsg = espg;
	}
	std::string getPointCsvEpsg() {
		return pointCsvEpsg;
	}
	void setPointCsvEpsg(std::string espg) {
		pointCsvEpsg = espg;
	}
	std::string getPointOutEpsg() {
		return pointOutLasEpsg;
	}
	std::string getPointConfirmLasEpsg() {
		return pointConfirmLasEpsg;
	}
	void setPointOutLasEpsg(std::string espg) {
		pointOutLasEpsg = espg;
	}
	void setPointConfirmLasEpsg(std::string espg) {
		pointConfirmLasEpsg = espg;
	}
	
	//Las2Csvsp
	std::string getLas2csvspInput() {
		return las2csvspInput;
	}
	std::string getLas2csvspOutput() {
		return las2csvspOutput;
	}
	std::string getLas2csvspOutputIndex() {
		return las2csvspOutputIndex;
	}
	bool getIsLandCutLas() {
		return isLandCutLas;
	}
	std::string getLas2csvspOutputFormat() {
		return las2csvspOutputFormat;
	}
	std::string getLas2csvpsFileExt() {
		std::string las2csvspFileExt = ".csv";
		if (las2csvspOutputFormat == "las") {
			las2csvspFileExt = ".las";
		};
		return las2csvspFileExt;
	}


	//Csvsp2Space
	std::string getCsvsp2spaceInput() {
		return csvsp2spaceInput;
	}
	std::string getCsvsp2spaceOutput() {
		return csvsp2spaceOutput;
	}
	std::string getCsvsp2spaceInputIndex() {
		return csvsp2spaceInputIndex;
	}
	std::string getPointMatchOutType() {
		return pointMatchOutType;
	}

	//PointMatch
	std::string getPointMatchInput() {
		return pointMatchInput;
	}
	std::string getPointMatchOutput() {
		return pointMatchOutput;
	}
	std::string getPointMatchOutputSteelTower() {
		return pointMatchOutputSteelTower;
	}
	std::string getShapeFileSuffix() {
		return shapeFileSuffix;
	}
	bool getNoExpandMode() {
		return noExpandMode;
	}
	int getOutPolygon() {
		return outPolygon;
	}
	int getThreadCount() {
		if (appId == "Las2Csvsp") {
			return las2csvspThreadCount;
		}
		return threadCount;
	}
	int getThreadCountForOutput() {
		if (appId == "Las2Csvsp") {
			return las2csvspThreadCountForOutput;
		}
		return 0;
	}

	//MakeVoxel
	std::string getMakeVoxelInput() {
		return makeVoxelInput;
	}
	std::string getMakeVoxelInputProcessingResults() {
		return makeVoxelInputProcessingResults;
	}
	std::string getMakeVoxelOutput() {
		return makeVoxelOutput;
	}
	std::string getMakeVoxelFileExt() {
		std::string makeVoxelFileExt = ".vxl";
		if (makeVoxelOutputFormat == "las") {
			makeVoxelFileExt = ".las";
		};
		if (makeVoxelOutputFormat == "gz") {
			makeVoxelFileExt = ".gz";
		};
		return makeVoxelFileExt;
	}
	std::string getMakeVoxelOutputLevel() {
		return makeVoxelOutputLevel;
	}

	//MergeVoxel
	std::string getMergeVoxelInput() {
		return mergeVoxelInput;
	}
	std::string getMergeVoxelOutput() {
		return mergeVoxelOutput;
	}
	std::string getMergeVoxelFileExt() {
		std::string mergeVoxelFileExt = ".vxl";
		if (mergeVoxelOutputFormat == "gz") {
			mergeVoxelFileExt = ".gz";
		};
		return mergeVoxelFileExt;
	}
	std::string getMergeVoxelOutputLevel() {
		return mergeVoxelOutputLevel;
	}

	//MakeHash
	std::string getMakeHashInput() {
		return makeHashInput;
	}
	std::string getMakeHashOutput() {
		return makeHashOutput;
	}
	std::string getMakeHashOutputFormat() {
		return makeHashOutputFormat;
	}

};

