/**
 * @file spatial_polygons.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-30
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "spatial_polygons.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "common/const.h"
#include "common/object/enum.h"
#include "common/object/line.h"
#include "common/object/projected_point.h"
#include "common/object/vector.h"
#include "common/spatial_line.h"
#include "common/spatial_point.h"
#include "common/transformer.h"
#include "convert.h"
#include "spatial_api.h"

namespace spatialid {

// Prototype Declare
void get_internal_spatial_ids_on_polygons(
    const std::vector<Triangle>& triangles,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs,
    bool include_inner_voxel);
void get_triangle_side_spatial_ids(
    const Triangle& geographic_triangle, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom);
void get_triangle_plane_spatial_ids(
    const Triangle& geographic_triangle, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom);
void get_cross_points_of_voxel_plane(
    const Plane& plane, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom);
void valid_closed_polygons(const std::vector<Triangle>& triangles);
void get_inner_voxel(const std::vector<Triangle>& rectangular_triangles,
                     const std::unordered_set<std::string>& spatial_id_list,
                     std::unordered_set<std::string>& inner_spatial_ids,
                     std::unordered_set<std::string>& outer_spatial_ids);
void get_cross_point_cordinate(
    const Triangle& projected_triangle, const Point& start_orth_point,
    std::vector<std::tuple<Point, int, int>>& cross_point_cordinate_results);
void make_triangle_group(
    const std::tuple<Point, int, int>& cross_point_cordinate_result,
    std::map<std::string, Point>& vertex_dict,
    std::vector<std::string>& edge_list,
    std::map<std::string, std::tuple<Line, std::unordered_set<int>>>&
        edge_line_triangle_group,
    std::map<std::string, std::unordered_set<int>>& vertex_triangle_group,
    std::map<std::string, int>& cross_group_index, int triangle_index,
    const std::vector<Triangle>& rectangular_triangles);
std::string change_point_str(const Point& point);
std::string change_line_str(const Line& line);
void make_line_group(std::vector<Line>& lines,
                     std::vector<std::vector<Point>>& line_group);
void make_right_rotation_line(std::vector<Point>& start_line,
                              std::vector<Line>& lines);
void make_left_rotation_line(std::vector<Point>& start_line,
                             std::vector<Line>& lines);
bool is_same_line(const Line& line_a, const Line& line_b);
void cross_check_edge_vertex(
    const std::map<std::string, std::unordered_set<int>>& vertex_triangle_group,
    const std::map<std::string, std::tuple<Line, std::unordered_set<int>>>&
        edge_line_triangle_group,
    const std::map<std::string, Point>& vertex_dict,
    const std::vector<Triangle>& rectangular_triangles,
    std::unordered_set<std::string>& judgment_completed_cross_points,
    std::vector<bool>& inside_results);
void grouping_triangle(
    std::unordered_set<std::string>& cross_point_set,
    std::unordered_set<int>& union_triangle_indexes,
    const std::map<std::string, std::unordered_set<int>>& edge_triangle_group,
    const std::map<std::string, std::unordered_set<int>>&
        vertex_triangle_group);
bool is_inside(const std::vector<Point>& vertex_point_list,
               const Point& target_point);
bool check_cross_one_vertex(
    const Point& cross_point,
    const std::unordered_set<int>& group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles);
bool check_cross_one_edge(const Line& xy_cross_edge,
                          const std::unordered_set<int>& group_triangle_indexes,
                          const std::vector<Triangle>& rectangular_triangles);
std::tuple<int, int> count_edge_side(
    const Line& xy_cross_edge,
    const std::unordered_set<int>& group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles);
bool check_cross_two_edges(
    const Line& xy_cross_edge,
    const std::unordered_set<int>& max_group_triangle_indexes,
    const std::unordered_set<int>& min_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles);
bool check_cross_vertex_and_edge(
    const Point& cross_point, const Line& xy_cross_edge,
    const std::unordered_set<int>& vertex_group_triangle_indexes,
    const std::unordered_set<int>& edge_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles);
std::tuple<int, int> count_vertex_side(
    const std::vector<std::tuple<const Point&, const std::unordered_set<int>&>>&
        cross_point_infos,
    const Line& xy_cross_line,
    const std::vector<Triangle>& rectangular_triangles);
int get_share_triangle_index(
    const std::string& max_z_point,
    const std::map<std::string, std::unordered_set<int>>&
        vertex_triangle_group);
bool check_cross_two_vertexes(
    const Point& max_cross_point, const Point& min_cross_point,
    const Line& xy_cross_edge,
    const std::unordered_set<int>& max_group_triangle_indexes,
    const std::unordered_set<int>& min_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles);
bool search_judge_vertex(int index, Point xy_front_start_point,
                         const Line& xy_vertex_line,
                         const std::vector<Line>& share_front_edges,
                         const std::vector<Line>& share_back_edges,
                         const std::vector<Line>& no_cross_triangle_edges,
                         Point& target_vertex);

// [API] 三角ポリゴンの集合で表されるモデルが含まれるグローバル空間IDを取得する
void get_global_spatial_ids_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::unordered_set<std::string>& spatial_id_list, CRS crs,
    bool needs_closed_checking, bool include_inner_voxel) {
    get_spatial_ids_on_polygons(barrier_triangles, space_triangles,
                                spatial_id_list, kGlobalZoom, crs,
                                needs_closed_checking, include_inner_voxel);
}

// [API]
// 三角ポリゴンの集合で表されるモデルが含まれるグローバル空間ID単位のボクセルデータを取得する
void get_global_spatial_voxel_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list, CRS crs,
    bool needs_closed_checking, bool include_inner_voxel) {
    get_spatial_voxel_on_polygons(barrier_triangles, space_triangles,
                                  voxel_list, kGlobalZoom, kLocalZoom, crs,
                                  needs_closed_checking, include_inner_voxel);
}

// [API] 三角ポリゴンの集合で表されるモデルが含まれる空間IDを取得する
void get_spatial_ids_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs,
    bool needs_closed_checking, bool include_inner_voxel) {
    LOG_DEBUG("ポリゴン処理開始");

    // 三角ポリゴンが形成するモデルが閉塞しているかチェック
    if (needs_closed_checking) {
        LOG_DEBUG("三角ポリゴンが形成するモデルが閉塞しているかチェック");
        valid_closed_polygons(barrier_triangles);
        valid_closed_polygons(space_triangles);
    }

    spatial_id_list.clear();

    if (barrier_triangles.size() == 0) {
        // 有効な範囲の三角ポリゴン指定なし
        LOG_DEBUG("有効な範囲の三角ポリゴン指定なし");
        return;
    }

    // 有効な範囲の空間IDを取得
    get_internal_spatial_ids_on_polygons(barrier_triangles, spatial_id_list,
                                         zoom, crs, include_inner_voxel);
    if (spatial_id_list.size() == 0) {
        // 有効な範囲の空間IDなし
        LOG_DEBUG("有効な範囲の空間IDなし");
        return;
    }

    if (space_triangles.size() > 0) {
        // 除外する三角ポリゴンが指定された場合、除外対象の空間IDを取得
        LOG_DEBUG("除外する三角形ポリゴンが指定あり");
        std::unordered_set<std::string> invalid_spatial_id_list;
        get_internal_spatial_ids_on_polygons(space_triangles,
                                             invalid_spatial_id_list, zoom, crs,
                                             include_inner_voxel);
        if (invalid_spatial_id_list.size() > 0) {
            // 有効な範囲から、除外対象の空間IDを取り除く
            LOG_DEBUG("除外する範囲の空間IDを除外");
            std::unordered_set<std::string> valid_spatial_id_list =
                spatial_id_list;
            spatial_id_list.clear();
            std::set_difference(
                valid_spatial_id_list.begin(), valid_spatial_id_list.end(),
                invalid_spatial_id_list.begin(), invalid_spatial_id_list.end(),
                std::inserter(spatial_id_list, spatial_id_list.end()));
            return;
        }
    }

    LOG_DEBUG("ポリゴン処理終了");
}

// [API] 三角ポリゴンの集合で表されるモデルが含まれる空間ID単位のボクセルデータを取得する
void get_spatial_voxel_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs, bool needs_closed_checking,
    bool include_inner_voxel) {
    if (global_zoom > local_zoom) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    voxel_list.clear();

    // ローカル空間IDの精度レベルで表現した場合の空間IDを取得
    std::unordered_set<std::string> local_zoom_spatial_ids;
    get_spatial_ids_on_polygons(barrier_triangles, space_triangles,
                                local_zoom_spatial_ids, local_zoom, crs,
                                needs_closed_checking, include_inner_voxel);

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

/**
 * @brief 三角ポリゴンの集合で表されるモデルが含まれる空間IDを取得する
 *
 * @param triangles 空間IDを取得する三角ポリゴンの集合で表されるモデル
 * @param[out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 * @param crs 座標のEPSGコード
 * @param include_inner_voxel 内部ボクセルを含む
 * @throw std::invalid_argument 引数エラー
 */
void get_internal_spatial_ids_on_polygons(
    const std::vector<Triangle>& triangles,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs,
    bool include_inner_voxel) {
    // 精度が範囲内以外の場合、エラーとする
    if (!(check_zoom(zoom))) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    spatial_id_list.clear();
    std::vector<Triangle> rectangular_triangles;

    // 入力座標から地理座標に設定
    Transformer* transformer1 = nullptr;
    if (crs != CRS::kWGS84) {
        transformer1 = Transformer::GetInstance(crs, CRS::kWGS84);
    }
    // 地理座標から投影座標に設定
    Transformer* transformer2 =
        Transformer::GetInstance(CRS::kWGS84, CRS::kWebMercator);

    for (const auto& triangle : triangles) {
        Triangle geographic_triangle;
        Triangle projected_triangle;

        for (int i = 0; i < 3; i++) {
            // 地理座標に座標変換
            if (transformer1) {
                transformer1->Transform(
                    triangle.points[i].x, triangle.points[i].y,
                    triangle.points[i].z, geographic_triangle.points[i].x,
                    geographic_triangle.points[i].y,
                    geographic_triangle.points[i].z);
            } else {
                geographic_triangle.points[i] = triangle.points[i];
            }

            // 地理座標の経度・緯度の桁揃え
            Point::alignment_lonlat(geographic_triangle.points[i].x,
                                    geographic_triangle.points[i].y);

            // 投影座標に変換
            transformer2->Transform(
                geographic_triangle.points[i].x,
                geographic_triangle.points[i].y,
                geographic_triangle.points[i].z, projected_triangle.points[i].x,
                projected_triangle.points[i].y, projected_triangle.points[i].z);
        }

        LOG_DEBUG(
            "三角ポリゴンの座標(地理座標)=p1(%.10f,%.10f,%.10f) "
            "p2(%.10f,%.10f,%.10f) p3(%.10f,%.10f,%.10f)",
            geographic_triangle.points[0].x, geographic_triangle.points[0].y,
            geographic_triangle.points[0].z, geographic_triangle.points[1].x,
            geographic_triangle.points[1].y, geographic_triangle.points[1].z,
            geographic_triangle.points[2].x, geographic_triangle.points[2].y,
            geographic_triangle.points[2].z);

        LOG_DEBUG(
            "三角ポリゴンの座標(投影座標)=p1(%.10f,%.10f,%.10f) "
            "p2(%.10f,%.10f,%.10f) p3(%.10f,%.10f,%.10f)",
            projected_triangle.points[0].x, projected_triangle.points[0].y,
            projected_triangle.points[0].z, projected_triangle.points[1].x,
            projected_triangle.points[1].y, projected_triangle.points[1].z,
            projected_triangle.points[2].x, projected_triangle.points[2].y,
            projected_triangle.points[2].z);

        if (include_inner_voxel) {
            rectangular_triangles.push_back(projected_triangle);
        }

        // 三角ポリゴンの各辺の空間IDを取得
        get_triangle_side_spatial_ids(geographic_triangle, projected_triangle,
                                      spatial_id_list, zoom);

        // 三角ポリゴンの面の空間ID
        get_triangle_plane_spatial_ids(geographic_triangle, projected_triangle,
                                       spatial_id_list, zoom);
    }

    if (include_inner_voxel) {
        // 有効なボクセルを取得
        std::unordered_set<std::string> inner_spatial_ids;
        std::unordered_set<std::string> outer_spatial_ids;
        get_inner_voxel(rectangular_triangles, spatial_id_list, inner_spatial_ids,
                        outer_spatial_ids);

        LOG_DEBUG_UNORDERED_SET_STRING(spatial_id_list,
                                    "有効なボクセルを取得前の空間ID=");
        LOG_DEBUG_UNORDERED_SET_STRING(inner_spatial_ids,
                                    "有効なボクセルを取得した空間ID=");
        LOG_DEBUG_UNORDERED_SET_STRING(outer_spatial_ids,
                                    "無効なボクセルを取得した空間ID=");

        if (inner_spatial_ids.size() > 0) {
            std::copy(inner_spatial_ids.begin(), inner_spatial_ids.end(),
                    std::inserter(spatial_id_list, spatial_id_list.end()));
        }
    } else {
        LOG_DEBUG_UNORDERED_SET_STRING(spatial_id_list,
                                       "有効なボクセルを取得前の空間ID=");
    }
}

