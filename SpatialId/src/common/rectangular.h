/**
 * @file rectangular.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __RECTANGULAR__
#define __RECTANGULAR__

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "common/object/line.h"
#include "common/object/plane.h"
#include "common/object/point.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 始点・終点の２面が正方形の直方体の空間ID
 *
 */
class Rectangular {
   protected:
    Point start_point_;                    // 始点(投影座標)
    Point end_point_;                      // 終点(投影座標)
    double radius_;                        // 半径(単位:m)
    int zoom_;                             // 空間IDの精度レベル
    std::unordered_set<Point, Point::hash_function> all_cross_points_;  // 全交点(投影座標)
    std::unordered_set<std::string> all_spatial_ids_;  // 全空間ID
    double height_;                                    // 図形の長さ
    Vector unit_axis_vector_;  // 軸方向の単位ベクトル
    Vector radius_orth_vector_;  // 軸に対する半径長さの直交ベクトル
    Point contact_point_;        // 直交ベクトルと図形の交点
    Vector radius_normal_vector_;  // 軸方向のベクトルと直交ベクトルの外積

   public:
    /**
     * @brief Construct a new Rectangular object
     *
     * @param start_point 始点
     * @param end_point 終点
     * @param radius 半径(単位:m)
     * @param zoom 空間IDの精度レベル
     */
    Rectangular(const Point& start_point, const Point& end_point, double radius,
                int zoom)
        : start_point_(start_point),
          end_point_(end_point),
          radius_(radius),
          zoom_(zoom),
          height_(0) {}

    /**
     * @brief Destroy the Rectangular object
     *
     */
    virtual ~Rectangular() {}

    /**
     * @brief 空間IDを計算する
     *
     */
    virtual void calc_spatial_ids();

    /**
     * @brief 空間IDリストを取得する
     *
     * @param[out] spatial_id_list 空間IDリスト
     */
    virtual void get_spatial_ids(
        std::unordered_set<std::string>& spatial_id_list);

   protected:
    /**
     * @brief 図形を構成する軸・ベクトルを初期化
     *
     */
    virtual void init_axis();

   private:
    /**
     * @brief 直方体の頂点を取得し、境界面との交点を取得
     *
     */
    void calc_rectangular_apex();

    /**
     * @brief 交点の空間IDを取得
     *
     */
    void calc_cross_spatial_ids();

    /**
     * @brief 境界面との交点取得
     *
     * @param rect_lines 直方体の辺全て
     * @param first_voxel_planes 第一ボクセル境界面
     * @param second_voxel_planes 第二ボクセル境界面
     * @param third_voxel_planes 第三ボクセル境界面
     * @param axis 処理時に基準とするXYZ軸(X=0, Y=1, Z=2)
     */
    void get_voxel_plane_cross_point(
        const std::vector<Line>& rect_lines,
        const std::vector<Plane>& first_voxel_planes,
        const std::vector<Plane>& second_voxel_planes,
        const std::vector<Plane>& third_voxel_planes, int axis);
};

}  // namespace spatialid

#endif  // __RECTANGULAR__
