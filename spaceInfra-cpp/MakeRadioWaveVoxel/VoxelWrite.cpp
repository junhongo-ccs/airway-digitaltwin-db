#include "VoxelWrite.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <cmath>
#include <liblas/liblas.hpp>
#include <laszip/laszip_api.h>
#include <filesystem>

#include <sstream>
#include "Config.h"
#include "DataAccessAdapter.h"

// boundaryRegistry の実体、インスタンス間で共有する
std::unordered_map<std::string, BoundaryInfo> VoxelWrite::boundaryRegistry;

//! @brief ボクセル書き込みクラスのコンストラクタ
VoxelWrite::VoxelWrite(int utmZone, bool isNorthern, double height, std::string confirmLasEpsg, std::string colorCode){
    this->utmZone = utmZone;
    this->isNorthern = isNorthern;
    this->height = height;
    initializeTransform(confirmLasEpsg);
	// 16進数のカラーコードをRGBに変換
    if (sscanf_s(colorCode.c_str(), "%2hhx%2hhx%2hhx", &red, &green, &blue) != 3) {
        throw std::runtime_error("Invalid color code format");
    }
}

//! @brief ボクセル書き込みクラスのデストラクタ
VoxelWrite::~VoxelWrite() {
    if (transformUTMtoLatLng) proj_destroy(transformUTMtoLatLng);
    if (ctx) proj_context_destroy(ctx);
}

//! @brief projライブラリの初期化
//! @param confirmLasEpsg 確認用のEPSGコード
void VoxelWrite::initializeTransform(std::string confirmLasEpsg) {
	// PROJの初期化
    ctx = proj_context_create();
    if (ctx == nullptr) {
        throw std::runtime_error("Failed to create PROJ context");
    }

    // UTMゾーンに対応するEPSGコードを設定
    std::string src_utm = "+proj=utm +zone=" + std::to_string(utmZone) + (isNorthern ? " +north" : " +south")
        + " +datum=WGS84 +units=m";

	std::string target_epsg = "EPSG:" + confirmLasEpsg;

    transformUTMtoLatLng = proj_create_crs_to_crs(ctx, src_utm.c_str(), target_epsg.c_str(), nullptr);
    if (transformUTMtoLatLng == nullptr) {
        proj_context_destroy(ctx);
        throw std::runtime_error("Failed to create transformation");
    }
}


//! @brief UTM座標から緯度経度を求める
//! @param utm_x UTM座標x
//! @param utm_y UTM座標y
//! @param lat 緯度
//! @param lng 経度
void VoxelWrite::utmToLatLon(double utm_x, double utm_y, double& lat, double& lng) {
    PJ_COORD input_coord = proj_coord(utm_x, utm_y, 0.0, 0.0);
    PJ_COORD coord = proj_trans(transformUTMtoLatLng, PJ_FWD, input_coord);
    // エラーチェック
    if (proj_errno(transformUTMtoLatLng) != 0) {
        int err = proj_errno(transformUTMtoLatLng);
        const char* err_msg = proj_errno_string(err);
        std::cerr << "座標変換エラー: " << err_msg << std::endl;
        lat = lng = std::numeric_limits<double>::quiet_NaN();
        return;
    }
    // 浮動小数誤差を無くす
    lat = roundCoord(coord.lp.lam, 6); // 緯度
	lng = roundCoord(coord.lp.phi, 6); // 経度
}


//! @brief ボクセルサイズを計算する関数
//! @param zoomLevel ズームレベル
//! @param lat 緯度
VoxelSize VoxelWrite::calculateVoxelSize(int zoomLevel, double lat) {
    VoxelSize size;
	double latRad = lat * M_PI / 180.0;  // 緯度をラジアンに変換

    // 東西・南北方向のサイズ計算
    double horizontalSize = EARTH_RADIUS * 2 * M_PI * cos(latRad) / pow(2, zoomLevel);
    size.x = horizontalSize;
    size.y = horizontalSize;

    // 鉛直方向のサイズ計算
    const double MAX_HEIGHT = 33554432.0;
    size.z = MAX_HEIGHT / pow(2, zoomLevel);

    return size;
}