/**
 * @brief 三角ポリゴンの辺の空間IDを取得
 *
 * @param geographic_triangle 三角ポリゴンの頂点座標(地理座標)
 * @param projected_triangle 三角ポリゴンの頂点座標(投影座標)
 * @param[in,out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 */
void get_triangle_side_spatial_ids(
    const Triangle& geographic_triangle, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom) {
    // 三角ポリゴンの頂点
    ProjectedPoint point[3] = {
        {geographic_triangle.points[0].x, geographic_triangle.points[0].y,
         geographic_triangle.points[0].z, projected_triangle.points[0].x,
         projected_triangle.points[0].y},
        {geographic_triangle.points[1].x, geographic_triangle.points[1].y,
         geographic_triangle.points[1].z, projected_triangle.points[1].x,
         projected_triangle.points[1].y},
        {geographic_triangle.points[2].x, geographic_triangle.points[2].y,
         geographic_triangle.points[2].z, projected_triangle.points[2].x,
         projected_triangle.points[2].y}};

    // 三角ポリゴンの1-2点間の辺上の空間ID取得
    std::vector<std::string> spatial_ids;
    get_spatial_ids_on_line(point[0], point[1], spatial_ids, zoom);
    std::copy(spatial_ids.begin(), spatial_ids.end(),
              std::inserter(spatial_id_list, spatial_id_list.end()));

    LOG_DEBUG_VECTOR_STRING(spatial_ids,
                            "三角ポリゴンの各辺の空間ID(1-2点間)=");

    // 三角ポリゴンの2-3点間の辺上の空間ID取得
    spatial_ids.clear();
    get_spatial_ids_on_line(point[1], point[2], spatial_ids, zoom);
    std::copy(spatial_ids.begin(), spatial_ids.end(),
              std::inserter(spatial_id_list, spatial_id_list.end()));

    LOG_DEBUG_VECTOR_STRING(spatial_ids,
                            "三角ポリゴンの各辺の空間ID(2-3点間)=");

    // 三角ポリゴンの3-1点間の辺上の空間ID取得
    spatial_ids.clear();
    get_spatial_ids_on_line(point[2], point[0], spatial_ids, zoom);
    std::copy(spatial_ids.begin(), spatial_ids.end(),
              std::inserter(spatial_id_list, spatial_id_list.end()));

    LOG_DEBUG_VECTOR_STRING(spatial_ids,
                            "三角ポリゴンの各辺の空間ID(3-1点間)=");
}

/**
 * @brief
 * ボクセル境界面と三角ポリゴンの面との交点間の空間IDを取得
 *
 * @param geographic_triangle 三角ポリゴンの頂点座標(地理座標)
 * @param projected_triangle 三角ポリゴンの頂点座標(投影座標)
 * @param[in,out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 */
void get_triangle_plane_spatial_ids(
    const Triangle& geographic_triangle, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom) {
    // 頂点の空間ID
    Zfxy zfxy[3];
    for (int i = 0; i < 3; i++) {
        zfxy[i] = get_spatial_id_on_point(
            geographic_triangle.points[i].x, geographic_triangle.points[i].y,
            geographic_triangle.points[i].z, zoom);
    }

    // X境界面上の空間ID取得
    int max_id =
        std::max({zfxy[0].lon_index, zfxy[1].lon_index, zfxy[2].lon_index});
    int min_id =
        std::min({zfxy[0].lon_index, zfxy[1].lon_index, zfxy[2].lon_index});

    // X軸ボクセル境界面を取得
    std::vector<Plane> plane_list;  // 投影座標
    generate_x_voxel_plane(max_id, min_id, plane_list, zoom);

    // Xボクセル境界面との交点取得
    for (const auto& plane : plane_list) {
        std::unordered_set<std::string> spatial_ids;
        get_cross_points_of_voxel_plane(plane, projected_triangle, spatial_ids,
                                        zoom);
        std::copy(spatial_ids.begin(), spatial_ids.end(),
                  std::inserter(spatial_id_list, spatial_id_list.end()));

        LOG_DEBUG_UNORDERED_SET_STRING(spatial_ids,
                                       "三角ポリゴンの面の空間ID(X境界面)=");
    }

    // Y境界面上の空間ID取得
    max_id =
        std::max({zfxy[0].lat_index, zfxy[1].lat_index, zfxy[2].lat_index});
    min_id =
        std::min({zfxy[0].lat_index, zfxy[1].lat_index, zfxy[2].lat_index});

    // Y軸ボクセル境界面を取得
    plane_list.clear();
    generate_y_voxel_plane(max_id, min_id, plane_list, zoom);

    // Yボクセル境界面との交点取得
    for (const auto& plane : plane_list) {
        std::unordered_set<std::string> spatial_ids;
        get_cross_points_of_voxel_plane(plane, projected_triangle, spatial_ids,
                                        zoom);
        std::copy(spatial_ids.begin(), spatial_ids.end(),
                  std::inserter(spatial_id_list, spatial_id_list.end()));

        LOG_DEBUG_UNORDERED_SET_STRING(spatial_ids,
                                       "三角ポリゴンの面の空間ID(Y境界面)=");
    }

    // Z境界面上の空間ID取得
    max_id =
        std::max({zfxy[0].alt_index, zfxy[1].alt_index, zfxy[2].alt_index});
    min_id =
        std::min({zfxy[0].alt_index, zfxy[1].alt_index, zfxy[2].alt_index});

    // Z軸ボクセル境界面を取得
    plane_list.clear();
    generate_z_voxel_plane(max_id, min_id, plane_list, zoom);

    // Zボクセル境界面との交点取得
    for (const auto& plane : plane_list) {
        std::unordered_set<std::string> spatial_ids;
        get_cross_points_of_voxel_plane(plane, projected_triangle, spatial_ids,
                                        zoom);
        std::copy(spatial_ids.begin(), spatial_ids.end(),
                  std::inserter(spatial_id_list, spatial_id_list.end()));

        LOG_DEBUG_UNORDERED_SET_STRING(spatial_ids,
                                       "三角ポリゴンの面の空間ID(Z境界面)=");
    }
}

/**
 * @brief
 * 三角ポリゴンの頂点から線分を取得し、線分と衝突判定面が交差する空間IDリストを取得する
 *
 * @param plane 衝突判定面
 * @param projected_triangle 三角ポリゴンの頂点座標(投影座標)
 * @param[out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 */
void get_cross_points_of_voxel_plane(
    const Plane& plane, const Triangle& projected_triangle,
    std::unordered_set<std::string>& spatial_id_list, int zoom) {
    spatial_id_list.clear();

    // 三角ポリゴンの線を取得
    Line line_ab = Line::from_points(projected_triangle.points[0],
                                     projected_triangle.points[1]);
    Line line_bc = Line::from_points(projected_triangle.points[1],
                                     projected_triangle.points[2]);
    Line line_ca = Line::from_points(projected_triangle.points[2],
                                     projected_triangle.points[0]);

    // 同じ境界面でとれた2点を始点と終点
    std::vector<Point> start_point;
    std::vector<Point> end_point;

    // ボクセル境界と線の交点の座標を取得
    Point cross_point;
    if (detect_collision(plane, line_ab, cross_point)) {
        // 同じ境界面でとれた始点
        start_point.push_back(cross_point);
    }
    if (detect_collision(plane, line_bc, cross_point)) {
        if (start_point.size() == 0) {
            start_point.push_back(cross_point);
        } else if (start_point[0] != cross_point) {
            // 同じ境界面でとれた終点
            end_point.push_back(cross_point);
        }
    }
    if (start_point.size() > 0 && end_point.size() == 0) {
        if (detect_collision(plane, line_ca, cross_point)) {
            if (start_point[0] != cross_point) {
                end_point.push_back(cross_point);
            }
        }
    }

    // 始点と終点が取得できた場合
    if (start_point.size() > 0 && end_point.size() > 0) {
        // 投影座標から地理座標に設定
        Transformer* transformer =
            Transformer::GetInstance(CRS::kWebMercator, CRS::kWGS84);

        ProjectedPoint projected_start_point = {
            0, 0, start_point[0].z, start_point[0].x, start_point[0].y};
        ProjectedPoint projected_end_point = {0, 0, end_point[0].z,
                                              end_point[0].x, end_point[0].y};

        // 投影座標から地理座標に座標変換
        transformer->Transform(start_point[0].x, start_point[0].y,
                               start_point[0].z, projected_start_point.lon,
                               projected_start_point.lat,
                               projected_start_point.alt);
        transformer->Transform(end_point[0].x, end_point[0].y, end_point[0].z,
                               projected_end_point.lon, projected_end_point.lat,
                               projected_end_point.alt);

        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(projected_start_point.lon,
                                projected_start_point.lat);
        Point::alignment_lonlat(projected_end_point.lon,
                                projected_end_point.lat);

        // 境界面と三角ポリゴンが交差する点の空間IDを取得
        std::vector<std::string> spatial_ids;
        get_spatial_ids_on_line(projected_start_point, projected_end_point,
                                spatial_ids, zoom);
        std::copy(spatial_ids.begin(), spatial_ids.end(),
                  std::inserter(spatial_id_list, spatial_id_list.end()));
    }
}

