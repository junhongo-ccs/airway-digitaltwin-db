#include <iostream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include "../commonLib/SpatialUtil.h"
//#include "ApiSpatialId.h"

int center_point_on_spatial_id(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "wrong parameters!";
        return 9;
    }
    std::string spatial_id = argv[2];
    std::string crs_str = argv[3];
    spatialid::Point centerPoint;

    try {
        spatialid::CRS crs = SpatialUtil::getCrsFromArgEpsg(crs_str);
        centerPoint = spatialid::get_center_point_on_spatial_id(spatial_id, crs);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }
       
    std::cout << std::setprecision(12) 
        << centerPoint.y << " " << centerPoint.x << " " << centerPoint.z;

    return 0;
}
