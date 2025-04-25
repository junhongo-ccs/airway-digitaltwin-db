/**
 * @file spatial_voxel.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-10
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "spatial_voxel.h"

#include "common/const.h"
#include "common/spatial_point.h"
#include "spatial_api.h"

namespace spatialid {

std::mutex SpatialVoxel::mutex_;
int SpatialVoxel::group_id_ = 0;
bool SpatialVoxel::option_use_in_memory = false;

// コンストラクタ
SpatialVoxel::SpatialVoxel(const std::string &spatial_id, int local_zoom)
    : spatial_id_(spatial_id) {
    // 空間IDの各頂点の座標を取得
    std::vector<Point> vertex;
    get_vertex_points_on_spatial_id(spatial_id, vertex);

    // 北西下のローカル空間ID
    local_spatial_zfxy_ = get_spatial_zfxy_on_point(vertex[0].x, vertex[0].y,
                                                    vertex[0].z, local_zoom);

    // 南東上のローカル空間ID
    Zfxy south_west_top(get_spatial_zfxy_on_point(
        vertex[6].x, vertex[6].y, vertex[6].z, local_zoom, false));

    // ローカル空間ID内の経度方向、緯度方向、高さの数
    lon_index_size_ = south_west_top.lon_index - local_spatial_zfxy_.lon_index;
    lat_index_size_ = south_west_top.lat_index - local_spatial_zfxy_.lat_index;
    alt_index_size_ = south_west_top.alt_index - local_spatial_zfxy_.alt_index;

    // 経度方向は 1ID を 1bit、8ID を 1byte で表現
    int width_units = lon_index_size_ / kBoundaryBits;
    if ((lon_index_size_ % kBoundaryBits) > 0) {
        width_units++;
    }
    lon_index_byte_ = width_units * (kBoundaryBits / 8);

    // 1平面（経度x緯度）のボクセルデータサイズ[bytes]
    plane_size_ = lon_index_byte_ * lat_index_size_;

    // 1ボクセルデータサイズ[bytes]
    voxel_size_ = plane_size_ * alt_index_size_;

    LOG_DEBUG(
        "lon_index_size=%d (lon_bytes=%d) lat_index_size=%d "
        "alt_index_size=%d "
        "spatial_id=%s plane_size=%d voxel_size=%llu",
        lon_index_size_, lon_index_byte_, lat_index_size_, alt_index_size_,
        spatial_id.c_str(), plane_size_, voxel_size_);
}

// デストラクタ
SpatialVoxel::~SpatialVoxel() {
    if (voxel_buf_) {
        // [メモリ管理]
        // なし
    } else {
        // [SQLite管理]
        sqlite_.Close();
        sqlite_.DeleteFile(sqlite_filename_);
    }
}

// ボクセルデータ一覧を作成する
void SpatialVoxel::Create(
    std::map<std::string, std::unordered_set<Zfxy, Zfxy::hash_function>>
        &spatial_id_map,
    int local_zoom, std::map<std::string, std::unique_ptr<SpatialVoxel>>
        &voxel_list) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        group_id_++;
    }

    voxel_list.clear();

    // グローバル空間ID単位に処理
    for (const auto &[spatial_id, zfxy_list] : spatial_id_map) {
        // 1グローバル空間IDのボクセルデータクラスを用意
        SpatialVoxel *voxel = new SpatialVoxel(spatial_id, local_zoom);
        // メモリ上で管理する場合、メモリ確保
        if (option_use_in_memory) {
            voxel->voxel_buf_.reset(new unsigned char[voxel->voxel_size_]());
        }
        voxel_list[spatial_id].reset(voxel);

        // 1グローバル空間IDのボクセルデータにローカル空間IDのビットを立てる
        if (voxel->voxel_buf_) {
            // [メモリ管理]
            for (const auto &zfxy : zfxy_list) {
                voxel->SetVoxelBuf(zfxy);
            }
        } else {
            // [SQLite管理]
            std::map<int, std::unique_ptr<unsigned char[]>> voxel_data;
            for (const auto &zfxy : zfxy_list) {
                voxel->Set(zfxy, voxel_data);
            }

            // 1グローバル空間IDのボクセルデータをSQLiteに保存
            SpatialSqliteMetaData metadata;
            metadata.spatial_id = spatial_id;
            metadata.lon_index_size = voxel->GetLonIndexSize();
            metadata.lat_index_size = voxel->GetLatIndexSize();
            metadata.alt_index_size = voxel->GetAltIndexSize();
            metadata.lon_index_byte = voxel->GetLonIndexBytes();
            metadata.zoom_level = local_zoom;
            voxel->sqlite_.Create(group_id_, metadata, voxel->sqlite_filename_);
            for (const auto &[alt_index, plane_ptr] : voxel_data) {
                voxel->sqlite_.SetPlaneData(alt_index, plane_ptr.get(),
                                            voxel->GetPlaneSize());
            }
            voxel->sqlite_.Close();
        }
    }
}

// 指定した位置のボクセルデータのビットフラグをセット[SQLite管理]
void SpatialVoxel::Set(
    const Zfxy &zfxy,
    std::map<int, std::unique_ptr<unsigned char[]>> &voxel_data) {
    // ローカル空間ID
    int lon_index = zfxy.lon_index - local_spatial_zfxy_.lon_index;
    if (lon_index < 0 || lon_index >= lon_index_size_) {
        LOG_VERBOSE("lon index out of range=%d", lon_index);
        return;
    }
    int lat_index = zfxy.lat_index - local_spatial_zfxy_.lat_index;
    if (lat_index < 0 || lat_index >= lat_index_size_) {
        LOG_VERBOSE("lat index out of range=%d", lat_index);
        return;
    }
    int alt_index = zfxy.alt_index - local_spatial_zfxy_.alt_index;
    if (alt_index < 0 || alt_index >= alt_index_size_) {
        LOG_VERBOSE("alt index out of range=%d", alt_index);
        return;
    }

    // 1平面のポインタを取得(初回はメモリ割当)
    unsigned char *plane = nullptr;
    auto it = voxel_data.find(alt_index);
    if (it != voxel_data.end()) {
        plane = it->second.get();
    } else {
        plane = new unsigned char[plane_size_]();
        voxel_data[alt_index].reset(plane);
    }

    // 該当位置までのオフセット
    uint64_t offset = 0;
    if (lat_index > 0) {
        offset += (lon_index_byte_ * lat_index);
    }
    if (lon_index > 0) {
        offset += (lon_index / 8);
    }

    // 該当ビットを立てる
    int bit_shift = lon_index % 8;
    *(plane + offset) |= (1 << bit_shift);
}

// 指定した位置のボクセルデータのビットフラグをセット[メモリ管理]
void SpatialVoxel::SetVoxelBuf(const Zfxy &zfxy) {
    // ローカル空間ID
    int lon_index = zfxy.lon_index - local_spatial_zfxy_.lon_index;
    if (lon_index < 0 || lon_index >= lon_index_size_) {
        LOG_VERBOSE("lon index out of range=%d", lon_index);
        return;
    }
    int lat_index = zfxy.lat_index - local_spatial_zfxy_.lat_index;
    if (lat_index < 0 || lat_index >= lat_index_size_) {
        LOG_VERBOSE("lat index out of range=%d", lat_index);
        return;
    }
    int alt_index = zfxy.alt_index - local_spatial_zfxy_.alt_index;
    if (alt_index < 0 || alt_index >= alt_index_size_) {
        LOG_VERBOSE("alt index out of range=%d", alt_index);
        return;
    }

    // ボクセルデータの該当位置までのオフセット
    uint64_t offset = 0;
    if (alt_index > 0) {
        offset += (lon_index_byte_ * lat_index_size_ * alt_index);
    }
    if (lat_index > 0) {
        offset += (lon_index_byte_ * lat_index);
    }
    if (lon_index > 0) {
        offset += (lon_index / 8);
    }
    int bit_shift = lon_index % 8;

    // ボクセルデータの該当ビットを立てる
    unsigned char *p = voxel_buf_.get();
    *(p + offset) |= (1 << bit_shift);
}

// ボクセルデータのポインタ取得
const unsigned char *SpatialVoxel::GetPtr() {
    if (voxel_buf_) {
        // [メモリ管理]
        return voxel_buf_.get();
    } else {
        // [SQLite管理]
        tmp_buf_.reset(new unsigned char[voxel_size_]());

        if (!sqlite_.IsOpen()) {
            sqlite_.Open(sqlite_filename_);
        }

        unsigned char *p = tmp_buf_.get();
        for (int i = 0; i < alt_index_size_; i++) {
            const unsigned char *src = sqlite_.GetPlanePtr(i);
            if (src) {
                uint64_t offset = plane_size_ * i;
                std::memcpy((p + offset), src, plane_size_);
            }
        }

        sqlite_.Close();

        return p;
    }
}

// ボクセルデータのポインタ解放
void SpatialVoxel::ReleasePtr() {
    if (voxel_buf_) {
        // [メモリ管理]
        // なし
    } else {
        // [SQLite管理]
        tmp_buf_.reset();
    }
}

// ボクセルデータ1平面（経度x緯度）のポインタ取得
const unsigned char *SpatialVoxel::GetPlanePtr(int alt_index) {
    if (voxel_buf_) {
        // [メモリ管理]
        if (alt_index < 0 || alt_index >= alt_index_size_) {
            throw std::invalid_argument("INPUT_VALUE_ERROR");
        }

        unsigned char *p = voxel_buf_.get();
        if (alt_index > 0) {
            return p + (lon_index_byte_ * lat_index_size_ * alt_index);
        }
        return p;
    } else {
        // [SQLite管理]
        if (!sqlite_.IsOpen()) {
            sqlite_.Open(sqlite_filename_);
        }
        return sqlite_.GetPlanePtr(alt_index);
    }
}

// ボクセルデータ1平面（経度x緯度）のポインタ解放
void SpatialVoxel::ReleasePlanePtr() {
    if (voxel_buf_) {
        // [メモリ管理]
    } else {
        // [SQLite管理]
        sqlite_.Close();
    }
}

// ボクセルデータを内部で全てメモリ上で管理するオプション
void set_option_voxel_in_memory(bool enabled) {
    SpatialVoxel::SetOptionInMemory(enabled);
}

}  // namespace spatialid
