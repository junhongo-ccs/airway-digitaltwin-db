
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <bitset>
#include <regex>
#include <liblas/liblas.hpp>
#include <zlib.h>
#include "../../SpatialId/src/spatial_api.h"
#include "../../SpatialId/src/common/spatial_point.h"
#include "../commonLib/SpatialUtil.h"
#include <laszip/laszip_api.h>
#include "voxelWrite.h"

void las2laz(std::string& lasFileName) {
	clock_t ts = clock();

	std::string lazFileName = std::regex_replace(lasFileName, std::regex("\\.las"), ".laz");
	if (lasFileName == lazFileName) {
		throw std::runtime_error("LASファイルの形式が間違っています");
	}
	//std::cout << "lazFile:" << lazFileName << std::endl;

	//create reader
	laszip_POINTER laszip_reader;
	if (laszip_create(&laszip_reader)) {
		throw std::runtime_error("failed to create laszip reader");
	}

	//open reader
	laszip_BOOL is_compressed = 0;
	if (laszip_open_reader(laszip_reader, lasFileName.c_str(), &is_compressed))
	{
		throw std::runtime_error("failed to open laszip reader");
	}

	//get a ponter to the header
	laszip_header* header;

	if (laszip_get_header_pointer(laszip_reader, &header))
	{
		throw std::runtime_error("failed to get the header pointer from laszip reader");
	}

	// how many points does the file have
	laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

	//logは書かないように変更
	//logger->trace("points in las: {0}", npoints);

	// get a pointer to the points that will be read
	laszip_point* point;

	if (laszip_get_point_pointer(laszip_reader, &point))
	{
		throw std::runtime_error("failed to get point pointer from laszip reader");
	}

	// create the writer
	laszip_POINTER laszip_writer;
	if (laszip_create(&laszip_writer))
	{
		throw std::runtime_error("failed to create laszip writer");
	}

	// initialize the header for the writer using the header of the reader
	if (laszip_set_header(laszip_writer, header))
	{
		throw std::runtime_error("failed to set header for laszip writer");
	}

	// open the writer
	laszip_BOOL compress = true;

	if (laszip_open_writer(laszip_writer, lazFileName.c_str(), compress))
	{
		throw std::runtime_error("failed to open laszip writer");
	}

	// read the points

	laszip_I64 p_count = 0;
	while (p_count < npoints)
	{
		// read a point
		if (laszip_read_point(laszip_reader))
		{
			throw std::runtime_error("failed to read point");
		}

		// copy the point
		if (laszip_set_point(laszip_writer, point))
		{
			throw std::runtime_error("failed to set point");
		}

		// write the point

		if (laszip_write_point(laszip_writer))
		{
			throw std::runtime_error("failed to write point");
		}

		p_count++;
	}

	// close the writer
	if (laszip_close_writer(laszip_writer))
	{
		throw std::runtime_error("failed to close laszip writer");
	}

	// destroy the writer
	if (laszip_destroy(laszip_writer))
	{
		throw std::runtime_error("failed to destroy laszip writer");
	}

	// close the reader
	if (laszip_close_reader(laszip_reader))
	{
		throw std::runtime_error("failed to close laszip reader");
	}

	// destroy the reader

	if (laszip_destroy(laszip_reader))
	{
		throw std::runtime_error("failed to destroy laszip reader");
	}

	//削除処理
	/*if (!config.needKeepLas()) {
		std::filesystem::remove(lasFileName);
	}*/
	//logは書かない様に変更
	//logger->debug("+++ las2laz: p_count:{0}, elapsed time:{1:.2f}", p_count, (double)(clock() - ts) / CLOCKS_PER_SEC);
}

