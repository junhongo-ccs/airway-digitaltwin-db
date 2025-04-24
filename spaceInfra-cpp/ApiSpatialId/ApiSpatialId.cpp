// testMainHashCal.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <cstddef>
#include <time.h>  
#include <zlib.h>
#include <iomanip>
#include <string>
#include "ApiSpatialId.h"


int	main(int argc, char* argv[])
{
    std::string str_functionId = argv[1];
    int  functionId = std::stoi(str_functionId);
    switch (functionId) {
    case 1:        
        return vertex_points_on_spatial_id(argc, argv);
        break;
    case 2:
        return spatial_ids_cylinders_sort(argc, argv);
        break;
    case 3:
        return spatial_ids_on_points(argc, argv);
        break;
    case 4:
        return center_point_on_spatial_id(argc, argv);
        break;
    case 5:
        return spatial_ids_adjacent_to_faces(argc, argv);
        break;
    case 6:
        return spatial_ids_around_voxel(argc, argv);
        break;
    case 7:
        return spatial_ids_cylinders(argc, argv);
        break;
    default:
        break;
    }
}