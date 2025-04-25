/**
 * @file spatial_api.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __SPATIAL_API_H__
#define __SPATIAL_API_H__

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/const.h"
#include "common/object/enum.h"
#include "common/object/point.h"
#include "common/object/triangle.h"
#include "common/spatial_voxel.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief 座標からグローバル空間IDを取得する
 *
 * @param point 座標
 * @param crs 座標のEPSGコード
 * @return std::string グローバル空間ID
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 空間IDの形式:z{精度レベル}/f{高さの位置}/x{経度方向の位置}/y{緯度方向の位置}
 */
std::string get_global_spatial_id_on_point(const Point& point,
                                           CRS crs = CRS::kWGS84);

/**
 * @brief 座標から空間IDを取得する
 *
 * @param x 経度(またはX座標)
 * @param y 緯度(またはY座標)
 * @param z 高さ
 * @param zoom 空間IDの精度レベル
 * @param crs 座標リストのEPSGコード
 * @return std::string 空間ID
 * @throw std::invalid_argument 引数エラー
 */
std::string get_spatial_id_on_point(double x, double y, double z, int zoom,
                                    CRS crs = CRS::kWGS84);

/**
 * @brief 座標リストからグローバル空間IDリストを取得する
 *
 * @param point_list 座標リスト
 * @param[out] spatial_id_list グローバル空間IDリスト
 * @param crs 座標のEPSGコード
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 空間IDの形式:z{精度レベル}/f{高さの位置}/x{経度方向の位置}/y{緯度方向の位置}
 */
void get_global_spatial_ids_on_points(const std::vector<Point>& point_list,
                                      std::vector<std::string>& spatial_id_list,
                                      CRS crs = CRS::kWGS84);

/**
 * @brief 座標リストから座標リストの順で空間IDリストを取得する
 *
 * @param point_list 座標リスト
 * @param[out] spatial_id_list 空間IDリスト(座標リストの順)
 * @param zoom 空間IDの精度レベル
 * @param crs 座標リストのEPSGコード
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_ids_on_points(const std::vector<Point>& point_list,
                               std::vector<std::string>& spatial_id_list,
                               int zoom, CRS crs = CRS::kWGS84);

/**
 * @brief 座標リストからグローバル空間ID単位のボクセルデータを取得する
 *
 * @param point_list 座標リスト
 * @param[out] voxel_list ボクセルデータが格納された連想配列
 * @param crs 座標リストのEPSGコード
 * @throw std::invalid_argument 引数エラー
 * @remark
 * ボクセルデータが不要になった場合、連想配列のclearメソッドで呼び出して
 * リソースを解放してください。
 * (ただし、第二引数の変数がスコープから外れた場合も自動的にリソースは解放されます。)
 */
void get_global_spatial_voxel_on_points(
    const std::vector<Point>& point_list,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    CRS crs = CRS::kWGS84);

/**
 * @brief 座標リストから空間ID単位のボクセルデータを取得する
 *
 * @param point_list 座標リスト
 * @param[out] voxel_list ボクセルデータ
 * @param global_zoom グローバル空間IDの精度レベル
 * @param local_zoom ローカル空間IDの精度レベル
 * @param crs 座標リストのEPSGコード
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_voxel_on_points(
    const std::vector<Point>& point_list,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs = CRS::kWGS84);

/**
 * @brief 空間IDの頂点座標を取得する
 *
 * @param spatial_id 空間ID
 * @param[out] point_list 頂点座標リスト
 * @param crs 座標のEPSGコード
 * @throw std::invalid_argument 引数エラー
 */
void get_vertex_points_on_spatial_id(const std::string& spatial_id,
                                     std::vector<Point>& point_list,
                                     CRS crs = CRS::kWGS84);

/**
 * @brief 空間IDの中心座標を取得する
 *
 * @param spatial_id 空間ID
 * @param crs 座標のEPSGコード
 * @return Point 中心座標
 * @throw std::invalid_argument 引数エラー
 */
Point get_center_point_on_spatial_id(const std::string& spatial_id,
                                     CRS crs = CRS::kWGS84);

/**
 * @brief 空間IDの面に直接、接している6個の空間IDを取得する
 *
 * @param spatial_id 空間ID
 * @param[out] spatial_id_list 空間IDリスト
 * @throw std::invalid_argument 引数エラー
 */
void get_6spatial_ids_adjacent_to_faces(
    const std::string& spatial_id, std::vector<std::string>& spatial_id_list);

