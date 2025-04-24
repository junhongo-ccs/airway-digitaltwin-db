/**
 * @file base_bullet.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __BASE_BULLET_H__
#define __BASE_BULLET_H__

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdexcept>
#include <tuple>

#include "common/object/point.h"
#include "common/object/vector.h"
#include "common/shape/bullet_engine.h"
#include "common/shape/bullet_object.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief ベース Bullet(投影座標)
 *
 */
class BaseBullet {
   protected:
    BulletEngine* engine_;
    BulletObject object_;

   public:
    /**
     * @brief Construct a new Base Bullet object
     *
     */
    BaseBullet(){};
    /**
     * @brief Destroy the Base Bullet object
     *
     */
    virtual ~BaseBullet(){};

    /**
     * @brief 衝突判定エンジンを設定する
     *
     * @param engine_
     */
    void set_engine(BulletEngine* engine) { engine_ = engine; }

    /**
     * @brief ボクセルとの衝突判定
     *
     * @param voxel_center ボクセル中心
     * @param half_extent ボクセル中心からの頂点までの成分
     * @return true 衝突している
     * @return false 衝突していない
     */
    bool is_collide_voxcel(const Point& voxel_center, const Point& half_extent);

   protected:
    /**
     * @brief Create a Capsule object
     *
     * @param radius 半径
     * @param height 高さ
     * @param orientation 向き
     * @param center_point 中心座標
     */
    void createCapsule(
        double radius, double height,
        const std::tuple<double, double, double, double>& orientation,
        const Point& center_point);

    /**
     * @brief Create a Cylinder object
     *
     * @param radius 半径
     * @param height 高さ
     * @param orientation 向き
     * @param center_point 中心座標
     */
    void createCylinder(
        double radius, double height,
        const std::tuple<double, double, double, double>& orientation,
        const Point& center_point);

    /**
     * @brief Create a Sphere object
     * 
     * @param radius 半径
     * @param center_point 中心座標
     */
    void createSphere(double radius, const Point& center_point);

    /**
     * @brief 姿勢目標とする軸ベクトルからクォータニオンを取得する
     *
     * @param axis_vector
     * @param[out] orientation
     */
    void getQuaternion(
        const Vector& axis_vector,
        std::tuple<double, double, double, double>& orientation);

    /**
     * @brief 角度補正処理
     *
     * @param radian 角度(ラジアン)
     * @return double 90度を超える場合は、180°- 角度の値を返却
     */
    double angle_quart_correction(double radian) {
        return (M_PI_2 < radian) ? (M_PI - radian) : radian;
    }

    /**
     * @brief x軸周りの回転変換
     *
     * @param v 回転対象のベクトル
     * @param radian 回転角度(ラジアン)
     * @return Vector 回転結果のベクトル
     */
    Vector rotate_x(const Vector& v, double radian) const {
        double c = std::cos(radian);
        double s = std::sin(radian);
        return Vector(v.x, (v.y * c) - (v.z * s), (v.y * s) + (v.z * c));
    }

    /**
     * @brief y軸周りの回転行列
     *
     * @param v 回転対象のベクトル
     * @param radian 回転角度(ラジアン)
     * @return Vector 回転結果のベクトル
     */
    Vector rotate_y(const Vector& v, double radian) const {
        double c = std::cos(radian);
        double s = std::sin(radian);
        return Vector((v.z * s) + (v.x * c), v.y, (v.z * c) - (v.x * s));
    }

    /**
     * @brief z軸周りの回転行列
     *
     * @param v 回転対象のベクトル
     * @param radian 回転角度(ラジアン)
     * @return Vector 回転結果のベクトル
     */
    Vector rotate_z(const Vector& v, double radian) const {
        double c = std::cos(radian);
        double s = std::sin(radian);
        return Vector((v.x * c) - (v.y * s), (v.x * s) + (v.y * c), v.z);
    }
};

}  // namespace spatialid

#endif  // __BASE_BULLET_H__
