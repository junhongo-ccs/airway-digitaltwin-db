/**
 * @file base_bullet.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-13
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "base_bullet.h"

namespace spatialid {

// カプセルオブジェクトの作成
void BaseBullet::createCapsule(
    double radius, double height,
    const std::tuple<double, double, double, double>& orientation,
    const Point& center_point) {
    object_.geom_type = GeomType::kGeomCapsule;
    object_.radius = radius;
    object_.height = height;
    object_.orientations[0] = std::get<0>(orientation);
    object_.orientations[1] = std::get<1>(orientation);
    object_.orientations[2] = std::get<2>(orientation);
    object_.orientations[3] = std::get<3>(orientation);
    object_.center_point[0] = center_point.x;
    object_.center_point[1] = center_point.y;
    object_.center_point[2] = center_point.z;
}

// シリンダーオブジェクトの作成
void BaseBullet::createCylinder(
    double radius, double height,
    const std::tuple<double, double, double, double>& orientation,
    const Point& center_point) {
    object_.geom_type = GeomType::kGeomCylinder;
    object_.radius = radius;
    object_.height = height;
    object_.orientations[0] = std::get<0>(orientation);
    object_.orientations[1] = std::get<1>(orientation);
    object_.orientations[2] = std::get<2>(orientation);
    object_.orientations[3] = std::get<3>(orientation);
    object_.center_point[0] = center_point.x;
    object_.center_point[1] = center_point.y;
    object_.center_point[2] = center_point.z;
}

// 球オブジェクトの作成
void BaseBullet::createSphere(double radius, const Point& center_point) {
    object_.geom_type = GeomType::kGeomSphere;
    object_.radius = radius;
    object_.center_point[0] = center_point.x;
    object_.center_point[1] = center_point.y;
    object_.center_point[2] = center_point.z;
}

// ボクセルとの衝突判定
bool BaseBullet::is_collide_voxcel(const Point& voxel_center,
                                   const Point& half_extent) {
    if (engine_ && object_.geom_type != GeomType::kGeomUnknown) {
        // ボクセル(空間)
        BulletObject objectB;
        objectB.geom_type = GeomType::kGeomBox;
        objectB.half_extents[0] = half_extent.x;
        objectB.half_extents[1] = half_extent.y;
        objectB.half_extents[2] = half_extent.z;
        objectB.center_point[0] = voxel_center.x;
        objectB.center_point[1] = voxel_center.y;
        objectB.center_point[2] = voxel_center.z;

        return engine_->hitTest(object_, objectB);
    }
    return false;
}

// 姿勢目標とする軸ベクトルからクォータニオンを取得する
void BaseBullet::getQuaternion(const Vector& axis_vector, 
    std::tuple<double, double, double, double>& orientation) {
    Vector start_vector = Vector(0, 1, 0);
    Vector unit_axis_vector = axis_vector.unit();

    //回転軸と回転角
    double eps = 1.0e-06;
    Vector rot_axis = Vector(1, 0, 0); //初期値
    double angle = std::acos(start_vector.dot(unit_axis_vector)) / 2;
    
    if (std::fabs(angle) > eps && std::fabs(angle - M_PI / 2) > eps) {
        rot_axis = start_vector.cross(axis_vector).unit();
    }

    //クォータニオンを計算
    double c = std::cos(angle);
    double s = std::sin(angle);
    double qw = c;
    double qx = rot_axis.x * s;
    double qy = rot_axis.y * s;
    double qz = rot_axis.z * s;

    orientation = std::make_tuple(qx, qy, qz, qw);
}


}  // namespace spatialid
