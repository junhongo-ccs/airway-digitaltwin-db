#include <iostream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include "../commonLib/SpatialUtil.h"
//#include "ApiSpatialId.h"

int spatial_ids_adjacent_to_faces(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "wrong parameters!";
        return 9;
    }
    std::string spatial_id = argv[2];
    std::vector<std::string> sixSpatialIds;

    try {
        spatialid::get_6spatial_ids_adjacent_to_faces(spatial_id, sixSpatialIds);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }
    
    for (auto itr = sixSpatialIds.begin(); itr != sixSpatialIds.end(); )
    {
        std::cout << *itr;
        if (++itr != sixSpatialIds.end()) {
            std::cout << ":";
        }
    }

    return 0;
}
