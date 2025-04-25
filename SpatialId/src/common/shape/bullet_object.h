/**
 * @file bullet_object.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __BULLET_OBJECT_H__
#define __BULLET_OBJECT_H__

#include <string>

namespace spatialid {

/**
 * @brief 衝突判定オブジェクトの種類
 *
 */
enum class GeomType : int {
    kGeomUnknown = 0,
    kGeomSphere,
    kGeomBox,
    kGeomCylinder,
    kGeomCapsule,
};

/**
 * @brief 衝突判定用オブジェクト
 *
 */
typedef struct BulletObject {
    GeomType geom_type;         // 衝突判定オブジェクトの種類
    double half_extents[3];     // box
    double radius;              // shape, capsule, cylinder
    double height;              // capsule, cylinder
    double center_point[3];     // box, shape, capsule, cylinder
    double orientations[4];     // capsule, cylinder
} BulletObject;

}  // namespace spatialid

#endif  // __BULLET_OBJECT_H__
