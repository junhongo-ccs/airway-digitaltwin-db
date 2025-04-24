#pragma once

//! @file SpatialUtil.h
//! @brief 空間情報関連ユーティリティクラス　ヘッダー
#include <string>
#include "../../SpatialId/src/spatial_api.h"
#include "proj.h"

//! @brief 空間情報関連ユーティリティクラス
class SpatialUtil
{
public:

	static spatialid::CRS getCrsFromArgEpsg(std::string argEpsg);

	static std::string getXySpatialId(std::string spatialId);

};