/**
 * @brief 三角ポリゴンが形成するモデルが閉塞しているかをチェックする
 *
 * @param triangles 三角ポリゴンの集合で表されるモデル
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 以下のいずれかを満たす場合は、閉塞していないものとする。
 * ・頂点を共有する三角ポリゴンにおいて、共有する頂点以外の頂点を抽出したとき、同じ頂点が奇数個抽出された場合
 * ・辺を共有するポリゴンが奇数個の場合
 */
void valid_closed_polygons(const std::vector<Triangle>& triangles) {
    // 三角ポリゴンが共有する頂点をキー、共有する頂点以外の頂点の出現回数を値
    std::map<Point, std::map<Point, int>> point_to_polygons;
    // 三角ポリゴンが共有する辺をキー、辺を共有するポリゴンの数を値
    std::map<std::tuple<Point, Point>, int> line_to_polygons;

    for (const auto& triangle : triangles) {
        LOG_DEBUG(
            "triangle=p1(%.10f,%.10f,%.10f) p2(%.10f,%.10f,%.10f) p3 "
            "(%.10f,%.10f,%.10f)",
            triangle.points[0].x, triangle.points[0].y, triangle.points[0].z,
            triangle.points[1].x, triangle.points[1].y, triangle.points[1].z,
            triangle.points[2].x, triangle.points[2].y, triangle.points[2].z);

        // 三角ポリゴンの各座標を辞書のキー
        std::vector<Point> points;
        points.push_back(triangle.points[0]);
        points.push_back(triangle.points[1]);
        points.push_back(triangle.points[2]);

        // 順序を固定するためにソートする
        std::sort(points.begin(), points.end());

        // 三角ポリゴンが共有する頂点をキー、共有する頂点以外の頂点の出現回数を値
        point_to_polygons[triangle.points[0]][triangle.points[1]] += 1;
        point_to_polygons[triangle.points[0]][triangle.points[2]] += 1;
        point_to_polygons[triangle.points[1]][triangle.points[0]] += 1;
        point_to_polygons[triangle.points[1]][triangle.points[2]] += 1;
        point_to_polygons[triangle.points[2]][triangle.points[0]] += 1;
        point_to_polygons[triangle.points[2]][triangle.points[1]] += 1;

        // 三角ポリゴンが共有する辺をキー、辺を共有するポリゴンの数を値
        line_to_polygons[std::make_tuple(triangle.points[0],
                                         triangle.points[1])] += 1;
        line_to_polygons[std::make_tuple(triangle.points[1],
                                         triangle.points[2])] += 1;
        line_to_polygons[std::make_tuple(triangle.points[2],
                                         triangle.points[0])] += 1;
    }

    // 頂点を共有する三角ポリゴンにおいて、共有する頂点以外の頂点を抽出したとき、同じ頂点が奇数個抽出された場合
    for (const auto& pair1 : point_to_polygons) {
        for (const auto& pair2 : pair1.second) {
            LOG_DEBUG("apex_num=%d", pair2.second);
            if ((pair2.second % 2) == 1) {
                throw std::invalid_argument("POLYGON_NOT_CLOSED_MODEL");
            }
        }
    }

    // 辺を共有するポリゴンが奇数個の場合
    for (const auto& pair : line_to_polygons) {
        LOG_DEBUG("triangle_num=%d", pair.second);
        if ((pair.second % 2) == 1) {
            throw std::invalid_argument("POLYGON_NOT_CLOSED_MODEL");
        }
    }
}

/**
 * @brief 内部ボクセル取得
 *
 * @param rectangular_triangles 三角ポリゴン(投影座標)
 * @param spatial_id_list 空間IDリスト
 * @param[out] inner_spatial_ids 内部の空間IDリスト
 * @param[out] outer_spatial_ids 外部の空間IDリスト
 */
void get_inner_voxel(const std::vector<Triangle>& rectangular_triangles,
                     const std::unordered_set<std::string>& spatial_id_list,
                     std::unordered_set<std::string>& inner_spatial_ids,
                     std::unordered_set<std::string>& outer_spatial_ids) {
    inner_spatial_ids.clear();
    outer_spatial_ids.clear();

    // 三角形ポリゴンが重なっている空間IDの最大の高さ
    int alt_max_index = std::numeric_limits<int>::min();
    for (const auto& spatial_id : spatial_id_list) {
        Zfxy zfxy(spatial_id);
        alt_max_index = std::max(alt_max_index, zfxy.alt_index);
    }
    LOG_DEBUG("三角形ポリゴンが重なっている空間IDの最大の高さ=%d",
              alt_max_index);

    for (const auto& spatial_id : spatial_id_list) {
        // 内外判定対象空間ID
        std::string check_spatial_id =
            get_shifting_spatial_id(spatial_id, 0, 0, 1);
        LOG_DEBUG("内外判定対象空間ID=%s", check_spatial_id.c_str());

        // 内外判定対象空間IDが内判定済みの場合
        if (spatial_id_list.find(check_spatial_id) != spatial_id_list.end()) {
            LOG_DEBUG(
                "内外判定対象空間IDが三角ポリゴンの空間IDの場合はスキップ");
            continue;
        }

        // 内外判定対象空間IDから内部判定を行う空間ID
        std::unordered_set<std::string> check_spatial_ids;
        bool is_outer = false;
        while (true) {
            // 空間IDの高さ成分を取得
            Zfxy check_zfxy(check_spatial_id);

            if (inner_spatial_ids.find(check_spatial_id) !=
                inner_spatial_ids.end()) {
                // 内外判定対象空間IDが内判定済みの場合
                std::copy(
                    check_spatial_ids.begin(), check_spatial_ids.end(),
                    std::inserter(inner_spatial_ids, inner_spatial_ids.end()));
                is_outer = true;
                LOG_DEBUG("内外判定対象空間IDが内判定済み=%d",
                          check_spatial_ids.size());
                break;
            } else if (outer_spatial_ids.find(check_spatial_id) !=
                       outer_spatial_ids.end()) {
                // 内外判定対象空間IDが外判定済みの場合
                std::copy(
                    check_spatial_ids.begin(), check_spatial_ids.end(),
                    std::inserter(outer_spatial_ids, outer_spatial_ids.end()));
                is_outer = true;
                LOG_DEBUG("内外判定対象空間IDが外判定済み=%d",
                          check_spatial_ids.size());
                break;
            } else if (check_zfxy.alt_index > alt_max_index) {
                // 三角形ポリゴンが重なっている空間IDの最大の高さを超過
                std::copy(
                    check_spatial_ids.begin(), check_spatial_ids.end(),
                    std::inserter(outer_spatial_ids, outer_spatial_ids.end()));
                is_outer = true;
                LOG_DEBUG("空間IDの最大の高さを超過=%d",
                          check_spatial_ids.size());
                break;
            } else if (spatial_id_list.find(check_spatial_id) !=
                       spatial_id_list.end()) {
                // 三角ポリゴンが重なっている空間IDに衝突
                LOG_DEBUG("三角ポリゴンが重なっている空間IDに衝突=%d",
                          check_spatial_ids.size());
                check_spatial_id =
                    get_shifting_spatial_id(check_spatial_id, 0, 0, -1);
                break;
            }

            check_spatial_ids.insert(check_spatial_id);
            check_spatial_id =
                get_shifting_spatial_id(check_spatial_id, 0, 0, 1);
            LOG_VERBOSE("内外判定対象空間IDをZ方向に+1=%s",
                        check_spatial_id.c_str());
        }

        // 三角ポリゴンが重なる空間IDがない場合は次の空間IDの処理へ移る
        if (is_outer) {
            continue;
        }

        LOG_DEBUG_UNORDERED_SET_STRING(check_spatial_ids,
                                       "内外判定対象の空間ID=");

        // 内外判定
        // 始点ボクセルの中心座標(投影座標)
        Point start_orth_point =
            get_center_point_on_spatial_id(check_spatial_id, CRS::kWebMercator);

        LOG_DEBUG("始点ボクセルの中心座標(空間ID)=%s",
                  check_spatial_id.c_str());
        LOG_DEBUG("始点ボクセルの中心座標(投影座標)=(%.10f,%.10f,%.10f)",
                  start_orth_point.x, start_orth_point.y, start_orth_point.z);

        // ボクセルに対するグループ
        std::map<std::string, Point> vertex_dict;
        // 衝突辺の文字列リスト
        std::vector<std::string> edge_list;
        // 辺の線と三角ポリゴングループ（キー：衝突点の文字列にグループ番号を加えた値、値：辺の線情報と三角ポリゴンのインデックスのタプル）
        std::map<std::string, std::tuple<Line, std::unordered_set<int>>>
            edge_line_triangle_group;
        // 頂点の三角ポリゴングループ（キー：衝突点の文字列にグループ番号を加えた値、値：三角ポリゴンのインデックス）
        std::map<std::string, std::unordered_set<int>> vertex_triangle_group;
        // 衝突点に対するグループ数+1
        std::map<std::string, int> cross_group_index;
        // 処理済みの衝突点集合
        std::unordered_set<std::string> judgment_completed_cross_points;
        // 交差判定結果リスト
        std::vector<std::vector<std::tuple<Point, int, int>>>
            cross_point_cordinate_results_list;

        // 衝突回数
        int collision_count = 0;

        // 三角ポリゴン毎にレイとの衝突判定
        int triangle_index = 0;
        for (const auto& projected_triangle : rectangular_triangles) {
            LOG_DEBUG(
                "三角ポリゴン毎にレイとの衝突判定=%d "
                "p1(%.10f,%.10f,%.10f) "
                "p2(%.10f,%.10f,%.10f) p3(%.10f,%.10f,%.10f)",
                triangle_index, projected_triangle.points[0].x,
                projected_triangle.points[0].y, projected_triangle.points[0].z,
                projected_triangle.points[1].x, projected_triangle.points[1].y,
                projected_triangle.points[1].z, projected_triangle.points[2].x,
                projected_triangle.points[2].y, projected_triangle.points[2].z);

            // 交差判定
            std::vector<std::tuple<Point, int, int>>
                cross_point_cordinate_results;
            get_cross_point_cordinate(projected_triangle, start_orth_point,
                                      cross_point_cordinate_results);
            LOG_DEBUG("交差判定の結果=%d点",
                      cross_point_cordinate_results.size());

            // グループ作成実施(グループ作成出来ない場合は衝突判定・内部判定済み)
            for (const auto& cross_point_cordinate_result :
                 cross_point_cordinate_results) {
                make_triangle_group(cross_point_cordinate_result, vertex_dict,
                                    edge_list, edge_line_triangle_group,
                                    vertex_triangle_group, cross_group_index,
                                    triangle_index, rectangular_triangles);
            }

            if (cross_point_cordinate_results.size() > 0) {
                cross_point_cordinate_results_list.push_back(
                    cross_point_cordinate_results);
            }

            triangle_index++;
        }

        LOG_DEBUG("衝突点の結果を元に内部判定を実施開始=%s",
                  check_spatial_id.c_str());

        // 衝突点の結果を元に内部判定を実施
        for (const auto& cross_point_cordinate_results :
             cross_point_cordinate_results_list) {
            std::vector<bool> inside_results;
            if ((cross_point_cordinate_results.size() == 1) &&
                // 面と衝突する場合
                std::get<1>(cross_point_cordinate_results[0]) == -1 &&
                std::get<2>(cross_point_cordinate_results[0]) == -1) {
                LOG_DEBUG("面と衝突");
                inside_results.push_back(true);
            } else {
                // 辺頂点に衝突した場合の内部判定
                cross_check_edge_vertex(
                    vertex_triangle_group, edge_line_triangle_group,
                    vertex_dict, rectangular_triangles,
                    judgment_completed_cross_points, inside_results);
                LOG_DEBUG("辺頂点に衝突した場合の内部判定=%d件",
                          inside_results.size());
            }

            for (const auto& inside_result : inside_results) {
                if (inside_result) {
                    collision_count++;
                }
            }

            LOG_DEBUG("衝突回数=%d", collision_count);
        }

        if (collision_count % 2 == 1) {
            // 衝突回数が奇数の場合は内側判定
            LOG_DEBUG("衝突回数が奇数の場合は内側判定");
            std::copy(
                check_spatial_ids.begin(), check_spatial_ids.end(),
                std::inserter(inner_spatial_ids, inner_spatial_ids.end()));
        } else {
            // 衝突回数が偶数の場合は外側判定
            LOG_DEBUG("衝突回数が偶数の場合は外側判定");
            std::copy(
                check_spatial_ids.begin(), check_spatial_ids.end(),
                std::inserter(outer_spatial_ids, outer_spatial_ids.end()));
        }
    }
}