// @brief 空間IDを計算する
// @param lng 経度
// @param lat 緯度
// @param h 高さ
// @param z ズームレベル
std::string VoxelWrite::calculateSpatialId(double lng, double lat, double h, int z) {
    double lat_rad = lat * M_PI / 180.0;

    double n = pow(2, z);
    const int Z = 25;
    double H = pow(2, Z);

    int f = floor(n * h / H);
    int x = floor(n * (lng + 180) / 360);
    int y = floor(n / 2 * (1 - log(tan(lat_rad) + (1 / cos(lat_rad))) / M_PI));

    return std::to_string(z) + "_" + std::to_string(f) + "_" +
        std::to_string(x) + "_" + std::to_string(y);
}


/// <summary>
/// 読み込むUTM座標の開始位置をキャッシュする
/// </summary>
/// <param name="gridData"></param>
/// <param name="nrows"></param>
void VoxelWrite::preprocessGridData(const std::string& gridData, int nrows) {
    rowOffsets.clear();
    rowOffsets.reserve(nrows + 1);

    size_t pos = 0;
    rowOffsets.push_back(pos); // 最初の行は位置0から

    for (int i = 0; i < nrows; i++) {
        pos = gridData.find('\n', pos);
        if (pos != std::string::npos) {
            pos++;
            rowOffsets.push_back(pos);
        }
        else {
            break;
        }
    }
}


//! @brief グリッドデータから指定した行列の値を取得する
//! @param row 行
//! @param col 列
//! @param gridData グリッドデータ
//! @param header ヘッダ
int VoxelWrite::readGridValue(int row, int col, const std::string& gridData, const GridHeader& header) {
    if (row < 0 || row >= rowOffsets.size() - 1 || col < 0) {
        return header.NODATA_value;
    }

    size_t pos = rowOffsets[row];
    if (pos + col >= gridData.length()) {
        return header.NODATA_value;
    }

    char value = gridData[pos + col];
    if (value == '0') return 0;
    if (value == '1') return 1;
    return header.NODATA_value;
}


std::vector<GridData> VoxelWrite::readGridDataInBounds(const BoundingBox& bounds,
    const GridHeader& header,
    const VoxelSize& voxel28,
    const std::string& gridData) {
    std::vector<GridData> points;

    // 境界ボックスの幅と高さを計算
    double boxWidth = bounds.maxX - bounds.minX;
    double boxHeight = bounds.maxY - bounds.minY;

    // ズームレベル28のボクセルサイズで分割数を計算
	int numPointsX = floor(boxWidth / voxel28.x);   // ceil -> floor
    int numPointsY = floor(boxHeight / voxel28.y);

    // 境界ボックス内で点を生成
    for (int i = 0; i < numPointsX; i++) {
        for (int j = 0; j < numPointsY; j++) {
            // 点の座標を計算
            double x = bounds.minX + (i * voxel28.x); // delete offset (voxel28.x / 2)
			double y = bounds.minY + (j * voxel28.y); // delete offset (voxel28.y / 2)


            // 境界チェックを追加
            if (x >= bounds.minX && x < bounds.maxX && y >= bounds.minY && y < bounds.maxY) {
                // この点が含まれるグリッドセルを特定
                int col = static_cast<int>((x - header.xllcorner) / header.cellsize);
                int row = static_cast<int>((y - header.yllcorner) / header.cellsize);

                // グリッド範囲内かつ値が1（圏内）の場合のみ点を追加
                if (col >= 0 && col < header.ncols &&
                    row >= 0 && row < header.nrows) {
                    int value = readGridValue(row, col, gridData, header);
                    if (value == 1) {
                        points.emplace_back(GridData{ x, y, height, value });
                    }
                }
            }
        }
    }

    return points;
};


