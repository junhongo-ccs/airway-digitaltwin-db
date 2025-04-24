/**
 * @file spatial_sqlite.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-06-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "spatial_sqlite.h"

#include <chrono>
#include <filesystem>
#include <regex>
#include <sstream>
#include <stdexcept>

#ifdef _MSC_VER
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace spatialid {

/** SpatialSqliteTmpDir コンストラクタ */
SpatialSqliteTmpDir::SpatialSqliteTmpDir() : tmp_dirname_("./sqlite_tmp/") {
    // 作業用ルートディレクトリ
    if (!std::filesystem::exists(tmp_dirname_)) {
        std::filesystem::create_directory(tmp_dirname_);
    }
    // "日時_プロセスID" のサブディレクトリを作成（マルチプロセス対応）
    time_t current_time = time(nullptr);
    struct tm current_tm;
#ifdef _MSC_VER
    localtime_s(&current_tm, &current_time);
#else
    localtime_r(&current_time, &current_tm);
#endif

    std::stringstream ss;
    ss << std::put_time(&current_tm, "%Y%m%d%H%M%S_");

    tmp_dirname_.append(ss.str());
#ifdef _MSC_VER
    int pid = _getpid();
#else
    pid_t pid = getpid();
#endif
    tmp_dirname_.append(std::to_string(pid));
    tmp_dirname_.append("/");

    // 作業用ディレクトリの再作成
    std::filesystem::remove_all(tmp_dirname_);
    std::filesystem::create_directory(tmp_dirname_);
}

/** SpatialSqliteTmpDir デストラクタ */
SpatialSqliteTmpDir::~SpatialSqliteTmpDir() {
    // 作業用ディレクトリの削除
    std::filesystem::remove_all(tmp_dirname_);
}

/** SQLiteファイルバージョン */
const int SpatialSqlite::file_version_ = 1;

/** SQLiteファイルを作成する */
void SpatialSqlite::Create(int group_id, const SpatialSqliteMetaData& metadata,
                           std::string& filename) {
    Close();

    // SQLiteファイル名の生成
    MakeFilename(group_id, metadata.spatial_id, true, filename);

    // すでにSQLiteファイルが存在した場合、削除しておく
    DeleteFile(filename);

    // SQLiteファイルを開く
    int flags =
        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
    int ret = sqlite3_open_v2(filename.c_str(), &db_, flags, nullptr);
    if (ret != SQLITE_OK) {
        std::string err_msg = sqlite3_errmsg(db_);
        LOG_ERROR("sqlite3_open_v2=%d %s", ret, err_msg.c_str());
        Close();
        throw std::runtime_error(err_msg);
    }

    if (!ExistTable("t_information")) {
        // t_information テーブル作成
        char* err = nullptr;
        ret = sqlite3_exec(
            db_, "CREATE TABLE t_information(version INTEGER NOT NULL)",
            nullptr, nullptr, &err);
        if (ret != SQLITE_OK) {
            std::string err_msg = err;
            LOG_ERROR("sqlite3_exec=%d %s", ret, err_msg.c_str());
            sqlite3_free(err);
            Close();
            DeleteFile(filename);
            throw std::runtime_error(err_msg);
        }

        // バージョン情報の登録
        InsertVersion();
    } else {
        // バージョン情報の取得
        int version = GetVersion();
        if (version != file_version_) {
            // バージョン不一致
            // TODO: migration
            std::string err_msg = "sqlite3 file version unmatch";
            LOG_ERROR("%s %d %d", err_msg.c_str(), version, file_version_);
            Close();
            throw std::runtime_error(err_msg);
        }
    }

    if (!ExistTable("t_spatial_id")) {
        // t_spatial_id テーブル作成
        char* err = nullptr;
        ret = sqlite3_exec(db_,
                           "CREATE TABLE t_spatial_id("
                           "spatial_id TEXT NOT NULL,"
                           "lon_index_size INTEGER NOT NULL,"
                           "lat_index_size INTEGER NOT NULL,"
                           "alt_index_size INTEGER NOT NULL,"
                           "lon_index_byte INTEGER NOT NULL,"
                           "zoom_level INTEGER NOT NULL,"
                           "PRIMARY KEY(spatial_id)"
                           ")",
                           nullptr, nullptr, &err);
        if (ret != SQLITE_OK) {
            std::string err_msg = err;
            LOG_ERROR("sqlite3_exec=%d %s", ret, err_msg.c_str());
            sqlite3_free(err);
            Close();
            DeleteFile(filename);
            throw std::runtime_error(err_msg);
        }

        // メタ情報の登録
        InsertMetadata(metadata);

        // メタ情報のコピー
        metadata_ = metadata;
        plane_size_ = metadata_.lon_index_byte * metadata_.lat_index_size;

    } else {
        // メタ情報の取得
        GetMetadata(metadata_);
    }

    if (!ExistTable("t_spatial_voxel")) {
        // t_spatial_voxel テーブル作成
        char* err = nullptr;
        ret = sqlite3_exec(db_,
                           "CREATE TABLE t_spatial_voxel("
                           "alt_index INTEGER NOT NULL,"
                           "plane BLOB,"
                           "PRIMARY KEY(alt_index)"
                           ")",
                           nullptr, nullptr, &err);
        if (ret != SQLITE_OK) {
            std::string err_msg = err;
            LOG_ERROR("sqlite3_exec=%d %s", ret, err_msg.c_str());
            sqlite3_free(err);
            Close();
            DeleteFile(filename);
            throw std::runtime_error(err_msg);
        }
    }
}

