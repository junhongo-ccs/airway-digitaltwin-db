/**
 * @file projected_point.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __PROJECTED_POINT_H__
#define __PROJECTED_POINT_H__

#include <stdexcept>

#include "common/object/point.h"

namespace spatialid {

/**
 * @brief 投影座標
 *
 */
typedef struct ProjectedPoint {
    double lon;  // 地理座標 経度
    double lat;  // 地理座標 緯度
    double alt;  // 地理座標 高さ
    double x;    // 投影座標 X座標
    double y;    // 投影座標 Y座標

    /**
     * @brief Construct a new Projected Point object
     *
     * @param a_lon 地理座標 経度
     * @param a_lat 地理座標 緯度
     * @param a_alt 地理座標 高さ
     * @param xx 投影座標 X座標
     * @param yy 投影座標 Y座標
     * @throw std::invalid_argument 引数エラー
     */
    ProjectedPoint(double a_lon = 0, double a_lat = 0, double a_alt = 0,
                   double xx = 0, double yy = 0)
        : lon(a_lon), lat(a_lat), alt(a_alt), x{xx}, y{yy} {
        // 地理座標の経度・緯度の桁揃え
        Point::alignment_lonlat(lon, lat);
    }
} ProjectedPoint;

}  // namespace spatialid

#endif  // __PROJECTED_POINT_H__
