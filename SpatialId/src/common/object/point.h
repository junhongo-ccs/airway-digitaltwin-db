/**
 * @file point.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __POINT_H__
#define __POINT_H__

#include <cmath>
#include <stdexcept>
#include <unordered_set>

#include "common/const.h"

namespace spatialid {

/**
 * @brief 座標(地理座標 または 投影座標)
 *
 */
typedef struct Point {
    double x;  // 地理座標:経度、投影座標:X座標
    double y;  // 地理座標:緯度、投影座標:Y座標
    double z;  // 地理座標:高さ、投影座標:高さ

    /**
     * @brief Construct a new Point object
     *
     * @param xx 地理座標:経度、投影座標:X座標
     * @param yy 地理座標:緯度、投影座標:Y座標
     * @param zz 地理座標:高さ、投影座標:高さ
     */
    Point(double xx = 0, double yy = 0, double zz = 0) : x{xx}, y{yy}, z{zz} {}

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
     * @brief /演算子
     *
     * @param n
     * @return Point
     */
    Point operator/(double n) const { return Point(x / n, y / n, z / n); }

    /**
     * @brief 等値演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator==(const Point& rhs) const {
        return (equal(x, rhs.x) && equal(y, rhs.y) && equal(z, rhs.z));
    }

    /**
     * @brief 非等値演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator!=(const Point& rhs) const { return !(*this == rhs); }

    /**
     * @brief 比較演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator<(const Point& rhs) const {
        return equal(x, rhs.x) ? (equal(y, rhs.y) ? (z < rhs.z) : (y < rhs.y))
                               : (x < rhs.x);
    }

    /**
     * @brief 比較演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator>(const Point& rhs) const {
        return equal(x, rhs.x) ? (equal(y, rhs.y) ? (z > rhs.z) : (y > rhs.y))
                               : (x > rhs.x);
    }

    /**
     * @brief XYZの2点間の距離を返す(投影座標)
     *
     * @param other
     * @return double
     */
    double distance_point(const Point& other) const {
        return std::sqrt(std::pow((x - other.x), 2) +
                         std::pow((y - other.y), 2) +
                         std::pow((z - other.z), 2));
    }

    /**
     * @brief XYの2点間の距離を返す(投影座標)
     *
     * @param other
     * @return double
     */
    double distance_point_2d(const Point& other) const {
        return std::sqrt(std::pow((x - other.x), 2) +
                         std::pow((y - other.y), 2));
    }

    /**
     * @brief 値が近いかどうか(投影座標)
     *
     * @param other
     * @return true
     * @return false
     */
    bool is_close(const Point& other) const {
        return (std::fabs(x - other.x) < kMinima &&
                std::fabs(y - other.y) < kMinima &&
                std::fabs(z - other.z) < kMinima);
    }

    /**
     * @brief 地理座標の経度・緯度の桁揃え
     *
     * @param[in,out] lon 経度
     * @param[in,out] lat 緯度
     * @throw std::invalid_argument 引数エラー
     */
    static void alignment_lonlat(double& lon, double& lat) {
        // 経度の範囲チェック
        if (std::fabs(lon) > 180.0) {
            throw std::invalid_argument("INPUT_VALUE_ERROR");
        }

        // 緯度の範囲チェック
        // 小数点11桁以下は切り捨てる
        double p = std::pow(10, 10);
        lat = (lat >= 0) ? (std::floor(lat * p) / p) : (std::ceil(lat * p) / p);
        if (std::fabs(lat) > 85.0511287798) {
            throw std::invalid_argument("INPUT_VALUE_ERROR");
        }
    }

    /**
     * @brief unordered_set用のハッシュ関数
     *
     */
    struct hash_function {
        template <class T>
        static void hash_combine(std::size_t& seed, const T& v) {
            std::hash<T> hasher;
            const std::size_t kMul = 0x9ddfea08eb382d69ULL;
            std::size_t a = (hasher(v) ^ seed) * kMul;
            a ^= (a >> 47);
            std::size_t b = (seed ^ a) * kMul;
            b ^= (b >> 47);
            seed = b * kMul;
        }
        size_t operator()(const Point& point) const {
            size_t h = 0;
            hash_function::hash_combine(h, point.x);
            hash_function::hash_combine(h, point.y);
            hash_function::hash_combine(h, point.z);
            return h;
        }
    };

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
} Point;

}  // namespace spatialid

#endif  // __POINT_H__