/**
 * @brief ボクセルの中心点からZ軸方向へのレイと三角ポリゴンの接触点を算出
 *
 * @param projected_triangle 三角ポリゴン(投影座標)
 * @param start_orth_point ボクセルの中心点(投影座標)
 * @param[out] cross_point_cordinate_results
 * タプルリスト(衝突点、衝突点がある辺のキー, 衝突点のある頂点のキー)
 */
void get_cross_point_cordinate(
    const Triangle& projected_triangle, const Point& start_orth_point,
    std::vector<std::tuple<Point, int, int>>& cross_point_cordinate_results) {
    // ボクセルの中心点からZ軸方向へのレイ
    Line ray_line(start_orth_point, Vector(0, 0, 1));

    // 三角ポリゴンが属する平面
    Plane triangle_plane(projected_triangle.points[0],
                         projected_triangle.normal());

    cross_point_cordinate_results.clear();

    if (std::fabs(triangle_plane.normal.dot(ray_line.direction)) < kMinima) {
        // Z軸方向へのレイと三角ポリゴンが属する平面が水平の場合
        // ※交差点の数は0～2個

        // 三角ポリゴンの辺
        for (int key = 0; key < 3; key++) {
            int edge_key = -1;
            int vertex_key = -1;

            // 三角ポリゴンの辺
            Line edge = projected_triangle.line(key);

            // 辺とレイの交差点を取得
            Point edge_cross_point;
            try {
                edge_cross_point = edge.intersect_line(ray_line);
                if (edge_cross_point.z < start_orth_point.z) {
                    continue;
                }
            } catch (std::invalid_argument const& /*ex*/) {
                // 辺とレイが平行な場合は次の辺の処理へ移行
                // 辺とレイが重なる場合は他の辺の接点として、交差点は取得可能
                continue;
            }

            // 辺とレイの交差点が三角ポリゴンの範囲内であるかを確認
            double edge_length = edge.direction.norm();
            double edge_cross_length1 =
                Vector::from_points(edge_cross_point, edge.point).norm();
            double edge_cross_length2 =
                Vector::from_points(edge_cross_point, edge.to_point()).norm();

            // 交点までの長さの差がある場合は辺上に無いと判定する
            if (fabs(edge_length - edge_cross_length1 - edge_cross_length2) >
                kEdgeMinima) {
                continue;
            }

            edge_key = key;

            // 交差点が三角ポリゴンの頂点であるかを確認
            for (int vertex = 0; vertex < 3; vertex++) {
                if (projected_triangle.points[vertex].distance_point(
                        edge_cross_point) < kMinima) {
                    edge_key = -1;
                    vertex_key = vertex;
                    break;
                }
            }

            // 同じ交差点は除外する
            if (std::none_of(cross_point_cordinate_results.begin(),
                             cross_point_cordinate_results.end(),
                             [&edge_cross_point](
                                 const std::tuple<Point, int, int>& result) {
                                 return (std::get<0>(result).distance_point(
                                             edge_cross_point) < kMinima);
                             })) {
                cross_point_cordinate_results.push_back(
                    std::make_tuple(edge_cross_point, edge_key, vertex_key));
            }
        }
    } else {
        // Z軸方向へのレイと三角ポリゴンが属する平面が水平でない場合

        // 三角ポリゴンとZ方向のレイとの交点を求める
        Point collision_point = triangle_plane.intersect_line(ray_line);
        if (collision_point.z < start_orth_point.z) {
            cross_point_cordinate_results.clear();
            return;
        }

        // レイの開始点(XY平面)
        Point ray_start_point(start_orth_point.x, start_orth_point.y);

        int edge_key = -1;
        int vertex_key = -1;

        // 頂点との衝突確認
        for (int vertex = 0; vertex < 3; vertex++) {
            Point exp_vetex = projected_triangle.points[vertex];
            Point xy_exp_vetex(exp_vetex.x, exp_vetex.y);
            // XY平面上の距離で確認
            if (xy_exp_vetex.distance_point(ray_start_point) < kMinima) {
                // 衝突点を実際の頂点座標に補正
                collision_point = exp_vetex;
                vertex_key = vertex;
                break;
            }
        }

        // 頂点に衝突する場合
        if (vertex_key != -1) {
            cross_point_cordinate_results.push_back(
                std::make_tuple(collision_point, edge_key, vertex_key));
            return;
        }

        // 辺との衝突確認
        for (int edge = 0; edge < 3; edge++) {
            Line exp_edge = projected_triangle.line(edge);
            Line xy_exp_edge =
                Line(Point(exp_edge.point.x, exp_edge.point.y),
                     Vector(exp_edge.direction.x, exp_edge.direction.y));

            // XY平面上の距離で確認
            double edge_length = xy_exp_edge.direction.norm();
            double edge_cross_length1 =
                Vector::from_points(xy_exp_edge.point, ray_start_point).norm();
            double edge_cross_length2 =
                Vector::from_points(xy_exp_edge.to_point(), ray_start_point)
                    .norm();

            if (std::fabs(edge_length - edge_cross_length1 -
                          edge_cross_length2) < kEdgeMinima) {
                edge_key = edge;
                break;
            }
        }

        // 辺に衝突する場合
        if (edge_key != -1) {
            cross_point_cordinate_results.push_back(
                std::make_tuple(collision_point, edge_key, vertex_key));
            return;
        }

        // 各辺
        Vector edge1 = Vector::from_points(projected_triangle.points[1],
                                           projected_triangle.points[0]);
        Vector edge2 = Vector::from_points(projected_triangle.points[2],
                                           projected_triangle.points[1]);
        Vector edge3 = Vector::from_points(projected_triangle.points[0],
                                           projected_triangle.points[2]);

        // 各頂点から衝突点
        Vector collision_apex1 =
            Vector::from_points(collision_point, projected_triangle.points[1]);
        Vector collision_apex2 =
            Vector::from_points(collision_point, projected_triangle.points[2]);
        Vector collision_apex3 =
            Vector::from_points(collision_point, projected_triangle.points[0]);

        Vector c1 = edge1.cross(collision_apex1);
        Vector c2 = edge2.cross(collision_apex2);
        Vector c3 = edge3.cross(collision_apex3);

        if (c1.dot(c2) > 0 && c1.dot(c3) > 0) {
            // 三角形上の点の座標
            cross_point_cordinate_results.push_back(
                std::make_tuple(collision_point, edge_key, vertex_key));
        }
    }
}

/**
 * @brief 三角ポリゴングループ作成・更新
 *
 * @param cross_point_cordinate_result 衝突点情報（3次元）
 * @param[in,out] vertex_dict 衝突頂点辞書
 * @param[in,out] edge_list 衝突辺リスト
 * @param[in,out] edge_line_triangle_group
 * 衝突点の辺の線情報と三角ポリゴングループのrectangular_triangles内のインデックス
 * @param[in,out] vertex_triangle_group
 * 衝突点の頂点の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param[in,out] cross_group_index 衝突点の三角ポリゴンのインデックス
 * @param triangle_index 衝突点の三角ポリゴンのインデックス
 * @param rectangular_triangles 全三角ポリゴン
 */
