#pragma once

//! @file CommonUtil.h
//! @brief 共通関数群

#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <regex>
#include <proj.h>
#include "CommonDefs.h"

class CommonUtil
{
public:

	//! 処理対象のLASファイルリストを取得する
	//! 
	//! @param folder チェック対象のフォルダ
	//! @param ptn ファイルパターン
	//! @param batchFile 絞りたいファイル名 ※指定された場合高々1ファイル
	//! @return 取得リスト
	static std::vector<std::string> getFeatureIdList(const std::string& folder, const std::string& spatialIdWithSlash);

	static std::string getSpatialIdFromFilename(const std::string& lasFileName);



	static void transform(int numPoints, double* pLon, double* pLat, PJ* proj);
	static int calcScale(std::string& epsg, PJ* proj=nullptr);
	static int calcScale(PJ* proj);


	//static std::string sjisToUtf8(std::string& str);

};