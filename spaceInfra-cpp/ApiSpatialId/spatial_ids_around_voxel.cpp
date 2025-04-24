#include <iostream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include "../commonLib/SpatialUtil.h"
//#include "ApiSpatialId.h"

int spatial_ids_around_voxel(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "wrong parameters!";
        return 9;
    }
    std::string spatial_id = argv[2];
    std::vector<std::string> spatialIds;
    try {
        spatialid::get_26spatial_ids_around_voxel(spatial_id, spatialIds);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }

    for (auto itr = spatialIds.begin(); itr != spatialIds.end(); )
    {
        std::cout << *itr;
        if (++itr != spatialIds.end()) {
            std::cout << ":";
        }
    }

    return 0;
}
