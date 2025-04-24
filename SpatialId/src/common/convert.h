/**
 * @file convert.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __CONVERT_H__
#define __CONVERT_H__

#define _USE_MATH_DEFINES
#include <math.h>

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/object/point.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 文字列を区切り文字で分割する
 *
 * @param str 文字列
 * @param delimiter 区切り文字
 * @return std::vector<std::string>
 */
std::vector<std::string> split_string(const std::string& str, char delimiter);

/**
 * @brief 文字列を区切り文字で分割し、指定した番号の文字列を返す
 *
 * @param str 文字列
 * @param delimiter 区切り文字
 * @return std::string
 */
std::string split_string_get(const std::string& str, char delimiter, int index);

/**
 * @brief 度をラジアンに変換する
 *
 * @param deg 度
 * @return double ラジアン
 */
inline double deg_to_rad(double deg) { return ((deg / 360.0) * 2 * M_PI); }

/**
 * @brief ラジアンを度に変換する
 *
 * @param rad ラジアン
 * @return double 度
 */
inline double rad_to_deg(double rad) { return (((rad) / 2 / M_PI) * 360.0); }

/**
 * @brief 値を最小値と最大値の範囲に制限する
 * 
 * @param n 値
 * @param lower 最小値
 * @param upper 最大値
 * @return double 制限した値
 */
inline double clip(double n, double lower, double upper) {
    return std::max(lower, std::min(n, upper));
}

/**
 * @brief 値 a と b が近い場合、true を返す
 *
 * @param a 値
 * @param b 値
 * @param rel_tol 相対交差
 * @param abs_tol 絶対交差
 * @return true
 * @return false
 */
inline bool is_close(double a, double b, double rel_tol = 1e-9,
                    double abs_tol = 0.0) {
    return std::fabs(a - b) <=
           std::max(rel_tol * std::max(std::fabs(a), std::fabs(b)), abs_tol);
}

}  // namespace spatialid

#endif  // __CONVERT_H__
