/**
 * @file convert.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "convert.h"

#include <sstream>

namespace spatialid {

// 文字列を区切り文字で分割する
std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> str_list;
    std::stringstream ss(str);
    std::string buffer;
    while (std::getline(ss, buffer, delimiter)) {
        str_list.push_back(buffer);
    }
    return str_list;
}

// 文字列を区切り文字で分割し、指定した番号の文字列を返す
std::string split_string_get(const std::string& str, char delimiter, int index) {
    int count = 0;
    std::stringstream ss(str);
    std::string buffer;
    while (std::getline(ss, buffer, delimiter)) {
        if (count == index) {
            return buffer;
        }
        count++;
    }
    if (index == 0) {
        return str;
    }
    throw std::invalid_argument("STRING_FORMAT_ERROR");
}

}  // namespace spatialid
