/**
 * @file transformer.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __TRANSFORMER_H__
#define __TRANSFORMER_H__

#include <proj.h>

#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

#include "common/object/enum.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 座標変換クラス
 *
 */
class Transformer {
   private:
    static std::mutex mutex_;

    PJ *pj_;
    CRS source_crs_;
    CRS target_crs_;
    PJ_TYPE source_crs_type_;
    PJ_TYPE target_crs_type_;

   public:
    Transformer() = delete;
    Transformer(const Transformer &) = delete;
    Transformer(Transformer &&) = delete;
    Transformer &operator=(const Transformer &) = delete;
    Transformer &operator=(Transformer &&) = delete;

    /**
     * @brief Construct a new Transformer object
     *
     * @param from_crs
     * @param to_crs
     * @throw std::invalid_argument 引数エラー
     */
    Transformer(CRS from_crs, CRS to_crs);

    /**
     * @brief Destroy the Transformer object
     *
     */
    virtual ~Transformer();

    /**
     * @brief Get the Instance object
     *
     * @param from_crs
     * @param to_crs
     * @return Transformer*
     * @throw std::invalid_argument 引数エラー
     */
    static Transformer *GetInstance(CRS from_crs, CRS to_crs);

    /**
     * @brief 指定したCRSが投影座標かどうか
     *
     * @param crs
     * @return true
     * @return false
     * @throw std::invalid_argument 引数エラー
     */
    static bool IsProjected(CRS crs);

    /**
     * @brief 指定したCRSが地理座標かどうか
     *
     * @param crs
     * @return true
     * @return false
     * @throw std::invalid_argument 引数エラー
     */
    static bool IsGeographic(CRS crs);

    /**
     * @brief 座標を変換する
     *
     * @param from_x
     * @param from_y
     * @param from_z
     * @param to_x
     * @param to_y
     * @param to_z
     */
    void Transform(double from_x, double from_y, double from_z, double &to_x,
                   double &to_y, double &to_z);

    /**
     * @brief 変換前のCRSが投影座標かどうか
     *
     * @return true
     * @return false
     */
    bool IsSourceProjected() const {
        return IsProjectedPjType(source_crs_type_);
    }

    /**
     * @brief 変換後のCRSが投影座標かどうか
     *
     * @return true
     * @return false
     */
    bool IsTargetProjected() const {
        return IsProjectedPjType(target_crs_type_);
    }

    /**
     * @brief 変換前のCRSが地理座標かどうか
     *
     * @return true
     * @return false
     */
    bool IsSourceGeographic() const {
        return IsGeographicPjType(source_crs_type_);
    }

    /**
     * @brief 変換後のCRSが地理座標かどうか
     *
     * @return true
     * @return false
     */
    bool IsTargetGeographic() const {
        return IsGeographicPjType(target_crs_type_);
    }

   private:
    /**
     * @brief 指定のCRSが投影座標かどうか
     *
     * @param pj_type
     * @return true
     * @return false
     */
    static bool IsProjectedPjType(PJ_TYPE pj_type) {
        return (pj_type == PJ_TYPE_PROJECTED_CRS);
    }

    /**
     * @brief 指定のCRSが地理座標かどうか
     *
     * @param pj_type
     * @return true
     * @return false
     */
    static bool IsGeographicPjType(PJ_TYPE pj_type) {
        if (pj_type == PJ_TYPE_GEOGRAPHIC_CRS ||
            pj_type == PJ_TYPE_GEOGRAPHIC_2D_CRS ||
            pj_type == PJ_TYPE_GEOGRAPHIC_3D_CRS) {
            return true;
        }
        return false;
    }
};

}  // namespace spatialid

#endif  // __TRANSFORMER_H__
