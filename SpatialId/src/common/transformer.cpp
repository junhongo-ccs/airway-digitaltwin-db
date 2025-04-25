/**
 * @file transformer.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-05
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "transformer.h"

#include "common/proj_context.h"

namespace spatialid {

std::mutex Transformer::mutex_;

// インスタンスを取得する
Transformer *Transformer::GetInstance(CRS from_crs, CRS to_crs) {
    std::lock_guard<std::mutex> lock(mutex_);

    ProjContext &proj_ctx = ProjContext::GetInstance();
    return proj_ctx.GetTransformer(from_crs, to_crs);
}

// コンストラクタ
Transformer::Transformer(CRS from_crs, CRS to_crs)
    : pj_(nullptr),
      source_crs_(from_crs),
      target_crs_(to_crs),
      source_crs_type_(PJ_TYPE_UNKNOWN),
      target_crs_type_(PJ_TYPE_UNKNOWN) {
    ProjContext &proj_ctx = ProjContext::GetInstance();
    PJ_CONTEXT* pj_ctx = proj_ctx.GetContext();

    std::string from_epsg = "EPSG:";
    from_epsg.append(std::to_string(static_cast<int>(from_crs)));
    std::string to_epsg = "EPSG:";
    to_epsg.append(std::to_string(static_cast<int>(to_crs)));

    PJ *pj_tmp = proj_create(pj_ctx, from_epsg.c_str());
    if (!pj_tmp) {
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }
    source_crs_type_ = proj_get_type(pj_tmp);
    proj_destroy(pj_tmp);

    pj_tmp = proj_create(pj_ctx, to_epsg.c_str());
    if (!pj_tmp) {
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }
    target_crs_type_ = proj_get_type(pj_tmp);
    proj_destroy(pj_tmp);

    pj_ = proj_create_crs_to_crs(pj_ctx, from_epsg.c_str(), to_epsg.c_str(),
                                 nullptr);
    if (!pj_) {
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }

    pj_tmp = proj_normalize_for_visualization(pj_ctx, pj_);
    if (!pj_tmp) {
        proj_destroy(pj_);
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }
    proj_destroy(pj_);
    pj_ = pj_tmp;
}

// デストラクタ
Transformer::~Transformer() {
    if (pj_) {
        proj_destroy(pj_);
    }
}

// 指定したCRSが投影座標かどうか
bool Transformer::IsProjected(CRS crs) {
    std::lock_guard<std::mutex> lock(mutex_);

    ProjContext &proj_ctx = ProjContext::GetInstance();

    std::string epsg = "EPSG:";
    epsg.append(std::to_string(static_cast<int>(crs)));

    PJ *pj_tmp = proj_create(proj_ctx.GetContext(), epsg.c_str());
    if (!pj_tmp) {
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }
    PJ_TYPE crs_type = proj_get_type(pj_tmp);
    proj_destroy(pj_tmp);

    return IsProjectedPjType(crs_type);
}

// 指定したCRSが地理座標かどうか
bool Transformer::IsGeographic(CRS crs) {
    std::lock_guard<std::mutex> lock(mutex_);

    ProjContext &proj_ctx = ProjContext::GetInstance();

    std::string epsg = "EPSG:";
    epsg.append(std::to_string(static_cast<int>(crs)));

    PJ *pj_tmp = proj_create(proj_ctx.GetContext(), epsg.c_str());
    if (!pj_tmp) {
        throw std::invalid_argument("VALUE_CONVERT_ERROR");
    }
    PJ_TYPE crs_type = proj_get_type(pj_tmp);
    proj_destroy(pj_tmp);

    return IsGeographicPjType(crs_type);
}

// 座標を変換する
void Transformer::Transform(double from_x, double from_y, double from_z,
                            double &to_x, double &to_y, double &to_z) {
    PJ_COORD src_coord = proj_coord(from_x, from_y, from_z, 0);
    PJ_COORD dst_coord = proj_trans(pj_, PJ_FWD, src_coord);
    to_x = dst_coord.xyz.x;
    to_y = dst_coord.xyz.y;
    to_z = dst_coord.xyz.z;
}

}  // namespace spatialid
