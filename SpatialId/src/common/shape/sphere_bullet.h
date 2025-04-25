/**
 * @file sphere_bullet.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-11
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __SPHERE_BULLET_H__
#define __SPHERE_BULLET_H__

#include <stdexcept>

#include "common/object/point.h"
#include "common/shape/base_bullet.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 球 Bullet(投影座標)
 *
 */
class SphereBullet : public BaseBullet {
   public:
    /**
     * @brief Construct a new Sphere Bullet object
     *
     * @param radius 球の半径
     * @param center_point 球の中心
     */
    SphereBullet(double radius, const Point& center_point) : BaseBullet() {
        // 衝突判定用の球オブジェクトをセット
        createSphere(radius, center_point);
    }
};

}  // namespace spatialid

#endif  // __SPHERE_BULLET_H__
