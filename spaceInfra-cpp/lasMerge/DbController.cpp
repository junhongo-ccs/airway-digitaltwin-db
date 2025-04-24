#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <sstream>
#include <chrono>
#include <regex>
#include <iomanip>
#include "DbController.hpp"
#include <mysql_error.h>

//! @brief 文字列を指定文字で区切ってvectorに格納する
//! @param 文字列
//! @param デリミター文字
//! @return 結果のリスト
std::vector<std::string> split(std::string&, char);


DbController::DbController(HoldConfig  hcf)
{
    logger = hcf.getLoggerPointer();
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(hcf.getBtDbHost(), hcf.getBtDbUser(), hcf.getBtDbPass());
        stmt = con->createStatement();

        std::string  buff = "USE ";
        buff.append(hcf.getBtDbName());
        stmt->execute(buff);
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        exit(9);
    }
}

DbController::~DbController() {
    delete result;
    delete pstmt;
    delete stmt;
    delete con;
}

void DbController::updateDb(std::vector<std::string> vecSql) {
    sp_xmin   =  999999999999.9;
    sp_xmax   = -999999999999.9;
    sp_ymin   =  999999999999.9;
    sp_ymax   = -999999999999.9;
    sp_zmin   =  999999999999.9;
    sp_xmin_w =  999999999999.9;
    sp_xmax_w = -999999999999.9;
    sp_ymin_w =  999999999999.9;
    sp_ymax_w = -999999999999.9;
    xmin    =  999999999999.9;
    xmax    = -999999999999.9;
    ymin    =  999999999999.9;
    ymax    = -999999999999.9;
    zmin    =  999999999999.9;
    zmax    = -999999999999.9;
    xmin_w  =  999999999999.9;
    xmax_w  = -999999999999.9;
    ymin_w  =  999999999999.9;
    ymax_w  = -999999999999.9;
    points = 0;
    originalFileIds = "";
    wkABCD = "    ";
    first_flag = true;
    for (int idx = 0; idx < 22; idx++) {
        clsCount[idx] = 0;
    }
    for (int idx = 0; idx < vecSql.size(); idx++) {       
        std::vector<std::string> vecSqlPara = split(vecSql.at(idx), ',');
            if (vecSqlPara.at(0) == "get") {
                selectDb(vecSqlPara.at(1));
            }
            if (vecSqlPara.at(0) == "insert") {
                insertDb(vecSqlPara.at(1));
            }
            if (vecSqlPara.at(0) == "softDelete") {
                softDeleteDb(vecSqlPara.at(1));
            }
    }
}
void DbController::selectDb(std::string id) {
    try {
        std::cout << atoi(id.c_str()) << std::endl;
        pstmt = con->prepareStatement(sqlSELECT);
        pstmt->setInt(1, atoi(id.c_str()));
        result = pstmt->executeQuery();
        std::cout << result->rowsCount() << std::endl;
        while (result->next()) {
            owner_id = result->getString(2);
            batch_id = result->getString(3);
            spatial_id = result->getString(4);
            rec_id = result->getString(5);
            file_id = result->getString(6);
            file_path = result->getString(7);
            epsg_las = result->getString(8);
            epsg_csv = result->getString(9);
            data_unit = result->getString(10);
            data_io = result->getString(11);
            data_type = result->getString(12);
            data_status = result->getString(13);
            data_abcd = result->getString(14);
            for (int idx = 0; idx < data_abcd.length(); idx++) {
                switch (data_abcd[idx]) {
                case 'A':
                    wkABCD[0] = 'A';
                    break;
                case 'B':
                    wkABCD[1] = 'B';
                    break;
                case 'C':
                    wkABCD[2] = 'C';
                    break;
                case 'D':
                    wkABCD[3] = 'D';
                    break;
                }
            }
            double wk_sp_xmin = result->getDouble(15);
            double wk_sp_xmax = result->getDouble(16);
            double wk_sp_ymin = result->getDouble(17);
            double wk_sp_ymax = result->getDouble(18);
            double wk_sp_zmin = result->getDouble(19);
            double wk_sp_zmax = result->getDouble(20);
            double wk_sp_xmin_w = result->getDouble(21);
            double wk_sp_xmax_w = result->getDouble(22);
            double wk_sp_ymin_w = result->getDouble(23);
            double wk_sp_ymax_w = result->getDouble(24);
            if (wk_sp_xmax > sp_xmax) {
                sp_xmax = wk_sp_xmax;
            }
            if (wk_sp_xmax_w > sp_xmax_w) {
                sp_xmax_w = wk_sp_xmax_w;
            }
            if (wk_sp_ymax > sp_ymax) {
                sp_ymax = wk_sp_ymax;
            }
            if (wk_sp_ymax_w > sp_ymax_w) {
                sp_ymax_w = wk_sp_ymax_w;
            }
            if (wk_sp_zmax > sp_zmax) {
                sp_zmax = wk_sp_zmax;
            }
            if (wk_sp_xmin < sp_xmin) {
                sp_xmin = wk_sp_xmin;
            }
            if (wk_sp_xmin_w < sp_xmin_w) {
                sp_xmin_w = wk_sp_xmin_w;
            }
            if (wk_sp_ymin < sp_ymin) {
                sp_ymin = wk_sp_ymin;
            }
            if (wk_sp_ymin_w < sp_ymin_w) {
                sp_ymin_w = wk_sp_ymin_w;
            }
            if (wk_sp_zmin < sp_zmin) {
                sp_zmin = wk_sp_zmin;
            }
            if (first_flag) {
                first_flag = false;
            } else {
                originalFileIds.append(":");
            }
            originalFileIds.append(result->getString(25));
            int wk_points = result->getInt(26);
            points = points + wk_points;
            double wk_xmin = result->getDouble(27);
            double wk_xmax = result->getDouble(28);
            double wk_ymin = result->getDouble(29);
            double wk_ymax = result->getDouble(30);
            double wk_zmin = result->getDouble(31);
            double wk_zmax = result->getDouble(32);
            double wk_xmin_w = result->getDouble(33);
            double wk_xmax_w = result->getDouble(34);
            double wk_ymin_w = result->getDouble(35);
            double wk_ymax_w = result->getDouble(36);
            if (wk_xmax > xmax) {
                xmax = wk_xmax;
            }
            if (wk_xmax_w > xmax_w) {
                xmax_w = wk_xmax_w;
            }
            if (wk_ymax > ymax) {
                ymax = wk_ymax;
            }
            if (wk_ymax_w > ymax_w) {
                ymax_w = wk_ymax_w;
            }
            if (wk_zmax > zmax) {
                zmax = wk_zmax;
            }
            if (wk_xmin < xmin) {
                xmin = wk_xmin;
            }
            if (wk_xmin_w < xmin_w) {
                xmin_w = wk_xmin_w;
            }
            if (wk_ymin < ymin) {
                ymin = wk_ymin;
            }
            if (wk_ymin_w < ymin_w) {
                ymin_w = wk_ymin_w;
            }
            if (wk_zmin < zmin) {
                zmin = wk_zmin;
            }
            int idx1 = 37;
            for (int idx2 = 0; idx2 < 22; idx2++) {
                int wk_int = result->getInt(idx1);
                clsCount[idx2] = clsCount[idx2] + wk_int;
                idx1++;
            }
            logger->info("id:{:s} spatial_id:{:s} {:s}  points:{:d}  addSave ended ", id, spatial_id, data_abcd, wk_points );
        }
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        exit(9);
    }
}
void DbController::insertDb(std::string filePath) {
    std::vector<std::string> vecfilePath = split(filePath, '/');
    try {
        pstmt = con->prepareStatement(sqlSELECT2);
        pstmt->setString(1, owner_id);
        pstmt->setString(2, vecfilePath.at(vecfilePath.size() - 3));
        pstmt->setString(3, spatial_id);
        pstmt->setString(4, REC_ID);
        result = pstmt->executeQuery();
        //キーでSELECTしてゼロ件だったらINSERT、その他はUPDATE
        if (result->rowsCount() == 0) {
            pstmt = con->prepareStatement(sqlINSERT);
            pstmt->setString(1, owner_id);
            pstmt->setString(2, vecfilePath.at(vecfilePath.size() - 3));
            pstmt->setString(3, spatial_id);
            pstmt->setString(4, REC_ID);
            pstmt->setString(5, vecfilePath.back());
            pstmt->setString(6, filePath);
            pstmt->setString(7, epsg_las);
            pstmt->setString(8, epsg_csv);
            pstmt->setString(9, data_unit);
            pstmt->setString(10, TEMP_DATA_IO);
            pstmt->setString(11, data_type);
            pstmt->setString(12, data_status);
            pstmt->setString(13, regex_replace(wkABCD, std::regex(" "), ""));
            pstmt->setDouble(14, sp_xmin);
            pstmt->setDouble(15, sp_xmax);
            pstmt->setDouble(16, sp_ymin);
            pstmt->setDouble(17, sp_ymax);
            pstmt->setDouble(18, sp_zmin);
            pstmt->setDouble(19, sp_zmax);
            pstmt->setString(20, originalFileIds);
            pstmt->setInt(21, points);
            pstmt->setDouble(22, xmin);
            pstmt->setDouble(23, xmax);
            pstmt->setDouble(24, ymin);
            pstmt->setDouble(25, ymax);
            pstmt->setDouble(26, zmin);
            pstmt->setDouble(27, zmax);
            int idx2 = 28;
            for (int idx = 0; idx < 22; idx++) {
                pstmt->setInt(idx2, clsCount[idx]);
                idx2++;
            }
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
            pstmt->setDateTime(50, ss.str());
            pstmt->setDouble(51, sp_xmin_w);
            pstmt->setDouble(52, sp_xmax_w);
            pstmt->setDouble(53, sp_ymin_w);
            pstmt->setDouble(54, sp_ymax_w);
            pstmt->setDouble(55, xmin_w);
            pstmt->setDouble(56, xmax_w);
            pstmt->setDouble(57, ymin_w);
            pstmt->setDouble(58, ymax_w);
            pstmt->execute();
            logger->info("batch_id:{:s} spatial_id:{:s}  points:{:d}  insert ended ", vecfilePath.at(vecfilePath.size() - 3), spatial_id, points);
        }
        else {
            pstmt = con->prepareStatement(sqlUPDATE);
            pstmt->setString(1, vecfilePath.back());
            pstmt->setString(2, filePath);
            pstmt->setString(3, epsg_las);
            pstmt->setString(4, epsg_csv);
            pstmt->setString(5, data_unit);
            pstmt->setString(6, TEMP_DATA_IO);
            pstmt->setString(7, data_type);
            pstmt->setString(8, data_status);

            pstmt->setString(9, regex_replace(wkABCD, std::regex(" "), ""));
            pstmt->setDouble(10, sp_xmin);
            pstmt->setDouble(11, sp_xmax);
            pstmt->setDouble(12, sp_ymin);
            pstmt->setDouble(13, sp_ymax);
            pstmt->setDouble(14, sp_zmin);
            pstmt->setDouble(15, sp_zmax);
            pstmt->setString(16, originalFileIds);
            pstmt->setInt(17, points);
            pstmt->setDouble(18, xmin);
            pstmt->setDouble(19, xmax);
            pstmt->setDouble(20, ymin);
            pstmt->setDouble(21, ymax);
            pstmt->setDouble(22, zmin);
            pstmt->setDouble(23, zmax);
            int idx2 = 24;
            for (int idx = 0; idx < 22; idx++) {
                pstmt->setInt(idx2, clsCount[idx]);
                idx2++;
            }
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
            pstmt->setDateTime(46, ss.str());
            pstmt->setDouble(47, sp_xmin_w);
            pstmt->setDouble(48, sp_xmax_w);
            pstmt->setDouble(49, sp_ymin_w);
            pstmt->setDouble(50, sp_ymax_w);
            pstmt->setDouble(51, xmin_w);
            pstmt->setDouble(52, xmax_w);
            pstmt->setDouble(53, ymin_w);
            pstmt->setDouble(54, ymax_w);
            pstmt->setString(55, owner_id);
            pstmt->setString(56, vecfilePath.at(vecfilePath.size() - 3));
            pstmt->setString(57, spatial_id);
            pstmt->setString(58, REC_ID);
            result = pstmt->executeQuery();
            logger->info("batch_id:{:s} spatial_id:{:s}  points:{:d}  update ended  ", vecfilePath.at(vecfilePath.size() - 3), spatial_id, points);
        }
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        exit(9);
    }

}
void DbController::softDeleteDb(std::string id) {
    try {
        pstmt = con->prepareStatement(sqlSOFTDELETE);
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        pstmt->setDateTime(1, ss.str());
        pstmt->setInt(2, atoi(id.c_str()));
        result = pstmt->executeQuery();
        logger->info("id:{:s}  softDelete ended   " , id);
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        exit(9);
    }

}

//文字列を指定文字で区切ってvectorに格納する
std::vector<std::string> split(std::string& input, char delimiter)
{
    std::istringstream stream(input);
    std::string field;
    std::vector<std::string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}
