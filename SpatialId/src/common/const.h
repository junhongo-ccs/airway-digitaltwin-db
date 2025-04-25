/**
 * @file const.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-03
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __CONST_H__
#define __CONST_H__

#include "common/object/enum.h"

namespace spatialid {

/**
 * @brief 経度方向の境界bits数(8の倍数)
 *
 */
const int kBoundaryBits = sizeof(uint64_t) * 8;

/**
 * @brief グローバル空間IDの精度レベル
 *
 */
const int kGlobalZoom = 17;

/**
 * @brief ローカル空間IDの精度レベル
 *
 */
const int kLocalZoom = 28;

/**
 * @brief 浮動小数点誤差
 *
 */
const double kMinima = 1e-10;

/**
 * @brief 辺浮動小数点誤差
 *
 */
const double kEdgeMinima = 1e-8;

}  // namespace spatialid

#endif  // __CONST_H__
