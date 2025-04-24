#pragma once

//! @file CommonUtil.h
//! @brief 共通関数群

#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <regex>
#include "CommonDefs.h"
#include <proj.h>
#include "../ComLib/CommonUtil.h"

class CommonUtil
{
public:
	static void deleteFiles(const std::string& folder, const std::string& batchFile, std::string& dataType) {
		if (!std::filesystem::exists(folder)) {
			return;
		}

		std::string basePtn = "^[0-9]+-(-)?[0-9]+-[0-9]+-[0-9]+_";
		if (!batchFile.empty()) {
			std::string prefix = batchFile;
			size_t lastInd = batchFile.find_last_of('.');
			if (lastInd != std::string::npos) {
				prefix = batchFile.substr(0, lastInd);
			}
			basePtn = prefix + "_";
		}
		std::string delPtn = "";

		delPtn = basePtn + dataType + "\\.(gz|las|laz)";

		if (!delPtn.empty()) {
			ComLib::CommonUtil::deleteFiles(folder, delPtn);
		}
	}

	static void transform(int numPoints, double* pLon, double* pLat, PJ* proj) {

		if (proj != nullptr) {
			PJ_COORD input_coords, output_coords;

			for (int i = 0; i < numPoints; i++) {
				input_coords = proj_coord(pLon[i], pLat[i], 0, 0);
				output_coords = proj_trans(proj, PJ_FWD, input_coords);
				pLon[i] = output_coords.xy.x;
				pLat[i] = output_coords.xy.y;
			}
		}
	}

};
