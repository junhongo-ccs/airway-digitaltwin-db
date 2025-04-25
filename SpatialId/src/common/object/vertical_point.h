/**
 * @file vertical_point.h
 * @author 
 * @brief 
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 * 
 */

#ifndef __VERTICAL_POINT_H__
#define __VERTICAL_POINT_H__

#include <stdexcept>

namespace spatialid {

/**
 * @brief 高さ座標
 *
 */
typedef struct VerticalPoint {
    double z;           // 高さ
    double resolution;  // 分解能

    /**
     * @brief Construct a new Vertical Point object
     * 
     * @param zz 高さ
     * @param rr 分解能
     */
    VerticalPoint(double zz = 0, double rr = 0) : z{zz}, resolution{rr} {}
} VerticalPoint;

}  // namespace spatialid

#endif  // __VERTICAL_POINT_H__
