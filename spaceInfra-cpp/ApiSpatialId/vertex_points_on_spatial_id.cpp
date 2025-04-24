#include <iostream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include "../commonLib/SpatialUtil.h"
//#include "ApiSpatialId.h"

int vertex_points_on_spatial_id(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "wrong parameters!";
        return 9;
    }
    std::string spatial_id = argv[2];
    std::string crs_str = argv[3];
    spatialid::CRS crs = SpatialUtil::getCrsFromArgEpsg(crs_str);
    std::vector<spatialid::Point> vertex;

    try {
        spatialid::get_vertex_points_on_spatial_id(spatial_id, vertex, crs);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }

    std::cout << std::setprecision(12)
        << vertex[0].y << " " << vertex[0].x << " " << vertex[0].z << " "
        << vertex[1].y << " " << vertex[1].x << " " << vertex[1].z << " "
        << vertex[2].y << " " << vertex[2].x << " " << vertex[2].z << " "
        << vertex[3].y << " " << vertex[3].x << " " << vertex[3].z << " "
        << vertex[4].y << " " << vertex[4].x << " " << vertex[4].z << " "
        << vertex[5].y << " " << vertex[5].x << " " << vertex[5].z << " "
        << vertex[6].y << " " << vertex[6].x << " " << vertex[6].z << " "
        << vertex[7].y << " " << vertex[7].x << " " << vertex[7].z;

    return 0;
}
