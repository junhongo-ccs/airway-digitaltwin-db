/**
 * @file cylinder_bullet.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __CYLINDER_BULLET_H__
#define __CYLINDER_BULLET_H__

#include <stdexcept>

#include "common/object/point.h"
#include "common/object/vector.h"
#include "common/shape/base_bullet.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 円柱 Bullet(投影座標)
 *
 */
class CylinderBullet : public BaseBullet {
   public:
    /**
     * @brief Construct a new Cylinder Bullet object
     *
     * @param radius 円柱の半径
     * @param start_point 円柱の始点
     * @param end_point 円柱の終点
     */
    CylinderBullet(double radius, const Point& start_point,
                   const Point& end_point)
        : BaseBullet() {
        Vector axis_vector(end_point.x - start_point.x,
                           end_point.y - start_point.y,
                           end_point.z - start_point.z);
        Point center_point = (start_point + end_point) / 2;
        double height = axis_vector.norm();

        // 傾き(四元数)
        // ヨーイング角は円柱の形状的に任意でよいため、0ラジアンとする
        std::tuple<double, double, double, double> orientation;
        getQuaternion(axis_vector, orientation);

        // 衝突判定用の円柱オブジェクトをセット
        // デフォルトでは向きはz軸方向になっている
        createCylinder(radius, height, orientation, center_point);
    }
};

}  // namespace spatialid

#endif  // __CYLINDER_BULLET_H__
