#pragma once

#include <spdlog/spdlog.h>
#include "CommonDefs.h"
#include "DataAccessAdapter.h"
#include "Config.h"
#include <citygml/citygml.h>
#include <citygml/citymodel.h>
#include <citygml/cityobject.h>
#include <citygml/geometry.h>
#include <citygml/polygon.h>
#include <citygml/vecs.hpp>

class GeoDataset;
class Config;
class GeoPolygon;
class DataAccessAdapter;
class CityObject;
class Converter
{
	PJ_CONTEXT* projContext = nullptr;
	PJ* projLas2World = nullptr;
	PJ* projMap2Las = nullptr;

	std::shared_ptr<spdlog::logger> logger;
	bool loadCityGmlFile(GeoDataset& gds, CityGmlFileInfo& fileInfo, const std::string& filePath);
	void createVoxels(GeoDataset& gds, CityGmlFileInfo& fileInfo, Config& config, DataAccessAdapter& adapter);
	std::string getDataType(PolygonType type);
	std::shared_ptr<SpaceDataManagesInfo> initiaizeSpaceDataManagesInfo(Config& config, 
		std::string& spatialId, CityGmlFileInfo& fileInfo);
	void calcWorldMinMax(double xmin, double xmax, double ymin, double ymax,
		double& xminW, double& xmaxW, double& yminW, double& ymaxW);
	int getLasClassfication(std::string& featureId);
	void transform(GeoPolygon* pPolygon, PJ* proj);
	void deleteOldFiles(DataAccessAdapter& adapter, std::string& dataType, 
		std::map<std::string, std::vector<CityObject*>>& spatialIdToObjectMap);
	void recursiveLoadCityObject(const citygml::CityObject& cityObject, GeoDataset& gds, 
		CityGmlFileInfo& fileInfo, Config& config, CityObject*& pCityObject);
	void recursiveLoadGeometry(const citygml::Geometry& geometry, GeoDataset& gds, 
		CityGmlFileInfo& fileInfo, Config& config, CityObject*& pCityObject);
public:
	Converter();
	virtual ~Converter() {
		if (projLas2World != nullptr) {
			proj_destroy(projLas2World);
		}
		if (projMap2Las != nullptr) {
			proj_destroy(projMap2Las);
		}

		if (projContext != nullptr) {
			proj_context_destroy(projContext);
		}

	}

	void convert();

};