// @brief LAZファイルを新規作成する関数
void VoxelWrite::createNewLazFile(const std::string& spatialId,
    const std::vector<GridData>& points,
    const std::string& lazFilePath,
    const BoundingBox& bounds, spdlog::logger& logger) {

    // LAZファイルパスの設定
    laszip_POINTER laszip = nullptr;
    if (laszip_create(&laszip))
    {
        logger.error("DLL ERROR: creating laszip");
        throw EXIT_FAILURE;
    }

    // LASzipポインタの作成
    laszip_header_struct* header;
    if (laszip_get_header_pointer(laszip, &header))
    {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: getting header pointer");
        throw EXIT_FAILURE;
    }

    // ヘッダーポインタの取得
    header->file_source_ID = 0;
    header->global_encoding = 0;
    header->version_major = 1;
    header->version_minor = 2;
    strncpy_s(header->system_identifier, "MODIFICATION", 32);
    strncpy_s(header->generating_software, "MakeRadioWaveVoxel", 32);
    header->header_size = 227;
    header->offset_to_point_data = 227;
    header->number_of_variable_length_records = 0;
    header->point_data_format = 2;
    header->point_data_record_length = 28;
    header->number_of_point_records = points.size();
    header->number_of_points_by_return[0] = points.size();
    header->number_of_points_by_return[1] = 0;
    header->number_of_points_by_return[2] = 0;
    header->number_of_points_by_return[3] = 0;
    header->number_of_points_by_return[4] = 0;

    header->x_scale_factor = 0.00001; // 0.0000001 -> 0.001 -> 0.00001 で試す
	header->y_scale_factor = 0.00001; // 0.0000001 -> 0.001 -> 0.00001 で試す
    header->z_scale_factor = 0.00001;
    header->x_offset = bounds.minX;
    header->y_offset = bounds.minY;
    header->z_offset = 0.0;
    header->max_x = bounds.maxX;
    header->min_x = bounds.minX;
    header->max_y = bounds.maxY;
    header->min_y = bounds.minY;
    //header->max_z = height;
    //header->min_z = 0;

    // 出力先ディレクトリの確認と作成
    std::filesystem::path outputPath(lazFilePath);
    std::filesystem::path outputDir = outputPath.parent_path();
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directories(outputDir);
    }

    // LAZファイルを開く
    if (laszip_open_writer(laszip, lazFilePath.c_str(), 1))
    {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: opening writer");
        throw EXIT_FAILURE;
    }

	// 点データポインタの取得
    laszip_point_struct* point;
    if (laszip_get_point_pointer(laszip, &point))
    {
        laszip_close_writer(laszip);
        laszip_destroy(laszip);
        logger.error("DLL ERROR: getting point pointer");
        throw EXIT_FAILURE;
    }

    logger.debug("New points: " + std::to_string(points.size()));

    // 点群データの書き込み
    for (const auto& gridPoint : points)
    {
        point->X = static_cast<int>((gridPoint.x - header->x_offset) / header->x_scale_factor);
        point->Y = static_cast<int>((gridPoint.y - header->y_offset) / header->y_scale_factor);
        point->Z = static_cast<int>((gridPoint.z - header->z_offset) / header->z_scale_factor);

        // RGB値の設定
        point->rgb[0] = static_cast<unsigned short>(red) << 8;
        point->rgb[1] = static_cast<unsigned short>(green) << 8;
        point->rgb[2] = static_cast<unsigned short>(blue) << 8;

        if (laszip_write_point(laszip))
        {
            laszip_close_writer(laszip);
            laszip_destroy(laszip);
            logger.error("DLL ERROR: writing point");
            throw EXIT_FAILURE;
        }
    }

    // ファイルを閉じる
    if (laszip_close_writer(laszip))
    {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: closing writer");
        throw EXIT_FAILURE;
    }

    laszip_destroy(laszip);
}


