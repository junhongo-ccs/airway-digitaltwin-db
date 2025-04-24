/**
 * @file spatial_point.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "spatial_point.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>
#include <unordered_set>

#include "common/convert.h"
#include "common/object/vertical_point.h"
#include "common/transformer.h"
#include "spatial_api.h"

namespace spatialid {

// Prototype Declare
VerticalPoint get_altitude_on_vertical_index_and_zoom(int alt_index, int zoom);
Point get_center_point_on_voxel_offset(int lon_index, int lat_index, int zoom,
                                       const VerticalPoint vertical_point,
                                       CRS crs);
void get_vertex_on_voxel_offset(int lon_index, int lat_index, int zoom,
                                const VerticalPoint vertical_point,
                                std::vector<Point>& point_list);

// [API] 座標からグローバル空間IDを取得する
std::string get_global_spatial_id_on_point(const Point& point, CRS crs) {
    return get_spatial_id_on_point(point.x, point.y, point.z, kGlobalZoom, crs);
}

// [API] 座標リストからグローバル空間IDリストを取得する
void get_global_spatial_ids_on_points(const std::vector<Point>& point_list,
                                      std::vector<std::string>& spatial_id_list,
                                      CRS crs) {
    get_spatial_ids_on_points(point_list, spatial_id_list, kGlobalZoom, crs);
}

// [API] 座標リストからグローバル空間ID単位のボクセルデータを取得する
void get_global_spatial_voxel_on_points(
    const std::vector<Point>& point_list,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list, CRS crs) {
    get_spatial_voxel_on_points(point_list, voxel_list, kGlobalZoom, kLocalZoom,
                                crs);
}

// [API] 空間IDの面に直接、接している6個の空間IDを取得する
void get_6spatial_ids_adjacent_to_faces(
    const std::string& spatial_id, std::vector<std::string>& spatial_id_list) {
    spatial_id_list.clear();
    spatial_id_list.reserve(6);

    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, -1, 0, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, -1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, 0, -1));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 1, 0, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, 1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, 0, 1));
}

// [API] 空間IDの水平方向の一周分の8個の空間IDを取得する
void get_8spatial_ids_around_horizontal(
    const std::string& spatial_id, std::vector<std::string>& spatial_id_list) {
    spatial_id_list.clear();
    spatial_id_list.reserve(8);

    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, -1, 0, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, -1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, -1, -1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, -1, 1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 1, 0, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 0, 1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 1, 1, 0));
    spatial_id_list.push_back(get_shifting_spatial_id(spatial_id, 1, -1, 0));
}

// [API] 空間IDを囲う26個の空間IDを取得する
void get_26spatial_ids_around_voxel(const std::string& spatial_id,
                                    std::vector<std::string>& spatial_id_list) {
    spatial_id_list.clear();
    spatial_id_list.reserve(26);

    std::string shifting_spatial_id;
    std::vector<std::string> shifting_spatial_id_list;

    // １つ分低い位置の空間ID
    shifting_spatial_id = get_shifting_spatial_id(spatial_id, 0, 0, -1);
    spatial_id_list.push_back(shifting_spatial_id);

    // １つ分低い位置の空間IDの水平方向の周囲の空間ID
    get_8spatial_ids_around_horizontal(shifting_spatial_id,
                                       shifting_spatial_id_list);
    std::copy(shifting_spatial_id_list.begin(), shifting_spatial_id_list.end(),
              std::back_inserter(spatial_id_list));

    // 指定位置の空間IDの水平方向の周囲の空間ID
    get_8spatial_ids_around_horizontal(spatial_id, shifting_spatial_id_list);
    std::copy(shifting_spatial_id_list.begin(), shifting_spatial_id_list.end(),
              std::back_inserter(spatial_id_list));

    // １つ分高い位置の空間ID
    shifting_spatial_id = get_shifting_spatial_id(spatial_id, 0, 0, 1);
    spatial_id_list.push_back(shifting_spatial_id);

    // １つ分高い位置の空間IDの水平方向の周囲の空間ID
    get_8spatial_ids_around_horizontal(shifting_spatial_id,
                                       shifting_spatial_id_list);
    std::copy(shifting_spatial_id_list.begin(), shifting_spatial_id_list.end(),
              std::back_inserter(spatial_id_list));
}

// [API] 空間IDの中心座標を取得する
Point get_center_point_on_spatial_id(const std::string& spatial_id, CRS crs) {
    Zfxy zfxy(spatial_id);

    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zfxy.zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 高さの分解能を取得する
    VerticalPoint vertical_point =
        get_altitude_on_vertical_index_and_zoom(zfxy.alt_index, zfxy.zoom);

    // 中心点の座標を取得する
    Point point = get_center_point_on_voxel_offset(
        zfxy.lon_index, zfxy.lat_index, zfxy.zoom, vertical_point, crs);

    return point;
}

// [API] 空間IDの頂点座標を取得する
void get_vertex_points_on_spatial_id(const std::string& spatial_id,
                                     std::vector<Point>& point_list, CRS crs) {
    Zfxy zfxy(spatial_id);

    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zfxy.zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 高さの分解能を取得する
    VerticalPoint vertical_point =
        get_altitude_on_vertical_index_and_zoom(zfxy.alt_index, zfxy.zoom);

    // 頂点の座標(計8個)を取得する
    point_list.clear();
    get_vertex_on_voxel_offset(zfxy.lon_index, zfxy.lat_index, zfxy.zoom,
                               vertical_point, point_list);

    if (crs != CRS::kWGS84) {
        // 地理座標から出力座標に座標変換
        Transformer* transformer = Transformer::GetInstance(CRS::kWGS84, crs);
        for (auto& point : point_list) {
            transformer->Transform(point.x, point.y, point.z, point.x, point.y,
                                   point.z);
        }
    }
}

// [API] 座標から空間IDを取得する
std::string get_spatial_id_on_point(double x, double y, double z, int zoom,
                                    CRS crs) {
    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 入力座標から地理座標に設定
    Transformer* transformer = nullptr;
    if (crs != CRS::kWGS84) {
        transformer = Transformer::GetInstance(crs, CRS::kWGS84);
    }

    double lon = x;
    double lat = y;
    double alt = z;

    if (transformer) {
        // 地理座標に座標変換
        transformer->Transform(lon, lat, alt, lon, lat, alt);
    }

    // 地理座標の経度・緯度の桁揃え
    Point::alignment_lonlat(lon, lat);

    // 空間IDを作成する
    Zfxy zfxy = get_spatial_zfxy_on_point(lon, lat, alt, zoom);
    return zfxy.getString();
}

// [API] 座標リストから座標リストの順で空間IDリストを取得する
void get_spatial_ids_on_points(const std::vector<Point>& point_list,
                               std::vector<std::string>& spatial_id_list,
                               int zoom, CRS crs) {
    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 入力座標から地理座標に設定
    Transformer* transformer = nullptr;
    if (crs != CRS::kWGS84) {
        transformer = Transformer::GetInstance(crs, CRS::kWGS84);
    }

    spatial_id_list.clear();
    for (const auto& point : point_list) {
        double lon = point.x;
        double lat = point.y;
        double alt = point.z;

        if (transformer) {
            // 地理座標に座標変換
            transformer->Transform(lon, lat, alt, lon, lat, alt);
        }

        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(lon, lat);

        // 空間IDを作成する
        Zfxy zfxy = get_spatial_zfxy_on_point(lon, lat, alt, zoom);
        spatial_id_list.push_back(zfxy.getString());
    }
}

// 座標リストから重複なしの空間IDリストを取得する
void get_spatial_ids_on_points(const std::vector<Point>& point_list,
                               std::unordered_set<std::string>& spatial_id_list,
                               int zoom, CRS crs) {
    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 入力座標から地理座標に設定
    Transformer* transformer = nullptr;
    if (crs != CRS::kWGS84) {
        transformer = Transformer::GetInstance(crs, CRS::kWGS84);
    }

    spatial_id_list.clear();
    for (const auto& point : point_list) {
        double lon = point.x;
        double lat = point.y;
        double alt = point.z;

        if (transformer) {
            // 地理座標に座標変換
            transformer->Transform(lon, lat, alt, lon, lat, alt);
        }

        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(lon, lat);

        // 空間IDを作成する
        Zfxy zfxy = get_spatial_zfxy_on_point(lon, lat, alt, zoom);
        spatial_id_list.insert(zfxy.getString());
    }
}

// 重複なしの座標リストから重複なしの空間IDリストを取得する
void get_spatial_ids_on_points(
    const std::unordered_set<Point, Point::hash_function>& point_list,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs) {
    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 入力座標から地理座標に設定
    Transformer* transformer = nullptr;
    if (crs != CRS::kWGS84) {
        transformer = Transformer::GetInstance(crs, CRS::kWGS84);
    }

    spatial_id_list.clear();
    for (const auto& point : point_list) {
        double lon = point.x;
        double lat = point.y;
        double alt = point.z;

        if (transformer) {
            // 地理座標に座標変換
            transformer->Transform(lon, lat, alt, lon, lat, alt);
        }

        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(lon, lat);

        // 空間IDを作成する
        Zfxy zfxy = get_spatial_zfxy_on_point(lon, lat, alt, zoom);
        spatial_id_list.insert(zfxy.getString());
    }
}

// [API] 座標リストから空間ID単位のボクセルデータを取得する
void get_spatial_voxel_on_points(
    const std::vector<Point>& point_list,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs) {
    if (global_zoom > local_zoom) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    voxel_list.clear();

    // ローカル空間IDの精度レベルで表現した場合の空間IDを取得
    std::unordered_set<std::string> local_zoom_spatial_ids;
    get_spatial_ids_on_points(point_list, local_zoom_spatial_ids, local_zoom,
                              crs);

    // グローバル空間IDに対するローカル空間IDの精度レベルで表現した空間ID一覧を作成
    std::map<std::string, std::unordered_set<Zfxy, Zfxy::hash_function>>
        spatial_id_map;
    for (const auto& local_zoom_spatial_id : local_zoom_spatial_ids) {
        // 中心座標を取得
        Point wgs84_point =
            get_center_point_on_spatial_id(local_zoom_spatial_id, CRS::kWGS84);

        // 中心座標のグローバル空間IDを取得
        std::string spatial_id =
            get_spatial_id_on_point(wgs84_point.x, wgs84_point.y, wgs84_point.z,
                                    global_zoom, CRS::kWGS84);

        if (auto itr = spatial_id_map.find(spatial_id);
            itr != spatial_id_map.end()) {
            (itr->second).insert(Zfxy(local_zoom_spatial_id));
        } else {
            spatial_id_map[spatial_id] =
                std::unordered_set<Zfxy, Zfxy::hash_function>{
                    Zfxy(local_zoom_spatial_id)};
        }
    }

    // ボクセルデータの作成
    SpatialVoxel::Create(spatial_id_map, local_zoom, voxel_list);
}

// 経度、緯度、高さから空間IDを計算する
Zfxy get_spatial_zfxy_on_point(double lon, double lat, double alt, int zoom,
                               bool is_lon_wrap) {
    // 経度方向の位置の計算
    if (is_lon_wrap) {
        // 経度に180が入力されていると位置+1の値が出力されるため、補正する
        if (lon == 180) {
            lon = -lon;
        }
    }
    double p = std::pow(2, zoom);
    int x = static_cast<int>(std::floor(p * ((lon + 180.0) / 360.0)));

    // 緯度方向の位置の計算
    int y = static_cast<int>(
        std::floor(p *
                   (1 - std::log(std::tan(deg_to_rad(lat)) +
                                 (1 / std::cos(deg_to_rad(lat)))) /
                            M_PI) /
                   2));

    // 高さの位置を計算する
    double alt_resolution = std::pow(2, 25) / std::pow(2, zoom);
    int f = static_cast<int>(std::floor((alt) / alt_resolution));

    return Zfxy(zoom, f, x, y);
}

// 指定した数値分、移動した場合の空間IDを取得する
std::string get_shifting_spatial_id(const std::string& spatial_id, int x, int y,
                                    int f) {
    Zfxy zfxy(spatial_id);

    // インデックスの最大値
    int max_index = static_cast<int>(std::pow(2, zfxy.zoom)) - 1;

    // シフト後のインデックス：経度方向
    int shift_lon_index = zfxy.lon_index + x;
    if (shift_lon_index > max_index || shift_lon_index < 0) {
        // インデックスの範囲を超えている場合はn周分を無視する
        shift_lon_index =
            shift_lon_index % static_cast<int>(std::pow(2, zfxy.zoom));
    }

    // シフト後のインデックス：緯度方向
    int shift_lat_index = zfxy.lat_index + y;
    if (shift_lat_index > max_index || shift_lat_index < 0) {
        // インデックスの範囲を超えている場合はn周分を無視する
        shift_lat_index =
            shift_lat_index % static_cast<int>(std::pow(2, zfxy.zoom));
    }

    // シフト後のインデックス：高さ方向
    int shift_alt_index = zfxy.alt_index + f;

    // シフト後の空間ID
    Zfxy shift_zfxy(zfxy.zoom, shift_alt_index, shift_lon_index,
                    shift_lat_index);
    return shift_zfxy.getString();
}

/**
 * @brief 垂直方向の位置と精度から原点に近い高さと分解能を取得する
 *        入力された位置が負の場合は、原点に遠い高さと分解能を取得する
 *
 * @param alt_index 高さの位置
 * @param zoom 空間IDの精度レベル
 * @return VerticalPoint 高さ情報
 */
