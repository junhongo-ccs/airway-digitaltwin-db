/**
 * @file spatial_voxel.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-09
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __SPATIAL_VOXEL_H__
#define __SPATIAL_VOXEL_H__

#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "common/object/point.h"
#include "common/object/zfxy.h"
#include "logging/logger.h"
#include "sqlite/spatial_sqlite.h"

namespace spatialid {

/**
 * @brief 空間ボクセル
 *
 */
class SpatialVoxel {
   private:
    static std::mutex mutex_;
    static int group_id_;
    static bool option_use_in_memory;

    std::string spatial_id_;   // 空間ID
    Zfxy local_spatial_zfxy_;  // ローカル空間ID(北西下)
    int lon_index_size_;  // ローカル空間IDに分割した際の経度方向数
    int lat_index_size_;  // ローカル空間IDに分割した際の緯度方向数
    int alt_index_size_;  // ローカル空間IDに分割した際の高さ数
    int lon_index_byte_;  // 経度方向のデータ保持に必要なバイト数
    int plane_size_;  // 1平面（経度x緯度）のボクセルデータサイズ[bytes]
    uint64_t voxel_size_;  // ボクセルデータサイズ(bytes)

    // SQLite管理
    SpatialSqlite sqlite_;         // 空間ID用SQLiteインスタンス
    std::string sqlite_filename_;  // 空間ID用SQLiteファイル名
    std::unique_ptr<unsigned char[]> tmp_buf_;  // ボクセルデータ(一時用)

    // メモリ管理
    std::unique_ptr<unsigned char[]> voxel_buf_;  // ボクセルデータ

   public:
    SpatialVoxel() = delete;
    SpatialVoxel(const SpatialVoxel&) = delete;
    SpatialVoxel(SpatialVoxel&&) = delete;
    SpatialVoxel& operator=(const SpatialVoxel&) = delete;
    SpatialVoxel& operator=(SpatialVoxel&&) = delete;

    /**
     * @brief Destroy the Voxel object
     *
     */
    virtual ~SpatialVoxel();

    /**
     * @brief
     * ボクセルデータをSQLiteを使わずに全てメモリで管理するオプション
     *
     * @param enabled true:全てメモリで管理 false:内部でSQLiteを使用
     */
    static void SetOptionInMemory(bool enabled) {
        option_use_in_memory = enabled;
    }

    /**
     * @brief ボクセルデータ一覧を作成する
     *
     * @param spatial_id_map
     * グローバル空間IDに対するローカル空間IDの精度レベルで表現した空間ID一覧
     * @param local_zoom ローカル空間IDの精度レベル
     * @param[out] voxel_list ボクセルデータ一覧
     */
    static void Create(
        std::map<std::string, std::unordered_set<Zfxy, Zfxy::hash_function>>&
            spatial_id_map,
        int local_zoom,
        std::map<std::string, std::unique_ptr<SpatialVoxel>>& voxel_list);

    /**/

    /**
     * @brief ボクセルデータのポインタ取得
     *
     * @return const unsigned*
     */
    const unsigned char* GetPtr();

    /**
     * @brief ボクセルデータのポインタ解放
     *
     * @remark
     * GetPtrメソッドの呼び出しが不要になった場合にこのメソッドを呼び出してください
     * 内部的に、一時使用メモリを解放します
     */
    void ReleasePtr();

    /**
     * @brief ボクセルデータサイズ(bytes)
     *
     * @return uint64_t
     */
    uint64_t GetSize() const { return voxel_size_; }

    /**
     * @brief ボクセルデータ1平面（経度x緯度）のポインタ取得
     *
     * @param alt_index 取得したい高さの開始位置
     * @return const unsigned*
     * @throw std::invalid_argument 引数エラー
     * @remark
     * 高さは、0以上GetAltIndexSizeメソッド値未満の値を指定してください
     * 戻り値のデータサイズは GetPlaneSizeメソッドで取得できます
     * 戻り値が nullptr の場合、1平面すべて0ビットを意味します
     */
    const unsigned char* GetPlanePtr(int alt_index = 0);

    /**
     * @brief ボクセルデータ1平面（経度x緯度）のポインタ解放
     *
     * @remark
     * GetPlanePtrメソッドの呼び出しが不要になった場合にこのメソッドを呼び出してください
     * 内部的に、SQLiteファイルを閉じて使用メモリを解放します
     */
    void ReleasePlanePtr();

    /**
     * @brief 1平面（経度x緯度）のボクセルデータサイズ[bytes]
     *
     * @return int
     */
    int GetPlaneSize() const { return plane_size_; }

    /**
     * @brief 経度方向数
     *
     * @return int
     */
    int GetLonIndexSize() const { return lon_index_size_; }

    /**
     * @brief 緯度方向数
     *
     * @return int
     */
    int GetLatIndexSize() const { return lat_index_size_; }

    /**
     * @brief 高さ数
     *
     * @return int
     */
    int GetAltIndexSize() const { return alt_index_size_; }

    /**
     * @brief 経度方向バイト数
     *
     * @return int
     */
    int GetLonIndexBytes() const { return lon_index_byte_; }

   private:
    /**
     * @brief Construct a new Spatial Voxel object
     *
     * @param spatial_id 空間ID
     * @param local_zoom ローカル空間IDの精度レベル
     * @throw std::invalid_argument 引数エラー
     */
    SpatialVoxel(const std::string& spatial_id, int local_zoom);

    /**
     * @brief 指定した位置のボクセルデータのビットフラグをセット[SQLite管理]
     *
     * @param zfxy
     * @param[out] voxel_data
     * @throw std::invalid_argument 引数エラー
     */
    void Set(const Zfxy& zfxy,
             std::map<int, std::unique_ptr<unsigned char[]>>& voxel_data);

    /**
     * @brief 指定した位置のボクセルデータのビットフラグをセット[メモリ管理]
     *
     * @param zfxy
     * @param[out] voxel_data
     * @throw std::invalid_argument 引数エラー
     */
    void SetVoxelBuf(const Zfxy& zfxy);
};

}  // namespace spatialid

#endif  // __SPATIAL_VOXEL_H__
