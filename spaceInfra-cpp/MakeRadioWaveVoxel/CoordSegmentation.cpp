#include "CoordSegmentation.h"
#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <proj.h>

// コンストラクタでPROJコンテキストと変換オブジェクトを初期化
CoordSegmentation::CoordSegmentation(int zone, bool isNorthern, std::string confirmLasEpsg) {
    // PROJコンテキストの作成
    C = proj_context_create();
    if (C == nullptr) {
        std::cerr << "Failed to create PROJ context" << std::endl;
        throw std::runtime_error("PROJ context creation failed");
    }

    std::string src_epsg = "EPSG:" + confirmLasEpsg;
	std::string utm = "+proj=utm +zone=" + std::to_string(zone) + (isNorthern ? " +north" : " +south") + " +datum=WGS84 +units=m";

    // 座標系変換オブジェクトの作成
    P = proj_create_crs_to_crs(C, src_epsg.c_str(), utm.c_str(), NULL);
    if (P == nullptr) {
        std::cerr << "Transformation initialization failed" << std::endl;
        proj_context_destroy(C);
        throw std::runtime_error("PROJ transformation creation failed");
    }
}

// デストラクタでPROJリソースを解放
CoordSegmentation::~CoordSegmentation() {
    if (P) {
        proj_destroy(P);
    }
    if (C) {
        proj_context_destroy(C);
    }
}

//! @brief 緯度・経度をUTM座標に変換する
//! @ref https://proj.org/en/stable/development/reference/cpp/operation.html#_CPPv4N5osgeo4proj9operation21CoordinateTransformer9transformE8PJ_COORD
//! @param lat 緯度
//! @param lng 経度
//! @param x UTM x座標
//! @param y UTM y座標
void CoordSegmentation::transformLatLngToUtm(double lat, double lng, int& x, int& y) {
    if (P == nullptr) {
        std::cerr << "Invalid PROJ transformation object" << std::endl;
        throw std::runtime_error("PROJ transformation object is not initialized");
    }

    PJ_COORD coord;

    coord.v[0] = lat;   // latitude in degree
	coord.v[1] = lng;   // longitude in degree
	coord.v[2] = 0.0;   // height
    coord.v[3] = HUGE_VAL;
	coord = proj_trans(P, PJ_FWD, coord);
	x = static_cast<int>(std::round(coord.v[0]));
	y = static_cast<int>(std::round(coord.v[1]));
}


std::vector<std::string> CoordSegmentation::readFile(const std::string& filePath) {
    std::vector<std::string> data;
    std::ifstream ifs(filePath);
    if (!ifs) return data;
    std::string line;
    while (std::getline(ifs, line)) {
        data.push_back(line);
    }
    return data;
}

HeaderInfo CoordSegmentation::parseHeader(const std::vector<std::string>& data) {
    HeaderInfo info{};
    for (int i = 0; i < 6 && i < static_cast<int>(data.size()); ++i) {
        std::istringstream iss(data[i]);
        std::string key;
        double value;
        iss >> key >> value;
        if (key == "ncols") info.ncols = static_cast<int>(value);
        if (key == "nrows") info.nrows = static_cast<int>(value);
        if (key == "xllcorner") info.xllcorner = static_cast<int>(value);
        if (key == "yllcorner") info.yllcorner = static_cast<int>(value);
        if (key == "cellsize") info.cellsize = static_cast<int>(value);
        if (key == "NODATA_value") info.NODATA_value = static_cast<int>(value);
    }
    return info;
}


