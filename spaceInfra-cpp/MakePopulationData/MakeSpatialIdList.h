#ifndef __MAKESPATIALIDLIST_H_INCLUDED__
#define __MAKESPATIALIDLIST_H_INCLUDED__

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "../commonlib/CommonUtil.h"
#include "../../SpatialId/src/spatial_api.h"
#include "../../SpatialId/src/common/object/point.h"

std::vector<std::tuple<std::string, double>> makeSpatialIdList(std::string mesh4);
double calcOverlappingArea(std::string spatialId, double swLatM, double swLonM, double nwLatM, double seLonM);


#endif