#include "CommonUtil.h"
#include <vector>
#include <filesystem>
#include "Config.h"
//#ifdef _WIN32
//#include <windows.h>
//#endif

//! @brief ファイル名から空間IDを取得（f:0に固定）
//! 
//! @param lasFileName ファイル名
//! @return 取得できた空間ID
std::string CommonUtil::getSpatialIdFromFilename(const std::string& lasFileName) {
	std::regex re(R"(^(\d+)-(-?\d+)-(\d+)-(\d+)[_a-zA-Z]*\.[a-z]+$)");
	std::smatch match;

	std::string fileName = std::filesystem::path(lasFileName).filename().string();

	if (std::regex_match(fileName, match, re)) {
		std::string result(match[1]);
		result.append("/").append(match[2]).append("/").append(match[3]).append("/").append(match[4]);
		return result;
	}

	return "";
}



std::vector<std::string> CommonUtil::getFeatureIdList(const std::string& folder, const std::string& spatialIdWithSlash) {
	Config& config = Config::getInstance();
	
	std::string spatialId = spatialIdWithSlash;
	std::replace(spatialId.begin(), spatialId.end(), '/', '-');

	std::string ptn = "^" + spatialId + "_([A-Z]+)\\." + config.getInputVoxelExt() + "$";

	std::regex re(ptn);
	std::smatch match;

	std::vector<std::string> resultList;

	for (const auto& file : std::filesystem::directory_iterator(folder)) {
		if (file.is_regular_file()) {
			std::string fileName = file.path().filename().string();

			if (std::regex_match(fileName, match, re)) {
				std::string result(match[1]);
				resultList.push_back(result);
			}
		}
	}

	return resultList;

}


int CommonUtil::calcScale(std::string& epsg, PJ* proj) {
	int code = std::atoi(epsg.c_str());

	//平面直角座標系
	if (code >= 6669 && code <= 6687 || code >= 2443 && code <= 2461) {
		return 1000;//実際に100でも十分だが
	}
	else if (code >= 6688 && code <= 6692 || code >= 3097 && code <= 3101) {//UTM
		return 100; //適当。今のところ利用しない
	}
	else if (code == 3857) {
		return 100; //適当。今のところ利用しない
	}
	else {
		return calcScale(proj);
	}

}

int CommonUtil::calcScale(PJ* proj) {
	//東京駅の位置を利用してスケールを決める
	double refLat = 35.6809591;
	double refLon = 139.7673068;

	if (proj != nullptr) {
		//座標変換
		transform(1, &refLon, &refLat, proj);
	}

	double refValue = std::max(refLat, refLon);

	if (refValue < 0)
		refValue = -refValue;
	if (refValue < 1.0e-6)
		return 1;

	if (refValue <= 180.0)//緯度経度とみなし
		return 10000000;

	int intMax = INT_MAX >> 3;
	int factor = static_cast<int>(intMax / refValue);
	int scale = 1;

	while (factor / 10 > 0) {
		scale *= 10;
		factor /= 10;
	}

	return scale;
}

void CommonUtil::transform(int numPoints, double* pLon, double* pLat, PJ* proj) {

	if (proj != nullptr) {
		PJ_COORD input_coords, output_coords;

		for (int i = 0; i < numPoints; i++) {
			input_coords = proj_coord(pLat[i], pLon[i], 0, 0);
			output_coords = proj_trans(proj, PJ_FWD, input_coords);
			pLon[i] = output_coords.xy.y;
			pLat[i] = output_coords.xy.x;
		}
	}
}


//std::string CommonUtil::sjisToUtf8(std::string& str) {
//#ifdef _WIN32
//	if (GetACP() != 932 || str.empty()) {
//		return str;
//	}
//	UINT fromAcp = CP_ACP;
//	UINT toAcp = CP_UTF8;
//
//	int bufLen = MultiByteToWideChar(fromAcp, 0, str.c_str(), str.size(), nullptr, 0);
//	wchar_t* ws = (wchar_t*)std::malloc(bufLen * sizeof(wchar_t));
//	if (ws == nullptr) {
//		return str;
//	}
//	int newLen = MultiByteToWideChar(fromAcp, 0, str.c_str(), str.size(), ws, bufLen);
//
//	bufLen = WideCharToMultiByte(toAcp, 0, ws, newLen, nullptr, 0, nullptr, nullptr);
//	char* buf = (char*)std::malloc((bufLen + 1) * sizeof(char));
//	if (buf == nullptr) {
//		std::free(ws);
//		return str;
//	}
//	newLen = WideCharToMultiByte(toAcp, 0, ws, newLen, buf, bufLen, nullptr, nullptr);
//	buf[newLen] = '\0';
//
//	std::string result(buf);
//
//	std::free(ws);
//	std::free(buf);
//
//	return result;
//#else
//	return str;
//#endif
//}
