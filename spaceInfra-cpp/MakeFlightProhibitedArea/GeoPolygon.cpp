//! @file GeoPolygon.cpp
//! @brief ポリゴンクラス関連のcppファイル
 
#include "GeoPolygon.h"
#include "Config.h"
#include "earcut/earcut.hpp"

//! @brief コンストラクタ
GeoPolygon::GeoPolygon() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());


}

//! @brief デストラクタ
GeoPolygon::~GeoPolygon() {
}


//! @brief 属性データを追加する
//! 
//! @param num 属性数
//! @param pAttribute 属性データの先頭へのポインタ
void GeoPolygon::addAttribute(int num, GeoAttribute* pAttribute) {
	attributeContainer.addObjects(num, pAttribute);

}
//! @brief ポリゴンのポイントを追加する
//! 
//! @param num ポイント数
//! @param pPoint ポイント先頭へのポインタ
void GeoPolygon::addPoints(int num, GeoPoint* pPoint) {
	pointContainer.addObjects(num, pPoint);
}

void GeoPolygon::decidePlaneType() {
	if (getPointCount() < 3) {
		logger->debug("id:{0}, 点数:{1}", polygonId, getPointCount());
		return;
	}

	//最初の３点を利用して外積を求める
	GeoPoint* p1 = getPoint(0);
	GeoPoint* p2 = getPoint(1);
	GeoPoint* p3 = getPoint(2);

	double a1 = p2->x - p1->x;
	double a2 = p2->y - p1->y;
	double a3 = p2->z - p1->z;

	double b1 = p3->x - p2->x;
	double b2 = p3->y - p2->y;
	double b3 = p3->z - p2->z;

	double productX = a2 * b3 - a3 * b2;
	double productY = a3 * b1 - a1 * b3;
	double productZ = a1 * b2 - a2 * b1;

	//法線とX, Y, Z軸の角度を求める
	double dist = std::sqrt(productX * productX + productY * productY + productZ * productZ);

	double angleZ = std::acos(productZ / dist) * 180 / M_PI;
	double angleY = std::acos(productY / dist) * 180 / M_PI;
	double angleX = std::acos(productX / dist) * 180 / M_PI;

	if (angleZ > 90) {
		angleZ = 180 - angleZ;
	}
	if (angleY > 90) {
		angleY = 180 - angleY;
	}
	if (angleX > 90) {
		angleX = 180 - angleX;
	}

	if (angleZ < angleX && angleZ < angleY) {
		planeType = PlaneType::PLANE_XY;
	}
	else if (angleY < angleX && angleY < angleZ) {
		planeType = PlaneType::PLANE_ZX;
	}
	else {
		planeType = PlaneType ::PLANE_YZ;
	}

}


void GeoPolygon::doTriangulation() {
	using Point2D = std::array<double, 2>;
	std::vector<Point2D> vertices;

	int pointCount = getPointCount();
	GeoPoint** ppPoint = getPointsRef();
	std::vector<int> indices;

	double eps = 1.0e-7;
	if (pointCount == 3 || pointCount == 4 && std::fabs(ppPoint[0]->x - ppPoint[3]->x) <= eps &&
		std::fabs(ppPoint[0]->y - ppPoint[3]->y) <= eps &&
		std::fabs(ppPoint[0]->z - ppPoint[3]->z) <= eps) {

		//すでに三角形
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
	}
	else {
		for (int i = 0; i < pointCount; i++) {
			GeoPoint* pPoint = ppPoint[i];

			Point2D pt;
			if (planeType == PlaneType::PLANE_XY) {
				pt[0] = pPoint->x;
				pt[1] = pPoint->y;
			}
			else if (planeType == PlaneType::PLANE_YZ) {
				pt[0] = pPoint->y;
				pt[1] = pPoint->z;
			}
			else {
				pt[0] = pPoint->z;
				pt[1] = pPoint->x;
			}

			vertices.push_back(pt);
		}

		std::vector<std::vector<Point2D>> polygon;
		polygon.push_back(vertices);

		indices = mapbox::earcut<int>(polygon);
	}

	triangleIndices.swap(indices);
	//std::cout << "triangleIndices:" << triangleIndices.size() << std::endl;
}
