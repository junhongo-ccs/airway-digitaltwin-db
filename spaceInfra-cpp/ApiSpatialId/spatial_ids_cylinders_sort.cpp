#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstddef>
#include <time.h>  
#include <iomanip>
#include <unordered_map>
#include "../commonLib/SpatialUtil.h"
#include "../commonLib/CommonUtil.h"

int spatial_ids_cylinders_sort(int argc, char* argv[]) {

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
    std::vector<std::string> spatial_id_list;

    try {
        for (int i = 0; i < point_list.size() - 1; i++) {
            //空間IDを2点間毎に求める
            std::vector<spatialid::Point> point_list_tmp;
            std::unordered_set<std::string> spatial_id_list_tmp;
            point_list_tmp.push_back(point_list[i]);
            point_list_tmp.push_back(point_list[i + 1]);
            //始点の空間IDを求める
            spatialid::Point point;
            point = point_list[i];
            std::string spatial_from = spatialid::get_spatial_id_on_point(point.x, point.y, point.z, zoom, crs);
            //std::string spatial_from = spatialid::get_global_spatial_id_on_point(point_list[i], crs);
            std::vector<std::string> id_from = CommonUtil::split(spatial_from, '/');
            int from_x = stoi(id_from[2]);
            int from_y = stoi(id_from[3]);
            int from_z = stoi(id_from[1]);
            //std::cout << "from_x = " << from_x << "from_y = " << from_y << "from_z = " << from_z << std::endl;
            spatialid::get_spatial_ids_on_cylinders(point_list_tmp, radius, spatial_id_list_tmp, zoom, crs, true);
            
            //始点に近い順に並べ直す           
            std::vector<std::pair<int, std::string> > spatial_sort;
            for (auto& spatial : spatial_id_list_tmp) {
                std::vector<std::string> ids = CommonUtil::split(spatial, '/');
                int diff_x = stoi(ids[2]) - from_x;
                int diff_y = stoi(ids[3]) - from_y;
                int diff_z = stoi(ids[1]) - from_z;
                int diff = diff_x*diff_x + diff_y*diff_y + diff_z*diff_z;
                std::pair<int, std::string> pair_tmp = std::make_pair(diff, spatial);
                spatial_sort.push_back(pair_tmp);
            }           
            std::sort(spatial_sort.begin(), spatial_sort.end());

            //始点に近い順にvectorに挿入する(既に空間IDがvectorにあれば何もしない)
            for (int idx2 = 0; idx2 < spatial_sort.size(); idx2++) {
                if (spatial_id_list.end() == 
                    std::find(spatial_id_list.begin(), spatial_id_list.end(), spatial_sort[idx2].second)) {                 
                    spatial_id_list.push_back(spatial_sort[idx2].second);
                }
            }

        }

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