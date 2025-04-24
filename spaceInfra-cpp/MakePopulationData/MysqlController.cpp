#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "MysqlController.hpp"
#include <mysql_error.h>


MysqlController::MysqlController(std::string btDbHost, std::string btDbName, std::string btDbUser, std::string btDbPass, std::shared_ptr<spdlog::logger> logPointer)
{
    logger = logPointer;
    try {

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(btDbHost, btDbUser, btDbPass);
        stmt = con->createStatement();


        std::string  buff = "USE " + btDbName;
        stmt->execute(buff);
        //pstmt_s = con->prepareStatement(sqlSELECT);
        //pstmt_d = con->prepareStatement(sqlDELETE);
        pstmt_i = con->prepareStatement(sqlINSERT);
        pstmt_u = con->prepareStatement(sqlUPDATE);
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        std::cout << " MySQL error code: " << e.getErrorCode() << "  SQLState: " << e.getSQLState() ;
        exit(EXIT_FAILURE);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        std::cout <<  " # ERR: " << e.what();
        exit(EXIT_FAILURE);
    }
}

MysqlController::~MysqlController() {
    delete result;
    delete pstmt_s;
    delete pstmt_d;
    delete pstmt_i;
    delete stmt;
    delete con;
}

bool MysqlController::MysqlInsert(std::vector<std::string> vecLine, double stay, double move)
{
   try {  
         bool insert_flg = true;
         std::vector<std::tuple<std::string, double>> spatial_ids = makeSpatialIdList(vecLine[4]);
         std::string wk_YY = vecLine[0].substr(0, 4);
         std::string wk_MM = vecLine[0].substr(4, 2);
         std::string wk_from_datetime = wk_YY + "-" + wk_MM + "-" + "01 00:00:00";
         //std::string wk_to_datetime = wk_YY + "-" + wk_MM + "-" + "31 23:59:59";
         std::string wk_to_datetime = wk_YY + "-" + wk_MM + "-" + "01";
         std::vector<std::string> wk_hour_v = CommonUtil::split(vecLine[2], ':');
         int wk_hour = stoi(wk_hour_v[0]);
         int wk_holiday = stoi(vecLine[1]);
         uint64_t  wk_mesh = stoll(vecLine[4]);
         int wk_city = stoi(vecLine[5]);

         /*pstmt = con->prepareStatement(sqlSELECT);
         pstmt_s->setDateTime(1, wk_from_datetime);
         pstmt_s->setInt(2, wk_hour);
         pstmt_s->setInt(3, wk_holiday);
         pstmt_s->setInt(4, wk_mesh);
         result = pstmt_s->executeQuery();
         

         //key‚Å‘¶Ý‚µ‚Ä‚ê‚Îdelete‚·‚é
         if (result->rowsCount() != 0) {
             //pstmt_d = con->prepareStatement(sqlDELETE);
             pstmt_d->setDateTime(1, wk_from_datetime);
             pstmt_d->setInt(2, wk_hour);
             pstmt_d->setInt(3, wk_holiday);
             pstmt_d->setInt(4, wk_mesh);
             pstmt_d->execute();
         }
         */
         //pstmt = con->prepareStatement(sqlINSERT);
         for (std::tuple<std::string, double> id : spatial_ids) {
             try{
                 pstmt_i->setString(1, std::get<0>(id));
                 pstmt_i->setDateTime(2, wk_from_datetime);
                 pstmt_i->setDateTime(3, wk_to_datetime);
                 pstmt_i->setInt(4, wk_hour);
                 pstmt_i->setInt(5, wk_holiday);
                 pstmt_i->setUInt64(6, wk_mesh);
                 pstmt_i->setInt(7, wk_city);
                 pstmt_i->setDouble(8, stay);
                 pstmt_i->setDouble(9, move);
                 pstmt_i->setDouble(10, stay * std::get<1>(id));
                 pstmt_i->setDouble(11, move * std::get<1>(id));
                 pstmt_i->execute();
             }
             catch (sql::SQLException& e) {
                 if (e.getErrorCode() == 1062) {
                     pstmt_u->setInt(1, wk_city);
                     pstmt_u->setDouble(2, stay);
                     pstmt_u->setDouble(3, move);
                     pstmt_u->setDouble(4, stay * std::get<1>(id));
                     pstmt_u->setDouble(5, move * std::get<1>(id));
                     pstmt_u->setString(6, std::get<0>(id));
                     pstmt_u->setDateTime(7, wk_from_datetime);
                     pstmt_u->setInt(8, wk_hour);
                     pstmt_u->setInt(9, wk_holiday);
                     pstmt_u->setUInt64(10, wk_mesh);
                     result = pstmt_u->executeQuery();
                     insert_flg = false;
                     //return insert_flg;
                 }
                 else {
                     logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
                     logger->error(" ERR: {:s}", e.what());
                     logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
                     std::cout << " MySQL error code: " << e.getErrorCode() << "  SQLState: " << e.getSQLState();
                     exit(9);
                 }

             }

         } 
         return insert_flg;
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error(" ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        std::cout << " MySQL error code: " << e.getErrorCode() << "  SQLState: " << e.getSQLState();
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error(" ERR: {:s}", e.what());
        std::cout << " # ERR: " << e.what();
        exit(9);
    }
}

/*void  MysqlController::MysqlSoftDelete(int pointCloudId) {
    try {
        pstmt = con->prepareStatement(sqlSOFTDELETE);
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        pstmt->setDateTime(1, ss.str());
        pstmt->setInt(2, pointCloudId);
        result = pstmt->executeQuery();
        if (result->rowsCount() != 0) {
            logger->info("point_cloud_id:{:d}  softDelete ended! count = {:d}.", pointCloudId, result->rowsCount());
        }
    }
    catch (sql::SQLException& e) {
        logger->error("# ERR: SQLException in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        logger->error(" (MySQL error code: {:d} , SQLState: {:s} )", e.getErrorCode(), e.getSQLState());
        std::cout << " MySQL error code: " << e.getErrorCode() << "  SQLState: " << e.getSQLState();
        exit(9);
    }
    catch (std::runtime_error& e) {
        logger->error("# ERR: runtime_error in  {:s}  on line  {:d}", __FILE__, __LINE__);
        logger->error("# ERR: {:s}", e.what());
        std::cout << " # ERR: " << e.what();
        exit(9);
    }
}*/