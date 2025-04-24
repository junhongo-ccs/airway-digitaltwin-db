/**
 * @file spatial_sqlite.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-06-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __SPATIAL_SQLITE_H__
#define __SPATIAL_SQLITE_H__

#include <sqlite3.h>

#include <string>

#include "logging/logger.h"

namespace spatialid {

/**
 * @brief t_spatial_id テーブルに記録する空間IDのメタデータ
 *
 */
typedef struct SpatialSqliteMetaData {
    /** 空間ID */
    std::string spatial_id;
    /** ローカル空間IDに分割した際の経度方向数 */
    int lon_index_size;
    /** ローカル空間IDに分割した際の緯度方向数 */
    int lat_index_size;
    /** ローカル空間IDに分割した際の高さ数 */
    int alt_index_size;
    /** 経度方向のデータ保持に必要なバイト数 */
    int lon_index_byte;
    /** ローカル空間の精度レベル */
    int zoom_level;
} SpatialSqliteMetaData;

/**
 * @brief 作業用ディレクトリクラス(Singleton)
 *
 */
class SpatialSqliteTmpDir {
    friend class SpatialSqlite;

   private:
    /** 作業用ディレクトリ名 */
    std::string tmp_dirname_;

   private:
    /**
     * @brief Construct a new SpatialSqliteTmpDir object
     *
     */
    SpatialSqliteTmpDir();

    /**
     * @brief Destroy the SpatialSqliteTmpDir object
     *
     */
    ~SpatialSqliteTmpDir();

   public:
    SpatialSqliteTmpDir(const SpatialSqliteTmpDir&) = delete;
    SpatialSqliteTmpDir& operator=(const SpatialSqliteTmpDir&) = delete;
    SpatialSqliteTmpDir(SpatialSqliteTmpDir&&) = delete;
};

/**
 * @brief 空間ID用SQLiteラッパークラス
 *
 */
class SpatialSqlite {
   protected:
    /** ファイルバージョン */
    static const int file_version_;
    /** sqlite3ハンドル */
    sqlite3* db_;
    /** メタ情報 */
    SpatialSqliteMetaData metadata_;
    /** 1平面（経度x緯度）のボクセルデータ */
    std::unique_ptr<unsigned char[]> plane_buf_;
    /** 1平面（経度x緯度）のボクセルデータサイズ(SQLiteのBLOBの最大はSQLITE_MAX_LENGTH(2GB) */
    int plane_size_;

   public:
    /**
     * @brief Construct a new SpatialSqliteobject
     *
     * @param path 保存ディレクトリのパス
     * @param spatial_id 空間ID
     */
    SpatialSqlite() : db_(nullptr){};

    /**
     * @brief Destroy the SpatialSqliteobject
     *
     */
    virtual ~SpatialSqlite() { Close(); }

    /**
     * @brief SQLiteファイルを作成する、または、開く
     *
     * @param group_id グループID
     * @param metadata 空間IDのメタデータ
     * @param[out] filename SQLiteファイル名
     * @throw std::runtime_error
     */
    void Create(int group_id, const SpatialSqliteMetaData& metadata,
                std::string& filename);

    /**
     * @brief SQLiteファイルを開く
     *
     * @param filename ファイル名
     * @param readonly true:読み取り専用 false:読み書き可
     * @throw std::runtime_error
     * @remark
     * ファイルが存在しない場合、false を返す
     */
    void Open(const std::string& filename, bool readonly = true);

    /**
     * @brief SQLiteファイルが開いているかどうか
     *
     * @return true
     * @return false
     */
    bool IsOpen() const { return (db_) ? true : false; }

    /**
     * @brief SQLiteファイルを閉じる
     *
     */
    void Close();

    /**
     * @brief 空間ボクセルの1平面データを登録する
     *
     * @param alt_index 高さインデックス
     * @param plane_buf 1平面データ
     * @param plane_size 1平面データのサイズ[bytes]
     * @throw std::runtime_error
     */
    bool SetPlaneData(int alt_index, unsigned char* plane_buf, int plane_size);

    /**
     * @brief ボクセルデータ1平面（経度x緯度）のポインタ取得
     *
     * @param alt_index 取得したい高さの開始位置
     * @return const unsigned*
     */
    const unsigned char* GetPlanePtr(int alt_index = 0);

    /**
     * @brief SQLiteファイルを削除する
     *
     * @param filename
     */
    void DeleteFile(const std::string& filename);

   private:
    /**
     * @brief SQLiteファイル名を生成する
     *
     * @param group_id グループID
     * @param spatial_id 空間ID
     * @param make_dir 保存グループ用ディレクトリの作成有無
     * @param[out] filename ファイル名
     */
    void MakeFilename(int group_id, const std::string& spatial_id,
                      bool make_dir, std::string& filename);

    /**
     * @brief テーブルが存在するか
     *
     * @param tablename テーブル名
     * @return true
     * @return false
     * @throw std::runtime_error
     */
    bool ExistTable(const char* tablename);

    /**
     * @brief バージョン情報を登録する
     *
     * @throw std::runtime_error
     */
    void InsertVersion();

    /**
     * @brief バージョン情報を取得する
     *
     * @return int
     */
    int GetVersion();

    /**
     * @brief メタ情報を登録する
     *
     * @param metadata メタ情報
     * @throw std::runtime_error
     */
    void InsertMetadata(const SpatialSqliteMetaData& metadata);

    /**
     * @brief メタ情報を取得する
     *
     * @param[out] metadata メタ情報
     * @throw std::runtime_error
     */
    void GetMetadata(SpatialSqliteMetaData& metadata);
};

}  // namespace spatialid

#endif  // __SPATIAL_SQLITE_H__
