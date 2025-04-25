/**
 * @file line.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-31
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __LINE_H__
#define __LINE_H__

#include <stdexcept>

#include "common/object/line.h"
#include "common/object/point.h"
#include "common/object/vector.h"

namespace spatialid {

/**
 * @brief 線(投影座標)
 *
 */
typedef struct Line {
    Point point;
    Vector direction;

    /**
     * @brief Construct a new Line object
     *
     */
    Line() : point{}, direction{} {};

    /**
     * @brief Construct a new Line object
     *
     * @param p 始点
     * @param d ベクトル
     */
    Line(const Point& p, const Vector& d) : point(p), direction(d) {}

    /**
     * @brief 比較演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator<(const Line& rhs) const {
        return (equal(point.x, rhs.point.x))
                   ? (equal(point.y, rhs.point.y) ? (point.z < rhs.point.z)
                                                  : (point.y < rhs.point.y))
                   : (point.x < rhs.point.x);
    }

    /**
     * @brief 2点からLineオブジェクトを生成する
     *
     * @param a 始点
     * @param b 終点
     * @return Line 線
     */
    static Line from_points(const Point& a, const Point& b) {
        return Line(a, Vector::from_points(a, b));
    }

    /**
     * @brief 2点からLineオブジェクトを生成する(2D)
     *
     * @param a 始点
     * @param b 終点
     * @return Line 線
     */
    static Line from_points_2d(const Point& a, const Point& b) {
        return Line(a, Vector::from_points_2d(a, b));
    }

    /**
     * @brief 線に沿った点を返す
     *
     * @param t ベクトルの倍率
     * @return Point 点
     */
    Point to_point(double t = 1) const {
        return Point(point.x + t * direction.x, point.y + t * direction.y,
                     point.z + t * direction.z);
    }

    /**
     * @brief ほかの線との交差点を返す
     *
     * @param other
     * @return Point 交差点
     * @throw std::invalid_argument 引数エラー
     */
    Point intersect_line(const Line& other) const {
        if (direction.is_parallel(other.direction)) {
            throw std::invalid_argument("LINES_PARALLEL");
        }
        Vector vector_ab = Vector::from_points(point, other.point);
        Vector vector_perpendicular = direction.cross(other.direction);
        double num = vector_ab.cross(other.direction).dot(vector_perpendicular);
        double denom = std::pow(vector_perpendicular.norm(), 2);
        return direction * (num / denom) + point;
    }

    /**
     * @brief 点が線のどちら側にあるか(2D)
     *
     * @param p
     * @return int -1:線の左側に点がある 0:線上に点がある 1:線の右側に点がある
     */
    int side_point_2d(const Point& p) const {
        Vector vector_to_point = Vector::from_points(point, p);
        return direction.side_vector_2d(vector_to_point);
    }

    /**
     * @brief 浮動小数点の等価比較
     *
     * @param a
     * @param b
     * @return true
     * @return false
     */
    static bool equal(double a, double b) {
        // return (std::fabs(a - b) < __DBL_EPSILON__);
        return (std::fabs(a - b) < kMinima);
    }

} Line;

}  // namespace spatialid

#endif  // __LINE_H__
