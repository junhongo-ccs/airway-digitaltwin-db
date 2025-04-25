/**
 * @file spatial_point.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-11
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __SPATIALID_H__
#define __SPATIALID_H__

#include <stdexcept>
#include <vector>

#include "common/const.h"
#include "common/object/enum.h"
#include "common/object/point.h"
#include "common/object/zfxy.h"
#include "common/spatial_voxel.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 座標リストから重複なしの空間IDリストを取得する
 *
 * @param point_list 座標リスト
 * @param[out] spatial_id_list 空間IDリスト(重複なし)
 * @param zoom 空間IDの精度レベル
 * @param crs 座標リストのEPSGコード
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_ids_on_points(const std::vector<Point>& point_list,
                               std::unordered_set<std::string>& spatial_id_list,
                               int zoom, CRS crs = CRS::kWGS84);

/**
 * @brief 重複なしの座標リストから重複なしの空間IDリストを取得する
 *
 * @param point_list 座標リスト(重複なし)
 * @param[out] spatial_id_list 空間IDリスト(重複なし)
 * @param zoom 空間IDの精度レベル
 * @param crs 座標リストのEPSGコード
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_ids_on_points(
    const std::unordered_set<Point, Point::hash_function>& point_list,
    std::unordered_set<std::string>& spatial_id_list, int zoom,
    CRS crs = CRS::kWGS84);

/**
 * @brief 経度、緯度、高さから空間IDを計算する
 *
 * @param lon 経度
 * @param lat 緯度
 * @param alt 高さ
 * @param zoom 空間IDの精度レベル
 * @param is_lon_wrap 経度180の補正有無
 * @return Zfxy 空間ID
 * @throw std::invalid_argument 引数エラー
 */
Zfxy get_spatial_zfxy_on_point(double lon, double lat, double alt, int zoom,
                               bool is_lon_wrap = true);

/**
 * @brief 入力の精度が0-35の範囲内か判定をする
 *
 * @param zoom
 * @return true
 * @return false
 */
inline bool check_zoom(int zoom) { return (0 <= zoom) && (zoom <= 35); }

/**
 * @brief 指定した数値分、移動した場合の空間IDを取得する
 *
 * @param spatial_id 元の位置となる空間ID
 * @param x 経度方向に動かす数値
 * @param y 緯度方向に動かす数値
 * @param f 高さ方向に動かす数値
 * @return std::string 移動した場合の空間ID
 * @throw std::invalid_argument 引数エラー
 */
std::string get_shifting_spatial_id(const std::string& spatial_id, int x, int y,
                                    int f);

}  // namespace spatialid

#endif  // __SPATIALID_H__