void make_triangle_group(
    const std::tuple<Point, int, int>& cross_point_cordinate_result,
    std::map<std::string, Point>& vertex_dict,
    std::vector<std::string>& edge_list,
    std::map<std::string, std::tuple<Line, std::unordered_set<int>>>&
        edge_line_triangle_group,
    std::map<std::string, std::unordered_set<int>>& vertex_triangle_group,
    std::map<std::string, int>& cross_group_index, int triangle_index,
    const std::vector<Triangle>& rectangular_triangles) {
    //
    const auto& cross_point = std::get<0>(cross_point_cordinate_result);
    int edge_key = std::get<1>(cross_point_cordinate_result);
    int vertex_key = std::get<2>(cross_point_cordinate_result);

    // 交差点を文字列に変換
    std::string cross_point_str = change_point_str(cross_point);

    // 事前に処理を行った頂点を共有しない辺・頂点のグループ数分グループ番号をずらす
    int start_offset = 1;
    if (cross_group_index.find(cross_point_str) != cross_group_index.end()) {
        start_offset = cross_group_index[cross_point_str];
    }

    // 辺と交差する場合
    if (edge_key != -1) {
        // 衝突辺
        Line cross_edge = rectangular_triangles[triangle_index].line(edge_key);
        std::string cross_edge_str = change_line_str(cross_edge);

        // 衝突辺リストに衝突辺が登録済みの場合は三角ポリゴンがグルーピング済みなのでスキップ
        if (std::find(edge_list.begin(), edge_list.end(), cross_edge_str) !=
            edge_list.end()) {
            return;
        }

        // 衝突辺をリストに保存
        edge_list.push_back(cross_edge_str);

        // 衝突辺の両端の点を取得
        Point point_a = cross_edge.point;
        Point point_b = cross_edge.to_point();

        // グループ対象辺リスト
        std::vector<Line> line_group;
        // 頂点Aを含まない辺で頂点Bを含む辺を持つ三角ポリゴンの辞書
        std::map<int, Line> exp_triangle;
        // グループ対象の三角ポリゴンを決定
        int share_triangle_index = 0;
        for (const auto& share_triangle : rectangular_triangles) {
            for (int vertex = 0; vertex < 3; vertex++) {
                // 頂点Aを共有する三角ポリゴン
                if (share_triangle.points[vertex].distance_point(point_a) <
                    kMinima) {
                    // 頂点Aを含まない辺
                    Line share_line = share_triangle.line(vertex);
                    // 頂点Aを含まない辺が頂点Bを含まない辺であるかを確認
                    double distance_a =
                        share_line.point.distance_point(point_b);
                    double distance_b =
                        share_line.to_point().distance_point(point_b);

                    if (!(distance_a < kMinima) && !(distance_b < kMinima)) {
                        // 頂点Aを含まない辺が頂点Bを含まない辺である場合
                        // グループ対象辺リストへ追加
                        line_group.push_back(share_line);
                    } else {
                        // 頂点Aを含まない辺で頂点Bを含む辺である場合
                        // 頂点Aを含まない辺で頂点Bを含む辺を持つ三角ポリゴンの辞書に追加
                        exp_triangle[share_triangle_index] = share_line;
                    }
                }
            }
            share_triangle_index++;
        }

        // グループ対象辺リストを繋がる辺同士でグループ化する
        // 要素数は辺ABを共有するオブジェクトの個数
        std::vector<std::vector<Point>> connect_line_group;
        make_line_group(line_group, connect_line_group);

        // 次回処理での衝突点のグループ番号を更新
        cross_group_index[cross_point_str] =
            start_offset + static_cast<int>(connect_line_group.size());

        // グループ対象辺リストをグルーピングしたグループごとに処理
        // 事前に処理を行った頂点を共有しない辺・頂点のグループ数分グループ番号をずらす
        int index = start_offset;
        for (const auto& connect_line : connect_line_group) {
            // 頂点Aを含まない辺で頂点Bを含む辺を持つ三角ポリゴンのインデックス
            std::string group_cross_point_str(cross_point_str);
            group_cross_point_str.append("%");
            group_cross_point_str.append(std::to_string(index++));

            // 連絡先がない頂点と頂点Bを持つ辺を持つ三角ポリゴン
            int share_triangle_index = 0;
            for (const auto& exp_line : exp_triangle) {
                // グループ対象辺リストを繋けた結果の端と頂点Bを繋げた辺と
                // 頂点Aを含まない辺で頂点Bを含む辺が一致するか確認する
                // 繋げた結果の始端の辺と一致
                Line start_line =
                    Line::from_points(connect_line.front(), point_b);
                bool is_line_a = is_same_line(exp_line.second, start_line);

                // 繋げた結果の終端の辺と一致
                Line end_line = Line::from_points(connect_line.back(), point_b);
                bool is_line_b = is_same_line(exp_line.second, end_line);

                if (is_line_a || is_line_b) {
                    // 頂点Aを含まない辺で頂点Bを含む辺を持つ三角ポリゴンのインデックスを
                    // グループに所属する三角ポリゴンとして登録
                    if (auto itr = edge_line_triangle_group.find(
                            group_cross_point_str);
                        itr != edge_line_triangle_group.end()) {
                        std::get<1>(itr->second).insert(share_triangle_index);
                    } else {
                        edge_line_triangle_group[group_cross_point_str] =
                            std::make_tuple(
                                cross_edge,
                                std::unordered_set<int>{share_triangle_index});
                    }
                }
                share_triangle_index++;
            }
        }
    } else if (vertex_key != -1) {
        // 衝突頂点辞書に衝突点が登録済みの場合は三角ポリゴンがグルーピング済みなのでスキップ
        if (vertex_dict.find(cross_point_str) != vertex_dict.end()) {
            return;
        }

        // 衝突頂点を辞書に保存
        vertex_dict[cross_point_str] = cross_point;

        std::vector<Line> line_group;
        std::map<int, Line> exp_triangle;
        int check_triangle_index = 0;
        for (const auto& check_triangle : rectangular_triangles) {
            for (int vertex = 0; vertex < 3; vertex++) {
                // 頂点Aを共有する三角ポリゴン
                if (check_triangle.points[vertex].distance_point(cross_point) <
                    kMinima) {
                    // 頂点Aを含まない辺
                    Line check_line = check_triangle.line(vertex);
                    line_group.push_back(check_line);
                    exp_triangle[check_triangle_index] = check_line;
                }
            }
        }

        // 頂点Aを含まない辺を互いに繋がる辺同士でグルーピング
        // 要素数は頂点Aを共有するオブジェクトの個数
        std::vector<std::vector<Point>> connect_line_group;
        make_line_group(line_group, connect_line_group);

        // 次回処理での衝突点のグループ番号を更新
        cross_group_index[cross_point_str] =
            start_offset + static_cast<int>(connect_line_group.size());

        int group_index = start_offset;
        for (const auto& connect_line : connect_line_group) {
            // グループ化した辺を含む三角ポリゴンのインデックス
            std::string group_cross_point_str(cross_point_str);
            group_cross_point_str.append("%");
            group_cross_point_str.append(std::to_string(group_index++));

            // グループ内の辺がどの三角ポリゴンの辺か確認する
            for (std::vector<Point>::const_iterator it = connect_line.begin();
                 it != connect_line.end(); it++) {
                const Point& p1 = *it;
                if (++it == connect_line.end()) {
                    break;
                }
                const Point& p2 = *it;
                Line line = Line::from_points(p1, p2);
                int share_triangle_index = 0;
                for (const auto& exp_line : exp_triangle) {
                    // グループ化した辺を含む三角ポリゴンを選択する
                    if (is_same_line(exp_line.second, line)) {
                        // グループ化した辺を含む三角ポリゴンのインデックスを
                        // グループに所属する三角ポリゴンとして登録
                        if (auto itr = vertex_triangle_group.find(
                                group_cross_point_str);
                            itr != vertex_triangle_group.end()) {
                            (itr->second).insert(share_triangle_index);
                        } else {
                            vertex_triangle_group[group_cross_point_str] =
                                std::unordered_set<int>{share_triangle_index};
                        }
                    }
                    share_triangle_index++;
                }
            }
        }
    }
}

/**
 * @brief 点を文字列形式に変換する
 *
 * @param point 点
 * @return std::string
 */
std::string change_point_str(const Point& point) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(10) << point.x << "_";
    oss << std::fixed << std::setprecision(10) << point.y << "_";
    oss << std::fixed << std::setprecision(10) << point.z;
    return oss.str();
}

/**
 * @brief 線を文字列形式に変換する
 *
 * @param line 線
 * @return std::string
 */
std::string change_line_str(const Line& line) {
    std::string s1 = change_point_str(line.point);
    std::string s2 = change_point_str(line.to_point());
    std::ostringstream oss;
    oss << std::max(s1, s2) << "_" << std::min(s1, s2);
    return oss.str();
}

/**
 * @brief 線を点が繋がるように連結してグループ化
 *
 * @param[in,out] lines 線のリスト
 * @param[in,out] line_group 連結してグループ化した点配列のリスト
 * @remark
 * 再帰を含めた最大呼び出し回数はlinesの要素数分
 * 最大呼び出し回数になる場合は全ての線が別グループとなる場合
 */
void make_line_group(std::vector<Line>& lines,
                     std::vector<std::vector<Point>>& line_group) {
    // 線のリストが空の場合は空リストを返却
    if (lines.size() == 0) {
        return;
    }

    // 線のリストの先頭を開始線、残りを繋げる候補の線として右回りに連結
    std::vector<Point> start_line;
    start_line.push_back(lines[0].point);
    start_line.push_back(lines[0].to_point());
    lines.erase(lines.begin());
    make_right_rotation_line(start_line, lines);

    // 右回りに連結済みの線を開始線、残りを繋げる候補の線として左回りに連結
    make_left_rotation_line(start_line, lines);

    // 左右に連結してグループ化した点配列を返却値に追加
    line_group.push_back(start_line);

    // 繋げる候補の線が残っている場合
    if (lines.size() > 0) {
        // 繋げる候補の線で再帰的に処理を呼び出し
        // 繋げる候補の線は1グループ分の線が抜かれた線のリスト
        make_line_group(lines, line_group);
    }
}

/**
 * @brief 線を開始線から右周りで繋がるように連結する
 *
 * @param[in,out] start_line [in]開始線 [out]連結した点配列
 * @param[in,out] lines 連結した点配列を除いた余った線の配列
 * @remark
 * 再帰を含めた最大呼び出し回数はlinesの要素数分
 * 最大呼び出し回数になる場合は開始線から全ての線リストが右回りに連結できる場合
 */
void make_right_rotation_line(std::vector<Point>& start_line,
                              std::vector<Line>& lines) {
    // 開始線の終点から開始（右回り）
    Point& end_point = start_line.back();

    // 右回りの次の線があるかの判定
    bool has_next = false;

    // 線のリストの要素ごとに処理
    auto it = lines.begin();
    while (it != lines.end()) {
        // 線の始点・終点毎に確認
        for (int t = 0; t < 2; t++) {
            // 線の端点が開始線の終点と一致する場合
            if (end_point.distance_point(it->to_point(t)) < kMinima) {
                // 線のもう片方の端点を開始線の終点の後ろに追加
                start_line.push_back(it->to_point(1 - t));
                // 判定した線を線のリストから除外
                it = lines.erase(it);
                has_next = true;
                break;
            }
        }
        // 線の端点が開始線の終点と一致しない場合は処理継続
        if (!has_next) {
            it++;
            continue;
        }
        // 線の端点が開始線の終点と一致する場合は線のリストの要素の確認を終了
        break;
    }

    // 次の線がある場合
    if (has_next) {
        // 開始線から右回りで線が繋がりきるまで再帰的に処理を行う
        make_right_rotation_line(start_line, lines);
    }
}

/**
 * @brief 線を開始線から左周りで繋がるように連結する
 *
 * @param[in,out] start_line [in]開始線 [out]連結した点配列
 * @param[in,out] lines 連結した点配列を除いた余った線の配列
 * @remark
 * 再帰を含めた最大呼び出し回数はlinesの要素数分
 * 最大呼び出し回数になる場合は開始線から全ての線リストが左回りに連結できる場合
 */
void make_left_rotation_line(std::vector<Point>& start_line,
                             std::vector<Line>& lines) {
    // 開始線の終点から開始（左回り）
    Point& end_point = start_line.front();

    // 左回りの次の線があるかの判定
    bool has_next = false;

    // 線のリストの要素ごとに処理
    auto it = lines.begin();
    while (it != lines.end()) {
        // 線の始点・終点毎に確認
        for (int t = 0; t < 2; t++) {
            // 線の端点が開始線の終点と一致する場合
            if (end_point.distance_point(it->to_point(t)) < kMinima) {
                // 線のもう片方の端点を開始線の終点の後ろに追加
                start_line.push_back(it->to_point(1 - t));
                // 判定した線を線のリストから除外
                it = lines.erase(it);
                has_next = true;
                break;
            }
        }
        // 線の端点が開始線の終点と一致しない場合は処理継続
        if (!has_next) {
            it++;
            continue;
        }
        // 線の端点が開始線の終点と一致する場合は線のリストの要素の確認を終了
        break;
    }

    // 次の線がある場合
    if (has_next) {
        // 開始線から左回りで線が繋がりきるまで再帰的に処理を行う
        make_left_rotation_line(start_line, lines);
    }
}