int  voxelWrite::lasReadVxlWrite(std::string inputFileId, std::string outputFileIdFull, std::string lasEpsg, std::string gzLevel, std::shared_ptr<spdlog::logger> logger, int* wsize_cnt)
{
	int wkClassfication = TYPE_ETC;
	/*  色は以下の設定
		水面　#4169e1　WATER
		ビル　#deb887　BUILDING
		道路　#a9a9a9　ROAD
		鉄道　#006400　RAILWAY
		電線　#ff1493　POWERLINE
		鉄塔　#ff0000　STEELTOWER*/
	liblas::Color color(65535, 60000, 60000);
	std::string buff = "etc.";
	size_t	position1 = inputFileId.find_last_of('_');
	size_t	position2 = inputFileId.find_last_of('.');
	size_t	position3 = inputFileId.find_last_of('/');
	
	//ファイル名から空間IDの文字列を求める
	std::string spatialId_tmp = "";
	if (position1 != std::string::npos) {
		buff = inputFileId.substr(position1 + 1, position2 - position1 - 1);
		spatialId_tmp = inputFileId.substr(position3 + 1, position1 - position3 -1);
		//建造種別毎に色を変更
		if (buff == "BUILDING") {
			wkClassfication = TYPE_BUILDING;
			color.SetRed(TYPE_BUILDING_RED);
			color.SetGreen(TYPE_BUILDING_GREEN);
			color.SetBlue(TYPE_BUILDING_BLUE);
		}
		if (buff == "STEELTOWER") {
			wkClassfication = TYPE_POWERTOWER;
			color.SetRed(TYPE_POWERTOWER_RED);
			color.SetGreen(TYPE_POWERTOWER_GREEN);
			color.SetBlue(TYPE_POWERTOWER_BLUE);
		}
		if (buff == "POWERLINE") {
			wkClassfication = TYPE_POWERLINE;
			color.SetRed(TYPE_POWERLINE_RED);
			color.SetGreen(TYPE_POWERLINE_GREEN);
			color.SetBlue(TYPE_POWERLINE_BLUE);
		}
		if (buff == "ROAD") {
			wkClassfication = TYPE_ROAD;
			color.SetRed(TYPE_ROAD_RED);
			color.SetGreen(TYPE_ROAD_GREEN);
			color.SetBlue(TYPE_ROAD_BLUE);
		}
		if (buff == "WATER") {
			wkClassfication = TYPE_WATER;
			color.SetRed(TYPE_WATER_RED);
			color.SetGreen(TYPE_WATER_GREEN);
			color.SetBlue(TYPE_WATER_BLUE);
		}
		if (buff == "RAILWAY") {
			wkClassfication = TYPE_RAILWAY;
			color.SetRed(TYPE_RAILWAY_RED);
			color.SetGreen(TYPE_RAILWAY_GREEN);
			color.SetBlue(TYPE_RAILWAY_BLUE);
		}

	}
	else {
		spatialId_tmp = inputFileId.substr(position3 + 1, position2 - position3 - 1);
	}
	//ファイル名を空間ID変換
	std::string spatialId_str = std::regex_replace(std::regex_replace(spatialId_tmp, std::regex("-"), "/"), std::regex("//"), "/-");

	std::vector<spatialid::Point> point_list;
    std::map<std::string, std::unique_ptr<spatialid::SpatialVoxel>> voxel_list;
	int	rtn_code = 0;	
	std::ifstream	ifs;
	ifs.open(inputFileId, std::ios::in | std::ios::binary);
	if (!ifs.is_open())
	{
		rtn_code = 1;
		return rtn_code;
	}

	liblas::Reader reader(ifs);
	liblas::Header const& header_in = reader.GetHeader();
	
	//lasを書くための準備
	liblas::Header header_out;
	liblas::SpatialReference srs = header_in.GetSRS();
	header_out.SetSRS(srs);
	header_out.SetDataFormatId(header_in.GetDataFormatId());
	//header_out.SetDataFormatId(liblas::ePointFormat3);
	//スケールを固定値から入力引継ぎに変更　2025.01.07
	header_out.SetScale(header_in.GetScaleX(), header_in.GetScaleY(), header_in.GetScaleZ());
	//header_out.SetScale(0.0001, 0.0001, 0.0001);
	header_out.SetOffset(header_in.GetOffsetX(), header_in.GetOffsetY(), header_in.GetOffsetZ());

	int	nPoints = header_in.GetPointRecordsCount();

	//lasのポイントをvectorにプールする(APIコールの準備)
	while (reader.ReadNextPoint())
	{
		liblas::Point const& p = reader.GetPoint();
		double x = p.GetX();
		double y = p.GetY();
		double z = p.GetZ();
		spatialid::Point point(x, y, z);
		point_list.push_back(point);
		
	}

	ifs.close();
	if (ifs.fail()) {
		rtn_code = 1;
		return rtn_code;
	}
	//clock_t t_read_end = clock();
	//bit配列作成のAPIコール     
	spatialid::get_global_spatial_voxel_on_points(point_list, voxel_list, SpatialUtil::getCrsFromArgEpsg(lasEpsg));
	//spatialid::get_global_spatial_voxel_on_points(point_list, voxel_list, static_cast<spatialid::CRS>(atoi(lasEpsg.c_str())));
	//複数グルーバル空間IDになったらwarningとする
	if (voxel_list.size() != 1) {
		logger->info("Multi SpatialID Warning!! {:s} ",inputFileId);
	}
	//戻りのvoxel配列に空間IDがなかったらエラー
	auto find_res = voxel_list.find(spatialId_str);
	if (find_res == voxel_list.end()) {
		rtn_code = 2;
		return rtn_code;
	}

	//サイズが2048^3にならない場合log出力する
	if (find_res->second->GetSize() != 1073741824) {
		logger->info("xyz-size wrong !! {:s} size = {:d}", find_res->first, find_res->second->GetSize());
		(*wsize_cnt)++;
	}

	size_t	pos = outputFileIdFull.find_last_of('.');
	
	if (outputFileIdFull.substr(pos) == ".vxl") {
		//vxl指定時はbit配列をそのまま出力
		std::ofstream ofs(outputFileIdFull, std::ios_base::out | std::ios_base::binary);
		if (ofs.fail()) {
			rtn_code = 3;
			return rtn_code;
		}

		ofs.write(reinterpret_cast<const char*>(find_res->second->GetPtr()), find_res->second->GetSize());
		if (ofs.fail()) {
			rtn_code = 3;
			return rtn_code;
		}

		ofs.close();
		if (ofs.fail()) {
			rtn_code = 3;
			return rtn_code;
		}
	}
	else {
		//gz指定の時はgzip圧縮する			
		std::string outputMode = "w" + gzLevel;
		//std::cout << "outputMode = " << outputMode << std::endl;
		gzFile zp;
		zp = gzopen(outputFileIdFull.c_str(), outputMode.c_str());
		if (zp == NULL) {
			rtn_code = 3;
			return rtn_code;
		}
		gzwrite(zp, reinterpret_cast<const char*>(find_res->second->GetPtr()), find_res->second->GetSize());
		gzclose(zp);
	}

	std::string outputLasFileId = outputFileIdFull.substr(0, pos) + ".las";
	// バイナリ形式でlas用outファイルオープン
	std::ofstream oflas(outputLasFileId, std::ios::out | std::ios::binary);
	if (oflas.fail()) {
		rtn_code = 3;
		return rtn_code;
	}

	liblas::Writer writer(oflas, header_out);
	liblas::Point lasPoint(&header_out);

	int aRecCount[5] = { 0, 0, 0, 0, 0 };
	double xmin = 999999999999.9, ymin = 999999999999.9, zmin = 999999999999.9,
		xmax = -999999999999.9, ymax = -999999999999.9, zmax = -999999999999.9;
	int pointCount = 0;
	//グローバル空間IDの8隅座標をget
	std::string spatialId = find_res->first;
	std::vector<spatialid::Point> vertex;
	spatialid::get_vertex_points_on_spatial_id(spatialId, vertex);

	//グローバル空間ID内の北西下ローカル空間IDをゲット
	spatialid::Zfxy local_spatial_zfxy_;
	local_spatial_zfxy_ = spatialid::get_spatial_zfxy_on_point(vertex[0].x, vertex[0].y, vertex[0].z, spatialid::kLocalZoom);

	const unsigned char* p = find_res->second->GetPtr();
	uint64_t bitCount = 0;
	uint64_t offset = 0;
	uint64_t wk_lonIndex = 0;
	uint64_t wk_latIndex = 0;
	uint64_t wk_altIndex = 0;
	int max_lon_index = -9999999999;
	int min_lon_index = 9999999999;
	//ビットが立っている配列順からローカル空間IDを計算、中心座標のpointを作成して出力
	for (int altIndex = 0; altIndex < 2048; altIndex++) {
		for (int latIndex = 0; latIndex < 2048; latIndex++) {
			for (int lonIndex = 0; lonIndex < 256; lonIndex++) {
				for (int bit_shift = 0; bit_shift < 8; bit_shift++) {
					if (*(p + offset) & (1 << bit_shift)) {
						bitCount++;
						if (lonIndex > wk_lonIndex) wk_lonIndex = lonIndex;
						if (latIndex > wk_latIndex) wk_latIndex = latIndex;
						if (altIndex > wk_altIndex) wk_altIndex = altIndex;
						std::string localSpatialId = std::to_string(spatialid::kLocalZoom)
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.alt_index + altIndex))
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.lon_index + (lonIndex * 8) + bit_shift))
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.lat_index + latIndex));
						spatialid::Zfxy zfxy(localSpatialId.c_str());

						if (max_lon_index < zfxy.lon_index) max_lon_index = zfxy.lon_index;
						if (min_lon_index > zfxy.lon_index) min_lon_index = zfxy.lon_index;
						//APIでローカル空間IDの中心点を求める
						spatialid::Point point = get_center_point_on_spatial_id(localSpatialId, SpatialUtil::getCrsFromArgEpsg(lasEpsg));
						//spatialid::Point point = get_center_point_on_spatial_id(localSpatialId, static_cast<spatialid::CRS>(atoi(lasEpsg.c_str())));
						lasPoint.SetX(point.x);
						lasPoint.SetY(point.y);
						lasPoint.SetZ(point.z);
						if (point.x > xmax) {
							xmax = point.x;
						}
						if (point.y > ymax) {
							ymax = point.y;
						}
						if (point.z > zmax) {
							zmax = point.z;
						}
						if (point.x < xmin) {
							xmin = point.x;
						}
						if (point.y < ymin) {
							ymin = point.y;
						}
						if (point.z < zmin) {
							zmin = point.z;
						}
						liblas::Classification cl;
						cl.SetClass(wkClassfication);
						lasPoint.SetClassification(cl);
						lasPoint.SetColor(color);
						writer.WritePoint(lasPoint);
						aRecCount[lasPoint.GetReturnNumber()]++;
						pointCount++;
					}

				}
				offset++;
			}
		}
	}
	header_out.SetMin(xmin, ymin, zmin);
	header_out.SetMax(xmax, ymax, zmax);
	header_out.SetPointRecordsCount(pointCount);
	header_out.SetPointRecordsByReturnCount(0, aRecCount[0]);
	header_out.SetPointRecordsByReturnCount(1, aRecCount[1]);
	header_out.SetPointRecordsByReturnCount(2, aRecCount[2]);
	header_out.SetPointRecordsByReturnCount(3, aRecCount[3]);
	header_out.SetPointRecordsByReturnCount(4, aRecCount[4]);
	writer.SetHeader(header_out);
	writer.WriteHeader();

	oflas.close();
	if (oflas.fail()) {
		rtn_code = 3;
		return rtn_code;
	}

	las2laz(outputLasFileId);

	return 0;
}

