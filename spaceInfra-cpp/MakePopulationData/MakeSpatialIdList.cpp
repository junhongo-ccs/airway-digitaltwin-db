#include "MakeSpatialIdList.h"

std::vector<std::tuple<std::string, double>> makeSpatialIdList(std::string mesh)
{
    std::vector<std::tuple<std::string, double>> id_list = {};
    size_t mesh_size = mesh.length();
    if (mesh_size < 9 || mesh_size > 11) {
        return id_list;
    }
    std::string wk = mesh.substr(0, 2);
    int x1 = stoi(wk);
    wk = mesh.substr(2, 2);
    int y1 = stoi(wk);
    wk = mesh.substr(4, 1);
    int x2 = stoi(wk);
    wk = mesh.substr(5, 1);
    int y2 = stoi(wk);
    wk = mesh.substr(6, 1);
    int x3 = stoi(wk);
    wk = mesh.substr(7, 1);
    int y3 = stoi(wk);
    wk = mesh.substr(8, 1);
    int xy4 = stoi(wk);

    int x4 = 0;
    int y4 = 0;
    if (xy4 == 2) {
        y4 = 1;
    }
    if (xy4 == 3) {
        x4 = 1;
    }
    if (xy4 == 4) {
        x4 = 1;
        y4 = 1;
    }
    double lat4 = x4 * 1.0 / 240.0;
    double lon4 = y4 * 1.0 / 160.0;

    double lat5 = 0.0;
    double lon5 = 0.0;
    if (mesh_size > 9) {
        wk = mesh.substr(9, 1);
        int xy5 = stoi(wk);
        int x5 = 0;
        int y5 = 0;
        if (xy5 == 2) {
            y5 = 1;
        }
        if (xy5 == 3) {
            x5 = 1;
        }
        if (xy5 == 4) {
            x5 = 1;
            y5 = 1;
        }
        lat5 = x5 * 1.0 / 480.0;
        lon5 = y5 * 1.0 / 320.0;
    }

    double lat6 = 0.0;
    double lon6 = 0.0;
    if (mesh_size > 10) {
        wk = mesh.substr(10, 1);
        int xy6 = stoi(wk);
        int x6 = 0;
        int y6 = 0;
        if (xy6 == 2) {
            y6 = 1;
        }
        if (xy6 == 3) {
            x6 = 1;
        }
        if (xy6 == 4) {
            x6 = 1;
            y6 = 1;
        }
        lat6 = x6 * 1.0 / 960.0;
        lon6 = y6 * 1.0 / 640.0;       
    }
    //wk = mesh4.substr(9, 1);
    //int xy5 = stoi(wk);
    /*
    std::cout << "x1=" << x1 << std::endl;
    std::cout << "y1=" << y1 << std::endl;
    std::cout << "x2=" << x2 << std::endl;
    std::cout << "y2=" << y2 << std::endl;
    std::cout << "x3=" << x3 << std::endl;
    std::cout << "y3=" << y3 << std::endl;
    std::cout << "xy4=" << xy4 << std::endl;
    //std::cout << "xy5=" << xy5 << std::endl;
    */
    double lat1 = x1 * 2.0 / 3.0;
    double lon1 = y1 + 100.0;

    double lat2 = x2 * 1.0 / 12.0;
    double lon2 = y2 * 1.0 / 8.0;

    double lat3 = x3 * 1.0 / 120.0;
    double lon3 = y3 * 1.0 / 80.0;

    double swLatM = 0;
    double swLonM = 0;
    double nwLatM = 0;
    double nwLonM = 0;
    double seLatM = 0;
    double seLonM = 0;
    double neLatM = 0;
    double neLonM = 0;

    double meshArea = 0;
   
    if (mesh_size == 9) {  //mesh4の時
        //mesh4の南西端の座標
        swLatM = lat1 + lat2 + lat3 + lat4;
        swLonM = lon1 + lon2 + lon3 + lon4;
        //mesh4の北西端の座標
        nwLatM = swLatM + 1.0 / 240.0;
        nwLonM = swLonM;
        //mesh4の南東端の座標
        seLatM = swLatM;
        seLonM = swLonM + 1.0 / 160.0;
        //mesh4の北東端の座標
        neLatM = nwLatM;
        neLonM = seLonM;
        //mesh4の面積
        meshArea = 1.0 / 240.0 * 1.0 / 160.0;
    }

    if (mesh_size == 10) {   //mesh5の時
        //mesh5の南西端の座標
        swLatM = lat1 + lat2 + lat3 + lat4 + lat5;
        swLonM = lon1 + lon2 + lon3 + lon4 + lon5;
        //mesh5の北西端の座標
        nwLatM = swLatM + 1.0 / 480.0;
        nwLonM = swLonM;
        //mesh5の南東端の座標
        seLatM = swLatM;
        seLonM = swLonM + 1.0 / 320.0;
        //mesh5の北東端の座標
        neLatM = nwLatM;
        neLonM = seLonM;
        //mesh5の面積
        meshArea = 1.0 / 480.0 * 1.0 / 320.0;
    }
    
    if (mesh_size == 11) {   //mesh6の時
        //mesh6の南西端の座標
        swLatM = lat1 + lat2 + lat3 + lat4 + lat5 + lat6;
        swLonM = lon1 + lon2 + lon3 + lon4 + lon5 + lon6;
        //mesh6の北西端の座標
        nwLatM = swLatM + 1.0 / 960.0;
        nwLonM = swLonM;
        //mesh6の南東端の座標
        seLatM = swLatM;
        seLonM = swLonM + 1.0 / 640.0;
        //mesh6の北東端の座標
        neLatM = nwLatM;
        neLonM = seLonM;
        //mesh6の面積
        meshArea = 1.0 / 960.0 * 1.0 / 640.0;
    }
    /*test_cording
    std::cout << "mesh_code=" << mesh << std::endl;
    std::cout << std::fixed << std::setprecision(15) << "sw_point=" << swLatM << "  " << swLonM << std::endl;
    std::cout << std::fixed << std::setprecision(15) << "nw_point=" << nwLatM << "  " << nwLonM << std::endl;
    std::cout << std::fixed << std::setprecision(15) << "se_point=" << seLatM << "  " << seLonM << std::endl;
    */
    //spatialid::Point const& p_sw = spatialid::Point(swLonM, swLatM, 0.0);
    //std::string spatial_id_sw = get_global_spatial_id_on_point(p_sw);

    spatialid::Point const& p_nw = spatialid::Point(nwLonM, nwLatM, 0.0);
    std::string spatial_id_nw = get_global_spatial_id_on_point(p_nw);
    //std::cout << "id_nw=" << spatial_id_nw << std::endl;
    
    spatialid::Point const& p_se = spatialid::Point(seLonM, seLatM, 0.0);
    std::string spatial_id_se = get_global_spatial_id_on_point(p_se);
    //std::cout << "id_se=" << spatial_id_se << std::endl;
    
    //spatialid::Point const& p_ne = spatialid::Point(neLonM, neLatM, 0.0);
    //std::string spatial_id_ne = get_global_spatial_id_on_point(p_ne);
    

    std::vector<std::string> wk_id_nw = CommonUtil::split(spatial_id_nw, '/');
    int wk_min_x = stoi(wk_id_nw[2]);
    int wk_min_y = stoi(wk_id_nw[3]);

    std::vector<std::string> wk_id_se = CommonUtil::split(spatial_id_se, '/');
    int wk_max_x = stoi(wk_id_se[2]) + 1;
    int wk_max_y = stoi(wk_id_se[3]) + 1;

    //std::cout << "mash " << mesh << std::endl;
    for (int i = wk_min_x; i < wk_max_x; i++) {
        for (int ii = wk_min_y; ii < wk_max_y; ii++) {
            std::string wk = "17/0/" + std::to_string(i) + "/" + std::to_string(ii);
            //std::cout << "spatial_id = " << wk << std::endl;
            double area_rate = calcOverlappingArea(wk, swLatM, swLonM, nwLatM, seLonM) / meshArea;
            //std::cout << "         " <<  " area_rate = " << area_rate << std::endl;
            std::tuple<std::string, double> wk_tuple(wk, area_rate);
            id_list.push_back(wk_tuple);
        }
    }

    /*test
    std::cout << "meshと重なる空間IDは" << std::endl;
    for (std::string id : id_list) {
        std::cout << "                  " << id << std::endl;
    }
    */
    return id_list;
}