/**
 * @brief 線分が同一であるかの判定
 *
 * @param line_a 辺A
 * @param line_b 辺B
 * @return true
 * @return false
 */
bool is_same_line(const Line& line_a, const Line& line_b) {
    // 辺Aの端点
    Point line_a_start = line_a.point;
    Point line_a_end = line_a.to_point();
    // 辺Bの端点
    Point line_b_start = line_b.point;
    Point line_b_end = line_b.to_point();

    // 辺の端点の一致を確認
    bool is_pos_line = (line_a_start.distance_point(line_b_start) < kMinima) &&
                       (line_a_end.distance_point(line_b_end) < kMinima);
    bool is_neg_line = (line_a_start.distance_point(line_b_end) < kMinima) &&
                       (line_a_end.distance_point(line_b_start) < kMinima);

    // どちらかの組で一致すればTrue
    return is_pos_line || is_neg_line;
}

/**
 * @brief 辺・頂点の内部判定
 *
 * @param vertex_triangle_group 頂点三角ポリゴングループ
 * @param edge_line_triangle_group 辺三角ポリゴングループ
 * @param vertex_dict 衝突頂点辞書
 * @param rectangular_triangles 全三角ポリゴン情報
 * @param[in,out] judgment_completed_cross_points 処理済みの衝突点集合
 * @param[out] inside_results 内部判定結果
 */
void cross_check_edge_vertex(
    const std::map<std::string, std::unordered_set<int>>& vertex_triangle_group,
    const std::map<std::string, std::tuple<Line, std::unordered_set<int>>>&
        edge_line_triangle_group,
    const std::map<std::string, Point>& vertex_dict,
    const std::vector<Triangle>& rectangular_triangles,
    std::unordered_set<std::string>& judgment_completed_cross_points,
    std::vector<bool>& inside_results) {
    //
    inside_results.clear();

    // 辺文字列と三角ポリゴンの辞書を作成
    std::map<std::string, std::unordered_set<int>> edge_triangle_group;
    for (const auto& edge_line_triangle : edge_line_triangle_group) {
        edge_triangle_group[edge_line_triangle.first] =
            std::get<1>(edge_line_triangle.second);
    }

    std::map<std::string, std::unordered_set<int>> triangle_group(
        vertex_triangle_group);
    for (const auto& edge_triangle : edge_triangle_group) {
        triangle_group[edge_triangle.first] = edge_triangle.second;
    }

    // 衝突判定
    for (const auto& triangle : triangle_group) {
        const std::string& group_cross_point_str = triangle.first;
        const std::unordered_set<int>& triangle_indexes = triangle.second;

        // 判定済みは飛ばす
        if (judgment_completed_cross_points.find(group_cross_point_str) !=
            judgment_completed_cross_points.end()) {
            continue;
        }

        // 辺と衝突するか判定
        bool is_edge = false;
        if (edge_triangle_group.find(group_cross_point_str) !=
            edge_triangle_group.end()) {
            is_edge = true;
        }

        std::string cross_point_str =
            split_string_get(group_cross_point_str, '%', 0);

        // 共有する三角ポリゴンをもつグルーピング済み衝突点
        std::unordered_set<std::string> cross_point_set{group_cross_point_str};

        // 共有する三角ポリゴンのインデックス
        std::unordered_set<int> union_triangle_indexes{triangle_indexes};

        // 共有する三角ポリゴンを持つグループを一つにまとめる
        grouping_triangle(cross_point_set, union_triangle_indexes,
                          edge_triangle_group, vertex_triangle_group);

        // 判定済み衝突点に追加
        judgment_completed_cross_points.insert(cross_point_set.begin(),
                                               cross_point_set.end());

        bool inside_result = false;
        if (cross_point_set.size() == 1 && !is_edge) {
            // 1頂点との衝突
            // 頂点に衝突した三角ポリゴンのグループに対して、グループに所属する三角ポリゴンが他のグループに所属しない場合

            // 接触点である頂点
            const Point& cross_point =
                vertex_dict.find(cross_point_str)->second;

            // 判定
            inside_result = check_cross_one_vertex(
                cross_point, triangle_indexes, rectangular_triangles);
        } else if (cross_point_set.size() == 1 && is_edge) {
            // 一辺との衝突
            // 辺に衝突した三角ポリゴンのグループに対して、グループに所属する三角ポリゴンが他のグループに所属しない場合

            // 衝突点がある辺
            const auto& cross_edge = std::get<0>(
                edge_line_triangle_group.find(group_cross_point_str)->second);

            // 衝突点が通過する辺のXY射影
            Line xy_cross_edge =
                Line::from_points_2d(cross_edge.point, cross_edge.to_point());

            // 判定
            inside_result = check_cross_one_edge(
                xy_cross_edge, triangle_indexes, rectangular_triangles);
        } else {
            // 2辺・2頂点・1頂点1辺との衝突

            // 最大・最小の衝突点を算出する
            std::string max_z_point;
            std::string min_z_point;
            constexpr double max_z = std::numeric_limits<double>::min();
            constexpr double min_z = std::numeric_limits<double>::max();
            for (auto itr = cross_point_set.begin();
                 itr != cross_point_set.end(); itr++) {
                double z = std::stod(
                    split_string_get(split_string_get(*itr, '%', 0), '_', 2));
                if (z > max_z) {
                    max_z_point = *itr;
                }
                if (z < min_z) {
                    min_z_point = *itr;
                }
            }

            // 最大・最小の衝突点が辺で衝突するか頂点で衝突するか判定する
            bool is_max_edge = edge_triangle_group.find(max_z_point) !=
                               edge_triangle_group.end();
            bool is_min_edge = edge_triangle_group.find(min_z_point) !=
                               edge_triangle_group.end();

            if (is_max_edge && is_min_edge) {
                // 2辺との衝突

                // 衝突点がある辺
                const auto& cross_edge = std::get<0>(
                    edge_line_triangle_group.find(max_z_point)->second);

                // 衝突点が通過する辺のXY射影
                Line xy_cross_edge = Line::from_points_2d(
                    cross_edge.point, cross_edge.to_point());

                // 判定
                inside_result = check_cross_two_edges(
                    xy_cross_edge,
                    edge_triangle_group.find(max_z_point)->second,
                    edge_triangle_group.find(min_z_point)->second,
                    rectangular_triangles);
            } else if (is_max_edge || is_min_edge) {
                // 1頂点1辺との衝突

                // 衝突点がある頂点
                const Point& cross_point =
                    vertex_dict.find((is_max_edge) ? min_z_point : max_z_point)
                        ->second;

                // 衝突点がある辺
                const auto& cross_edge = std::get<0>(
                    edge_line_triangle_group
                        .find((is_max_edge) ? max_z_point : min_z_point)
                        ->second);

                // 衝突点が通過する辺のXY射影
                Line xy_cross_edge = Line::from_points_2d(
                    cross_edge.point, cross_edge.to_point());

                // 判定
                inside_result = check_cross_vertex_and_edge(
                    cross_point, xy_cross_edge,
                    vertex_triangle_group
                        .find((is_max_edge) ? min_z_point : max_z_point)
                        ->second,
                    edge_triangle_group
                        .find((is_max_edge) ? max_z_point : min_z_point)
                        ->second,
                    rectangular_triangles);
            } else {
                // 2頂点との衝突

                std::string max_cross_point_str =
                    split_string_get(max_z_point, '%', 0);
                std::string min_cross_point_str =
                    split_string_get(min_z_point, '%', 0);

                // 頂点に衝突した三角ポリゴンで複数グループに含まれる三角ポリゴン
                int share_triangle_index = get_share_triangle_index(
                    max_z_point, vertex_triangle_group);

                // 複数グループに含まれる三角ポリゴンが無い場合
                if (share_triangle_index == -1) {
                    inside_results.push_back(false);
                    continue;
                }

                // 複数グループに含まれる三角ポリゴンの頂点をXY平面に射影
                // 直線L
                const Triangle& share_triangle =
                    rectangular_triangles[share_triangle_index];
                Line xy_vertex_line;
                if (share_triangle.points[0].distance_point_2d(
                        share_triangle.points[1]) < kMinima) {
                    xy_vertex_line = Line::from_points_2d(
                        share_triangle.points[0], share_triangle.points[2]);
                } else {
                    xy_vertex_line = Line::from_points_2d(
                        share_triangle.points[0], share_triangle.points[1]);
                }

                // 判定
                inside_result = check_cross_two_vertexes(
                    vertex_dict.find(max_cross_point_str)->second,
                    vertex_dict.find(min_cross_point_str)->second,
                    xy_vertex_line,
                    vertex_triangle_group.find(max_z_point)->second,
                    vertex_triangle_group.find(min_z_point)->second,
                    rectangular_triangles);
            }
        }

        // 内外判定結果格納
        inside_results.push_back(inside_result);
    }
}

/**
 * @brief 三角ポリゴンを共有するグループをまとめる
 *
 * @param[in,out] cross_point_set 衝突点の集合
 * @param[in,out] union_triangle_indexes 共有グループとなる三角ポリゴン
 * @param edge_triangle_group 辺の三角ポリゴン辞書
 * @param vertex_triangle_group 頂点の三角ポリゴン辞書
 */
void grouping_triangle(
    std::unordered_set<std::string>& cross_point_set,
    std::unordered_set<int>& union_triangle_indexes,
    const std::map<std::string, std::unordered_set<int>>& edge_triangle_group,
    const std::map<std::string, std::unordered_set<int>>&
        vertex_triangle_group) {
    // 共有する三角ポリゴンを持つ衝突点があるかの有無
    bool has_some_cross_point = false;

    // 辺の衝突点に対する三角ポリゴングループに関して、共有する三角ポリゴンがあるかチェック
    for (const auto& edge_triangle : edge_triangle_group) {
        const std::string& check_point_str = edge_triangle.first;
        const std::unordered_set<int>& edege_triangle_indexes =
            edge_triangle.second;

        // 既に共有していると判定済みの衝突点は除く
        if (cross_point_set.find(check_point_str) != cross_point_set.end()) {
            continue;
        }

        // 共有する三角ポリゴンがある場合
        if (std::any_of(edege_triangle_indexes.begin(),
                        edege_triangle_indexes.end(),
                        [&union_triangle_indexes](int index) {
                            return union_triangle_indexes.find(index) !=
                                   union_triangle_indexes.end();
                        })) {
            // 共有三角ポリゴングループを更新
            union_triangle_indexes.insert(edege_triangle_indexes.begin(),
                                          edege_triangle_indexes.end());

            // 共有判定済みの衝突点に衝突点を追加
            cross_point_set.insert(check_point_str);

            // 共有する三角ポリゴンを持つ衝突点があると判定
            has_some_cross_point = true;
        }
    }

    // 頂点の衝突点に対する三角ポリゴングループに関して、共有する三角ポリゴンがあるかチェック
    for (const auto& vertex_triangle : vertex_triangle_group) {
        const std::string& check_point_str = vertex_triangle.first;
        const std::unordered_set<int> vertex_triangle_indexes =
            vertex_triangle.second;

        // 既に共有していると判定済みの衝突点は除く
        if (cross_point_set.find(check_point_str) != cross_point_set.end()) {
            continue;
        }

        // 共有する三角ポリゴンがある場合
        if (std::any_of(vertex_triangle_indexes.begin(),
                        vertex_triangle_indexes.end(),
                        [&union_triangle_indexes](int index) {
                            return union_triangle_indexes.find(index) !=
                                   union_triangle_indexes.end();
                        })) {
            // 共有三角ポリゴングループを更新
            union_triangle_indexes.insert(vertex_triangle_indexes.begin(),
                                          vertex_triangle_indexes.end());

            // 共有判定済みの衝突点に衝突点を追加
            cross_point_set.insert(check_point_str);

            // 共有する三角ポリゴンを持つ衝突点があると判定
            has_some_cross_point = true;
        }
    }

    // 共有する三角ポリゴンを持つ衝突点無くなるまで再帰的に処理を実行
    if (has_some_cross_point) {
        grouping_triangle(cross_point_set, union_triangle_indexes,
                          edge_triangle_group, vertex_triangle_group);
    }
}