VerticalPoint get_altitude_on_vertical_index_and_zoom(int alt_index, int zoom) {
    // 高さ全体の精度あたりの分解能を取得する
    double resolution = std::pow(2, 25) / std::pow(2, zoom);
    VerticalPoint vertial_point(alt_index * resolution, resolution);
    return vertial_point;
}

/**
 * @brief
 * 緯度、経度、高さから精度ごとの一辺の長さのボクセルの中心点の座標を取得する
 *
 * @param lon_index 経度方向の位置
 * @param lat_index 緯度方向の位置
 * @param zoom 空間IDの精度レベル
 * @param vertical_point 高さ情報
 * @return Point 中心点の座標
 */
Point get_center_point_on_voxel_offset(int lon_index, int lat_index, int zoom,
                                       const VerticalPoint vertical_point,
                                       CRS crs) {
    // 頂点座標から最大値と最小値を取得する
    std::vector<Point> point_list;
    get_vertex_on_voxel_offset(lon_index, lat_index, zoom, vertical_point,
                               point_list);

    // 頂点座標から最大値と最小値を取得する
    double lon_max = point_list[0].x;
    double lon_min = lon_max;
    double lat_max = point_list[0].y;
    double lat_min = lat_max;
    double alt_max = point_list[0].z;
    double alt_min = alt_max;
    for (int i = 1; i < point_list.size(); i++) {
        lon_max = std::max(lon_max, point_list[i].x);
        lon_min = std::min(lon_min, point_list[i].x);
        lat_max = std::max(lat_max, point_list[i].y);
        lat_min = std::min(lat_min, point_list[i].y);
        alt_max = std::max(alt_max, point_list[i].z);
        alt_min = std::min(alt_min, point_list[i].z);
    }

    // ボクセルの中心座標を計算する
    double center_lon = (lon_max + lon_min) / 2;
    double center_lat = (lat_max + lat_min) / 2;
    double center_alt = (alt_max + alt_min) / 2;

    // 地理座標の経度・緯度の桁揃え
    Point::alignment_lonlat(center_lon, center_lat);

    if (crs != CRS::kWGS84) {
        // 地理座標から出力座標に座標変換
        Transformer* transformer = Transformer::GetInstance(CRS::kWGS84, crs);
        transformer->Transform(center_lon, center_lat, center_alt, center_lon,
                               center_lat, center_alt);
    }

    return Point(center_lon, center_lat, center_alt);
}

