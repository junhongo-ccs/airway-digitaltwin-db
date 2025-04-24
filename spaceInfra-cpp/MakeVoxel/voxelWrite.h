#ifndef __VOXELWRITE_H_INCLUDED__
#define __VOXELWRITE_H_INCLUDED__
#pragma once
#include <string>
#include <vector>
#include "../commonLib/CommonConfig.h"

class voxelWrite {

	enum PolygonType {
		TYPE_ROAD = 2,
		TYPE_ETC = 4,
		TYPE_BUILDING = 6,
		TYPE_POWERLINE = 7,
		TYPE_POWERTOWER = 8,		
		TYPE_WATER = 9,
		TYPE_RAILWAY = 12
	};

	static const int TYPE_BUILDING_RED =     0xde * 256;
	static const int TYPE_BUILDING_GREEN =   0xb8 * 256;
	static const int TYPE_BUILDING_BLUE =    0x87 * 256;

	static const int TYPE_POWERTOWER_RED =   0xff * 256;
	static const int TYPE_POWERTOWER_GREEN = 0x00 * 256;
	static const int TYPE_POWERTOWER_BLUE =  0x00 * 256;

	static const int TYPE_POWERLINE_RED =    0xff * 256;
	static const int TYPE_POWERLINE_GREEN =  0x14 * 256;
	static const int TYPE_POWERLINE_BLUE =   0x93 * 256;

	static const int TYPE_ROAD_RED =         0xa9 * 256;
	static const int TYPE_ROAD_GREEN =       0xa9 * 256;
	static const int TYPE_ROAD_BLUE =        0xa9 * 256;

	static const int TYPE_WATER_RED =        0x41 * 256;
	static const int TYPE_WATER_GREEN =      0x69 * 256;
	static const int TYPE_WATER_BLUE =       0xe1 * 256;

	static const int TYPE_RAILWAY_RED =      0x00 * 256;
	static const int TYPE_RAILWAY_GREEN =    0x64 * 256;
	static const int TYPE_RAILWAY_BLUE =     0x00 * 256;


public:

//! @brief lasを読んでボクセルbit配列データを出力(拡張子がgzなら圧縮) 
//! @param インプットファイル名
//! @param アウトプットファイル名
//! @param  EPSGコード
//! @param  圧縮時のレベル
//! @param  logger
//! @param  int*
//! 
	static int lasReadVxlWrite(std::string, std::string, std::string, std::string, std::shared_ptr<spdlog::logger>, int*);

//! @brief lasのポイントを読んで存在するローカル空間の中心座標のポイントlasを出力 
//! @param インプットファイル名
//! @param アウトプットファイル名
//! @param  EPSGコード
//! 
	static int lasReadLasWrite(std::string, std::string, std::string, std::shared_ptr<spdlog::logger>);

};
#endif