std::vector<std::string> CoordSegmentation::clipData(const std::vector<std::string>& data,
    const HeaderInfo& headerInfo,
    double xMin, double xMax,
    double yMin, double yMax) {
    int xStart = std::max(0, static_cast<int>(std::floor((xMin - headerInfo.xllcorner) / headerInfo.cellsize)));
    int xEnd = std::min(headerInfo.ncols, static_cast<int>(std::ceil((xMax - headerInfo.xllcorner) / headerInfo.cellsize)));
    int yStart = std::max(0, static_cast<int>(std::floor((yMin - headerInfo.yllcorner) / headerInfo.cellsize)));
    int yEnd = std::min(headerInfo.nrows, static_cast<int>(std::ceil((yMax - headerInfo.yllcorner) / headerInfo.cellsize)));

	// 電波情報内に指定範囲が含まれていない場合は例外をスローする
	if (xStart >= headerInfo.ncols || xEnd <= 0 || yStart >= headerInfo.nrows || yEnd <= 0) {
		throw std::runtime_error("指定範囲が電波情報内に含まれていません");
	}

    // xEnd と xStart の関係を確認し、負の値にならないようにする
    if (xEnd < xStart) {
        std::swap(xStart, xEnd);
    }
    // yEnd と yStart の関係を確認し、負の値にならないようにする
    if (yEnd < yStart) {
        std::swap(yStart, yEnd);
    }

    std::vector<std::string> clipped;
    for (int row = yStart; row < yEnd; ++row) {
        if (6 + row >= static_cast<int>(data.size())) break;
        auto line = data[6 + row];
        if (static_cast<int>(line.size()) <= xStart) continue;
        int currentXEnd = xEnd;
        if (static_cast<int>(line.size()) < currentXEnd) {
            currentXEnd = static_cast<int>(line.size());
        }
        clipped.push_back(line.substr(xStart, currentXEnd - xStart));
    }
    return clipped;
}

HeaderInfo CoordSegmentation::calculateClippedHeader(const HeaderInfo& headerInfo,
    double xMin, double xMax,
    double yMin, double yMax) {
    int xStart = std::max(0, static_cast<int>(std::floor((xMin - headerInfo.xllcorner) / headerInfo.cellsize)));
    int xEnd = std::min(headerInfo.ncols, static_cast<int>(std::ceil((xMax - headerInfo.xllcorner) / headerInfo.cellsize)));
    int yStart = std::max(0, static_cast<int>(std::floor((yMin - headerInfo.yllcorner) / headerInfo.cellsize)));
    int yEnd = std::min(headerInfo.nrows, static_cast<int>(std::ceil((yMax - headerInfo.yllcorner) / headerInfo.cellsize)));

    // xEnd と xStart の関係を確認し、負の値にならないようにする
    if (xEnd < xStart) {
        std::swap(xStart, xEnd);
    }
    // yEnd と yStart の関係を確認し、負の値にならないようにする
    if (yEnd < yStart) {
        std::swap(yStart, yEnd);
    }

    HeaderInfo newInfo{};
    newInfo.ncols = xEnd - xStart;
    newInfo.nrows = yEnd - yStart;
    newInfo.xllcorner = headerInfo.xllcorner + xStart * headerInfo.cellsize;
    newInfo.yllcorner = headerInfo.yllcorner + yStart * headerInfo.cellsize;
    newInfo.cellsize = headerInfo.cellsize;
    newInfo.NODATA_value = headerInfo.NODATA_value;
    return newInfo;
}

void CoordSegmentation::writeClippedData(const std::string& outputFile,
    const std::vector<std::string>& clippedData,
    const HeaderInfo& newHeader) {
    std::ofstream ofs(outputFile);
    if (!ofs) return;
    ofs << "ncols " << newHeader.ncols << "\n";
    ofs << "nrows " << newHeader.nrows << "\n";
    ofs << "xllcorner " << newHeader.xllcorner << "\n";
    ofs << "yllcorner " << newHeader.yllcorner << "\n";
    ofs << "cellsize " << newHeader.cellsize << "\n";
    ofs << "NODATA_value " << newHeader.NODATA_value << "\n";
    for (auto& line : clippedData) {
        ofs << line << "\n";
    }
}

void CoordSegmentation::run(const std::string& inputFile,
    const std::string& outputFile) {

	Config& config = Config::getInstance();
	double startLng = config.getMapSegPointStartLng();
	double startLat = config.getMapSegPointStartLat();
	double endLng = config.getMapSegPointEndLng();
	double endLat = config.getMapSegPointEndLat();

	int xMin, yMin, xMax, yMax;
	// 緯度経度をUTM座標に変換
    transformLatLngToUtm(startLat, startLng, xMin, yMin);
    transformLatLngToUtm(endLat, endLng, xMax, yMax);

    // xMin と xMax の順序を確認し、必要なら入れ替え
    if (xMin > xMax) {
        std::swap(xMin, xMax);
    }
    // yMin と yMax の順序を確認し、必要なら入れ替え
    if (yMin > yMax) {
        std::swap(yMin, yMax);
    }

    auto data = readFile(inputFile);
    auto header = parseHeader(data);
    auto clipped = clipData(data, header, xMin, xMax, yMin, yMax);
    auto newHeader = calculateClippedHeader(header, xMin, xMax, yMin, yMax);
    writeClippedData(outputFile, clipped, newHeader);
}