/**
 * @brief 空間IDの水平方向の一周分の8個の空間IDを取得する
 *
 * @param spatial_id 空間ID
 * @param[out] spatial_id_list 空間IDリスト
 * @throw std::invalid_argument 引数エラー
 */
void get_8spatial_ids_around_horizontal(
    const std::string& spatial_id, std::vector<std::string>& spatial_id_list);

/**
 * @brief 空間IDを囲う26個の空間IDを取得する
 *
 * @param spatial_id 空間ID
 * @param[out] spatial_id_list 空間IDリスト
 * @throw std::invalid_argument 引数エラー
 */
void get_26spatial_ids_around_voxel(const std::string& spatial_id,
                                    std::vector<std::string>& spatial_id_list);

/**
 * @brief
 * 三角ポリゴンの集合で表されるモデルが含まれるグローバル空間IDを取得する
 *
 * @param barrier_triangles 空間IDを取得する三角ポリゴンの集合で表されるモデル
 * @param space_triangles 除外する三角ポリゴンの集合で表されるモデル
 * @param[out] spatial_id_list 空間IDリスト
 * @param crs 座標のEPSGコード
 * @param needs_closed_checking 閉塞チェックフラグ
 * @param include_inner_voxel 内部ボクセルを含む
 * @remark
 * 三角ポリゴンが接しない内部の空間IDも合わせて取得する。
 * 三角ポリゴンに接しない内部のボクセルの内外判定は、Z方向に行う。
 *
 * 三角ポリゴンの集合で表されるモデルが閉塞していない場合でも、内外判定は行われるが、
 * Z軸上方向に三角ポリゴンがない場合は外部として判定する。
 * 外部として判定された箇所の空間IDは返却される空間IDには含まれない。
 *
 * 内部に空間IDとして取得しない空間がある場合は、
 * 除外する三角ポリゴンの集合で表されるモデルとして定義する。
 *
 * 三角ポリゴンの集合で表されるモデルの空間IDを取得後、
 * 除外する三角ポリゴンの集合で表されるモデルの空間IDを除外する。
 */
void get_global_spatial_ids_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::unordered_set<std::string>& spatial_id_list, CRS crs = CRS::kWGS84,
    bool needs_closed_checking = true, bool include_inner_voxel = true);

/**
 * @brief 三角ポリゴンの集合で表されるモデルが含まれる空間IDを取得する
 *
 * @param barrier_triangles 空間IDを取得する三角ポリゴンの集合で表されるモデル
 * @param space_triangles 除外する三角ポリゴンの集合で表されるモデル
 * @param[out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 * @param crs 座標のEPSGコード
 * @param needs_closed_checking 閉塞チェックフラグ
 * @param include_inner_voxel 内部ボクセルを含む
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_ids_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::unordered_set<std::string>& spatial_id_list, int zoom, CRS crs,
    bool needs_closed_checking, bool include_inner_voxel);

/**
 * @brief
 * 三角ポリゴンの集合で表されるモデルが含まれるグローバル空間ID単位のボクセルデータを取得する
 *
 * @param barrier_triangles 空間IDを取得する三角ポリゴンの集合で表されるモデル
 * @param space_triangles 除外する三角ポリゴンの集合で表されるモデル
 * @param voxel_list ボクセルデータが格納された連想配列
 * @param crs 座標のEPSGコード
 * @param needs_closed_checking 閉塞チェックフラグ
 * @param include_inner_voxel 内部ボクセルを含む
 * @throw std::invalid_argument 引数エラー
 * @remark
 * ボクセルデータが不要になった場合、連想配列のclearメソッドで呼び出して
 * リソースを解放してください。
 * (ただし、第二引数の変数がスコープから外れた場合も自動的にリソースは解放されます。)
 */
void get_global_spatial_voxel_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    CRS crs = CRS::kWGS84, bool needs_closed_checking = true,
    bool include_inner_voxel = true);

/**
 * @brief
 * 三角ポリゴンの集合で表されるモデルが含まれる空間ID単位のボクセルデータを取得する
 *
 * @param barrier_triangles
 * 空間IDを取得する三角ポリゴンの集合で表されるモデル
 * @param space_triangles 除外する三角ポリゴンの集合で表されるモデル
 * @param voxel_list ボクセルデータ
 * @param global_zoom グローバル空間IDの精度レベル
 * @param local_zoom ローカル空間IDの精度レベル
 * @param crs 座標のEPSGコード
 * @param needs_closed_checking 閉塞チェックフラグ
 * @param include_inner_voxel 内部ボクセルを含む
 * @throw std::invalid_argument 引数エラー
 */