/**
 * @brief 1頂点との衝突判定
 *
 * @param cross_point 衝突点（3次元）
 * @param group_triangle_indexes
 * 衝突点の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return true 内部
 * @return false 外部
 */
bool check_cross_one_vertex(
    const Point& cross_point,
    const std::unordered_set<int>& group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles) {
    // 頂点を通る三角ポリゴンの接触点以外の辺のリスト
    std::vector<Line> xy_not_cross_edges;

    for (const auto& group_triangle_index : group_triangle_indexes) {
        // 衝突点の三角ポリゴン
        const Triangle& cross_triangle =
            rectangular_triangles[group_triangle_index];
        for (int vertex_key = 0; vertex_key < 3; vertex_key++) {
            const Point& vertex = cross_triangle.points[vertex_key];
            if (cross_point.distance_point(vertex) < kMinima) {
                // 衝突点がない辺
                const Line& not_cross_edge = cross_triangle.line(vertex_key);

                if (not_cross_edge.point.distance_point(
                        not_cross_edge.to_point()) < kMinima) {
                    continue;
                }

                // 衝突点がない辺のXY射影
                xy_not_cross_edges.push_back(Line::from_points_2d(
                    not_cross_edge.point, not_cross_edge.to_point()));
            }
        }
    }

    // 頂点を通る三角ポリゴンの接触点以外の辺を連結した図形
    std::vector<std::vector<Point>> rect_line_group;
    make_line_group(xy_not_cross_edges, rect_line_group);

    // 頂点のXY射影
    Point xy_cross_vertex(cross_point.x, cross_point.y);

    if (is_inside(rect_line_group[0], xy_cross_vertex)) {
        // 1頂点の接点射影内部と判定
        return true;
    } else {
        // 1頂点の接点射影外部と判定
        return false;
    }
}

/**
 * @brief 判定対象点が多角形の内部であるかの判定
 *
 * @param vertex_point_list 多角形の頂点（2次元）
 * @param target_point 判定座標（2次元）
 * @return true
 * @return false
 * @remark
 * 判定対象点が図形の頂点・辺上に存在する場合はこのメソッドの呼び出し元で考慮しているため、
 * 上記パターンはこのメソッドでは想定しない
 */
bool is_inside(const std::vector<Point>& vertex_point_list,
               const Point& target_point) {
    // 多角形の頂点が全て同一線上にある場合は外部として判定する
    bool sameline = true;
    for (std::vector<Point>::const_iterator it = vertex_point_list.begin();
         it != vertex_point_list.end();) {
        const Point& a = *it;
        if (++it == vertex_point_list.end()) {
            break;
        }
        const Point& b = *it;
        Vector vector_a_b = Vector::from_points(a, b).unit();
        while (++it != vertex_point_list.end()) {
            const Point& c = *it;
            Vector vector_a_c = Vector::from_points(a, c).unit();
            if (vector_a_b.cross(vector_a_c).norm() < kMinima) {
                continue;
            }
            // 同一線上にない点が見つかったので判定終了
            it = vertex_point_list.end();
            sameline = false;
            break;
        }
    }
    if (sameline) {
        return false;
    }

    // 角度
    double theta = 0;
    for (std::vector<Point>::const_iterator it = vertex_point_list.begin();
         it != vertex_point_list.end();) {
        // 確認対象の点から多角形の点へのベクトル1
        Vector v1 = Vector::from_points(target_point, *it);

        // 確認対象の点から多角形の点へのベクトル2
        Vector v2;
        if (++it == vertex_point_list.end()) {
            v2 =
                Vector::from_points(target_point, *(vertex_point_list.begin()));
        } else {
            v2 = Vector::from_points(target_point, *it);
            ++it;
        }

        // ベクトル1,2の角度を算出（ラジアン単位）
        double angle_radian = v1.angle_signed_2d(v2);
        // 算出した角度を加算
        theta += angle_radian;
    }

    // 多角形の各頂点を巡回した最終的な角度が1周以上していれば内部として判定する
    return !(std::fabs(rad_to_deg(theta)) < kMinima);
}

/**
 * @brief 1辺との衝突判定
 *
 * @param xy_cross_edge 衝突点が通過する辺（2次元）
 * @param group_triangle_indexes
 * 同グループの三角ポリゴンのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return true 内部
 * @return false 外部
 */
bool check_cross_one_edge(const Line& xy_cross_edge,
                          const std::unordered_set<int>& group_triangle_indexes,
                          const std::vector<Triangle>& rectangular_triangles) {
    std::tuple<int, int> count = count_edge_side(
        xy_cross_edge, group_triangle_indexes, rectangular_triangles);
    if (std::get<0>(count) > 0 && std::get<1>(count) > 0) {
        // 1辺の接点射影内部と判定
        return true;
    } else {
        // 1辺の接点射影外部と判定
        return false;
    }
}

/**
 * @brief 辺に対する左右判定
 *
 * @param xy_cross_edge 衝突点が通過する辺（2次元）
 * @param group_triangle_indexes
 * 同グループの三角ポリゴンのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return std::tuple<int, int> 左判定回数, 右判定回数
 */
std::tuple<int, int> count_edge_side(
    const Line& xy_cross_edge,
    const std::unordered_set<int>& group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles) {
    // 左判定回数
    int left_count = 0;
    // 右判定回数
    int right_count = 0;

    // 辺に対してグループ化された三角ポリゴンのインデックス
    for (const auto& group_triangle_index : group_triangle_indexes) {
        const Triangle& cross_triangle =
            rectangular_triangles[group_triangle_index];
        // 同グループの三角ポリゴンの頂点を取得
        for (int vertex_key = 0; vertex_key < 3; vertex_key++) {
            const Point& vertex = cross_triangle.points[vertex_key];
            // 頂点をXY平面に射影
            Point xy_vertex(vertex.x, vertex.y);

            // 辺に対して判定対象の頂点が左右どちらにあるかカウントする
            int side = xy_cross_edge.side_point_2d(xy_vertex);
            if (side == -1) {
                // 左判定
                left_count++;
            } else if (side == 1) {
                // 右判定
                right_count++;
            }
        }
    }

    return std::make_tuple(left_count, right_count);
}

/**
 * @brief 2辺との衝突判定
 *
 * @param xy_cross_edge 衝突点が通過する辺（2次元）
 * @param max_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点の高さが最大の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param min_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点の高さが最小の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return true 内部
 * @return false 外部
 */
bool check_cross_two_edges(
    const Line& xy_cross_edge,
    const std::unordered_set<int>& max_group_triangle_indexes,
    const std::unordered_set<int>& min_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles) {
    // 左判定回数
    int left_count = 0;
    // 右判定回数
    int right_count = 0;

    // 衝突点の高さが最大の三角ポリゴングループで内部判定
    {
        auto count = count_edge_side(xy_cross_edge, max_group_triangle_indexes,
                                     rectangular_triangles);
        left_count += std::get<0>(count);
        right_count += std::get<1>(count);
    }
    // 衝突点の高さが最小の三角ポリゴングループで内部判定
    {
        auto count = count_edge_side(xy_cross_edge, min_group_triangle_indexes,
                                     rectangular_triangles);
        left_count += std::get<0>(count);
        right_count += std::get<1>(count);
    }

    if (left_count > 0 && right_count > 0) {
        // 2辺の接点射影内部と判定
        return true;
    } else {
        // 2辺の接点射影外部と判定
        return false;
    }
}

/**
 * @brief 1頂点1辺との衝突判定
 *
 * @param cross_point 衝突点（3次元）
 * @param xy_cross_edge 衝突点が通過する辺（2次元）
 * @param vertex_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点が頂点の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param edge_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点がが辺の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return true 内部
 * @return false 外部
 */
bool check_cross_vertex_and_edge(
    const Point& cross_point, const Line& xy_cross_edge,
    const std::unordered_set<int>& vertex_group_triangle_indexes,
    const std::unordered_set<int>& edge_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles) {
    // 左判定回数
    int left_count = 0;
    // 右判定回数
    int right_count = 0;

    // 辺との左右判定
    {
        auto count = count_edge_side(xy_cross_edge, edge_group_triangle_indexes,
                                     rectangular_triangles);
        left_count += std::get<0>(count);
        right_count += std::get<1>(count);
    }
    // 頂点との左右判定
    {
        std::vector<std::tuple<const Point&, const std::unordered_set<int>&>>
            cross_point_infos;
        cross_point_infos.push_back(
            std::make_tuple(cross_point, vertex_group_triangle_indexes));

        auto count = count_vertex_side(cross_point_infos, xy_cross_edge,
                                       rectangular_triangles);
        left_count += std::get<0>(count);
        right_count += std::get<1>(count);
    }

    if (left_count > 0 && right_count > 0) {
        // 1頂点1辺の接点射影内部と判定
        return true;
    } else {
        // 1頂点1辺の接点射影外部と判定
        return false;
    }
}

/**
 * @brief 頂点に対する左右判定
 *
 * @param cross_point_infos
 * 衝突点（3次元）と同グループの三角ポリゴンのrectangular_triangles内のインデックスのタプル配列
 * @param xy_cross_line 衝突点が通過する直線L（2次元）
 * @param rectangular_triangles 全三角ポリゴン
 * @return std::tuple<int, int> 左判定回数, 右判定回数
 */
