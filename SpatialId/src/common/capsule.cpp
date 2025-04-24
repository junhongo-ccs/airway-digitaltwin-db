/**
 * @file capsule.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "capsule.h"

#include <algorithm>
#include <cassert>
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

// コンストラクタ
Capsule::Capsule(BulletEngine* engine, const Point& start_point,
                 const Point& end_point, double radius, int zoom,
                 bool is_capsule)
    : Rectangular(start_point, end_point, radius, zoom),
      is_capsule_(is_capsule),
      is_sphere_(false),
      approximate_voxel_num_(0) {
    // Bulletの生成
    if (start_point_.is_close(end_point_)) {
        bullet_.reset(new SphereBullet(radius, start_point));
        is_sphere_ = true;
    } else if (is_capsule_) {
        bullet_.reset(new CapsuleBullet(radius, start_point, end_point));
    } else {
        bullet_.reset(new CylinderBullet(radius, start_point, end_point));
    }
    bullet_->set_engine(engine);
}

// 空間IDを計算する
void Capsule::calc_spatial_ids() {
    Rectangular::calc_spatial_ids();

    // 交点の有効な空間ID取得
    calc_valid_spatial_ids();
}

// 空間IDリストを取得する
void Capsule::get_spatial_ids(
    std::unordered_set<std::string>& spatial_id_list) {
    spatial_id_list = include_spatial_ids_;
}

// 球の空間IDから中心から終点方向へ半径分延ばした立方体分の空間IDを削る
void Capsule::shave_sphere(const Point& direction_point) {
    // 単位軸ベクトル
    Vector unit_axis_vector =
        Vector::from_points(start_point_, direction_point).unit();

    // 直方体の終点
    Point direction_end_point = unit_axis_vector * radius_ + start_point_;

    // 中心から終点方向へ半径分延ばした立方体分の空間IDを取得
    std::unique_ptr<Rectangular> except_rectangular(
        new Rectangular(start_point_, direction_end_point, radius_, zoom_));
    except_rectangular->calc_spatial_ids();
    std::unordered_set<std::string> except_rectangular_spatial_ids;
    except_rectangular->get_spatial_ids(except_rectangular_spatial_ids);

    LOG_DEBUG_UNORDERED_SET_STRING(
        except_rectangular_spatial_ids,
        "中心から半径分延ばした立方体分の空間IDを取得");

    std::unordered_set<std::string> result;
    std::set_difference(include_spatial_ids_.begin(),
                        include_spatial_ids_.end(),
                        except_rectangular_spatial_ids.begin(),
                        except_rectangular_spatial_ids.end(),
                        std::inserter(result, result.end()));
    include_spatial_ids_ = result;
}

// 図形を構成する軸・ベクトルを初期化
void Capsule::init_axis() {
    // 軸ベクトル(投影座標)
    Vector axis_vector = Vector::from_points(start_point_, end_point_);
    LOG_DEBUG("軸ベクトル=(%.10f,%.10f,%.10f)", axis_vector.x, axis_vector.y,
              axis_vector.z);

    // 縦の長さ
    height_ = axis_vector.norm();
    // カプセル/球の場合
    if (is_capsule_ || is_sphere_) {
        height_ += (2 * radius_);
    }
    LOG_DEBUG("縦の長さ=%.10f", height_);

    // 開始点の直交ベクトルを一つ求める
    // 直交ベクトル
    Vector start_orth_vector;
    if (std::fabs(axis_vector.y) < kMinima || is_sphere_) {
        start_orth_vector = Vector(0, 1, 0);
    } else if (std::fabs(axis_vector.z) < kMinima) {
        start_orth_vector = Vector(0, 0, 1);
    } else {
        start_orth_vector = Vector(0, -axis_vector.z, axis_vector.y);
    }
    LOG_DEBUG("直交ベクトル=(%.10f,%.10f,%.10f)", start_orth_vector.x,
              start_orth_vector.y, start_orth_vector.z);

    // 直交ベクトルを機体半径の大きさにする
    radius_orth_vector_ = start_orth_vector.unit() * radius_;
    LOG_DEBUG("機体半径の大きさ=(%.10f,%.10f,%.10f)", radius_orth_vector_.x,
              radius_orth_vector_.y, radius_orth_vector_.z);

    // 単位軸ベクトル
    if (is_sphere_) {
        unit_axis_vector_ = Vector(1, 0, 0);
    } else {
        unit_axis_vector_ = axis_vector.unit();
    }
    LOG_DEBUG("単位軸ベクトル=(%.10f,%.10f,%.10f)", unit_axis_vector_.x,
              unit_axis_vector_.y, unit_axis_vector_.z);

    // 円柱は軸ベクトルを零ベクトルとする
    Vector radius_axis_vector;
    if (!is_capsule_ && !is_sphere_) {
        radius_axis_vector = Vector(0, 0, 0);
    } else {
        radius_axis_vector = unit_axis_vector_ * radius_;
    }
    LOG_DEBUG("円柱は軸ベクトルを零ベクトルとする=(%.10f,%.10f,%.10f)",
              radius_axis_vector.x, radius_axis_vector.y, radius_axis_vector.z);

    // 円との接点
    contact_point_ = (radius_orth_vector_ - radius_axis_vector) + start_point_;
    LOG_DEBUG("円との接点=(%.10f,%.10f,%.10f)", contact_point_.x,
              contact_point_.y, contact_point_.z);

    // 軸ベクトルと直交ベクトルの法線ベクトル(大きさは機体半径)
    radius_normal_vector_ =
        radius_orth_vector_.cross(unit_axis_vector_).unit() * radius_;
    LOG_DEBUG("軸ベクトルと直交ベクトルの法線ベクトル=(%.10f,%.10f,%.10f)",
              radius_normal_vector_.x, radius_normal_vector_.y,
              radius_normal_vector_.z);
}

// 直方体の空間IDから実際の図形分の有効な空間IDを取得
void Capsule::calc_valid_spatial_ids() {
    // 対角線分の単位ボクセルの距離(概算距離)
    Vector unit_voxel_vector = calc_unit_voxel_vector();

    // 始点から対角線分の単位ボクセルの座標
    double approximate_distance = unit_voxel_vector.norm();

    // 概算ボクセル数（概算距離が機体半径内のボクセル数）
    approximate_voxel_num_ =
        static_cast<int>(std::floor(radius_ / approximate_distance));

    LOG_DEBUG("対角線分の単位ボクセルの距離(概算距離)=(%.10f,%.10f,%.10f)",
              unit_voxel_vector.x, unit_voxel_vector.y, unit_voxel_vector.z);
    LOG_DEBUG("概算ボクセル数（概算距離が機体半径内のボクセル数）=%d",
              approximate_voxel_num_);

    // 始点終点の座標(投影座標)
    ProjectedPoint start_project_point = {0, 0, start_point_.z, start_point_.x,
                                          start_point_.y};
    ProjectedPoint end_project_point = {0, 0, end_point_.z, end_point_.x,
                                        end_point_.y};

    // 投影座標から地理座標に座標変換
    Transformer* transformer =
        Transformer::GetInstance(CRS::kWebMercator, CRS::kWGS84);
    transformer->Transform(start_project_point.x, start_project_point.y,
                           start_project_point.alt, start_project_point.lon,
                           start_project_point.lat, start_project_point.alt);
    transformer->Transform(end_project_point.x, end_project_point.y,
                           end_project_point.alt, end_project_point.lon,
                           end_project_point.lat, end_project_point.alt);

    // 地理座標の経度・緯度の桁揃え
    Point::alignment_lonlat(start_project_point.lon, start_project_point.lat);
    Point::alignment_lonlat(end_project_point.lon, end_project_point.lat);

    // 始点終点間の空間ID取得
    std::vector<std::string> line_spatial_ids;
    if (!is_sphere_) {
        // 円柱・カプセルの場合
        get_spatial_ids_on_line(start_project_point, end_project_point,
                                line_spatial_ids, zoom_);

        LOG_DEBUG_VECTOR_STRING(line_spatial_ids,
                                "円柱・カプセルの場合、始点終点間の空間ID=");
    } else {
        // 球の場合
        line_spatial_ids.push_back(get_spatial_id_on_point(
            start_project_point.lon, start_project_point.lat,
            start_project_point.alt, zoom_));
        line_spatial_ids.push_back(get_spatial_id_on_point(
            end_project_point.lon, end_project_point.lat, end_project_point.alt,
            zoom_));

        LOG_DEBUG_VECTOR_STRING(line_spatial_ids, "球の場合、接続点の空間ID=");
    }

    // 円柱の空間ID簡易取得
    for (const auto& line_spatial_id : line_spatial_ids) {
        std::unordered_set<std::string> approximate_distance_radius_spatial_ids;
        approximate_distance_radius_points(
            line_spatial_id, approximate_distance_radius_spatial_ids);

        LOG_DEBUG("円柱の空間ID簡易取得(起点)=%s", line_spatial_id.c_str());
        LOG_DEBUG_UNORDERED_SET_STRING(approximate_distance_radius_spatial_ids,
                                       "円柱の空間ID簡易取得(結果)=");

        for (const auto& id1 : approximate_distance_radius_spatial_ids) {
            for (const auto& id2 : all_spatial_ids_) {
                if (id1 == id2) {
                    include_spatial_ids_.insert(id1);
                }
            }
        }
    }

    // Bulletと衝突判定を行う空間ID
    std::unordered_set<std::string> exclude_spatial_ids;
    std::set_difference(
        all_spatial_ids_.begin(), all_spatial_ids_.end(),
        include_spatial_ids_.begin(), include_spatial_ids_.end(),
        std::inserter(exclude_spatial_ids, exclude_spatial_ids.end()));

    LOG_DEBUG_UNORDERED_SET_STRING(exclude_spatial_ids,
                                   "Bulletと衝突判定を行う空間ID=");

    // ボクセルの中心からの頂点までのベクトル
    Point half_extent = unit_voxel_vector / 2;
    for (const auto& exclude_spatial_id : exclude_spatial_ids) {
        // ボクセルの中心座標
        Point center_point = get_center_point_on_spatial_id(exclude_spatial_id,
                                                            CRS::kWebMercator);

        LOG_DEBUG("ボクセルの中心座標=(%.10f,%.10f,%.10f)", center_point.x,
                  center_point.y, center_point.z);

        // Bulletと衝突判定
        if (bullet_->is_collide_voxcel(center_point, half_extent)) {
            LOG_DEBUG("Bulletと衝突=%s", exclude_spatial_id.c_str());

            include_spatial_ids_.insert(exclude_spatial_id);
        }
    }
}

// 概算距離内の空間ID取得
void Capsule::approximate_distance_radius_points(
    const std::string& spatial_id,
    std::unordered_set<std::string>& approximate_distance_radius_spatial_ids) {
    approximate_distance_radius_spatial_ids.clear();

    for (int x_index = approximate_voxel_num_;
         x_index <= approximate_voxel_num_; x_index++) {
        int y_range = approximate_voxel_num_ - std::abs(x_index);
        for (int y_index = -y_range; y_index <= y_range; y_index++) {
            int z_range = y_range - std::abs(y_index);
            for (int z_index = -z_range; z_index <= z_range; z_index++) {
                // 概算距離内のボクセル数を上限に空間IDをシフト
                std::string approximate_id = get_shifting_spatial_id(
                    spatial_id, x_index, y_index, z_index);
                approximate_distance_radius_spatial_ids.insert(approximate_id);
            }
        }
    }
}

// 概算距離用の単位ボクセルベクトルを算出
Vector Capsule::calc_unit_voxel_vector() {
    // 任意の空間IDの頂点座標を取得
    auto itr = all_spatial_ids_.begin();
    assert(itr != all_spatial_ids_.end());
    std::string spatial_id(*itr);
    std::vector<Point> points;
    get_vertex_points_on_spatial_id(spatial_id, points, CRS::kWebMercator);

    // 各XYZ成分座標から単位ボクセルベクトルを算出
    double point_x_max = points[0].x;
    double point_x_min = point_x_max;
    double point_y_max = points[0].y;
    double point_y_min = point_y_max;
    double point_z_max = points[0].z;
    double point_z_min = point_z_max;
    for (int i = 1; i < points.size(); i++) {
        point_x_max = std::max(point_x_max, points[i].x);
        point_x_min = std::min(point_x_min, points[i].x);
        point_y_max = std::max(point_y_max, points[i].y);
        point_y_min = std::min(point_y_min, points[i].y);
        point_z_max = std::max(point_z_max, points[i].z);
        point_z_min = std::min(point_z_min, points[i].z);
    }
    return Vector(point_x_max - point_x_min, point_y_max - point_y_min,
                  point_z_max - point_z_min);
}

}  // namespace spatialid
