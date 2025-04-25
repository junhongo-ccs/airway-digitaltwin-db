/**
 * @file zfxy.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-11
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __ZFXY_H__
#define __ZFXY_H__

#include <stdexcept>
#include <string>
#include <vector>

#include "common/convert.h"

namespace spatialid {

/**
 * @brief 空間IDのZFXY構造体
 *
 */
typedef struct Zfxy {
    int zoom;       // 空間IDの精度
    int alt_index;  // 高さの位置
    int lon_index;  // 経度方向の位置
    int lat_index;  // 緯度方向の位置

    /**
     * @brief Construct a new Zfxy object
     *
     * @param z 精度
     * @param f 高さの位置
     * @param x 経度方向の位置
     * @param y 緯度方向の位置
     */
    Zfxy(int z = 0, int f = 0, int x = 0, int y = 0)
        : zoom(z), alt_index(f), lon_index(x), lat_index(y) {}

    /**
     * @brief Construct a new Zfxy object
     *
     * @param spatial_id 空間ID
     * @throw std::invalid_argument 引数エラー
     */
    Zfxy(const std::string& spatial_id) {
        std::vector<std::string> id = split_string(spatial_id, '/');
        if (id.size() != 4) {
            throw std::invalid_argument("INPUT_VALUE_ERROR");
        }
        zoom = atoi(id[0].c_str());
        alt_index = atoi(id[1].c_str());
        lon_index = atoi(id[2].c_str());
        lat_index = atoi(id[3].c_str());
    }

    /**
     * @brief
     * z{精度レベル}/f{高さの位置}/x{経度方向の位置}/y{緯度方向の位置}形式の文字列で取得する
     *
     * @return std::string
     */
    std::string getString() const {
        return std::to_string(zoom)
            .append("/")
            .append(std::to_string(alt_index))
            .append("/")
            .append(std::to_string(lon_index))
            .append("/")
            .append(std::to_string(lat_index));
    }

    /**
     * @brief 等値演算子
     *
     * @param rhs 比較対象のオブジェクト
     * @return true
     * @return false
     */
    bool operator==(const Zfxy& rhs) const {
        return (zoom == rhs.zoom && alt_index == rhs.alt_index &&
                lon_index == rhs.lon_index && lat_index == rhs.lat_index);
    }

    /**
     * @brief unordered_set用のハッシュ関数
     *
     */
    struct hash_function {
        template <class T>
        static void hash_combine(std::size_t& seed, const T& v) {
            std::hash<T> hasher;
            const std::size_t kMul = 0x9ddfea08eb382d69ULL;
            std::size_t a = (hasher(v) ^ seed) * kMul;
            a ^= (a >> 47);
            std::size_t b = (seed ^ a) * kMul;
            b ^= (b >> 47);
            seed = b * kMul;
        }
        size_t operator()(const Zfxy& zfxy) const {
            size_t h = 0;
            hash_function::hash_combine(h, zfxy.zoom);
            hash_function::hash_combine(h, zfxy.alt_index);
            hash_function::hash_combine(h, zfxy.lon_index);
            hash_function::hash_combine(h, zfxy.lat_index);
            return h;
        }
    };

} Zfxy;

}  // namespace spatialid

#endif  // __ZFXY_H__
