/**
 * @file capsule_bullet.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __CAPSULE_BULLET_H__
#define __CAPSULE_BULLET_H__

#include <stdexcept>

#include "common/shape/base_bullet.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief カプセル Bullet(投影座標)
 *
 */
class CapsuleBullet : public BaseBullet {
   public:
    /**
     * @brief Construct a new Capsule Bullet object
     *
     * @param radius カプセルの半径
     * @param start_point カプセルの始点(投影座標)
     * @param end_point カプセルの終点(投影座標)
     */
    CapsuleBullet(double radius, const Point& start_point,
                  const Point& end_point) {
        Vector axis_vector(end_point.x - start_point.x,
                           end_point.y - start_point.y,
                           end_point.z - start_point.z);
        Point center_point = (start_point + end_point) / 2;
        double height = axis_vector.norm();

        // 傾き(四元数)
        // ヨーイング角は円柱の形状的に任意でよいため、0ラジアンとする
        std::tuple<double, double, double, double> orientation;
        getQuaternion(axis_vector, orientation);

        // 衝突判定用のカプセルオブジェクトをセット
        // デフォルトでは向きはz軸方向になっている
        createCapsule(radius, height, orientation, center_point);
    }
};

}  // namespace spatialid

#endif  // __CAPSULE_BULLET_H__