std::tuple<int, int> count_vertex_side(
    const std::vector<std::tuple<const Point&, const std::unordered_set<int>&>>&
        cross_point_infos,
    const Line& xy_cross_line,
    const std::vector<Triangle>& rectangular_triangles) {
    // 左判定回数
    int left_count = 0;
    // 右判定回数
    int right_count = 0;

    // 衝突点の前にある三角ポリゴンの衝突点を含まない辺
    std::vector<Line> share_front_edges;
    // 衝突点の後にある三角ポリゴンの衝突点を含まない辺
    std::vector<Line> share_back_edges;
    // 衝突点の前後にあるか判定できない三角ポリゴンの衝突点を含まない辺
    std::vector<Line> no_cross_triangle_edges;

    for (const auto& cross_point_info : cross_point_infos) {
        const auto& cross_point = std::get<0>(cross_point_info);
        const auto& group_triangle_indexes = std::get<1>(cross_point_info);

        // 頂点に対してグループ化された三角ポリゴンのインデックス
        for (const auto& group_triangle_index : group_triangle_indexes) {
            // 交点CXを求める三角ポリゴン
            const Triangle& check_triangle =
                rectangular_triangles[group_triangle_index];

            // 三角ポリゴンで衝突点が含まれない辺
            bool is_next = true;
            Line xy_check_edge;
            for (int vertex_key = 0; vertex_key < 3; vertex_key++) {
                if (check_triangle.points[vertex_key].distance_point(
                        cross_point) < kMinima) {
                    const Line& check_edge = check_triangle.line(vertex_key);
                    xy_check_edge = Line::from_points_2d(check_edge.point,
                                                         check_edge.to_point());
                    is_next = false;
                    break;
                }
            }
            if (is_next) {
                // 三角ポリゴンで衝突点が含まれない辺(2次元)が1つに定まらない場合は対象の三角ポリゴンをスキップ
                // 上記は実際の処理上は発生しないルート
                continue;
            }

            // 交点CXが無い場合はスキップ
            if (xy_cross_line.side_point_2d(xy_check_edge.point) *
                    xy_cross_line.side_point_2d(xy_check_edge.to_point()) ==
                1) {
                no_cross_triangle_edges.push_back(xy_check_edge);
                continue;
            }

            // 交点CX(2次元)
            Point cx_cross_point;
            try {
                cx_cross_point = xy_check_edge.intersect_line(xy_cross_line);
            } catch (std::invalid_argument const& /*ex*/) {
                // 直線Lに平行な辺は衝突点の前後にあるか判定できない三角ポリゴンの衝突点を含まない辺とする
                no_cross_triangle_edges.push_back(xy_check_edge);
                continue;
            }

            // 交点CXの前後判定
            if (std::fabs(xy_cross_line.direction.x < kMinima)) {
                // 直線LがX軸に平行な場合はY軸を前後判定の基準にする
                if (cx_cross_point.y > cross_point.y) {
                    share_front_edges.push_back(xy_check_edge);
                } else if (cx_cross_point.y < cross_point.y) {
                    share_back_edges.push_back(xy_check_edge);
                }
            } else {
                // 直線LがX軸に平行でない場合
                if (cx_cross_point.x > cross_point.x) {
                    share_front_edges.push_back(xy_check_edge);
                } else if (cx_cross_point.x < cross_point.x) {
                    share_back_edges.push_back(xy_check_edge);
                }
            }
        }
    }

    // 交点CXが前後に存在しない場合
    if (share_front_edges.size() == 0 || share_back_edges.size() == 0) {
        return std::make_tuple(left_count, right_count);
    }

    // 衝突点の前にある三角ポリゴンの衝突点を含まない辺から辿る
    std::vector<Point> target_vertexes;
    // 対象判定頂点が見つからない場合は別の辺から辿る
    int front_index = 0;
    for (const auto& share_front_edge : share_front_edges) {
        // 右方向・左方向それぞれ見つかるまでつなげる
        for (int t = 0; t < 2; t++) {
            Point xy_front_start_point = share_front_edge.to_point(t);
            // 直線Lに対して対象判定頂点を求める
            Point target_vertex;
            if (search_judge_vertex(front_index, xy_front_start_point,
                                    xy_cross_line, share_front_edges,
                                    share_back_edges, no_cross_triangle_edges,
                                    target_vertex)) {
                target_vertexes.push_back(target_vertex);
            }
        }
        front_index++;
    }

    // 対象判定頂点がある場合
    for (const auto& target_vertex : target_vertexes) {
        // 直線Lに対して判定対象の頂点が左右どちらにあるかカウントする
        int side = xy_cross_line.side_point_2d(target_vertex);
        if (side == -1) {
            left_count++;
        } else if (side == 1) {
            right_count++;
        }
    }

    return std::make_tuple(left_count, right_count);
}

/**
 * @brief 最大頂点共有三角ポリゴン取得
 *
 * @param max_z_point 最大頂点
 * @param vertex_triangle_group 頂点三角ポリゴングループ
 * @return int 共有三角ポリゴンインデックス (-1:なし 0〜2:インデックス)
 */
int get_share_triangle_index(
    const std::string& max_z_point,
    const std::map<std::string, std::unordered_set<int>>&
        vertex_triangle_group) {
    // 最大頂点グループ三角ポリゴンインデックス
    const std::unordered_set<int>& max_group_triangle_indexes =
        vertex_triangle_group.find(max_z_point)->second;

    // 共有三角ポリゴンインデックス
    int share_triangle_index = -1;
    for (const auto& max_group_triangle_index : max_group_triangle_indexes) {
        bool is_next = true;
        for (const auto& vertex_triangle : vertex_triangle_group) {
            // 自身は除く
            if (max_z_point == vertex_triangle.first) {
                continue;
            }
            // 頂点三角ポリゴングループの中で最大頂点グループ三角ポリゴンを持つものがいた場合
            if (vertex_triangle.second.find(max_group_triangle_index) !=
                vertex_triangle.second.end()) {
                share_triangle_index = max_group_triangle_index;
                is_next = false;
                break;
            }
        }
        if (is_next) {
            // 最大頂点グループ三角ポリゴンのインデクスを共有する三角ポリゴンが無い場合は
            // 他の最大頂点グループ三角ポリゴンのインデクスへ移動
            continue;
        }
        break;
    }

    return share_triangle_index;
}

/**
 * @brief 2頂点との衝突判定
 *
 * @param max_cross_point 最大高さの衝突点
 * @param min_cross_point 最小高さの衝突点
 * @param xy_cross_edge 衝突点が通過する辺（2次元）
 * @param max_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点の高さが最大の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param min_group_triangle_indexes
 * 共通の三角ポリゴンがあるグループ群の中で衝突点の高さが最小の三角ポリゴングループのrectangular_triangles内のインデックス
 * @param rectangular_triangles 全三角ポリゴン
 * @return true 内部
 * @return false 外部
 */
bool check_cross_two_vertexes(
    const Point& max_cross_point, const Point& min_cross_point,
    const Line& xy_cross_edge,
    const std::unordered_set<int>& max_group_triangle_indexes,
    const std::unordered_set<int>& min_group_triangle_indexes,
    const std::vector<Triangle>& rectangular_triangles) {
    // 左判定回数
    int left_count = 0;
    // 右判定回数
    int right_count = 0;

    // 頂点との左右判定
    std::vector<std::tuple<const Point&, const std::unordered_set<int>&>>
        cross_point_infos;
    cross_point_infos.push_back(
        std::make_tuple(max_cross_point, max_group_triangle_indexes));
    cross_point_infos.push_back(
        std::make_tuple(min_cross_point, min_group_triangle_indexes));

    auto count = count_vertex_side(cross_point_infos, xy_cross_edge,
                                   rectangular_triangles);
    left_count += std::get<0>(count);
    right_count += std::get<1>(count);

    if (left_count > 0 && right_count > 0) {
        // 2頂点の接点射影内部と判定
        return true;
    } else {
        // 2頂点の接点射影外部と判定
        return false;
    }
}

/**
 * @brief XY上の判定対象頂点を検索取得
 *
 * @param index
 * 処理を開始する衝突点の前にある三角ポリゴンの衝突点を含まない辺のインデックス
 * @param xy_front_start_point
 * 処理を開始する衝突点の前にある三角ポリゴンの衝突点を含まない辺の端点（2次元）
 * @param xy_vertex_line 衝突点が通過する直線L（2次元）
 * @param share_front_edges
 * 衝突点の前にある三角ポリゴンの衝突点を含まない辺（2次元）
 * @param share_back_edges
 * 衝突点の後にある三角ポリゴンの衝突点を含まない辺（2次元）
 * @param no_cross_triangle_edges
 * 衝突点の前後判定が出来なかった三角ポリゴンの衝突点を含まない辺（2次元）
 * @param[out] target_vertex XY上の判定対象頂点(2次元)
 * @return true 判定対象の頂点あり
 * @return false 判定対象の頂点なし
 */
bool search_judge_vertex(int index, Point xy_front_start_point,
                         const Line& xy_vertex_line,
                         const std::vector<Line>& share_front_edges,
                         const std::vector<Line>& share_back_edges,
                         const std::vector<Line>& no_cross_triangle_edges,
                         Point& target_vertex) {
    bool is_target_vertex = false;

    std::vector<Line> copy_no_cross_triangle_edges(no_cross_triangle_edges);

    // 交点CXを通る辺から連結される辺を辿る
    while (true) {
        // 衝突点の後にある三角ポリゴンの衝突点を含まない辺を確認
        for (const auto& share_back_edge : share_back_edges) {
            // 衝突点の後にある三角ポリゴンの衝突点を含まない辺が開始点を端点に持つ場合
            if (share_back_edge.point.distance_point_2d(xy_front_start_point) <
                    kMinima ||
                share_back_edge.to_point().distance_point_2d(
                    xy_front_start_point) < kMinima) {
                // 直線Lの開始頂点と終了頂点は除外
                if (xy_vertex_line.point.distance_point_2d(
                        xy_front_start_point) < kMinima ||
                    xy_vertex_line.to_point().distance_point_2d(
                        xy_front_start_point) < kMinima) {
                    continue;
                }
                target_vertex = xy_front_start_point;
                is_target_vertex = true;
                break;
            }
        }
        if (is_target_vertex) {
            break;
        }

        // 他の交点CXがＬ上で交点である頂点を挟んで反対にない場合かの判定
        bool has_cross_front = false;
        int front_index = 0;
        for (const auto& share_front_edge : share_front_edges) {
            // 自身は除く
            if (front_index != index) {
                // 他の交点CXがＬ上で交点である頂点を挟んで反対で無いか確認
                for (int t = 0; t < 2; t++) {
                    if (share_front_edge.to_point(t).distance_point_2d(
                            xy_front_start_point) < kMinima) {
                        has_cross_front = true;
                        break;
                    }
                }
                if (has_cross_front) {
                    break;
                }
            }
            front_index++;
        }

        // 他の交点CXがＬ上で交点である頂点を挟んで反対にない場合
        if (has_cross_front) {
            break;
        }

        // 交点CXを通る辺から連結される辺を連結
        bool has_connect_line = false;
        auto it = copy_no_cross_triangle_edges.begin();
        while (it != copy_no_cross_triangle_edges.end()) {
            bool remove = false;
            for (int t = 0; t < 2; t++) {
                if ((*it).to_point(t).distance_point_2d(xy_front_start_point) <
                    kMinima) {
                    // 開始点を頂点を共有する辺のもう一端の頂点とする
                    xy_front_start_point = (*it).to_point(1 - t);
                    remove = true;
                    has_connect_line = true;
                    break;
                }
            }
            if (remove) {
                // チェック対象の辺から除く
                it = copy_no_cross_triangle_edges.erase(it);
            } else {
                it++;
            }
        }

        // 交点に達しなかった
        if (!has_connect_line) {
            break;
        }
    }

    return is_target_vertex;
}

}  // namespace spatialid
