//! @file SpatialUtil.cpp
//! @brief 空間情報関連ユーティリティ関数

#include <string>
#include "SpatialUtil.h"
#include <vector>
#include <iostream>
#include "proj.h"

//! @brief EPSG４桁の数字から　API引数用　spatialid::CRS定義を取得する
//! 
//! @param argEpsg EPSG４桁の数字文字列
//! @return API用spatialid::CRS定義
//! 
spatialid::CRS SpatialUtil::getCrsFromArgEpsg(std::string argEpsg) {

    if (argEpsg == "WGS84" || argEpsg == "4326") {
        return spatialid::CRS::kWGS84;
    }
    else if (argEpsg == "6697") {
        return spatialid::CRS::kWGS84;
    }
    else if (argEpsg == "JDD2011" || argEpsg == "6668") {       //高さ無し」
        return spatialid::CRS::kJGD2011;
    }
    else if (argEpsg == "JGD2011_1" || argEpsg == "JGD1" || argEpsg == "6669") {
        return spatialid::CRS::KJGD2011_1;
    }
    else if (argEpsg == "JGD2011_2" || argEpsg == "JGD2" || argEpsg == "6670") {
        return spatialid::CRS::KJGD2011_2;
    }
    else if (argEpsg == "JGD2011_3" || argEpsg == "JGD3" || argEpsg == "6671") {
        return spatialid::CRS::KJGD2011_3;
    }
    else if (argEpsg == "JGD2011_4" || argEpsg == "JGD4" || argEpsg == "6672") {
        return spatialid::CRS::KJGD2011_4;
    }
    else if (argEpsg == "JGD2011_5" || argEpsg == "JGD5" || argEpsg == "6673") {
        return spatialid::CRS::KJGD2011_5;
    }
    else if (argEpsg == "JGD2011_6" || argEpsg == "JGD6" || argEpsg == "6674") {
        return spatialid::CRS::KJGD2011_6;
    }
    else if (argEpsg == "JGD2011_7" || argEpsg == "JGD7" || argEpsg == "6675") {
        return spatialid::CRS::KJGD2011_7;
    }
    else if (argEpsg == "JGD2011_8" || argEpsg == "JGD8" || argEpsg == "6676") {
        return spatialid::CRS::KJGD2011_8;
    }
    else if (argEpsg == "JGD2011_9" || argEpsg == "JGD9" || argEpsg == "6677") {
        return spatialid::CRS::KJGD2011_9;
    }
    else if (argEpsg == "JGD2011_10" || argEpsg == "JGD10" || argEpsg == "6678") {
        return spatialid::CRS::KJGD2011_10;
    }
    else if (argEpsg == "JGD2011_11" || argEpsg == "JGD11" || argEpsg == "6679") {
        return spatialid::CRS::KJGD2011_11;
    }
    else if (argEpsg == "JGD2011_12" || argEpsg == "JGD12" || argEpsg == "6680") {
        return spatialid::CRS::KJGD2011_12;
    }
    else if (argEpsg == "JGD2011_13" || argEpsg == "JGD13" || argEpsg == "6681") {
        return spatialid::CRS::KJGD2011_13;
    }
    else if (argEpsg == "JGD2011_14" || argEpsg == "JGD14" || argEpsg == "6682") {
        return spatialid::CRS::KJGD2011_14;
    }
    else if (argEpsg == "JGD2011_15" || argEpsg == "JGD15" || argEpsg == "6683") {
        return spatialid::CRS::KJGD2011_15;
    }
    else if (argEpsg == "JGD2011_16" || argEpsg == "JGD16" || argEpsg == "6684") {
        return spatialid::CRS::KJGD2011_16;
    }
    else if (argEpsg == "JGD2011_17" || argEpsg == "JGD17" || argEpsg == "6685") {
        return spatialid::CRS::KJGD2011_17;
    }
    else if (argEpsg == "JGD2011_18" || argEpsg == "JGD18" || argEpsg == "6686") {
        return spatialid::CRS::KJGD2011_18;
    }
    else if (argEpsg == "JGD2011_19" || argEpsg == "JGD19" || argEpsg == "6687") {
        return spatialid::CRS::KJGD2011_19;
    }
    return spatialid::CRS::kWGS84;
}

//! @brief 空間情報IDから　高さを無視した空間情報ID（z=0)を取得する
//! 
//! @param spatialId 空間情報ID
//! @return 高さを無視した空間情報ID（z=0)
//! 
std::string SpatialUtil::getXySpatialId(std::string spatialId) {
    auto string = std::string(spatialId);    // 分割対象の文字列
    auto separator = std::string("/");         // 区切り文字
    auto separator_length = separator.length(); // 区切り文字の長さ
    std::vector<std::string> list;// = std::vector<std::string>();

    if (separator_length == 0) {
        list.push_back(string);
    }
    else {
        auto offset = std::string::size_type(0);
        while (1) {
            auto pos = string.find(separator, offset);
            if (pos == std::string::npos) {
                list.push_back(string.substr(offset));
                break;
            }
            list.push_back(string.substr(offset, pos - offset));
            offset = pos + separator_length;
        }
    }
    return list[0] + "/0/" + list[2] + "/" + list[3];
}

