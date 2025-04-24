/**
 * @file spatial_cylinders.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "spatial_cylinders.h"

#include <iterator>
#include <memory>
#include <string>
#include <unordered_set>

#include "common/capsule.h"
#include "common/spatial_point.h"
#include "common/transformer.h"
#include "spatial_api.h"

namespace spatialid {

// Prototype Declare
void convert_projected_point(const Point& point, Point& projected_point,
                             CRS crs);

// [API] 円柱を複数つなげた経路を通るグローバル空間IDを取得する
void get_global_spatial_ids_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::unordered_set<std::string>& spatial_id_list, CRS crs,
    bool is_capsule) {
    get_spatial_ids_on_cylinders(point_list, radius, spatial_id_list,
                                 kGlobalZoom, crs, is_capsule);
}

// [API]
// 円柱を複数つなげた経路を通るグローバル空間ID単位のボクセルデータを取得する
void get_global_spatial_voxel_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list, CRS crs,
    bool is_capsule) {
    get_spatial_voxel_on_cylinders(point_list, radius, voxel_list, kGlobalZoom,
                                   kLocalZoom, crs, is_capsule);
}

// [API] 円柱を複数つなげた経路を通る空間IDを取得する
void get_spatial_ids_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs,
    bool is_capsule) {
    spatial_id_list.clear();

    // 半径が0以下の場合
    if (radius <= kMinima) {
        LOG_DEBUG("半径が0以下");
        std::invalid_argument("INPUT_VALUE_ERROR");
    }

    // 接続点数が0個の場合
    if (point_list.size() == 0) {
        LOG_DEBUG("接続点数が0個");
        return;
    }

    // 衝突判定エンジンのインスタンス生成
    std::unique_ptr<BulletEngine> bulle_engine(new BulletEngine());

    // 接続点の球
    std::unique_ptr<Capsule> sphere;
    // 接続点数
    int connect_point_num = 1;

    size_t len = (point_list.size() - 1) - 1;
    size_t index = 0;
    for (std::vector<Point>::const_iterator it = point_list.begin();
         it != point_list.end(); index++) {
        // 始点終点(入力座標)
        const Point& start = *it;
        if (++it == point_list.end()) {
            break;
        }
        const Point& end = *it;

        LOG_DEBUG("接続点=%d〜%d", connect_point_num, connect_point_num + 1);
        LOG_DEBUG_POINT(start.x, start.y, start.z, "始点(入力座標)=");
        LOG_DEBUG_POINT(end.x, end.y, end.z, "終点(入力座標)=");

        // 同じ座標が連続する場合は次の座標へスキップ
        if (start == end) {
            continue;
        }

        // 接続点数をインクリメント
        connect_point_num += 1;

        // 始点終点(投影座標)
        Point start_orth_point;
        Point end_orth_point;
        convert_projected_point(start, start_orth_point, crs);
        convert_projected_point(end, end_orth_point, crs);

        LOG_DEBUG_POINT(start_orth_point.x, start_orth_point.y,
                        start_orth_point.z, "始点(投影座標)=");
        LOG_DEBUG_POINT(end_orth_point.x, end_orth_point.y, end_orth_point.z,
                        "終点(投影座標)=");

        // 始点終点の長さ
        double height =
            Vector::from_points(start_orth_point, end_orth_point).norm();

        if (sphere) {
            // 円柱の接続点で後方の円柱の長さが半径より短い場合
            if (height < radius) {
                // 接続点の球の空間IDより円柱の始点から終点方向へ半径距離延ばした直方体の空間IDを除外する
                sphere->shave_sphere(end_orth_point);
            }

            std::unordered_set<std::string> shave_sphere_spatial_ids;
            sphere->get_spatial_ids(shave_sphere_spatial_ids);
            std::copy(shave_sphere_spatial_ids.begin(),
                      shave_sphere_spatial_ids.end(),
                      std::inserter(spatial_id_list, spatial_id_list.end()));

            LOG_DEBUG_UNORDERED_SET_STRING(shave_sphere_spatial_ids,
                                           "接続点の空間ID=");
        }

        // 始点終点からカプセルの空間ID取得
        std::unique_ptr<Capsule> capsule(
            new Capsule(bulle_engine.get(), start_orth_point, end_orth_point,
                        radius, zoom, is_capsule));
        capsule->calc_spatial_ids();
        std::unordered_set<std::string> capsule_spatial_ids;
        capsule->get_spatial_ids(capsule_spatial_ids);
        std::copy(capsule_spatial_ids.begin(), capsule_spatial_ids.end(),
                  std::inserter(spatial_id_list, spatial_id_list.end()));

        LOG_DEBUG_UNORDERED_SET_STRING(capsule_spatial_ids,
                                       "接続点間の空間ID=");

        // 終点もしくはカプセルの場合は接続点の空間IDは取得しない
        if (index == len || is_capsule) {
            continue;
        }

        // 接続点の球の空間ID取得
        sphere.reset(new Capsule(bulle_engine.get(), end_orth_point,
                                 end_orth_point, radius, zoom, is_capsule));
        sphere->calc_spatial_ids();

        // 円柱の接続点で前方の円柱の長さが半径より短い場合
        if (height < radius) {
            // 接続点の球の空間IDより円柱の終点から始点方向へ半径距離延ばした直方体の空間IDを除外する
            sphere->shave_sphere(start_orth_point);
        }
    }

    // 接続点数が1の場合
    if (connect_point_num == 1) {
        LOG_DEBUG("接続点数が1個");

        // 中心座標(投影座標)
        Point center_orth_point;
        convert_projected_point(point_list[0], center_orth_point, crs);

        LOG_DEBUG_POINT(center_orth_point.x, center_orth_point.y,
                        center_orth_point.z, "中心座標(投影座標)=");

        // 接続点の球の空間ID取得
        sphere.reset(new Capsule(bulle_engine.get(), center_orth_point,
                                 center_orth_point, radius, zoom, is_capsule));
        sphere->calc_spatial_ids();
        sphere->get_spatial_ids(spatial_id_list);

        LOG_DEBUG_UNORDERED_SET_STRING(spatial_id_list, "球の空間ID=");
    }
}

// [API] 円柱を複数つなげた経路を通る空間ID単位のボクセルデータを取得する
void get_spatial_voxel_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs, bool is_capsule) {
    if (global_zoom > local_zoom) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    voxel_list.clear();

    // ローカル空間IDの精度レベルで表現した場合の空間IDを取得
    std::unordered_set<std::string> local_zoom_spatial_ids;
    get_spatial_ids_on_cylinders(point_list, radius, local_zoom_spatial_ids,
                                 local_zoom, crs, is_capsule);

    // グローバル空間IDに対するローカル空間IDの精度レベルで表現した空間ID一覧を作成
    std::map<std::string, std::unordered_set<Zfxy, Zfxy::hash_function>>
        spatial_id_map;
    for (const auto& local_zoom_spatial_id : local_zoom_spatial_ids) {
        // 中心座標を取得
        Point wgs84_point =
            get_center_point_on_spatial_id(local_zoom_spatial_id, CRS::kWGS84);

        // 中心座標のグローバル空間IDを取得
        std::string spatial_id =
            get_spatial_id_on_point(wgs84_point.x, wgs84_point.y, wgs84_point.z,
                                    global_zoom, CRS::kWGS84);

        if (auto itr = spatial_id_map.find(spatial_id);
            itr != spatial_id_map.end()) {
            (itr->second).insert(Zfxy(local_zoom_spatial_id));
        } else {
            spatial_id_map[spatial_id] =
                std::unordered_set<Zfxy, Zfxy::hash_function>{
                    Zfxy(local_zoom_spatial_id)};
        }
    }

    // ボクセルデータの作成
    SpatialVoxel::Create(spatial_id_map, local_zoom, voxel_list);
}

/**
 * @brief 座標を投影座標に変換する
 *
 * @param point 座標
 * @param[out] projected_point 投影座標の座標
 * @param crs 座標のEPSGコード
 */
void convert_projected_point(const Point& point, Point& projected_point,
                             CRS crs) {
    Point wgs84_point = point;

    // 入力座標から地理座標に設定
    Transformer* transformer = nullptr;
    if (crs != CRS::kWGS84) {
        transformer = Transformer::GetInstance(crs, CRS::kWGS84);
        transformer->Transform(point.x, point.y, point.z, wgs84_point.x,
                               wgs84_point.y, wgs84_point.z);
    }

    // 地理座標の経度・緯度の桁揃え
    Point::alignment_lonlat(wgs84_point.x, wgs84_point.y);

    // 投影座標に座標変換
    transformer = Transformer::GetInstance(CRS::kWGS84, CRS::kWebMercator);
    transformer->Transform(wgs84_point.x, wgs84_point.y, wgs84_point.z,
                           projected_point.x, projected_point.y,
                           projected_point.z);
}

}  // namespace spatialid
