#ifndef VOXELWRITE_H
#define VOXELWRITE_H

#include <string>
#include <spdlog/spdlog.h>
#include <proj.h>
#include <map>

// 入力グリッドのヘッダ情報
struct GridHeader {
    int ncols;
    int nrows;
    double xllcorner;
    double yllcorner;
    double cellsize;
    int NODATA_value;
};

// ボクセルサイズ
struct VoxelSize {
    double x, y, z;
};

// ボクセル境界定義
struct BoundingBox {
    double minX, minY;
    double maxX, maxY;
};

// グリッドデータを格納する構造体
struct GridData {
    double x, y, z;    // 点群座標
    int value;      // 0: 圏外, 1: 圏内
};

// 境界情報を管理する構造体
struct BoundaryInfo {
    BoundingBox bounds;
    std::string lazFilePath;
    bool isProcessed;
};

class VoxelWrite {
private:
	const double EARTH_RADIUS = 6378137.0; // 赤道半径
    const double M_PI = 3.14159265358979323846;
	const int ZOOM_17 = 17; // ズームレベル17
	PJ_CONTEXT* ctx;
	PJ* transformUTMtoLatLng;   // UTM -> 緯度経度変換
	PJ* transformLatLngtoPsuedoMercator;   // TODO 緯度経度 -> メルカトル図法変換
	int utmZone;   // UTMゾーン番号
	bool isNorthern;  // 北半球か
	double height;  // 電波情報の高度
	unsigned char red, green, blue;
    std::string batchDateTime;
    void initializeTransform(std::string confirmLasEspg);
    std::string calculateSpatialId(double lng, double lat, double h, int z);
	void utmToLatLon(double utm_x, double utm_y, double& lat, double& lng);
    int readGridValue(int row, int col, const std::string& gridData, const GridHeader& header);
    static std::unordered_map<std::string, BoundaryInfo> boundaryRegistry;
    void createNewLazFile(const std::string& spatialId, const std::vector<GridData>& points, const std::string& outputDir, const BoundingBox& bounds, spdlog::logger& logger);
    void appendToExistingLaz(const std::string& spatialId, const std::vector<GridData>& newPoints, const std::string& outputDir, const BoundingBox& bounds, spdlog::logger& logger);
    std::vector<GridData> readExistingLazFile(const std::string& lazFilePath);
    std::vector<GridData> readGridDataInBounds(const BoundingBox& bounds,
        const GridHeader& header,
        const VoxelSize& voxel28,
        const std::string& gridData);
    double roundCoord(double v, int decimals);
    std::vector<size_t> rowOffsets;
	void preprocessGridData(const std::string& gridData, int nrows);
public:
    VoxelWrite(int utmZone, bool isNorthern, double height, std::string confirmLasEpsg, std::string colorCode);
    ~VoxelWrite();
    int writeVoxel(const std::string& inputFile, const std::string& outputFile, spdlog::logger& logger, const std::string& batchDateTime);
    VoxelSize calculateVoxelSize(int zoomLevel, double latitude);
    int writeDB(const std::string& spatialId, const std::string& lasFilePath, spdlog::logger& logger);

};

#endif // VOXELWRITE_H
