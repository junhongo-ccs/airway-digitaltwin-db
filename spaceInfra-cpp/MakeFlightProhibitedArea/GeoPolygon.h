#pragma once

/*!
* @file GeoPolygon.h
* @brief ポリゴン関連のC++ヘッダー
*/

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdlib>
#include <mutex>
#include "CommonDefs.h"
#include "ObjectRefContainer.h"
#include "spdlog/spdlog.h"


enum class PlaneType {
	PLANE_XY = 1,
	PLANE_YZ,
	PLANE_ZX
};

//! @brief ポリゴンクラス
class GeoPolygon {
	std::shared_ptr<spdlog::logger> logger;

	//ゼンリンポイントデータ
	ObjectRefContainer<GeoPoint> pointContainer{ 10 };

	//属性
	ObjectRefContainer<GeoAttribute> attributeContainer{ 10 };

	int blockSize = 200;

	bool outputProcessed = false;

	PolygonType polygonType;
	int polygonId = -1;
	PlaneType planeType;
	std::vector<int> triangleIndices;
	std::vector<spatialid::Triangle> triangles;
	std::unordered_set<std::string> spatialIds;

public:
	GeoPolygon();
	virtual ~GeoPolygon();

	void decidePlaneType();
	void doTriangulation();

	int getNumOfPoints() {
		return pointContainer.getObjectCount();
	}
	GeoPoint** getPointsRef() {
		return pointContainer.getObjectRef();
	}
	int getNumOfAttributes() {
		return attributeContainer.getObjectCount();
	}
	GeoAttribute** getAttributesRef() {
		return attributeContainer.getObjectRef();
	}

	//! @brief 指定名称の属性を取得
	//! 
	//! @param name 取得対象の名称
	//! @return 取得結果
	GeoAttribute* getAttribute(std::string name) {
		int numAttributes = attributeContainer.getObjectCount();
		for (int i = 0; i < numAttributes; i++) {
			GeoAttribute* pAttribute = attributeContainer[i];
			if (pAttribute->name == name) {
				return pAttribute;
			}
		}

		return nullptr;
	}

	void addAttribute(int num, GeoAttribute* attributes);
	void addPoints(int num, GeoPoint* points);


	void setOutpuProcessed(bool flag) {
		outputProcessed = flag;
	}
	bool isOutputProcessed() {
		return outputProcessed;
	}


	PolygonType getPolygonType() {
		return polygonType;
	}
	void setPolygonType(PolygonType type) {
		polygonType = type;
	}


	void setPolygonId(int val) {
		polygonId = val;
	}
	int getPolygonId() {
		return polygonId;
	}

	int getPointCount() {
		return pointContainer.getObjectCount();
	}

	GeoPoint* getPoint(int i) {
		return pointContainer.getObject(i);
	}

	//GeoPoint** getPointRef() {
	//	return pointContainer.getObjectRef();
	//}

	PlaneType getPlaneType() {
		return planeType;
	}

	std::vector<int>& getTriangleIndices() {
		return triangleIndices;
	}

	void setPlaneType(PlaneType planeType) {
		this->planeType = planeType;
	}

	std::vector<spatialid::Triangle>& getTriangles() {
		return triangles;
	}

	std::unordered_set<std::string>& getSpatialIds() {
		return spatialIds;
	}


};