//! @brief 既存のLAZファイルに追加する関数
//! @param spatialId 空間ID
//! @param newPoints 新しい点群
//! @param outputDir 出力ディレクトリ
//! @param bounds 境界ボックス
//! @param logger ロガー
void VoxelWrite::appendToExistingLaz(const std::string& spatialId, const std::vector<GridData>& newPoints,const std::string& outputDir, const BoundingBox& bounds, spdlog::logger& logger) {
	// 1) 既存ファイルのパスを組み立て
    std::string existingFilePath = boundaryRegistry[spatialId].lazFilePath;
    std::string fullExistingPath = outputDir + "/" + spatialId + "/waveObjects/"
                                 + this->batchDateTime + "/voxel/" + existingFilePath;

    // 2) 既存ファイルを読み込み（laszip_open_reader & laszip_read_point）
    std::vector<GridData> existingPoints = readExistingLazFile(fullExistingPath);
    logger.debug("Existing points: " + std::to_string(existingPoints.size()));

    // 3) 既存＋追加データをマージ
    existingPoints.insert(existingPoints.end(), newPoints.begin(), newPoints.end());
    std::sort(existingPoints.begin(), existingPoints.end(),
        [](const GridData& a, const GridData& b) {
            if (a.x != b.x) return a.x < b.x;
            return a.y < b.y;
        });
    existingPoints.erase(
        std::unique(existingPoints.begin(), existingPoints.end(),
            [](const GridData& a, const GridData& b) {
                // x, y, z すべてがほぼ同じ点を削除
                return (std::abs(a.x - b.x) < 1e-6) &&
                        (std::abs(a.y - b.y) < 1e-6) &&
                        (std::abs(a.z - b.z) < 1e-6);
            }),
        existingPoints.end()
    );

    // 4) 既存ファイルの削除（新たに書き込むため）
    if (std::filesystem::exists(fullExistingPath)) {
        std::filesystem::remove(fullExistingPath);
    }

    // 5) laszip_create & ヘッダー取得
    laszip_POINTER laszip = nullptr;
    if (laszip_create(&laszip)) {
        logger.error("DLL ERROR: creating laszip");
        throw EXIT_FAILURE;
    }

    laszip_header_struct* header;
    if (laszip_get_header_pointer(laszip, &header)) {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: getting header pointer");
        throw EXIT_FAILURE;
    }

    // 6) 既存ファイルのヘッダー情報を再考慮し、必要に応じてマージ
    //    (※必要であれば readExistingLazFile で既存ヘッダーを取得し、下記と統合する)
    header->file_source_ID = 0;
    header->global_encoding = 0;
    header->version_major = 1;
    header->version_minor = 2;
    strncpy_s(header->system_identifier, "MODIFICATION", 32);
    strncpy_s(header->generating_software, "MakeRadioWaveVoxel", 32);
    header->header_size = 227;
    header->offset_to_point_data = 227;
    header->number_of_variable_length_records = 0;
    header->point_data_format = 2;
    header->point_data_record_length = 28;
    header->number_of_point_records = existingPoints.size();
    header->number_of_points_by_return[0] = existingPoints.size();

    // スケール・オフセット統合（例：既存ファイルのスケールや座標範囲とマージする）
    header->x_scale_factor = 0.00001;
    header->y_scale_factor = 0.00001;
    header->z_scale_factor = 0.00001;
    header->x_offset = bounds.minX; // 必要に応じて既存ヘッダーと新規boundsをマージ
    header->y_offset = bounds.minY;
    header->z_offset = 0.0;
    header->max_x = bounds.maxX;
    header->min_x = bounds.minX;
    header->max_y = bounds.maxY;
    header->min_y = bounds.minY;

    // 7) 出力先ディレクトリのチェック
    std::filesystem::path outputPath(fullExistingPath);
    std::filesystem::path outDir = outputPath.parent_path();
    if (!std::filesystem::exists(outDir)) {
        std::filesystem::create_directories(outDir);
    }

    // 8) lazファイルを開く (laszip_open_writer)
    if (laszip_open_writer(laszip, fullExistingPath.c_str(), 1)) {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: opening writer");
        throw EXIT_FAILURE;
    }

    laszip_point_struct* point;
    if (laszip_get_point_pointer(laszip, &point)) {
        laszip_close_writer(laszip);
        laszip_destroy(laszip);
        logger.error("DLL ERROR: getting point pointer");
        throw EXIT_FAILURE;
    }

    // 9) 書き込み (laszip_write_point) -- 既存＋追加分
    //    (laszipdllexample.cpp の例のように point->メンバを設定して write)
    for (const auto& gridPoint : existingPoints) {
        point->X = static_cast<int>((gridPoint.x - header->x_offset) / header->x_scale_factor);
        point->Y = static_cast<int>((gridPoint.y - header->y_offset) / header->y_scale_factor);
        point->Z = static_cast<int>((gridPoint.z - header->z_offset) / header->z_scale_factor);

        point->rgb[0] = static_cast<unsigned short>(red) << 8;
        point->rgb[1] = static_cast<unsigned short>(green) << 8;
        point->rgb[2] = static_cast<unsigned short>(blue) << 8;

        if (laszip_write_point(laszip)) {
            laszip_close_writer(laszip);
            laszip_destroy(laszip);
            logger.error("DLL ERROR: writing point");
            throw EXIT_FAILURE;
        }
    }

    // 10) クローズ処理
    if (laszip_close_writer(laszip)) {
        laszip_destroy(laszip);
        logger.error("DLL ERROR: closing writer");
        throw EXIT_FAILURE;
    }

    laszip_destroy(laszip);
}


