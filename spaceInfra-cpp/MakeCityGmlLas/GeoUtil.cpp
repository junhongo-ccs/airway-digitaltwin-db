//! @file GeoUtil.cpp
//! @brief 地理関連のユーティリティ関数

#include <climits>
#include <regex>
#include <filesystem>
#include "GeoUtil.h"
#include <clipper2/clipper.h>
#include <spdlog/spdlog.h>

#include "..\..\SpatialId\src\\spatial_api.h"
#include "CommonUtil.h"
#include "Config.h"
#include <proj.h>

////! @brief ポイントがBBOXの中にあるかを判定する
////! 
////! @param px 点のX座標
////! @parma py 点のY座標
////! @param bbox BBOX
////! @return 判定結果(true: ポイントBBOX内にある）
//bool GeoUtil::pointInBBox(int px, int py, const BBox& bbox) {
//	return (px >= bbox.x1 && px <= bbox.x2 && py >= bbox.y1 && py <= bbox.y2);
//}
//
////! @brief ポイントがポリゴン内にあるかを判定
////! 
////! @param px 点のX座標
////! @param py 点のY座標
////! @param numPolygonPoint ポリゴンを構成するポイント数
////! @param ppPolygonPoints ポリゴンを構成するポイントの座標
////! @return 判定結果(true: ポリゴンの中にある）
//bool GeoUtil::pointInPolygon(int px, int py, int numPolygonPoints, GeoPoint**  ppPolygonPoints) {
//	
//	Clipper2Lib::Point<int> pt;
//	pt.x = px;
//	pt.y = py;
//
//	Clipper2Lib::Path<int> path;
//	path.reserve(numPolygonPoints);
//	for (int i = 0; i < numPolygonPoints; i++) {
//		path.push_back({ ppPolygonPoints[i]->x, ppPolygonPoints[i]->y });
//	}
//
//	Clipper2Lib::PointInPolygonResult result = Clipper2Lib::PointInPolygon(pt, path);
//	if (result == Clipper2Lib::PointInPolygonResult::IsOutside) {
//		return false;
//	}
//	else {
//		return true;
//	}
//
//}
//
////! @brief 二つのBBOXが交差しているかをチェック
////! 
////! @param bbox1 １番目のBBOX
////! @param bbox2 ２番目のBBOX
////! @return 結果（true: 交差している）
//bool GeoUtil::bboxIntersected(const BBox& bbox1, const BBox& bbox2) {
//	int xmin = (bbox1.x1 > bbox2.x1 ? bbox1.x1 : bbox2.x1);
//	int xmax = (bbox1.x2 <= bbox2.x2 ? bbox1.x2 : bbox2.x2);
//	int ymin = (bbox1.y1 > bbox2.y1 ? bbox1.y1 : bbox2.y1);
//	int ymax = (bbox1.y2 <= bbox2.y2 ? bbox1.y2 : bbox2.y2);
//
//	return (xmin <= xmax && ymin <= ymax);
//}
//
////! @brief 点列のBBOXを求める
////! 
////! @param nPoints チェック対象のポイント数
////! @param pPoints チェック対象のポイントの座標(OGRRawPoint形式)
////! @param 求められたBBOX
//BBox GeoUtil::getBBox(int nPoints, const OGRRawPoint* pPoints) {
//	
//	int xmin = std::numeric_limits<int>::max();
//	int ymin = xmin;
//	int xmax = std::numeric_limits<int>::min();
//	int ymax = xmax;
//	
//	for (int i = 0; i < nPoints; i++) {
//		int x = (int)pPoints[i].x;
//		int y = (int)pPoints[i].y;
//
//		if (x < xmin) xmin = x;
//		if (x > xmax) xmax = x;
//		if (y < ymin) ymin = y;
//		if (y > ymax) ymax = y;
//	}
//
//	return { xmin,ymin,xmax,ymax };
//
//}
//
////! @brief 点列のBBOXを求める
////! 
////! @param nPoints チェック対象のポイント数
////! @param pPoints チェック対象のポイントの座標(GeoPoint形式)
////! @param 求められたBBOX
//BBox GeoUtil::getBBox(int nPoints, const GeoPoint** ppPoints) {
//
//	int xmin = std::numeric_limits<int>::max();
//	int ymin = xmin;
//	int xmax = std::numeric_limits<int>::min();
//	int ymax = xmax;
//
//	for (int i = 0; i < nPoints; i++) {
//		int x = (int)ppPoints[i]->x;
//		int y = (int)ppPoints[i]->y;
//
//		if (x < xmin) xmin = x;
//		if (x > xmax) xmax = x;
//		if (y < ymin) ymin = y;
//		if (y > ymax) ymax = y;
//	}
//
//	return { xmin,ymin,xmax,ymax };
//
//}
//
//BBox GeoUtil::getBBox(std::vector<GeoPoint>& points) {
//	int xmin = std::numeric_limits<int>::max();
//	int ymin = xmin;
//	int xmax = std::numeric_limits<int>::min();
//	int ymax = xmax;
//
//	for (auto& pt : points) {
//		int x = (int)pt.x;
//		int y = (int)pt.y;
//
//		if (x < xmin) xmin = x;
//		if (x > xmax) xmax = x;
//		if (y < ymin) ymin = y;
//		if (y > ymax) ymax = y;
//	}
//
//	return { xmin,ymin,xmax,ymax };
//
//}
//
////! @brief ポリゴンのBBOXがほかのBBOXと交差しているかをチェック
////! 
////! @param nPoints ポリゴン・ポリラインのポイント数
////! @param pPoints ポリゴン・ポリラインの点の座標(OGRRawPoint形式)
////! @param bbox2 バウンディングボックス（空間IDのもの）
////! @param closed trueならポリゴン
////! @return 結果（true: 交差している）
//bool GeoUtil::bboxIntersected(int nPoints, const OGRRawPoint* pPoints, const BBox& bbox2, bool closed) {
//	BBox bbox1 = getBBox(nPoints, pPoints);
//
//	//まずbboxが交差しているかを先にチェック。こうしたほうが性能的によさそうなdので。
//	bool rc = bboxIntersected(bbox1, bbox2);
//	if (!rc) {
//		return rc;
//	}
//
//	//ポリゴン（ポリライン）がBBOXと交差しているかをチェック
//	Clipper2Lib::Clipper64 clipper64;
//
//	Clipper2Lib::Paths64 subject, clip, solution, openSolution;
//
//	Clipper2Lib::Path64 path1;
//	for (int i = 0; i < nPoints; i++) {
//		path1.push_back({ (int64_t)(pPoints[i].x), (int64_t)(pPoints[i].y) });
//	}
//	subject.push_back(path1);
//
//	if (closed) {
//		clipper64.AddSubject(subject);
//	}
//	else {
//		clipper64.AddOpenSubject(subject);
//	}
//
//
//	Clipper2Lib::Path64 path2;
//	path2.push_back({ (int64_t)bbox2.x1, (int64_t)bbox2.y1 });
//	path2.push_back({ (int64_t)bbox2.x2, (int64_t)bbox2.y1 });
//	path2.push_back({ (int64_t)bbox2.x2, (int64_t)bbox2.y2 });
//	path2.push_back({ (int64_t)bbox2.x1, (int64_t)bbox2.y2 });
//	//path2.push_back({ (int64_t)bbox2.x1, (int64_t)bbox2.y1 });
//	clip.push_back(path2);
//
//	clipper64.AddClip(clip);
//
//	
//	bool ret = clipper64.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, solution, openSolution);
//
//	//std::cout << "+++ ret:" << ret << ",solution.size:" << solution.size() << ", " << openSolution.size() << std::endl;
//
//	if (ret) {
//		return closed ? solution.size() > 0 : openSolution.size() > 0;
//	}
//
//	//return ret && solution.size() > 0;
//	return false;
//}
//
////! @brief ポリゴンのBBOXがほかのBBOXと交差しているかをチェック
////! 
////! @param nPoints ポリゴンのポイント数
////! @param pPoints ポリゴンの点の座標(OGRRawPoint形式)
////! @return 結果（true: 交差している）
//bool GeoUtil::bboxIntersected(int nPoints, const GeoPoint** ppPoints, const BBox& bbox2, bool closed) {
//	BBox bbox1 = getBBox(nPoints, ppPoints);
//
//	//まずbboxが交差しているかを先にチェック。こうしたほうが性能的によさそうなdので。
//	bool rc = bboxIntersected(bbox1, bbox2);
//	if (!rc) {
//		return rc;
//	}
//
//	//ポリゴン（ポリライン）がBBOXと交差しているかをチェック
//	Clipper2Lib::Clipper64 clipper64;
//
//	//Clipper2Lib::Paths64 subject, clip, solution;
//	Clipper2Lib::Paths64 subject, clip, solution, openSolution;
//
//	Clipper2Lib::Path64 path1;
//	for (int i = 0; i < nPoints; i++) {
//		path1.push_back({ (int64_t)(ppPoints[i]->x), (int64_t)(ppPoints[i]->y) });
//	}
//	subject.push_back(path1);
//
//	if (closed) {
//		clipper64.AddSubject(subject);
//	}
//	else {
//		clipper64.AddOpenSubject(subject);
//	}
//
//
//	Clipper2Lib::Path64 path2;
//	path2.push_back({ (int64_t)bbox2.x1, (int64_t)bbox2.y1 });
//	path2.push_back({ (int64_t)bbox2.x2, (int64_t)bbox2.y1 });
//	path2.push_back({ (int64_t)bbox2.x2, (int64_t)bbox2.y2 });
//	path2.push_back({ (int64_t)bbox2.x1, (int64_t)bbox2.y2 });
//	clip.push_back(path2);
//
//	//solution = Clipper2Lib::Intersect(subject, clip, Clipper2Lib::FillRule::EvenOdd);
//	clipper64.AddClip(clip);
//
//
//	bool ret = clipper64.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, solution, openSolution);
//
//	//std::cout << "+++ ret:" << ret << ",solution.size:" << solution.size() << ", " << openSolution.size() << std::endl;
//
//	if (ret) {
//		return closed ? solution.size() > 0 : openSolution.size() > 0;
//	}
//
//
//	//return solution.size() > 0;
//	return false;
//}
//
//bool GeoUtil::polygonIntersected(int nPointsOfPolygon, const GeoPoint** ppPointsOfPolygon,
//	int nPointsOfPolyline, const GeoPoint** ppPointsOfPolyline, bool closed) {
//
//	//ポリゴン（ポリライン）がBBOXと交差しているかをチェック
//	Clipper2Lib::Clipper64 clipper64;
//
//	//Clipper2Lib::Paths64 subject, clip, solution;
//	Clipper2Lib::Paths64 subject, clip, solution, openSolution;
//
//	Clipper2Lib::Path64 path1;
//	for (int i = 0; i < nPointsOfPolyline; i++) {
//		path1.push_back({ (int64_t)(ppPointsOfPolyline[i]->x), (int64_t)(ppPointsOfPolyline[i]->y) });
//	}
//	subject.push_back(path1);
//
//	if (closed) {
//		clipper64.AddSubject(subject);
//	}
//	else {
//		clipper64.AddOpenSubject(subject);
//	}
//
//
//	Clipper2Lib::Path64 path2;
//	for (int i = 0; i < nPointsOfPolygon; i++) {
//		path2.push_back({ (int64_t)(ppPointsOfPolygon[i]->x), (int64_t)(ppPointsOfPolygon[i]->y) });
//	}
//	clip.push_back(path2);
//
//	//solution = Clipper2Lib::Intersect(subject, clip, Clipper2Lib::FillRule::EvenOdd);
//	clipper64.AddClip(clip);
//
//
//	bool ret = clipper64.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, solution, openSolution);
//
//	//std::cout << "+++ ret:" << ret << ",solution.size:" << solution.size() << ", " << openSolution.size() << std::endl;
//
//	if (ret) {
//		return closed ? solution.size() > 0 : openSolution.size() > 0;
//	}
//
//
//	//return solution.size() > 0;
//	return false;
//
//}
//
//
//
////! @brief ファイル名から空間IDを取得（f:0に固定）
////! 
////! @param lasFileName ファイル名
////! @return 取得できた空間ID
//std::string GeoUtil::getSpatialIdFromFilename(const std::string& lasFileName) {
//	std::regex re(R"(^(\d+)-(-?\d+)-(\d+)-(\d+).[a-z]{3}$)");
//	std::smatch match;
//
//	if (std::regex_match(lasFileName, match, re)) {
//		std::string result(match[1]);
//		result.append("/").append(match[2]).append("/").append(match[3]).append("/").append(match[4]);
//		return result;
//	}
//
//	return "";
//}
//
////! @brief 空間IDのf値を算出する
////! 
////! @param z 高度座標
////! @param zoom ズームレベル
////! @return 算出されたf値
//int GeoUtil::calcZIndex(double z, int zoom) {
//	double alt_resolution = std::pow(2, 25) / std::pow(2, zoom);
//	int f = static_cast<int>(std::floor(z / alt_resolution));
//
//	return f;
//}
//
////! @brief 緯度経度からゼンリンのマップIDを取得する
////! 
////! @param 緯度
////! @param 経度
////! @return 取得できたマップID
//int GeoUtil::getZenrinMapId(double lat, double lon, PJ* proj)
//{
//	if (proj != nullptr) {
//		PJ_COORD input_coords, output_coords;
//
//		input_coords = proj_coord(lat, lon, 0, 0);
//		output_coords = proj_trans(proj, PJ_FWD, input_coords);
//		lat = output_coords.xy.x;
//		lon = output_coords.xy.y;
//	}
//
//	double  latBase, lonBase, w_buff;
//	static double  mapUint = 0.0625;  //ゼンリン地図の１枠単位　1度/8/2
//	int     lat1Mesh, lon1Mesh, lat2Mesh, lon2Mesh, latZenPos, lonZenPos, ZenPos, int_buff;
//	div_t   div_t_buff;
//
//	latBase = lat * 1.5;    //緯度は１次メッシュの単位が40分なので1度単位に変換
//	lonBase = lon - 100.0;  //経度は単位が１度で起点が100度のため
//
//	lat1Mesh = (int)latBase;    //一次メッシュは切り捨て整数化
//	lon1Mesh = (int)lonBase;
//
//	w_buff = latBase / mapUint;  //ゼンリン単位の総数を計算
//	int_buff = (int)w_buff;
//	div_t_buff = div(int_buff, 2);
//	latZenPos = div_t_buff.rem;     //２で割り切れたら2次メッシュ内南側、それ以外北側
//	lat2Mesh = div_t_buff.quot % 8; //さらに８で割った余りが2次メッシュコード
//
//
//	w_buff = lonBase / mapUint;
//	int_buff = (int)w_buff;
//	div_t_buff = div(int_buff, 2);
//	lonZenPos = div_t_buff.rem;     //２で割り切れたら2次メッシュ内西側、それ以外東側
//	lon2Mesh = div_t_buff.quot % 8;
//
//
//	//2次メッシュ内の緯度と経度の位置からゼンリンコードを決定
//	if (latZenPos == 0) {
//		if (lonZenPos == 0) {
//			ZenPos = 3;
//		}
//		else {
//			ZenPos = 4;
//		}
//	}
//	else if (lonZenPos == 0) {
//		ZenPos = 1;
//	}
//	else {
//		ZenPos = 2;
//	};
//
//
//	//それぞれのコードから１つのコード作成
//	return (lat1Mesh * 100000 + lon1Mesh * 1000 + lat2Mesh * 100 + lon2Mesh * 10 + ZenPos);
//}
//
////! @brief 整数に変換するための係数を求める
////! 
////! @param 参考値（取りうる値の絶対値の最大値を指定)
////! @return 計算結果
//int GeoUtil::calcScale(double refValue) {
//	if (refValue < 0) 
//		refValue = -refValue;
//	if (refValue < 1.0e-6)
//		return 1;
//
//	if (refValue <= 180.0)//緯度経度とみなしてミリ秒単位にする
//		return DMS_SCALE;
//
//	int intMax = INT_MAX >> 4;
//	int factor = static_cast<int>(intMax / refValue);
//	int scale = 1;
//
//	while (factor / 10 > 0) {
//		scale *= 10;
//		factor /= 10;
//	}
//
//	return scale;
//}
//
//int GeoUtil::calcScale2(std::string& epsg) {
//	int code = std::atoi(epsg.c_str());
//
//	//平面直角座標系
//	if (code >= 6669 && code <= 6687 || code >= 2443 && code <= 2461) {
//		return 1000;//実際に100でも十分だが
//	}
//	else if (code >= 6688 && code <= 6692 || code >= 3097 && code <= 3101) {//UTM
//		return 100; //適当。今のところ利用しない
//	}
//	else if (code == 3857) {
//		return 100; //適当。今のところ利用しない
//	}
//	else {
//		return DMS_SCALE;
//	}
//
//}
//
//int GeoUtil::getZenrinMapIdEx(double lat, double lon) {
//	int numLon = static_cast<int>((lon - 100) * 16); // 16 3600 / 225); //2次メッシュの幅：450秒
//	int numLat = static_cast<int>(lat * 24); // 3600 / 150); //2次メッシュの高さ：300秒
//
//	int lon1 = numLon >> 4;
//	int lon3 = numLon % 2;
//	int lon2 = (numLon - lon1 * 16 - lon3) >> 1;
//
//	int lat1 = numLat >> 4;
//	int lat3 = numLat % 2;
//	int lat2 = (numLat - lat1 * 16 - lat3) >> 1;
//
//	int meshCd = lat1 * 100000 + lon1 * 1000 + lat2 * 100 + lon2 * 10 + getLastDigit(lat3, lon3);
//
//	return meshCd;
//
//}
//
//
//
//void GeoUtil::getAllCityGmlFileList(std::vector<std::string>& cityGmlFileList) {
//	Config& config = Config::getInstance();
//	std::shared_ptr<spdlog::logger> logger = spdlog::get(config.getLoggerName());
//
//	std::string cityGmlInputRoot = config.getCityGmlFileFolder();
//
//	std::vector<std::string> fzidList;
//
//	for (std::string& lasFile : lasFileList) {
//		//ファイル名から空間IDを求める
//		std::string spatialId = getSpatialIdFromFilename(lasFile);
//		if (spatialId.empty()) {
//			throw std::runtime_error("空間IDが取得できません");
//		}
//
//		//４隅から対応するゼンリンメッシュIDを求める。メッシュID計算に経度緯度が必要なので、空間IDから４隅の緯度経度を求める
//		std::vector<Point> lonlatList;
//		get_vertex_points_on_spatial_id(spatialId.c_str(), lonlatList, static_cast<CRS>(std::atoi(config.getMapEpsg().c_str())));
//		std::vector<int> meshcdList;
//		for (auto& point : lonlatList) {
//			int id = GeoUtil::getZenrinMapId(point.y, point.x, proj); //引数：緯度、経度
//
//			std::vector<int>::iterator iter = std::find(meshcdList.begin(), meshcdList.end(), id);
//			if (iter == meshcdList.end()) {
//				meshcdList.push_back(id);
//			}
//		}
//
//		//近傍も追加
//		for (int meshcd : meshcdList) {
//			//std::cout << "meshcd:" << meshcd << std::endl; //## for debug
//			std::string curCd("S");
//			curCd += std::to_string(meshcd);
//
//			if (std::find(fzidList.begin(), fzidList.end(), curCd) == fzidList.end()) {
//				fzidList.push_back(curCd);
//			}
//
//			if (expandFlag) {//8近傍追加
//				std::vector<int> list = getNearestMeshcds(meshcd);
//				for (int neighbor : list) {
//					std::string strCd("S");
//					strCd += std::to_string(neighbor);
//
//					std::vector<std::string>::iterator iter = std::find(fzidList.begin(), fzidList.end(), strCd);
//					if (iter == fzidList.end()) {
//						fzidList.push_back(strCd);
//					}
//
//				}
//			}
//		}
//		
//	}
//
//	return fzidList;
//}

