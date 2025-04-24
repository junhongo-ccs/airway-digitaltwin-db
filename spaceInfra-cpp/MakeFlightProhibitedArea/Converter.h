#pragma once

#include <spdlog/spdlog.h>
#include "CommonDefs.h"
#include "DataAccessAdapter.h"
#include "Config.h"

class GeoDataset;
class Config;
class GeoPolygon;
class DataAccessAdapter;
class AreaObject;
class Converter
{
	PJ_CONTEXT* projContext = nullptr;
	PJ* projMap2Las = nullptr;
	PJ* projMap2World = nullptr;
	PJ* projWorld2Las = nullptr;

	std::shared_ptr<spdlog::logger> logger;
	void loadAreaObjectData(GeoDataset& gds, std::string& coordinates, int n);
	void createVoxels(GeoDataset& gds, uint64_t droneRouteId, std::string& areaId, Config& config, DataAccessAdapter& adapter, 
		std::string& today);
	void createPolygonVoxels(AreaObject* pAreaObject,
		GeoDataset& gds, uint64_t droneRouteId, std::string& areaId, Config& config, DataAccessAdapter& adapter,
		std::string& today);
	void createCircleVoxels(AreaObject* pAreaObject,
		GeoDataset& gds, uint64_t droneRouteId, std::string& areaId, Config& config, DataAccessAdapter& adapter,
		std::string& today);
	void transform(GeoPolygon* pPolygon, PJ* proj);
	//void deleteOldFiles(DataAccessAdapter& adapter, 
	//	std::map<std::string, std::vector<CityObject*>>& spatialIdToObjectMap);
	void getCircleSpatialIds(double centerX, double centerY, double radius, int zoomLevel, int crs,
		std::unordered_set<std::string>& allSpatialIds, std::unordered_set<std::string>& hollowSpatialIds);
public:
	Converter();
	virtual ~Converter() {
		if (projMap2Las != nullptr) {
			proj_destroy(projMap2Las);
		}

		if (projMap2World != nullptr) {
			proj_destroy(projMap2World);
		}

		if (projWorld2Las != nullptr) {
			proj_destroy(projWorld2Las);
		}

		if (projContext != nullptr) {
			proj_context_destroy(projContext);
		}


	}

	void convert();

};