//! @brief 既存のLAZファイルを読み込む関数
//! @param lazFilePath LAZファイルパス
std::vector<GridData> VoxelWrite::readExistingLazFile(const std::string& lazFilePath) {
    std::vector<GridData> points;
    laszip_POINTER laszip_reader = nullptr;
    laszip_BOOL is_compressed = 1;      // lazなので圧縮されているものとする

    // ファイルの存在チェックを先に行う
    if (!std::filesystem::exists(lazFilePath)) {
        // ファイルが存在しない場合は空のベクターを返す
        return points;
    }

    if (laszip_create(&laszip_reader)) {
        throw std::runtime_error("Failed to create laszip reader");
    }

    try {
        // リーダーを開く
        if (laszip_open_reader(laszip_reader, lazFilePath.c_str(), &is_compressed)) {
            std::string error_msg;
            char* laszip_error;
            if (laszip_get_error(laszip_reader, &laszip_error)) {
                error_msg = "Unknown error while opening LAZ file";
            } else {
                error_msg = laszip_error;
            }
            throw std::runtime_error("Failed to open LAZ file: " + error_msg);
        }

        laszip_header_struct* header;
        if (laszip_get_header_pointer(laszip_reader, &header)) {
            throw std::runtime_error("Failed to get header pointer");
        }

        laszip_point_struct* point;
        if (laszip_get_point_pointer(laszip_reader, &point)) {
            throw std::runtime_error("Failed to get point pointer");
        }

        // 点群データの読み込み
        for (long long i = 0; i < header->number_of_point_records; i++) {
            if (laszip_read_point(laszip_reader)) {
                throw std::runtime_error("Failed to read point");
            }

            GridData gridPoint;
            gridPoint.x = (point->X * header->x_scale_factor) + header->x_offset;
            gridPoint.y = (point->Y * header->y_scale_factor) + header->y_offset;
            gridPoint.z = (point->Z * header->z_scale_factor) + header->z_offset;
            gridPoint.value = 1;
            points.push_back(gridPoint);
        }

    } catch (const std::exception& e) {
        if (laszip_reader) {
            laszip_close_reader(laszip_reader);
            laszip_destroy(laszip_reader);
        }
        throw;
    }

    laszip_close_reader(laszip_reader);
    laszip_destroy(laszip_reader);

    return points;
}


//! @brief 緯度・経度を小数点第decimals位で丸める
//! @param v 小数値
//! @param decimals 小数点第何位まで残すか
//! @return 丸められた値
double VoxelWrite::roundCoord(double v, int decimals = 6) {
    double factor = std::pow(10, decimals);
    return std::round(v * factor) / factor;
}