//std::vector<int> GeoUtil::getNearestMeshcds(int meshcd) {
//	std::vector<int> list;
//	//----------------------------------------------------
//	// 5238414, 5238423, 5238424 ,5238433
//	//----------------------------------------------------    
//	// 5238312,|5238321, 5238322|,5238331
//	// 5238314,|5238323, 5238324|,5238333
//	// ---------------------------------------------------
//	// 5238212, 5238221, 5238222 ,5238231
//	//----------------------------------------------------
//
//	int lastDigit = meshcd % 10;
//
//	int lat1 = meshcd / 100000;
//	int lat2 = meshcd / 100 % 10;
//	int lat3 = 0;
//
//	int lon1 = meshcd / 1000 % 100;
//	int lon2 = meshcd / 10 % 10;
//	int lon3 = 0;
//
//	if (lastDigit == 1 || lastDigit == 2) {
//		lat3 = 1;
//	}
//	if (lastDigit == 2 || lastDigit == 4) {
//		lon3 = 1;
//	}
//
//	//------ 近傍 ------
//	std::array<int, 3> arrLat{ lat1, lat2, lat3 };
//	std::array<int, 3> arrLon{ lon1, lon2, lon3 };
//
//	//上
//	arrLat = calcNewLatlon(lat1, lat2, lat3, true);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//下
//	arrLat = calcNewLatlon(lat1, lat2, lat3, false);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//左
//	arrLat = { lat1, lat2, lat3 };
//	arrLon = calcNewLatlon(lon1, lon2, lon3, false);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//右
//	arrLon = calcNewLatlon(lon1, lon2, lon3, true);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//左上
//	arrLat = calcNewLatlon(lat1, lat2, lat3, true);
//	arrLon = calcNewLatlon(lon1, lon2, lon3, false);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//右上
//	arrLat = calcNewLatlon(lat1, lat2, lat3, true);
//	arrLon = calcNewLatlon(lon1, lon2, lon3, true);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//左下
//	arrLat = calcNewLatlon(lat1, lat2, lat3, false);
//	arrLon = calcNewLatlon(lon1, lon2, lon3, false);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	//右下
//	arrLat = calcNewLatlon(lat1, lat2, lat3, false);
//	arrLon = calcNewLatlon(lon1, lon2, lon3, true);
//	list.push_back(arrLat[0] * 100000 + arrLon[0] * 1000 + arrLat[1] * 100 + arrLon[1] * 10 +
//		getLastDigit(arrLat[2], arrLon[2]));
//
//	return list;
//}
//
//int GeoUtil::getLastDigit(int lat3, int lon3) {
//	int last;
//	if (lat3 == 0 && lon3 == 0)
//		last = 3;
//	else if (lat3 == 0 && lon3 == 1)
//		last = 4;
//	else if (lat3 == 1 && lon3 == 1)
//		last = 2;
//	else
//		last = 1;
//
//	return last;
//}
//
//std::array<int, 3> GeoUtil::calcNewLatlon(int part1, int part2, int part3, bool plusFlag) {
//	if (plusFlag) {
//		part3++;
//		if (part3 > 1) {
//			part3 = 0;
//			part2++;
//			if (part2 > 7) {
//				part2 = 0;
//				part1++;
//			}
//		}
//	}
//	else {
//		part3--;
//		if (part3 < 0) {
//			part3 = 1;
//			part2--;
//			if (part2 < 0) {
//				part2 = 7;
//				part1--;
//			}
//		}
//	}
//
//	return { part1, part2, part3 };
//}
//
//BBox GeoUtil::getBBox(std::string& meshCdStr) {
//	int meshcode;
//	if (CommonUtil::startsWith(meshCdStr, "S")) {
//		meshcode = std::atoi(meshCdStr.substr(1, meshCdStr.length() - 1).c_str());
//	}
//	else {
//		meshcode = std::atoi(meshCdStr.c_str());
//	}
//
//	return getBBox(meshcode);
//
//}
//
//BBox GeoUtil::getBBox(int meshcode) {
//	int submeshcd = 0;
//
//	if (meshcode >= 1000000 && meshcode <= 9999999) {
//		submeshcd = meshcode % 10;
//		meshcode /= 10;
//
//		if (submeshcd <= 0 || submeshcd > 4) {
//			throw std::runtime_error("サポートされていないメッシュコード");
//		}
//	}
//	else if (meshcode < 100000 || meshcode > 9999999) {
//		throw std::runtime_error("サポートされていないメッシュコード");
//	}
//
//	BBox bbox;
//	const int BASE_WIDTH = 3600;
//	const int BASE_HEIGHT = 2400;
//
//	int meshW = BASE_WIDTH >> 3;
//	int meshH = BASE_HEIGHT >> 3;
//
//	int latCd1 = meshcode / 10000; //1次メッシュ・経度の部分
//	int lonCd1 = (meshcode / 100) % 100; //1次メッシュ・緯度の部分
//	int latCd2 = (meshcode / 10) % 10; //2次メッシュ・経度の部分
//	int lonCd2 = meshcode % 10; //2次メッシュ・緯度の部分
//
//	int lon = lonCd1 * 8 + lonCd2;
//	int lat = latCd1 * 8 + latCd2;
//
//	if (submeshcd <= 0) {
//		bbox.x1 = static_cast<long>(((double)lon * meshW / 3600 + 100) * DMS_SCALE);
//		bbox.y1 = static_cast<long>(((double)lat * meshH / 3600) * DMS_SCALE);
//		bbox.x2 = static_cast<long>(((double)(lon + 1) * meshW / 3600 + 100) * DMS_SCALE);
//		bbox.y2 = static_cast<long>(((double)(lat + 1) * meshH / 3600) * DMS_SCALE);
//	}
//	else {
//		double x1 = (double)lon * meshW / 3600 + 100;
//		double y1 = (double)lat * meshH / 3600;
//		double x2, y2;
//		int halfMeshW = meshW >> 1;
//		int halfMeshH = meshH >> 1;
//		double diffW = (double)halfMeshW / 3600;
//		double diffH = (double)halfMeshH / 3600;
//		switch (submeshcd) {
//		case 1:
//			y1 += diffH;
//			break;
//		case 2:
//			x1 += diffW;
//			y1 += diffH;
//			break;
//		case 4:
//			x1 += diffW;
//			break;
//		}
//
//		x2 = x1 + diffW;
//		y2 = y1 + diffH;
//
//		////------------ debug
//		//int fzid1 = getZenrinMapId(y1+0.00001, x1+0.00001);
//		//int fzid2 = getZenrinMapId(y2-0.00001, x2-0.00001);
//		//std::cout << "fzid1:" << fzid1 << ", fzid2:" << fzid2 << std::endl;
//		//std::cout << "fzidEx1:" << getZenrinMapIdEx(y1+0.00001, x1+0.00001) << ", fzidEx2:" << getZenrinMapIdEx(y2-0.00001,x2-0.00001) << std::endl;
//		////--------------
//
//		bbox.x1 = static_cast<long>(x1 * DMS_SCALE);
//		bbox.y1 = static_cast<long>(y1 * DMS_SCALE);
//		bbox.x2 = static_cast<long>(x2 * DMS_SCALE);
//		bbox.y2 = static_cast<long>(y2 * DMS_SCALE);
//	}
//
//	return bbox;
//}
//
//std::vector<std::string> GeoUtil::getFileNameFromSpatialId(std::vector<std::string>& spatialIdList, bool checkExtraFlag) {
//	std::vector<std::string> list;
//	Config& config = Config::getInstance();
//	std::string lasFileFolder = config.getLasPointFileFolder();
//
//	std::vector<std::string> batchFolderList;
//	if (checkExtraFlag) {
//		batchFolderList = CommonUtil::getBatchFolderList(config.getPointInRoot());
//	}
//	
//	int minF = -1;//鉄塔用なので0でもいいかもしれない
//	int maxF = 15; //
//	for (auto& id : spatialIdList) {
//		//std::cout << "spatialId:" << id << std::endl;
//		Zfxy zfxy(id.c_str()); //入力fが0のはず
//
//		for (int find = minF; find <= maxF; find++) {
//			std::string str;
//			str += std::to_string(zfxy.zoom);
//			str += "-";
//			str += std::to_string(find);
//			str += "-";
//			str += std::to_string(zfxy.lon_index);
//			str += "-";
//			str += std::to_string(zfxy.lat_index);
//			if (config.getInputFormat() == FileFormat::FMT_CSV)
//				str += ".csv";
//			else
//				str += ".las";
//
//			std::string fullPath(CommonUtil::pathJoin(lasFileFolder, str));
//			std::filesystem::path path(fullPath);
//
//			if (std::filesystem::exists(path)) {
//				//std::cout << fullPath << std::endl;
//				list.push_back(fullPath);
//			}
//			else if (batchFolderList.size() > 0){
//				//ほかのチェック
//				for (std::string& batchFolder : batchFolderList) {
//					std::string tmpFolder = config.getLasPointFileFolder(batchFolder);
//					std::string tmpFullPath(CommonUtil::pathJoin(tmpFolder, str));
//					std::filesystem::path tmpPath(tmpFullPath);
//					if (std::filesystem::exists(tmpPath)) {
//						list.push_back(tmpFullPath);
//						break;
//					}
//				}
//			}
//		}
//	}
//
//	return list;
//}

//void GeoUtil::getCityGmlFileInfoFromFileName(const std::string& cityGmlFileName, CityGmlFileInfo& cityGmlFileInfo) {
//	Config& config = Config::getInstance();
//
//	std::regex re(R"(^(\d+)_([a-zA-Z0-9]+)_(\d+)_.+\.gml$)");
//	std::smatch match;
//
//	std::filesystem::path filePath(cityGmlFileName);
//	std::string fileName = filePath.filename().string();
//
//	if (std::regex_match(fileName, match, re)) {
//		cityGmlFileInfo.meshcd = match[1];
//		//cityGmlFileInfo.featureId = match[2];
//		std::string featureId = match[2];
//		if (featureId == "bldg") {
//			cityGmlFileInfo.featureId = "BUILDING";
//		}
//		else if (featureId == "tran") {
//			cityGmlFileInfo.featureId = "ROAD";
//		}
//
//		cityGmlFileInfo.crs = match[3];
//	}
//
//}