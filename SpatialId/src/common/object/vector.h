/**
 * @file vector.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-30
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <cassert>
#include <cmath>
#include <stdexcept>

#include "common/convert.h"
#include "common/object/point.h"

namespace spatialid {

/**
 * @brief ベクトル(投影座標)
 *
 */
typedef struct Vector {
    double x;
    double y;
    double z;

    /**
     * @brief Construct a new Vector object
     *
     * @param xx
     * @param yy
     * @param zz
     */
    Vector(double xx = 0, double yy = 0, double zz = 0) : x(xx), y(yy), z(zz) {}

    /**
     * @brief +演算子
     *
     * @param rhs
     * @return Vector
     */
    Vector operator+(const Vector& rhs) const {
        return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    /**
     * @brief +演算子
     *
     * @param rhs
     * @return Point
     */
    Point operator+(const Point& rhs) const {
        return Point(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    /**
     * @brief -演算子
     *
     * @param rhs
     * @return Vector
     */
    Vector operator-(const Vector& rhs) const {
        return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    /**
     * @brief *演算子
     *
     * @param n
     * @return Vector
     */
    Vector operator*(double n) const { return Vector(x * n, y * n, z * n); }

    /**
     * @brief /演算子
     *
     * @param n
     * @return Point
     */
    Point operator/(double n) const {
        assert(n != 0);
        return Point(x / n, y / n, z / n);
    }

    /**
     * @brief 点aから点bへのベクトル
     *
     * @param a
     * @param b
     * @return Vector
     */
    static Vector from_points(const Point& a, const Point& b) {
        return Vector(b.x - a.x, b.y - a.y, b.z - a.z);
    }

    /**
     * @brief 点aから点bへのベクトル(2D)
     *
     * @param a
     * @param b
     * @return Vector
     */
    static Vector from_points_2d(const Point& a, const Point& b) {
        return Vector(b.x - a.x, b.y - a.y, 0);
    }

    /**
     * @brief 単位ベクトル
     *
     * @return Vector
     */
    Vector unit() const {
        double magnitude = norm();
        if (magnitude == 0) {
            throw std::invalid_argument("ZERO_DIVISION");
        }
        return Vector(x / magnitude, y / magnitude, z / magnitude);
    }

    /**
     * @brief ベクトルのノルム(長さ)
     *
     * @return double
     */
    double norm() const {
        return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
    }

    /**
     * @brief ほかのベクトルとの外積
     *
     * @param other
     * @return * Vector
     */
    Vector cross(const Vector& other) const {
        return Vector(y * other.z - z * other.y, z * other.x - x * other.z,
                      x * other.y - y * other.x);
    }

    /**
     * @brief ほかのベクトルとの内積
     *
     * @param other
     * @return double
     */
    double dot(const Vector& other) const {
        return (x * other.x + y * other.y + z * other.z);
    }

    /**
     * @brief 指定のベクトルと平行か
     *
     * @param other
     * @return true
     * @return false
     */
    bool is_parallel(const Vector& other) const {
        if (is_zero() || other.is_zero()) {
            return true;
        }
        double similarity = cosine_similarity(other);
        return is_close(std::fabs(similarity), 1);
    }

    /**
     * @brief ゼロベクトルか
     *
     * @return true
     * @return false
     */
    bool is_zero() const { return is_close(dot(*this), 0); }

    /**
     * @brief ほかのベクトルとのコサイン類似性を返す
     *
     * @param other
     * @return double
     */
    double cosine_similarity(const Vector& other) const {
        double denom = norm() * other.norm();
        if (denom == 0) {
            throw std::invalid_argument("VECTORS_ZERO");
        }
        double cos_theta = dot(other) / denom;
        return clip(cos_theta, -1, 1);
    }

    /**
     * @brief ほかのベクトルと垂直であるかどうか
     *
     * @param other
     * @return true
     * @return false
     */
    bool is_perpendicular(const Vector& other) const {
        return is_close(dot(other), 0);
    }

    /**
     * @brief ほかのベクトルとの角度をラジアンで返す(2D)
     *
     * @param other
     * @return double 角度(ラジアン)
     */
    double angle_signed_2d(const Vector& other) const {
        double dot = (x * other.x) + (y * other.y);
        double det = (x * other.y) - (y * other.x);
        return std::atan2(dot, det);
    }

    /**
     * @brief ほかのベクトルが向けられているベクトルの側面を返す(2D)
     *
     * @param other
     * @return int -1:右側 0:平行 1:左側
     */
    int side_vector_2d(const Vector& other) const {
        double cross = (x * other.y) - (y * other.x);
        return (cross > 0) ? 1 : ((cross < 0) ? -1 : 0);
    }

    double angle_between(const Vector& other) const {
        double cos_theta = cosine_similarity(other);
        return std::acos(cos_theta);
    }
} Vector;

}  // namespace spatialid

#endif  // __VECTOR_H__
