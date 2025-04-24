/**
 * @file triangle.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <stdexcept>

#include "common/const.h"
#include "common/object/line.h"
#include "common/object/point.h"
#include "common/object/vector.h"

namespace spatialid {

/**
 * @brief 三角ポリゴン(投影座標)
 *
 */
typedef struct Triangle {
    Point points[3];  // 3頂点

    /**
     * @brief Construct a new Triangle object
     *
     */
    Triangle() : points{} {}

    /**
     * @brief Construct a new Triangle object
     *
     * @param a 頂点1
     * @param b 頂点2
     * @param c 頂点3
     * @throw std::invalid_argument 引数エラー
     */
    Triangle(const Point& a, const Point& b, const Point& c) : points{a, b, c} {
        Vector vector_a_c = Vector::from_points(a, b).unit();
        Vector vector_a_b = Vector::from_points(a, c).unit();
        if (vector_a_b.cross(vector_a_c).norm() < kMinima) {
            // 3点が同一直線上にある場合、面を形成しない
            throw std::invalid_argument("POLYGON_POINT_COLLINEAR");
        }
    }

    /**
     * @brief 法線ベクトル
     *
     * @return Vector
     */
    Vector normal() const {
        Vector vector_ab = Vector::from_points(points[0], points[1]);
        Vector vector_ac = Vector::from_points(points[0], points[2]);
        return vector_ab.cross(vector_ac);
    }

    /**
     * @brief 指定した頂点の向かい側の側面に沿った線を返す
     *
     * @param vertex_index 頂点番号(0-2)
     * @return Line
     */
    Line line(int vertex_index) const {
        if (vertex_index == 0) {
            return Line::from_points(points[1], points[2]);
        }
        if (vertex_index == 1) {
            return Line::from_points(points[2], points[0]);
        }
        if (vertex_index == 2) {
            return Line::from_points(points[0], points[1]);
        }
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }
} Triangle;

}  // namespace spatialid

#endif  // __TRIANGLE_H__
