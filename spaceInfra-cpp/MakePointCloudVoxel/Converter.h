#pragma once

#include <spdlog/spdlog.h>
#include "CommonDefs.h"
#include "DataAccessAdapter.h"
#include "Config.h"
#include <unordered_set>
#include <map>

class Config;
class DataAccessAdapter;
class GeoDataset;
class Converter
{
	std::shared_ptr<spdlog::logger> logger;
	bool loadLasFile(FileInfo& fileInfo, GeoDataset& gds);
	void createVoxels(std::string& curDatetime, int dataSourceId, 
		std::unordered_set<std::string>& spatialIds, GeoDataset& gds,
		FileInfo& fileInfo, DataAccessAdapter& adapter, std::string& originalFileName);
	void getSpatialIdList(std::string& spatialId, int zoomLevel, FileInfo& fileInfo,
		GeoDataset& gds, std::unordered_set<std::string>& spatialIds);
public:
	Converter();
	virtual ~Converter() {
		//noop now
	}

	void convert();

};