/**
 * @brief
 * 緯度、経度、高さから分解能を一辺の長さとしたボクセルの頂点を取得する
 *
 * @param lon_index 経度方向の位置
 * @param lat_index 緯度方向の位置
 * @param zoom 空間IDの精度レベル
 * @param vertical_point 高さ情報
 * @param[out] point_list 各頂点のリスト
 */
void get_vertex_on_voxel_offset(int lon_index, int lat_index, int zoom,
                                const VerticalPoint vertical_point,
                                std::vector<Point>& point_list) {
    // 緯度の取得
    double p = std::pow(2, zoom);
    if (lat_index > (static_cast<int>(p) - 1)) {
        lat_index = static_cast<int>(p) - 1;
    } else if (lat_index < 0) {
        lat_index = 0;
    }

    // タイルの上辺の緯度
    double north_lat =
        rad_to_deg(std::atan(std::sinh(M_PI * (1 - 2 * lat_index / p))));

    // タイルの下辺の緯度
    double south_lat =
        rad_to_deg(std::atan(std::sinh(M_PI * (1 - 2 * (lat_index + 1) / p))));

    // 経度の取得
    if (lon_index > (p - 1) || lon_index < 0) {
        // インデックスの範囲を超えている場合はn周分を無視する
        lon_index = static_cast<int>(fmod(lon_index, p));
    }
    double west_lon = lon_index * (360 / p) - 180;
    double east_lon = (lon_index + 1) * (360 / p) - 180;

    // 地理座標の経度・緯度の桁揃え
    Point::alignment_lonlat(west_lon, north_lat);
    Point::alignment_lonlat(east_lon, south_lat);

    // 各頂点を計算する（左下から反時計回りに取得をする）
    point_list.clear();
    point_list.reserve(8);
    point_list.push_back(
        Point(west_lon, north_lat, vertical_point.z));  // north_west_bottom
    point_list.push_back(
        Point(east_lon, north_lat, vertical_point.z));  // north_east_bottom
    point_list.push_back(
        Point(east_lon, south_lat, vertical_point.z));  // south_east_bottom
    point_list.push_back(
        Point(west_lon, south_lat, vertical_point.z));  // south_west_bottom
    double top = vertical_point.z + vertical_point.resolution;
    point_list.push_back(Point(west_lon, north_lat, top));  // north_west_top
    point_list.push_back(Point(east_lon, north_lat, top));  // north_east_top
    point_list.push_back(Point(east_lon, south_lat, top));  // south_east_top
    point_list.push_back(Point(west_lon, south_lat, top));  // south_west_top
}

}  // namespace spatialid