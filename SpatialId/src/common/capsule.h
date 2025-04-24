/**
 * @file capsule.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __CAPSULE_H__
#define __CAPSULE_H__

#include <memory>
#include <unordered_set>
#include <stdexcept>
#include <string>

#include "common/object/line.h"
#include "common/object/plane.h"
#include "common/object/point.h"
#include "common/rectangular.h"
#include "common/shape/base_bullet.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief カプセル・円柱・球の空間ID
 *
 */
class Capsule : public Rectangular {
   protected:
    bool is_sphere_;   // true:球 false:カプセル or 円柱
    bool is_capsule_;  // true:カプセル false:円柱
    std::unordered_set<std::string> include_spatial_ids_;  // 内部判定空間ID
    std::unique_ptr<BaseBullet> bullet_;                   // Bullet
    int approximate_voxel_num_;  // 概算ボクセル数（概算距離が機体半径内のボクセル数）

   public:
    /**
     * @brief Construct a new Capsule object
     *
     * @param engine 衝突判定エンジン
     * @param start_point 円柱・カプセル・球の始点(投影座標)
     * @param end_point 円柱・カプセル・球の終点(投影座標)
     * @param radius 円柱の半径(単位:m)
     * @param zoom 空間IDの精度レベル
     * @param is_capsule true:カプセル false:円柱
     */
    Capsule(BulletEngine* engine, const Point& start_point,
            const Point& end_point, double radius, int zoom, bool is_capsule);

    /**
     * @brief Destroy the Capsule object
     *
     */
    virtual ~Capsule() {}

    /**
     * @brief
     * 球の空間IDから中心から終点方向へ半径分延ばした立方体分の空間IDを削る
     *
     * @param direction_point 終点方向
     */
    void shave_sphere(const Point& direction_point);

    /**
     * @brief 空間IDを計算する
     *
     */
    void calc_spatial_ids() override;

    /**
     * @brief 空間IDリストを取得する
     *
     * @param[out] spatial_id_list 空間IDリスト
     */
    void get_spatial_ids(
        std::unordered_set<std::string>& spatial_id_list) override;

   protected:
    /**
     * @brief 図形を構成する軸・ベクトルを初期化
     *
     */
    void init_axis() override;

   private:
    /**
     * @brief 直方体の空間IDから実際の図形分の有効な空間IDを取得
     *
     */
    void calc_valid_spatial_ids();

    /**
     * @brief 概算距離内の空間ID取得
     *
     * @param spatial_id 概算距離の中心とする空間ID
     * @param[out] approximate_distance_radius_spatial_ids 概算距離内の空間ID
     */
    void approximate_distance_radius_points(
        const std::string& spatial_id,
        std::unordered_set<std::string>&
            approximate_distance_radius_spatial_ids);

    /**
     * @brief 概算距離用の単位ボクセルベクトルを算出
     *
     * @return Vector 単位ボクセルベクトル
     */
    Vector calc_unit_voxel_vector();
};

}  // namespace spatialid

#endif  //__CAPSULE_H__