double calcOverlappingArea(std::string spatialId, double swLatM, double swLonM, double nwLatM, double seLonM)
{
    std::vector<spatialid::Point> vertex;

    try {
        spatialid::get_vertex_points_on_spatial_id(spatialId, vertex);
    }
    catch (std::invalid_argument& e) {
        return 9;
    }
    catch (...) {
        return 8;
    }

    //$return_arr[0] = $points_arr[9];
    double swLatS = vertex[3].y;
    //$return_arr[1] = $points_arr[10];
    double swLonS = vertex[3].x;
    //$return_arr[2] = $points_arr[0];
    double nwLatS = vertex[0].y;
    //$return_arr[3] = $points_arr[7];
    double seLonS = vertex[2].x;

    /*
    std::cout << "   swLatS = " << swLatS << std::endl;
    std::cout << "   swLonS = " << swLonS << std::endl;
    std::cout << "   nwLatS = " << nwLatS << std::endl;
    std::cout << "   seLonS = " << seLonS << std::endl;
    */

    //緯度でmeshと空間IDで重なっている長さを計算
    double minLatMS = swLatS;
    if (swLatS < swLatM) {
        minLatMS = swLatM;
    }
    double maxLatMS = nwLatS;
    if (nwLatS > nwLatM) {
        maxLatMS = nwLatM;
    }
    //経度でmeshと空間IDで重なっている長さを計算
    double minLonMS = swLonS;
    if (swLonS < swLonM) {
        minLonMS = swLonM;
    }
    double maxLonMS = seLonS;
    if (seLonS > seLonM) {
        maxLonMS = seLonM;
    }

    if ((maxLatMS - minLatMS) < 0 || maxLonMS - minLonMS < 0) {
        return 0.0;
    }

    return (maxLatMS - minLatMS) * (maxLonMS - minLonMS);
}