void get_spatial_voxel_on_polygons(
    const std::vector<Triangle>& barrier_triangles,
    const std::vector<Triangle>& space_triangles,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs = CRS::kWGS84,
    bool needs_closed_checking = true, bool include_inner_voxel = true);

/**
 * @brief 円柱を複数つなげた経路を通るグローバル空間IDを取得する
 *
 * @param point_list 中心座標リスト
 * @param radius 円柱の半径(単位:m)
 * @param[out] spatial_id_list 空間IDリスト
 * @param crs 座標のEPSGコード
 * @param is_capsule 始点、終点が球状であるかを示す(true:カプセル false:円柱)
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 円柱間の接続面は球状とする。
 * ドローンの経路や地中埋設配管が通る経路を空間IDで表現する際に使用する。
 */
void get_global_spatial_ids_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::unordered_set<std::string>& spatial_id_list, CRS crs = CRS::kWGS84,
    bool is_capsule = false);

/**
 * @brief 円柱を複数つなげた経路を通る空間IDを取得する
 *
 * @param point_list
 * 円柱の中心の接続点の座標リスト(2つ目の接続点は1つ目の円柱の終点となるが、2つ目の円柱の始点にもなる)
 * @param radius 円柱の半径(単位:m)
 * @param[out] spatial_id_list 空間IDリスト
 * @param zoom 空間IDの精度レベル
 * @param crs 座標のEPSGコード
 * @param is_capsule 始点、終点が球状であるかを示す(true:カプセル false:円柱)
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 円柱間の接続面は球状とする。
 * ドローンの経路や地中埋設配管が通る経路を空間IDで表現する際に使用する。
 */
void get_spatial_ids_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::unordered_set<std::string>& spatial_id_list, int zoom,
    CRS crs = CRS::kWGS84, bool is_capsule = false);

/**
 * @brief
 * 円柱を複数つなげた経路を通るグローバル空間ID単位のボクセルデータを取得する
 *
 * @param point_list 中心座標リスト
 * @param radius 円柱の半径(単位:m)
 * @param voxel_list ボクセルデータが格納された連想配列
 * @param crs 座標のEPSGコード
 * @param is_capsule 始点、終点が球状であるかを示す(true:カプセル false:円柱)
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 円柱間の接続面は球状とする。
 * ドローンの経路や地中埋設配管が通る経路を空間IDで表現する際に使用する。
 * @remark
 * ボクセルデータが不要になった場合、連想配列のclearメソッドで呼び出して
 * リソースを解放してください。
 * (ただし、第二引数の変数がスコープから外れた場合も自動的にリソースは解放されます。)
 */
void get_global_spatial_voxel_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    CRS crs = CRS::kWGS84, bool is_capsule = false);

/**
 * @brief
 * 円柱を複数つなげた経路を通る空間ID単位のボクセルデータを取得する
 *
 * @param point_list 中心座標リスト
 * @param radius 円柱の半径(単位:m)
 * @param[out] spatial_id_list 空間IDリスト
 * @param global_zoom グローバル空間IDの精度レベル
 * @param local_zoom ローカル空間IDの精度レベル
 * @param crs 座標のEPSGコード
 * @param is_capsule 始点、終点が球状であるかを示す(true:カプセル false:円柱)
 * @throw std::invalid_argument 引数エラー
 * @remark
 * 円柱間の接続面は球状とする。
 * ドローンの経路や地中埋設配管が通る経路を空間IDで表現する際に使用する。
 */
void get_spatial_voxel_on_cylinders(
    const std::vector<Point>& point_list, double radius,
    std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list,
    int global_zoom, int local_zoom, CRS crs = CRS::kWGS84,
    bool is_capsule = false);

/**
 * @brief
 * ボクセルデータを全てメモリ上で管理するかどうかのオプション
 *
 * @param enabled true:メモリ管理 false:SQLite管理
 * @remark
 * ボクセルデータ取得API等でメモリ不足になると、std::bad_allocの例外がスローされます。
 */
void set_option_voxel_in_memory(bool enabled);

}  // namespace spatialid

#endif  // __SPATIAL_API_H__
