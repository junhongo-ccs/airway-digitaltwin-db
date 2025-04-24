/**
 * @file plane.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __PLANE_H__
#define __PLANE_H__

#include <stdexcept>

#include "common/object/point.h"
#include "common/object/vector.h"

namespace spatialid {

/**
 * @brief 平面(投影座標)
 *
 */
typedef struct Plane {
    Point point;
    Vector normal;

    /**
     * @brief Construct a new Plane object
     *
     */
    Plane() : point{}, normal{} {};

    /**
     * @brief Construct a new Plane object
     *
     * @param p 点
     * @param n 法線ベクトル
     */
    Plane(const Point& p, const Vector& n) : point(p), normal(n){};

    /**
     * @brief 線と平面の交点を返す
     *
     * @param line 線
     * @return Point 交点
     */
    Point intersect_line(const Line& line) const {
        if (normal.is_perpendicular(line.direction)) {            
            throw std::invalid_argument("LINE_AND_PLANE_PARALLEL");
        }
        Vector is_perpendicular = Vector::from_points(point, line.point);
        double num = -normal.dot(is_perpendicular);
        double denom = normal.dot(line.direction);
        Vector vector_line_scaled = line.direction * (num / denom);
        return vector_line_scaled + line.point;
    }
} Plane;

}  // namespace spatialid

#endif  // __PLANE_H__
