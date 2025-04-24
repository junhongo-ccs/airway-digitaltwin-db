/**
 * @file rectangular.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "rectangular.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

#include "common/const.h"
#include "common/object/projected_point.h"
#include "common/object/vector.h"
#include "common/shape/capsule_bullet.h"
#include "common/shape/cylinder_bullet.h"
#include "common/shape/sphere_bullet.h"
#include "common/spatial_line.h"
#include "common/spatial_point.h"
#include "common/transformer.h"
#include "spatial_api.h"

namespace spatialid {

// 空間IDを計算する
void Rectangular::calc_spatial_ids() {
    // 軸の初期化
    init_axis();
    // 直方体の境界面に対する交点取得
    calc_rectangular_apex();
    // 交点の空間ID取得
    calc_cross_spatial_ids();
}

// 空間IDリストを取得する
void Rectangular::get_spatial_ids(
    std::unordered_set<std::string>& spatial_id_list) {
    spatial_id_list = all_spatial_ids_;
}

// 図形を構成する軸・ベクトルを初期化
void Rectangular::init_axis() {
    // 軸ベクトル
    Vector axis_vector = Vector::from_points(start_point_, end_point_);
    LOG_DEBUG("軸ベクトル=(%.10f,%.10f,%.10f)", axis_vector.x, axis_vector.y,
              axis_vector.z);

    // 縦の長さ
    height_ = axis_vector.norm();
    LOG_DEBUG("縦の長さ=%.10f", height_);

    // 開始点の直交ベクトルを一つ求める
    // 直交ベクトル
    Vector start_orth_vector;
    if (std::fabs(axis_vector.y) < kMinima) {
        start_orth_vector = Vector(0, 1, 0);
    } else if (std::fabs(axis_vector.z) < kMinima) {
        start_orth_vector = Vector(0, 0, 1);
    } else {
        start_orth_vector = Vector(0, -axis_vector.z, axis_vector.y);
    }
    LOG_DEBUG("直交ベクトル=(%.10f,%.10f,%.10f)", start_orth_vector.x,
              start_orth_vector.y, start_orth_vector.z);

    // 直交ベクトルを機体半径の大きさにする。
    radius_orth_vector_ = start_orth_vector.unit() * radius_;
    LOG_DEBUG("機体半径の大きさ=(%.10f,%.10f,%.10f)", radius_orth_vector_.x,
              radius_orth_vector_.y, radius_orth_vector_.z);

    // 単位軸ベクトル
    unit_axis_vector_ = axis_vector.unit();
    LOG_DEBUG("単位軸ベクトル=(%.10f,%.10f,%.10f)", unit_axis_vector_.x,
              unit_axis_vector_.y, unit_axis_vector_.z);

    // 円との接点
    contact_point_ = radius_orth_vector_ + start_point_;
    LOG_DEBUG("円との接点=(%.10f,%.10f,%.10f)", contact_point_.x,
              contact_point_.y, contact_point_.z);

    // 軸ベクトルと直交ベクトルの法線ベクトル(大きさは機体半径)
    radius_normal_vector_ =
        radius_orth_vector_.cross(unit_axis_vector_).unit() * radius_;
    LOG_DEBUG("軸ベクトルと直交ベクトルの法線ベクトル=(%.10f,%.10f,%.10f)",
              radius_normal_vector_.x, radius_normal_vector_.y,
              radius_normal_vector_.z);
}

// 直方体の頂点を取得し、境界面との交点を取得
void Rectangular::calc_rectangular_apex() {
    // (投影座標)直方体の頂点
    std::vector<Point> apex_points;
    apex_points.reserve(8);
    apex_points.push_back(radius_normal_vector_ + contact_point_);
    apex_points.push_back((radius_normal_vector_ * -1) + contact_point_);
    apex_points.push_back((radius_orth_vector_ * -2) + apex_points[0]);
    apex_points.push_back((radius_orth_vector_ * -2) + apex_points[1]);
    apex_points.push_back((unit_axis_vector_ * height_) + apex_points[0]);
    apex_points.push_back((unit_axis_vector_ * height_) + apex_points[1]);
    apex_points.push_back((unit_axis_vector_ * height_) + apex_points[2]);
    apex_points.push_back((unit_axis_vector_ * height_) + apex_points[3]);

    if (Logger::IsOutput(LogLevel::kDebug)) {
        for (const auto& point : apex_points) {
            LOG_DEBUG_POINT(point.x, point.y, point.z, "直方体の頂点=");
        }
    }

    // (投影座標)直方体の辺
    std::vector<Line> rect_lines;
    rect_lines.reserve(12);
    rect_lines.push_back(Line::from_points(apex_points[0], apex_points[1]));
    rect_lines.push_back(Line::from_points(apex_points[0], apex_points[4]));
    rect_lines.push_back(Line::from_points(apex_points[1], apex_points[5]));
    rect_lines.push_back(Line::from_points(apex_points[4], apex_points[5]));
    rect_lines.push_back(Line::from_points(apex_points[2], apex_points[3]));
    rect_lines.push_back(Line::from_points(apex_points[2], apex_points[6]));
    rect_lines.push_back(Line::from_points(apex_points[6], apex_points[7]));
    rect_lines.push_back(Line::from_points(apex_points[3], apex_points[7]));
    rect_lines.push_back(Line::from_points(apex_points[0], apex_points[2]));
    rect_lines.push_back(Line::from_points(apex_points[1], apex_points[3]));
    rect_lines.push_back(Line::from_points(apex_points[4], apex_points[6]));
    rect_lines.push_back(Line::from_points(apex_points[5], apex_points[7]));

    if (Logger::IsOutput(LogLevel::kDebug)) {
        for (const auto& line : rect_lines) {
            LOG_DEBUG("直方体の辺=p(%.10f,%.10f,%.10f)d(%.10f,%.10f,%.10f)",
                      line.point.x, line.point.y, line.point.z,
                      line.direction.x, line.direction.y, line.direction.z);
        }
    }

    // 投影座標から地理座標に設定
    Transformer* transformer =
        Transformer::GetInstance(CRS::kWebMercator, CRS::kWGS84);

    // 直方体の各辺に対して空間IDを取得する
    for (const auto& rect_line : rect_lines) {
        ProjectedPoint edge_start = {0, 0, rect_line.point.z, rect_line.point.x,
                                     rect_line.point.y};
        Point p = rect_line.to_point();
        ProjectedPoint edge_end = {0, 0, p.z, p.x, p.y};

        // 投影座標から地理座標に座標変換
        transformer->Transform(edge_start.x, edge_start.y, edge_start.alt,
                               edge_start.lon, edge_start.lat, edge_start.alt);
        transformer->Transform(edge_end.x, edge_end.y, edge_end.alt,
                               edge_end.lon, edge_end.lat, edge_end.alt);

        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(edge_start.lon, edge_start.lat);
        Point::alignment_lonlat(edge_end.lon, edge_end.lat);

        // 空間ID
        std::vector<std::string> spatial_ids;
        get_spatial_ids_on_line(edge_start, edge_end, spatial_ids, zoom_);
        std::copy(spatial_ids.begin(), spatial_ids.end(),
                  std::inserter(all_spatial_ids_, all_spatial_ids_.end()));
    }

    LOG_DEBUG_UNORDERED_SET_STRING(all_spatial_ids_,
                                   "直方体の辺に対して空間ID");

    // 直方体頂点の空間IDを取得
    std::vector<std::string> edge_point_spatial_ids;
    get_spatial_ids_on_points(apex_points, edge_point_spatial_ids, zoom_,
                              CRS::kWebMercator);

    LOG_DEBUG_VECTOR_STRING(edge_point_spatial_ids, "直方体頂点の空間ID");

    // 直方体の各頂点の最大経度・最小経度を取得
    // 直方体の各頂点の最大緯度・最小緯度を取得
    // 直方体の各頂点の最大高さ・最小高さを取得
    int min_rect_x_index = std::numeric_limits<int>::max();
    int max_rect_x_index = std::numeric_limits<int>::min();
    int min_rect_y_index = min_rect_x_index;
    int max_rect_y_index = max_rect_x_index;
    int min_rect_z_index = min_rect_x_index;
    int max_rect_z_index = max_rect_x_index;
    for (const auto& edge_point_spatial_id : edge_point_spatial_ids) {
        Zfxy zfxy(edge_point_spatial_id);
        min_rect_x_index = std::min(min_rect_x_index, zfxy.lon_index);
        max_rect_x_index = std::max(max_rect_x_index, zfxy.lon_index);
        min_rect_y_index = std::min(min_rect_y_index, zfxy.lat_index);
        max_rect_y_index = std::max(max_rect_y_index, zfxy.lat_index);
        min_rect_z_index = std::min(min_rect_z_index, zfxy.alt_index);
        max_rect_z_index = std::max(max_rect_z_index, zfxy.alt_index);
    }

    LOG_DEBUG("直方体頂点空間IDのX成分範囲: %d〜%d", min_rect_x_index,
              max_rect_x_index);
    LOG_DEBUG("直方体頂点空間IDのY成分範囲: %d〜%d", min_rect_y_index,
              max_rect_y_index);
    LOG_DEBUG("直方体頂点空間IDのZ成分範囲: %d〜%d", min_rect_z_index,
              max_rect_z_index);

    // X軸ボクセル境界面を取得
    std::vector<Plane> x_voxel_planes;
    generate_x_voxel_plane(min_rect_x_index, max_rect_x_index, x_voxel_planes,
                           zoom_);

    // Y軸ボクセル境界面を取得
    std::vector<Plane> y_voxel_planes;
    generate_y_voxel_plane(min_rect_y_index, max_rect_y_index, y_voxel_planes,
                           zoom_);

    // Z軸ボクセル境界面を取得
    std::vector<Plane> z_voxel_planes;
    generate_z_voxel_plane(min_rect_z_index, max_rect_z_index, z_voxel_planes,
                           zoom_);

    // X境界面との交点を取得
    get_voxel_plane_cross_point(rect_lines, x_voxel_planes, y_voxel_planes,
                                z_voxel_planes, 2);

    // Y境界面との交点を取得
    get_voxel_plane_cross_point(rect_lines, y_voxel_planes, z_voxel_planes,
                                x_voxel_planes, 0);

    // Z境界面との交点を取得
    get_voxel_plane_cross_point(rect_lines, z_voxel_planes, x_voxel_planes,
                                y_voxel_planes, 1);
}

// 交点の空間IDを取得
void Rectangular::calc_cross_spatial_ids() {
    // 空間ID
    std::unordered_set<std::string> cross_spatial_ids;
    get_spatial_ids_on_points(all_cross_points_, cross_spatial_ids, zoom_,
                              CRS::kWebMercator);

    std::copy(cross_spatial_ids.begin(), cross_spatial_ids.end(),
              std::inserter(all_spatial_ids_, all_spatial_ids_.end()));
}

// 境界面との交点取得
void Rectangular::get_voxel_plane_cross_point(
    const std::vector<Line>& rect_lines,
    const std::vector<Plane>& first_voxel_planes,
    const std::vector<Plane>& second_voxel_planes,
    const std::vector<Plane>& third_voxel_planes, int axis) {
    // 第一ボクセル境界面毎に処理
    for (const auto& first_plane : first_voxel_planes) {
        // 第一ボクセル境界面に対する辺の交点を求める
        std::unordered_set<Point, Point::hash_function>
            unique_plane_collision_points;
        for (const auto& rect_line : rect_lines) {
            // 第一ボクセル境界と辺の交点の座標を取得
            Point collision_point;
            if (detect_collision(first_plane, rect_line, collision_point)) {
                unique_plane_collision_points.insert(collision_point);
            }
        }

        // 第一ボクセル境界への交点が1もしくは2の場合は辺の空間IDとして取得出来ているので、処理をスキップ
        if (unique_plane_collision_points.size() <= 2) {
            continue;
        }

        // 全交点リストに追加
        std::copy(unique_plane_collision_points.begin(),
                  unique_plane_collision_points.end(),
                  std::inserter(all_cross_points_, all_cross_points_.end()));

        if (Logger::IsOutput(LogLevel::kDebug)) {
            for (const auto& point : unique_plane_collision_points) {
                LOG_DEBUG_POINT(point.x, point.y, point.z,
                                "第一ボクセル境界への交点=");
            }
        }

        // 第一ボクセル境界への交点を結んだ交線リストを作成
        std::vector<Line> plane_collision_lines;
        for (const auto& p1 : unique_plane_collision_points) {
            for (const auto& p2 : unique_plane_collision_points) {
                plane_collision_lines.push_back(Line::from_points(p1, p2));
            }
        }

        if (Logger::IsOutput(LogLevel::kDebug)) {
            for (const auto& line : plane_collision_lines) {
                LOG_DEBUG(
                    "第一ボクセル境界への交点を結んだ交線リスト=p(%.10f,%.10f,%"
                    "."
                    "10f)d(%.10f,%.10f,%.10f)",
                    line.point.x, line.point.y, line.point.z, line.direction.x,
                    line.direction.y, line.direction.z);
            }
        }

        // 交線ごとの交点辞書を作成（キーは交線リストのインデックス）
        std::map<int, std::vector<Point>> collision_line_cross_points;

        // 第二ボクセル境界面毎に処理
        std::vector<Line> second_collision_lines;
        for (const auto& second_plane : second_voxel_planes) {
            std::unordered_set<Point, Point::hash_function>
                unique_second_collision_points;
            // 交線に対する第二ボクセル境界面の交点を取得
            int index = 0;
            for (const auto& plane_collision_line : plane_collision_lines) {
                // 第二ボクセル境界と辺の交点の座標を取得
                Point collision_point;
                if (detect_collision(second_plane, plane_collision_line,
                                     collision_point)) {
                    unique_second_collision_points.insert(collision_point);

                    collision_line_cross_points[index].push_back(
                        collision_point);
                }
                index++;
            }

            // 接触しない場合(0)、または、接触点は辺の場合(1)は取得済みとしてスキップ
            if (unique_second_collision_points.size() <= 1) {
                continue;
            }

            // 全交点リストに追加
            std::copy(
                unique_second_collision_points.begin(),
                unique_second_collision_points.end(),
                std::inserter(all_cross_points_, all_cross_points_.end()));

            if (Logger::IsOutput(LogLevel::kDebug)) {
                for (const auto& point : unique_second_collision_points) {
                    LOG_DEBUG_POINT(point.x, point.y, point.z,
                                    "第二ボクセル境界への交点=");
                }
            }

            // 第二ボクセル交線を取得
            auto it = unique_second_collision_points.begin();
            Point second_max_collision_point(*it);
            Point second_min_collision_point(*it);
            for (++it; it != unique_second_collision_points.end(); it++) {
                if (axis == 0) {
                    if ((*it).x > second_max_collision_point.x) {
                        second_max_collision_point = *it;
                    } else if ((*it).x < second_min_collision_point.x) {
                        second_min_collision_point = *it;
                    }
                } else if (axis == 1) {
                    if ((*it).y > second_max_collision_point.y) {
                        second_max_collision_point = *it;
                    } else if ((*it).y < second_min_collision_point.y) {
                        second_min_collision_point = *it;
                    }
                } else {
                    if ((*it).z > second_max_collision_point.z) {
                        second_max_collision_point = *it;
                    } else if ((*it).z < second_min_collision_point.z) {
                        second_min_collision_point = *it;
                    }
                }
            }
            second_collision_lines.push_back(Line::from_points(
                second_max_collision_point, second_min_collision_point));
        }

        // 第三ボクセル境界面毎に処理
        for (const auto& third_plane : third_voxel_planes) {
            std::vector<Point> third_collision_points;
            for (const auto& second_collision_line : second_collision_lines) {
                // 第三ボクセル境界と第二ボクセル交線の交点の座標を取得
                Point collision_point;
                if (detect_collision(third_plane, second_collision_line,
                                     collision_point)) {
                    LOG_DEBUG_POINT(
                        collision_point.x, collision_point.y, collision_point.z,
                        "第三ボクセル境界と第二ボクセル交線の交点=");

                    third_collision_points.push_back(collision_point);

                    // 全交点リストに追加
                    all_cross_points_.insert(collision_point);
                }
            }

            // 接触しない場合はスキップ
            if (third_collision_points.size() == 0) {
                continue;
            }

            // 第一交線に対する第三ボクセル境界面の交点を取得
            int index = 0;
            for (const auto& plane_collision_line : plane_collision_lines) {
                // 第三ボクセル境界と第一ボクセル交線の交点の座標を取得
                Point collision_point;
                if (detect_collision(third_plane, plane_collision_line,
                                     collision_point)) {
                    LOG_DEBUG_POINT(
                        collision_point.x, collision_point.y, collision_point.z,
                        "第三ボクセル境界と第一ボクセル交線の交点の交点=");

                    collision_line_cross_points[index].push_back(
                        collision_point);

                    // 全交点リストに追加
                    all_cross_points_.insert(collision_point);
                }
                index++;
            }
        }

        for (const auto& [index, cross_points] : collision_line_cross_points) {
            // 第一ボクセル境界面上の線の方向ベクトル
            Vector plane_collision_direction =
                plane_collision_lines[index].direction;

            // 並び替え
            std::vector<Point> sort_cross_points(cross_points.begin(),
                                                 cross_points.end());
            if (std::fabs(plane_collision_direction.x) > kMinima) {
                // 線の方向ベクトルのX成分がある場合はX軸
                std::sort(
                    sort_cross_points.begin(), sort_cross_points.end(),
                    [](const Point& l, const Point& r) { return (l.x < r.x); });
            } else if (std::fabs(plane_collision_direction.y) > kMinima) {
                // 線の方向ベクトルのY成分がある場合はY軸
                std::sort(
                    sort_cross_points.begin(), sort_cross_points.end(),
                    [](const Point& l, const Point& r) { return (l.y < r.y); });
            } else {
                // 線の方向ベクトルのz成分がある場合はZ軸
                std::sort(
                    sort_cross_points.begin(), sort_cross_points.end(),
                    [](const Point& l, const Point& r) { return (l.z < r.z); });
            }

            for (auto it = sort_cross_points.begin();
                 it != sort_cross_points.end();) {
                // 第一ボクセル交線の第二ボクセル境界・第三ボクセル境界との交点の中点取得
                const Point& p1 = *it;
                if (++it == sort_cross_points.end()) {
                    break;
                }
                const Point& p2 = *it;
                Point middle_cross_point = (p1 + p2) / 2;

                LOG_DEBUG_POINT(middle_cross_point.x, middle_cross_point.y,
                                middle_cross_point.z,
                                "第一ボクセル交線の第二ボクセル境界・第三ボクセ"
                                "ル境界との交点の中点=");

                // 全交点リストに追加
                all_cross_points_.insert(middle_cross_point);
            }
        }
    }
}

}  // namespace spatialid