/** SQLiteファイルを開く */
void SpatialSqlite::Open(const std::string& filename, bool readonly) {
    Close();

    // SQLiteファイルを開く
    int flags = SQLITE_OPEN_FULLMUTEX;
    flags |= (readonly) ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
    int ret = sqlite3_open_v2(filename.c_str(), &db_, flags, nullptr);
    if (ret != SQLITE_OK) {
        std::string err_msg = sqlite3_errmsg(db_);
        LOG_ERROR("sqlite3_open_v2=%d %s", ret, err_msg.c_str());
        Close();
        throw std::runtime_error(err_msg);
    }

    // バージョン情報の取得
    int version = GetVersion();
    if (version != file_version_) {
        // バージョン不一致
        // TODO: migration
        std::string err_msg = "sqlite3 file version unmatch";
        LOG_ERROR("%s %d %d", err_msg.c_str(), version, file_version_);
        Close();
        throw std::runtime_error(err_msg);
    }

    // メタ情報の取得
    GetMetadata(metadata_);
    plane_size_ = metadata_.lon_index_byte * metadata_.lat_index_size;
}

/** SQLiteファイルを閉じる */
void SpatialSqlite::Close() {
    // sqlite3クローズ
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    plane_buf_.reset();
}

/** 空間ボクセルの1平面データを登録する */
bool SpatialSqlite::SetPlaneData(int alt_index, unsigned char* plane_buf,
                                 int plane_size) {
    if (alt_index < 0 || alt_index >= metadata_.alt_index_size || !plane_buf ||
        plane_size != plane_size_) {
        return false;
    }

    std::string err_msg;

    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(
        db_, "REPLACE INTO t_spatial_voxel(alt_index, plane) VALUES (?,?) ", -1,
        &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
        LOG_ERROR("sqlite3_open_v2=%d %s", ret, err_msg.c_str());
    } else {
        sqlite3_bind_int(stmt, 1, alt_index);
        sqlite3_bind_blob(stmt, 2, plane_buf, plane_size, SQLITE_STATIC);

        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            err_msg = sqlite3_errmsg(db_);
            LOG_ERROR("sqlite3_step=%d %s", ret, err_msg.c_str());
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
                LOG_ERROR("sqlite3_finalize=%d %s", ret, err_msg.c_str());
            }
        }
    }

    if (!err_msg.empty()) {
        throw std::runtime_error(err_msg);
    }

    return true;
}

/** ボクセルデータ1平面（経度x緯度）のポインタ取得 */
const unsigned char* SpatialSqlite::GetPlanePtr(int alt_index) {
    if (alt_index < 0 || alt_index >= metadata_.alt_index_size) {
        throw std::invalid_argument("INPUT_VALUE_ERROR");
    }

    if (!plane_buf_) {
        plane_buf_.reset(new unsigned char[plane_size_]());
    }

    bool buf_copied = false;
    std::string err_msg;

    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(
        db_, "SELECT plane FROM t_spatial_voxel WHERE alt_index = ?", -1, &stmt,
        nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
        LOG_ERROR("sqlite3_open_v2=%d %s", ret, err_msg.c_str());
    } else {
        sqlite3_bind_int(stmt, 1, alt_index);

        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            int size = sqlite3_column_bytes(stmt, 0);
            if (size == plane_size_) {
                const unsigned char* buf =
                    reinterpret_cast<const unsigned char*>(
                        sqlite3_column_blob(stmt, 0));
                std::memcpy(plane_buf_.get(), buf, size);
                buf_copied = true;
            } else {
                err_msg = "t_spatial_voxel vixel size unmatch";
                LOG_ERROR("%s %d %d", err_msg.c_str(), size, plane_size_);
            }
        } else if (ret == SQLITE_DONE) {
            // 指定の高さの平面データは未登録 = すべて0bitの平面
        } else {
            err_msg = sqlite3_errmsg(db_);
            LOG_ERROR("sqlite3_step=%d %s", ret, err_msg.c_str());
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
                LOG_ERROR("sqlite3_finalize=%d %s", ret, err_msg.c_str());
            }
        }
    }

    if (!err_msg.empty()) {
        throw std::runtime_error(err_msg);
    }

    return (buf_copied) ? plane_buf_.get() : nullptr;
}

