/**
 * @file spatial_line.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-30
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "spatial_line.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <unordered_set>

#include "common/const.h"
#include "common/spatial_point.h"
#include "spatial_api.h"

namespace spatialid {

// Prototype Declare
void get_plane_cross_points(
    const std::vector<Plane>& plane_list, const Point& start_orth_point,
    const Point& end_orth_point,
    std::unordered_set<Point, Point::hash_function>& cross_point_list);
void get_middle_points(
    const Point& start_orth_point, const Point& end_orth_point,
    std::unordered_set<Point, Point::hash_function>& cross_point_list);

// 指定範囲(線分)の空間IDを取得する
void get_spatial_ids_on_line(const ProjectedPoint& start,
                             const ProjectedPoint& end,
                             std::vector<std::string>& spatial_id_list,
                             int zoom) {
    LOG_DEBUG("空間ID変換(線分)開始");
    LOG_DEBUG("始点(地理座標)=(%.10f,%.10f,%.10f)", start.lon, start.lat,
              start.alt);
    LOG_DEBUG("終点(地理座標)=(%.10f,%.10f,%.10f)", end.lon, end.lat, end.alt);

    spatial_id_list.clear();

    // 始点の空間ID
    std::string start_spatial_id =
        get_spatial_id_on_point(start.lon, start.lat, start.alt, zoom);
    // 終点の空間ID
    std::string end_spatial_id =
        get_spatial_id_on_point(end.lon, end.lat, end.alt, zoom);

    LOG_DEBUG("始点(空間ID)=%s", start_spatial_id.c_str());
    LOG_DEBUG("終点(空間ID)=%s", end_spatial_id.c_str());

    if (start.lon == end.lon && start.lat == end.lat && start.alt == end.alt) {
        // 始点と終点の空間IDが同じ
        LOG_DEBUG("始点終点の空間IDが同一");
        spatial_id_list.push_back(start_spatial_id);
    } else {
        LOG_DEBUG("始点(投影座標)=(%.10f,%.10f,%.10f)", start.x, start.y,
                  start.alt);
        LOG_DEBUG("終点(投影座標)=(%.10f,%.10f,%.10f)", end.x, end.y, end.alt);

        Point start_orth_point(start.x, start.y, start.alt);  // 投影座標の始点
        Point end_orth_point(end.x, end.y, end.alt);  // 投影座標の終点
        std::unordered_set<Point, Point::hash_function>
            cross_point_list;  // 交点及び交点間の中点リスト

        Zfxy start_zfxy(start_spatial_id);
        Zfxy end_zfxy(end_spatial_id);

        LOG_DEBUG("始点(空間ID成分)=(%d,%d,%d)", start_zfxy.lon_index,
                  start_zfxy.lat_index, start_zfxy.alt_index);
        LOG_DEBUG("終点(空間ID成分)=(%d,%d,%d)", end_zfxy.lon_index,
                  end_zfxy.lat_index, end_zfxy.alt_index);

        // X軸ボクセル境界面を取得
        std::vector<Plane> plane_list;  // 投影座標
        generate_x_voxel_plane(start_zfxy.lon_index, end_zfxy.lon_index,
                               plane_list, zoom);

        // X軸ボクセル境界面との交点を取得
        get_plane_cross_points(plane_list, start_orth_point, end_orth_point,
                               cross_point_list);

        // Y軸ボクセル境界面を取得
        plane_list.clear();
        generate_y_voxel_plane(start_zfxy.lat_index, end_zfxy.lat_index,
                               plane_list, zoom);

        // Y軸ボクセル境界面との交点を取得
        get_plane_cross_points(plane_list, start_orth_point, end_orth_point,
                               cross_point_list);

        // Z軸ボクセル境界面を取得
        plane_list.clear();
        generate_z_voxel_plane(start_zfxy.alt_index, end_zfxy.alt_index,
                               plane_list, zoom);

        // Z軸ボクセル境界面との交点を取得
        get_plane_cross_points(plane_list, start_orth_point, end_orth_point,
                               cross_point_list);

        // 始点を含む、交点間の中点を取得
        get_middle_points(start_orth_point, end_orth_point, cross_point_list);

        // 空間IDに変換
        std::unordered_set<std::string> cross_spatial_ids;
        get_spatial_ids_on_points(cross_point_list, cross_spatial_ids, zoom,
                                  CRS::kWebMercator);

        // 始点、終点の空間ID追加
        cross_spatial_ids.insert(start_spatial_id);
        cross_spatial_ids.insert(end_spatial_id);

        // 線分範囲の空間IDリスト
        spatial_id_list.clear();
        spatial_id_list.reserve(cross_spatial_ids.size());
        std::copy(cross_spatial_ids.begin(), cross_spatial_ids.end(),
                  std::back_inserter(spatial_id_list));

        if (Logger::IsOutput(LogLevel::kVerbose)) {
            for (const auto& spatial_id : spatial_id_list) {
                LOG_VERBOSE("線分範囲の空間ID=%s", spatial_id.c_str());
            }
        } else {
            LOG_DEBUG("線分範囲の空間ID数=%d", spatial_id_list.size());
        }
    }

    LOG_DEBUG("空間ID変換(線分)終了");
}

// X軸ボクセル境界を取得
void generate_x_voxel_plane(int start_x_id, int end_x_id,
                            std::vector<Plane>& x_plane_list, int zoom) {
    LOG_DEBUG("X境界面の取得=(%d,%d)", start_x_id, end_x_id);

    x_plane_list.clear();

    const Vector x_norm_vector = Vector(1, 0, 0);

    int start = 0;
    int end = 0;
    int num = end_x_id - start_x_id;
    if (num < 0) {
        start = end_x_id + 1;
        end = start_x_id + 1;
    } else if (num > 0) {
        start = start_x_id + 1;
        end = end_x_id + 1;
    }

    for (int x_id = start; x_id < end; x_id++) {
        // X軸の空間IDから頂点座標を投影座標で取得
        std::vector<Point> point_list;
        Zfxy zfxy(zoom, 0, x_id, 0);
        get_vertex_points_on_spatial_id(zfxy.getString(), point_list,
                                        CRS::kWebMercator);

        // 境界面上のX座標(投影座標)
        double x_point = point_list[0].x;
        for (size_t i = 1, n = point_list.size(); i < n; i++) {
            x_point = std::min(x_point, point_list[i].x);
        }
        const Point plane_orth_point(x_point, 0, 0);

        LOG_VERBOSE("X境界面上の空間ID=%s 座標=(%.10f,%.10f,%.10f)",
                    zfxy.getString().c_str(), plane_orth_point.x,
                    plane_orth_point.y, plane_orth_point.z);

        // X軸ボクセル境界面
        x_plane_list.push_back(Plane(plane_orth_point, x_norm_vector));
    }
}

// Y軸ボクセル境界を取得
void generate_y_voxel_plane(int start_y_id, int end_y_id,
                            std::vector<Plane>& y_plane_list, int zoom) {
    LOG_DEBUG("Y境界面の取得=(%d,%d)", start_y_id, end_y_id);

    y_plane_list.clear();

    const Vector y_norm_vector = Vector(0, 1, 0);

    int start = 0;
    int end = 0;
    int num = end_y_id - start_y_id;
    if (num < 0) {
        start = end_y_id + 1;
        end = start_y_id + 1;
    } else if (num > 0) {
        start = start_y_id + 1;
        end = end_y_id + 1;
    }

    for (int y_id = start; y_id < end; y_id++) {
        // Y軸の空間IDから頂点座標を投影座標で取得
        std::vector<Point> point_list;
        Zfxy zfxy(zoom, 0, 0, y_id);
        get_vertex_points_on_spatial_id(zfxy.getString(), point_list,
                                        CRS::kWebMercator);

        // 境界面上のY座標(投影座標)
        double y_point = point_list[0].y;
        for (size_t i = 1, n = point_list.size(); i < n; i++) {
            y_point = std::max(y_point, point_list[i].y);
        }
        const Point plane_orth_point(0, y_point, 0);

        LOG_VERBOSE("Y境界面上の空間ID=%s 座標=(%.10f,%.10f,%.10f)",
                    zfxy.getString().c_str(), plane_orth_point.x,
                    plane_orth_point.y, plane_orth_point.z);

        // Y軸ボクセル境界面
        y_plane_list.push_back(Plane(plane_orth_point, y_norm_vector));
    }
}

// Z軸ボクセル境界を取得
void generate_z_voxel_plane(int start_z_id, int end_z_id,
                            std::vector<Plane>& z_plane_list, int zoom) {
    LOG_DEBUG("Z境界面の取得=(%d,%d)", start_z_id, end_z_id);

    z_plane_list.clear();

    const Vector z_norm_vector = Vector(0, 0, 1);

    int start = 0;
    int end = 0;
    int num = end_z_id - start_z_id;
    if (num < 0) {
        start = end_z_id + 1;
        end = start_z_id + 1;
    } else if (num > 0) {
        start = start_z_id + 1;
        end = end_z_id + 1;
    }

    for (int z_id = start; z_id < end; z_id++) {
        // Z軸の空間IDから頂点座標を投影座標で取得
        std::vector<Point> point_list;
        Zfxy zfxy(zoom, z_id, 0, 0);
        get_vertex_points_on_spatial_id(zfxy.getString(), point_list,
                                        CRS::kWebMercator);

        // 境界面上のZ座標(投影座標)
        double z_point = point_list[0].z;
        for (size_t i = 1, n = point_list.size(); i < n; i++) {
            z_point = std::min(z_point, point_list[i].z);
        }
        const Point plane_orth_point(0, 0, z_point);

        LOG_VERBOSE("Z境界面上の空間ID=%s 座標=(%.10f,%.10f,%.10f)",
                    zfxy.getString().c_str(), plane_orth_point.x,
                    plane_orth_point.y, plane_orth_point.z);

        // Z軸ボクセル境界面
        z_plane_list.push_back(Plane(plane_orth_point, z_norm_vector));
    }
}

// 衝突判定
bool detect_collision(const Plane& plane, const Line& line,
                      Point& collision_point) {
    // 面上の一点から辺の始点へのベクトル
    Vector c0 = Vector::from_points(plane.point, line.point);

    // 進行方向と法線の内積
    double direct_dot = plane.normal.dot(line.direction);
    if (std::fabs(direct_dot) < kMinima) {
        // 進行方向と法線の内積が0(進行方向が面に平行)なので衝突なし
        LOG_DEBUG("線分と衝突判定面が平行");
        return false;
    }

    // 衝突時間算出
    double collision_time = -plane.normal.dot(c0) / direct_dot;
    if (0 <= collision_time && collision_time <= 1) {
        // 衝突時間が0～1の間にあれば衝突位置を算出
        collision_point = line.to_point(collision_time);
        return true;
    }

    return false;
}

/**
 * @brief 始点、終点を結んだ線分と、境界面との交点座標を取得してリストに追加
 *
 * @param plane_list 境界面を格納したリスト
 * @param start_orth_point 線分の始点(投影座標)
 * @param end_orth_point 線分の終点(投影座標)
 * @param[in,out] cross_point_list 交点座標リスト
 */