//! @brief ボクセルデータ作成メイン関数
//! @param inputFile 入力ファイル
//! @param outputDir 出力ディレクトリ
//! @param logger ロガー
//! @param batchDateTime バッチ実行時刻
int VoxelWrite::writeVoxel(const std::string& inputFile, const std::string& outputDir, spdlog::logger& logger, const std::string& batchDateTime) {
    // ファイルのサイズを事前に取得し、適切なバッファサイズを確保
    std::filesystem::path inputPath(inputFile);
    if (!std::filesystem::exists(inputPath)) {
        logger.error("Failed to open input file: {}", inputFile);
        throw EXIT_FAILURE;
    }

    size_t fileSize = std::filesystem::file_size(inputPath);
    // ヘッダー行は別途処理するため、データ部分用にバッファを確保
    std::string gridData;
    gridData.reserve(fileSize);

    // ファイルを開く
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) {
        logger.error("Failed to open input file: {}", inputFile);
        throw EXIT_FAILURE;
    }

	Config& config = Config::getInstance();

    this->batchDateTime = batchDateTime;

	// UTM電波情報ヘッダ
	GridHeader header;
    std::string line;

    // ヘッダ情報の取得
    try {
        // ヘッダー行を読み込み
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "ncols %d", &header.ncols) != 1) {
            logger.error("ncolsが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "nrows %d", &header.nrows) != 1) {
            logger.error("nrowsが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "xllcorner %lf", &header.xllcorner) != 1) {
            logger.error("xllcornerが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "yllcorner %lf", &header.yllcorner) != 1) {
            logger.error("yllcornerが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "cellsize %lf", &header.cellsize) != 1) {
            logger.error("cellsizeが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
        std::getline(inFile, line);
        if (sscanf_s(line.c_str(), "NODATA_value %d", &header.NODATA_value) != 1) {
            logger.error("NODATA_valueが見つかりませんでした。");
            throw EXIT_FAILURE;
        }
    }
    catch (const std::exception& e) {
        logger.error("ヘッダ情報取得中にエラーが発生しました: {}", e.what());
        throw EXIT_FAILURE;
    }

    // 効率的なグリッドデータ読み込み - 一度に大きなバッファで読み込む
    constexpr size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];

    while (inFile.read(buffer, BUFFER_SIZE)) {
        gridData.append(buffer, BUFFER_SIZE);
    }
    if (inFile.gcount() > 0) {
        gridData.append(buffer, inFile.gcount());
    }

    inFile.close();

    // 行オフセットを前処理でキャッシュ
    logger.debug("Preprocessing grid data with {} rows", header.nrows);
    preprocessGridData(gridData, header.nrows);
    logger.debug("Preprocessing complete, cached {} row offsets", rowOffsets.size());

    // 座標変換テーブルを事前計算（毎回計算する必要がない値）
    VoxelSize voxel17 = calculateVoxelSize(17, 33.0);
    VoxelSize voxel28 = calculateVoxelSize(28, 33.0);

    // パフォーマンス向上のための変数をキャッシュ
    double xllcorner = header.xllcorner;
    double yllcorner = header.yllcorner;

    // ボクセル数を計算 - 少し余裕を持たせて切り上げる
    int totalVoxelsX = ceil(header.ncols * header.cellsize / voxel17.x);
    int totalVoxelsY = ceil(header.nrows * header.cellsize / voxel17.y);

    logger.debug("Processing grid with {} x {} voxels at zoom level 17", totalVoxelsX, totalVoxelsY);
    logger.debug("voxel17: {:.6f} x {:.6f}, voxel28: {:.6f} x {:.6f}",
        voxel17.x, voxel17.y, voxel28.x, voxel28.y);

    // 並列処理のための準備
    int numThreads = std::min(12, static_cast<int>(std::thread::hardware_concurrency()));
    logger.debug("Using {} threads for processing", numThreads);

    // 処理状況追跡用の変数
    std::atomic<int> processedVoxels(0);
    std::atomic<int> writtenFiles(0);
    int totalVoxels = totalVoxelsX * totalVoxelsY;

    // スレッドセーフな処理のためのミューテックス
    std::mutex registryMutex;
    std::mutex loggerMutex;
    std::mutex dbMutex;

    // タスクリストを作成
    std::vector<std::pair<int, int>> tasks;
    tasks.reserve(totalVoxels);
    for (int voxelX = 0; voxelX < totalVoxelsX; voxelX++) {
        for (int voxelY = 0; voxelY < totalVoxelsY; voxelY++) {
            tasks.emplace_back(voxelX, voxelY);
        }
    }

    // 各スレッドに均等にタスクを割り当て
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    // 各スレッドが処理するタスクの数
    size_t tasksPerThread = tasks.size() / numThreads;
    size_t remainingTasks = tasks.size() % numThreads;

    // スレッド処理開始
    size_t taskStart = 0;
    for (int threadId = 0; threadId < numThreads; threadId++) {
        // このスレッドが処理するタスクの範囲を計算
        size_t threadTasks = tasksPerThread + (threadId < remainingTasks ? 1 : 0);
        size_t taskEnd = taskStart + threadTasks;

        threads.emplace_back([this, &tasks, taskStart, taskEnd, &header, &voxel17, &voxel28,
            &gridData, &outputDir, &logger, &registryMutex, &loggerMutex, &dbMutex,
            &processedVoxels, &writtenFiles, totalVoxels]() {
                // 各スレッドのローカル変数
                std::string threadLocalSpatialId;
                double threadLocalLat, threadLocalLng;

                // このスレッドに割り当てられたタスクを処理
                for (size_t taskIndex = taskStart; taskIndex < taskEnd; taskIndex++) {
                    const auto& [voxelX, voxelY] = tasks[taskIndex];

                    // ズームレベル17のボクセル境界を計算
                    BoundingBox bounds;
                    bounds.minX = header.xllcorner + (voxelX * voxel17.x);
                    bounds.minY = header.yllcorner + (voxelY * voxel17.y);
                    bounds.maxX = bounds.minX + voxel17.x;
                    bounds.maxY = bounds.minY + voxel17.y;

                    // ボクセルの中心座標を計算
                    double centerX = bounds.minX + (voxel17.x / 2);
                    double centerY = bounds.minY + (voxel17.y / 2);

                    // UTM座標から緯度経度に変換
                    utmToLatLon(centerX, centerY, threadLocalLat, threadLocalLng);

                    // 空間IDを計算
                    threadLocalSpatialId = calculateSpatialId(threadLocalLng, threadLocalLat, height, ZOOM_17);

                    // ボクセル内のグリッドデータを読み込む
                    std::vector<GridData> points = readGridDataInBounds(bounds, header, voxel28, gridData);

                    // 点群データが存在する場合のみ処理
                    if (!points.empty()) {
                        // LAZファイルパス
                        // {confirmRoot} / {batchID} / waveObjects / {yyyyMMddHHmmss(バッチ実行時刻)} / voxel / {batchID}_radioWave.laz
                        std::string lazFilePath = outputDir + "/" + threadLocalSpatialId + "/waveObjects/" +
                            this->batchDateTime + "/voxel/" + threadLocalSpatialId + "_radioWave.laz";

                        // スレッドセーフなregistry操作
                        bool isExistingFile = false;
                        {
                            std::lock_guard<std::mutex> lock(registryMutex);
                            isExistingFile = boundaryRegistry.find(threadLocalSpatialId) != boundaryRegistry.end();
                        }

                        try {
                            if (isExistingFile) {
                                appendToExistingLaz(threadLocalSpatialId, points, outputDir, bounds, logger);
                            }
                            else {
                                createNewLazFile(threadLocalSpatialId, points, lazFilePath, bounds, logger);

                                std::lock_guard<std::mutex> lock(registryMutex);
                                boundaryRegistry[threadLocalSpatialId] = { bounds, threadLocalSpatialId + "_radioWave.laz", false };
                            }

                            // MySQLへの書き込み - スレッドセーフに
                            std::string voxelBitFilePath = threadLocalSpatialId + "/waveObjects/" +
                                this->batchDateTime + "/voxel/" + threadLocalSpatialId + "_radioWave.laz";
                            std::string spatialIdForDB = std::regex_replace(threadLocalSpatialId, std::regex("_"), "/");

                            {
                                std::lock_guard<std::mutex> lock(dbMutex);
                                writeDB(spatialIdForDB, voxelBitFilePath, logger);
                            }

                            // 書き込み成功数をカウント
                            writtenFiles++;
                        }
                        catch (const std::exception& e) {
                            std::lock_guard<std::mutex> lock(loggerMutex);
                            logger.error("Error processing voxel at [{}, {}]: {}", voxelX, voxelY, e.what());
                        }
                    }

                    // 処理済みボクセル数を更新して進捗を表示
                    int processed = ++processedVoxels;
                    if (processed % 100 == 0 || processed == totalVoxels) {
                        std::lock_guard<std::mutex> lock(loggerMutex);
                        logger.debug("Progress: {}/{} voxels processed ({:.1f}%)",
                            processed, totalVoxels, 100.0f * processed / totalVoxels);
                    }
                }
            });

        // 次のスレッドの開始タスクインデックスを更新
        taskStart = taskEnd;
    }

    // すべてのスレッドが終了するのを待つ
    logger.debug("Waiting for all threads to complete...");
    for (auto& thread : threads) {
        thread.join();
    }
    return 0;
}


//! @brief 作成されたボクセルデータをMySQLに書き込む関数
//! @param spatialId 空間ID
//! @param lasFilePath LAZファイルパス
//! @param logger ロガー
int VoxelWrite::writeDB(const std::string& spatialId, const std::string& lasFilePath, spdlog::logger& logger) {
    Config& config = Config::getInstance();
    DataAccessAdapter& adapter = DataAccessAdapter::getInstance();
	
	// データベースに接続
    adapter.connect(config);

	// 電波情報オブジェクトを登録
	adapter.upsertRadioWaveObject(spatialId, lasFilePath);
	return 0;
}