/** SQLiteファイル名を生成する */
void SpatialSqlite::MakeFilename(int group_id, const std::string& spatial_id,
                                 bool make_dir, std::string& filename) {
    static SpatialSqliteTmpDir sqlite_tmp_dir;

    std::string group = std::to_string(group_id);
    size_t size = sqlite_tmp_dir.tmp_dirname_.size() + group.size() +
               spatial_id.size() + 9;
    filename.reserve(size);

    filename = sqlite_tmp_dir.tmp_dirname_;
    filename += group;
    if (make_dir) {
        std::filesystem::create_directory(filename);
    }
    filename += "/";
    filename += std::regex_replace(spatial_id, std::regex("/"), "_");
    filename += ".sqlite3";
}

/** SQLiteファイルを削除する */
void SpatialSqlite::DeleteFile(const std::string& filename) {
    if (filename.size() > 0 && std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
}

/** テーブルが存在するか */
bool SpatialSqlite::ExistTable(const char* tablename) {
    std::string err_msg;
    int count = -1;

    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(
        db_, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?",
        -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
    } else {
        sqlite3_bind_text(stmt, 1, tablename, -1, SQLITE_STATIC);

        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        } else {
            err_msg = sqlite3_errmsg(db_);
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
            }
        }
    }

    if (!err_msg.empty()) {
        Close();
        throw std::runtime_error(err_msg);
    }

    return (count > 0);
}

/** バージョン情報を登録する */
void SpatialSqlite::InsertVersion() {
    std::string err_msg;

    sqlite3_stmt* stmt = nullptr;
    int ret =
        sqlite3_prepare_v2(db_, "INSERT INTO t_information(version) VALUES (?)",
                           -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
    } else {
        sqlite3_bind_int(stmt, 1, file_version_);

        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            err_msg = sqlite3_errmsg(db_);
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
            }
        }
    }

    if (!err_msg.empty()) {
        Close();
        throw std::runtime_error(err_msg);
    }
}

/** バージョン情報を取得する */
int SpatialSqlite::GetVersion() {
    std::string err_msg;
    int version = -1;

    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(db_, "SELECT version FROM t_information", -1,
                                 &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
    } else {
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            version = sqlite3_column_int(stmt, 0);
        } else {
            err_msg = sqlite3_errmsg(db_);
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
            }
        }
    }

    if (!err_msg.empty()) {
        Close();
        throw std::runtime_error(err_msg);
    }

    return version;
}

/** メタ情報を登録する */
void SpatialSqlite::InsertMetadata(const SpatialSqliteMetaData& metadata) {
    std::string err_msg;

    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(
        db_,
        "INSERT INTO t_spatial_id(spatial_id, "
        "lon_index_size, lat_index_size, alt_index_size, "
        "lon_index_byte, zoom_level) VALUES (?, ?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
    } else {
        sqlite3_bind_text(stmt, 1, metadata.spatial_id.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, metadata.lon_index_size);
        sqlite3_bind_int(stmt, 3, metadata.lat_index_size);
        sqlite3_bind_int(stmt, 4, metadata.alt_index_size);
        sqlite3_bind_int(stmt, 5, metadata.lon_index_byte);
        sqlite3_bind_int(stmt, 6, metadata.zoom_level);

        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            err_msg = sqlite3_errmsg(db_);
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
            }
        }
    }

    if (!err_msg.empty()) {
        Close();
        throw std::runtime_error(err_msg);
    }
}

/** メタ情報を取得する */
void SpatialSqlite::GetMetadata(SpatialSqliteMetaData& metadata) {
    std::string err_msg;

    sqlite3_stmt* stmt = nullptr;
    int ret =
        sqlite3_prepare_v2(db_,
                           "SELECT spatial_id, "
                           "lon_index_size, lat_index_size, alt_index_size, "
                           "lon_index_byte, zoom_level FROM t_spatial_id",
                           -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        err_msg = sqlite3_errmsg(db_);
    } else {
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            metadata.spatial_id =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            metadata.lon_index_size = sqlite3_column_int(stmt, 1);
            metadata.lat_index_size = sqlite3_column_int(stmt, 2);
            metadata.alt_index_size = sqlite3_column_int(stmt, 3);
            metadata.lon_index_byte = sqlite3_column_int(stmt, 4);
            metadata.zoom_level = sqlite3_column_int(stmt, 5);
        } else {
            err_msg = sqlite3_errmsg(db_);
        }

        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK) {
            if (err_msg.empty()) {
                err_msg = sqlite3_errmsg(db_);
            }
        }
    }

    if (!err_msg.empty()) {
        Close();
        throw std::runtime_error(err_msg);
    }
}

}  // namespace spatialid
