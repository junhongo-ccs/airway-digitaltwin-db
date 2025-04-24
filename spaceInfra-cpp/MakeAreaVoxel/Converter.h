#pragma once

#include <spdlog/spdlog.h>
#include "CommonDefs.h"
#include "DataAccessAdapter.h"
#include "Config.h"

class GeoDataset;
class Config;
class GeoPolygon;
class DataAccessAdapter;
class Converter
{
	PJ_CONTEXT* projContext = nullptr;
	//PJ* projLas2World = nullptr;
	PJ* projMap2Las = nullptr;

	std::shared_ptr<spdlog::logger> logger;
	bool loadMeshData(GeoDataset& gds, std::string& coordinates);
	void createVoxels(GeoDataset& gds, uint64_t areaObjectId, std::string& areaId, Config& config, DataAccessAdapter& adapter, 
		std::string& today);
	void transform(GeoPolygon* pPolygon, PJ* proj);
	//void deleteOldFiles(DataAccessAdapter& adapter, 
	//	std::map<std::string, std::vector<CityObject*>>& spatialIdToObjectMap);
public:
	Converter();
	virtual ~Converter() {
		//f (projLas2World != nullptr) {
		//	proj_destroy(projLas2World);
		//}
		if (projMap2Las != nullptr) {
			proj_destroy(projMap2Las);
		}

		if (projContext != nullptr) {
			proj_context_destroy(projContext);
		}

	}

	void convert();

};

