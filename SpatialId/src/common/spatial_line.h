/**
 * @file spatial_line.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __SPATIAL_LINE_H__
#define __SPATIAL_LINE_H__

#include <stdexcept>
#include <string>
#include <vector>

#include "common/object/line.h"
#include "common/object/plane.h"
#include "common/object/projected_point.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 指定範囲(線分)の空間IDを取得する
 *
 * @param start 始点
 * @param end　終点
 * @param[out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_ids_on_line(const ProjectedPoint& start,
                             const ProjectedPoint& end,
                             std::vector<std::string>& spatial_id_list,
                             int zoom);

/**
 * @brief X軸ボクセル境界を取得
 *
 * @param start_x_id 始点ボクセルのX成分ID
 * @param end_x_id 終点ボクセルのX成分ID
 * @param[out] plane_x X軸ボクセル境界面
 * @param zoom 空間IDの精度
 */
void generate_x_voxel_plane(int start_x_id, int end_x_id,
                            std::vector<Plane>& x_plane_list, int zoom);

/**
 * @brief Y軸ボクセル境界を取得
 *
 * @param start_y_id 始点ボクセルのY成分ID
 * @param end_y_id 終点ボクセルのY成分ID
 * @param[out] plane_y Y軸ボクセル境界面
 * @param zoom 空間IDの精度
 */
void generate_y_voxel_plane(int start_y_id, int end_y_id,
                            std::vector<Plane>& y_plane_list, int zoom);

/**
 * @brief Z軸ボクセル境界を取得
 *
 * @param start_z_id 始点ボクセルのZ成分ID
 * @param end_z_id 終点ボクセルのZ成分ID
 * @param[out] plane_z Z軸ボクセル境界面
 * @param zoom 空間IDの精度
 */
void generate_z_voxel_plane(int start_z_id, int end_z_id,
                            std::vector<Plane>& z_plane_list, int zoom);

/**
 * @brief 衝突判定
 *
 * @param plane 衝突判定面
 * @param line 衝突判定線分
 * @param collision_point 衝突点
 * @return true
 * @return false
 */
bool detect_collision(const Plane& plane, const Line& line,
                      Point& collision_point);

}  // namespace spatialid

#endif  // __SPATIAL_LINE_H__
