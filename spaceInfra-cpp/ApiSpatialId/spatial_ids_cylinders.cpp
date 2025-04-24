#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include "../commonLib/SpatialUtil.h"
#include "../commonLib/CommonUtil.h"

int spatial_ids_cylinders(int argc, char* argv[]) {

    if (argc != 6) {
        std::cout << "wrong parameters!";
        return 9;
    }
    std::string str_points = argv[2];
    std::string str_radius = argv[3];
    double radius = std::stod(str_radius);
    std::string str_crs = argv[4];
    std::string str_zoom = argv[5];
    int zoom = std::stoi(str_zoom);

    std::vector<std::string> points = CommonUtil::split(str_points, ':');
    std::vector<spatialid::Point> point_list;

    if ((points.size() / 3 < 2) || (points.size() % 3 != 0)) {
        std::cout << "wrong parameters!";
        return 9;
    }
    for (int idx = 0; idx < points.size() / 3; idx++) {
        double tmp_lat = std::stod(points[1 + idx * 3]);
        double tmp_lon = std::stod(points[0 + idx * 3]);
        double tmp_alt = std::stod(points[2 + idx * 3]);
        point_list.push_back(spatialid::Point(
            tmp_lat, tmp_lon, tmp_alt
        ));
    }

    spatialid::CRS crs = SpatialUtil::getCrsFromArgEpsg(str_crs);
    std::unordered_set<std::string> spatial_id_list;

    try {
        spatialid::get_spatial_ids_on_cylinders(point_list, radius, spatial_id_list, zoom, crs, true);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }

    for (auto itr = spatial_id_list.begin(); itr != spatial_id_list.end(); )
    {
        std::cout << *itr;
        if (++itr != spatial_id_list.end()) {
            std::cout << ":";
        }
    }
    return 0;
}