void get_plane_cross_points(
    const std::vector<Plane>& plane_list, const Point& start_orth_point,
    const Point& end_orth_point,
    std::unordered_set<Point, Point::hash_function>& cross_point_list) {
    // 始点終点の線
    Line line = Line::from_points(start_orth_point, end_orth_point);

    for (const auto& plane : plane_list) {
        // ボクセル境界と線の交点の座標を取得
        Point cross_point;
        if (!detect_collision(plane, line, cross_point)) {
            // 線分と境界面の交点が存在しない場合はスキップ
            LOG_DEBUG("線分と境界面との交点なし");
            continue;
        }

        LOG_VERBOSE("ボクセル境界と線の交点=(%.10f,%.10f,%.10f)", cross_point.x,
                    cross_point.y, cross_point.z);

        // 交点を交点座標リストに格納
        cross_point_list.insert(cross_point);
    }
}

/**
 * @brief 中点座標の取得してリストに追加
 *
 * @param start_orth_point 始点(投影座標)
 * @param end_orth_point 終点(投影座標)
 * @param[in,out] cross_point_list 交点座標リスト
 */
void get_middle_points(
    const Point& start_orth_point, const Point& end_orth_point,
    std::unordered_set<Point, Point::hash_function>& cross_point_list) {
    // 始点、終点を交点との中点も合わせて取得
    cross_point_list.insert(start_orth_point);
    cross_point_list.insert(end_orth_point);

    // 交点座標をソート
    std::vector<Point> ordered_cross_point_list(cross_point_list.begin(),
                                                cross_point_list.end());
    Line line = Line::from_points(start_orth_point, end_orth_point);
    if (std::fabs(line.direction.x) > 0) {
        LOG_DEBUG("交点座標ソート=X軸");
        std::sort(ordered_cross_point_list.begin(),
                  ordered_cross_point_list.end(),
                  [](const Point& l, const Point& r) { return (l.x < r.x); });
    } else if (std::fabs(line.direction.y) > 0) {
        LOG_DEBUG("交点座標ソート=Y軸");
        std::sort(ordered_cross_point_list.begin(),
                  ordered_cross_point_list.end(),
                  [](const Point& l, const Point& r) { return (l.y < r.y); });
    } else {
        LOG_DEBUG("交点座標ソート=Z軸");
        std::sort(ordered_cross_point_list.begin(),
                  ordered_cross_point_list.end(),
                  [](const Point& l, const Point& r) { return (l.z < r.z); });
    }

    // 交点の中点取得
    auto itr = ordered_cross_point_list.begin();
    while (itr != ordered_cross_point_list.end()) {
        const Point& p1 = *itr;
        if (++itr == ordered_cross_point_list.end()) {
            break;
        }
        const Point& p2 = *itr;
        Point middle_cross_point = (p1 + p2) / 2;

        LOG_VERBOSE("交点間の中点=(%.10f,%.10f,%.10f)", middle_cross_point.x,
                    middle_cross_point.y, middle_cross_point.z);

        cross_point_list.insert(middle_cross_point);
    }
}

}  // namespace spatialid