int  voxelWrite::lasReadLasWrite(std::string inputFileId, std::string outputFileIdFull, std::string lasEpsg, std::shared_ptr<spdlog::logger> logger)
{
	int wkClassfication = TYPE_ETC;
	/*  色は以下の設定
	    水面　#4169e1　WATER
		ビル　#deb887　BUILDING
		道路　#a9a9a9　ROAD
		鉄道　#006400　RAILWAY
		電線　#ff1493　POWERLINE
		鉄塔　#ff0000　STEELTOWER*/
	liblas::Color color(65535, 60000, 60000);
	std::string buff = "etc.";
	size_t	position1 = inputFileId.find_last_of('_');
	size_t	position2 = inputFileId.find_last_of('.');
	size_t	position3 = inputFileId.find_last_of('/');

	//ファイル名から空間IDの文字列を求める
	std::string spatialId_tmp = "";
	if (position1 != std::string::npos) {
		buff = inputFileId.substr(position1 + 1, position2 - position1 - 1);
		spatialId_tmp = inputFileId.substr(position3 + 1, position1 - position3 - 1);
		//建造種別毎に色を変更
		if (buff == "BUILDING") {
			wkClassfication = TYPE_BUILDING;
			color.SetRed(TYPE_BUILDING_RED);
			color.SetGreen(TYPE_BUILDING_GREEN);
			color.SetBlue(TYPE_BUILDING_BLUE);
		}
		if (buff == "STEELTOWER") {
			wkClassfication = TYPE_POWERTOWER;
			color.SetRed(TYPE_POWERTOWER_RED);
			color.SetGreen(TYPE_POWERTOWER_GREEN);
			color.SetBlue(TYPE_POWERTOWER_BLUE);
		}
		if (buff == "POWERLINE") {
			wkClassfication = TYPE_POWERLINE;
			color.SetRed(TYPE_POWERLINE_RED);
			color.SetGreen(TYPE_POWERLINE_GREEN);
			color.SetBlue(TYPE_POWERLINE_BLUE);
		}
		if (buff == "ROAD") {
			wkClassfication = TYPE_ROAD;
			color.SetRed(TYPE_ROAD_RED);
			color.SetGreen(TYPE_ROAD_GREEN);
			color.SetBlue(TYPE_ROAD_BLUE);
		}
		if (buff == "WATER") {
			wkClassfication = TYPE_WATER;
			color.SetRed(TYPE_WATER_RED);
			color.SetGreen(TYPE_WATER_GREEN);
			color.SetBlue(TYPE_WATER_BLUE);
		}
		if (buff == "RAILWAY") {
			wkClassfication = TYPE_RAILWAY;
			color.SetRed(TYPE_RAILWAY_RED);
			color.SetGreen(TYPE_RAILWAY_GREEN);
			color.SetBlue(TYPE_RAILWAY_BLUE);
		}
		
	} else {
		spatialId_tmp = inputFileId.substr(position3 + 1, position2 - position3 - 1);
	}

	//ファイル名を空間ID変換
	std::string spatialId_str = std::regex_replace(std::regex_replace(spatialId_tmp, std::regex("-"), "/"), std::regex("//"), "/-");
		
	std::vector<spatialid::Point> point_list;
	std::map<std::string, std::unique_ptr<spatialid::SpatialVoxel>> voxel_list;
	int	rtn_code = 0;
	std::ifstream	ifs;
	ifs.open(inputFileId, std::ios::in | std::ios::binary);
	if (!ifs.is_open())
	{
		rtn_code = 1;
		return rtn_code;
	}

	liblas::Reader reader(ifs);
	liblas::Header const& header_in = reader.GetHeader();

	liblas::Header header_out;
	liblas::SpatialReference srs = header_in.GetSRS();
	header_out.SetSRS(srs);
	header_out.SetDataFormatId(header_in.GetDataFormatId());
	//header_out.SetDataFormatId(liblas::ePointFormat3);
	//スケールを固定値から入力引継ぎに変更　2025.01.07
	header_out.SetScale(header_in.GetScaleX(), header_in.GetScaleY(), header_in.GetScaleZ());
	//header_out.SetScale(0.0001, 0.0001, 0.0001);
	header_out.SetOffset(header_in.GetOffsetX(), header_in.GetOffsetY(), header_in.GetOffsetZ());

	int	nPoints = header_in.GetPointRecordsCount();
	//std::cout << "number of points  " << nPoints << std::endl;

	// バイナリ形式でオープン
	std::ofstream ofs(outputFileIdFull, std::ios::out | std::ios::binary);
	if (ofs.fail()) {
		rtn_code = 3;
		return rtn_code;
	}
	liblas::Writer writer(ofs, header_out);
	liblas::Point lasPoint(&header_out);
	//lasのポイントをvectorにプールする(APIコールの準備)
	while (reader.ReadNextPoint()) {
		liblas::Point const& p = reader.GetPoint();	
		//lasPoint = p;
		double x = p.GetX();
		double y = p.GetY();
		double z = p.GetZ();
		spatialid::Point point(x, y, z);
		point_list.push_back(point);

	}

	ifs.close();
	if (ifs.fail()) {
		rtn_code = 1;
		return rtn_code;
	}
	//bit配列作成のAPIコール     
	spatialid::get_global_spatial_voxel_on_points(point_list, voxel_list, SpatialUtil::getCrsFromArgEpsg(lasEpsg));
	//spatialid::get_global_spatial_voxel_on_points(point_list, voxel_list, static_cast<spatialid::CRS>(atoi(lasEpsg.c_str())));
	//複数グルーバル空間IDになったらwarningとする
	if (voxel_list.size() != 1) {
		logger->info("Multi SpatialID Warning!! {:s} ", inputFileId);
	}
	//戻りのvoxel配列に空間IDがなかったらエラー
	auto find_res = voxel_list.find(spatialId_str);
	if (find_res == voxel_list.end()) {
		rtn_code = 2;
		return rtn_code;
	}

	int aRecCount[5] = { 0, 0, 0, 0, 0 };
	double xmin = 999999999999.9, ymin = 999999999999.9, zmin = 999999999999.9,
		xmax = -999999999999.9, ymax = -999999999999.9, zmax = -999999999999.9;
	int pointCount = 0;
	//グローバル空間IDの8隅座標をget
	std::string spatialId = find_res->first;
	std::vector<spatialid::Point> vertex;
	spatialid::get_vertex_points_on_spatial_id(spatialId, vertex);

	//グローバル空間ID内の北西下ローカル空間IDをゲット
	spatialid::Zfxy local_spatial_zfxy_;
	local_spatial_zfxy_ = spatialid::get_spatial_zfxy_on_point(vertex[0].x, vertex[0].y,vertex[0].z, spatialid::kLocalZoom);

	const unsigned char* p = find_res->second->GetPtr();
	uint64_t bitCount = 0;
	uint64_t offset = 0;
	uint64_t wk_lonIndex = 0;
	uint64_t wk_latIndex = 0;
	uint64_t wk_altIndex = 0;
	int max_lon_index = -9999999999;
	int min_lon_index = 9999999999;
	//ビットが立っている配列順からローカル空間IDを計算、中心座標のpointを作成して出力
	for (int altIndex = 0; altIndex < 2048; altIndex++) {
		for (int latIndex = 0; latIndex < 2048; latIndex++) {
			for (int lonIndex = 0; lonIndex < 256; lonIndex++) {
				for (int bit_shift = 0; bit_shift < 8; bit_shift++) {
					if (*(p + offset) & (1 << bit_shift)) {
						bitCount++;
						if (lonIndex > wk_lonIndex) wk_lonIndex = lonIndex;
						if (latIndex > wk_latIndex) wk_latIndex = latIndex;
						if (altIndex > wk_altIndex) wk_altIndex = altIndex;
						std::string localSpatialId = std::to_string(spatialid::kLocalZoom)
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.alt_index + altIndex))
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.lon_index + (lonIndex*8) + bit_shift))
							.append("/")
							.append(std::to_string(local_spatial_zfxy_.lat_index + latIndex));
						spatialid::Zfxy zfxy(localSpatialId.c_str());
			
						if (max_lon_index < zfxy.lon_index) max_lon_index = zfxy.lon_index;
						if (min_lon_index > zfxy.lon_index) min_lon_index = zfxy.lon_index;
						//APIでローカル空間IDの中心点を求める
						spatialid::Point point = get_center_point_on_spatial_id(localSpatialId, SpatialUtil::getCrsFromArgEpsg(lasEpsg));
						//spatialid::Point point = get_center_point_on_spatial_id(localSpatialId, static_cast<spatialid::CRS>(atoi(lasEpsg.c_str())));
						lasPoint.SetX(point.x);
						lasPoint.SetY(point.y);
						lasPoint.SetZ(point.z);
						if (point.x > xmax) {
							xmax = point.x;
						}
						if (point.y > ymax) {
							ymax = point.y;
						}
						if (point.z > zmax) {
							zmax = point.z;
						}
						if (point.x < xmin) {
							xmin = point.x;
						}
						if (point.y < ymin) {
							ymin = point.y;
						}
						if (point.z < zmin) {
							zmin = point.z;
						}
						liblas::Classification cl;
						cl.SetClass(wkClassfication);
						lasPoint.SetClassification(cl);
						lasPoint.SetColor(color);
						writer.WritePoint(lasPoint);
						aRecCount[lasPoint.GetReturnNumber()]++;
						pointCount++;
					}
					
				}
				offset++;			
			}
		}
	}
	header_out.SetMin(xmin, ymin, zmin);
	header_out.SetMax(xmax, ymax, zmax);
	header_out.SetPointRecordsCount(pointCount);
	header_out.SetPointRecordsByReturnCount(0, aRecCount[0]);
	header_out.SetPointRecordsByReturnCount(1, aRecCount[1]);
	header_out.SetPointRecordsByReturnCount(2, aRecCount[2]);
	header_out.SetPointRecordsByReturnCount(3, aRecCount[3]);
	header_out.SetPointRecordsByReturnCount(4, aRecCount[4]);
	writer.SetHeader(header_out);
	writer.WriteHeader();

	ofs.close();
	if (ofs.fail()) {
		rtn_code = 3;
		return rtn_code;
	}

	las2laz(outputFileIdFull);

	return 0;
}
