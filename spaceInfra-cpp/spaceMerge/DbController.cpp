#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <sstream>
#include <chrono>
#include <regex>
#include <iomanip>
#include <vector>
#include "DbController.hpp"
#include <mysql_error.h>


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


std::vector<std::tuple<std::string, std::string>> DbController::selectSumDb(std::string owner_id) {
    try {
        pstmt = con->prepareStatement(sqlSELECT);
        pstmt->setString(1, owner_id);
        result = pstmt->executeQuery();
        std::vector<std::tuple<std::string, std::string>> vec1;
        while (result->next()) {
            std::tuple<std::string, std::string> tap1( result->getString(1) ,result->getString(2) );
            vec1.emplace_back(tap1);
        }
        return vec1;
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
std::vector<std::tuple<int, std::string>> DbController::selectDb(std::string owner_id, std::string spatial_id, std::string data_type) {
    try {
        pstmt = con->prepareStatement(sqlSELECT2);
        pstmt->setString(1, owner_id);
        pstmt->setString(2, spatial_id);
        pstmt->setString(3, data_type);
        result = pstmt->executeQuery();
        //ÉLÅ[Ç≈SELECTÇµÇƒÉ[ÉçåèÇæÇ¡ÇΩÇÁINSERTÅAÇªÇÃëºÇÕUPDATE
        std::vector<std::tuple <int, std::string>> vec1;
        while (result->next()) {
            std::tuple <int, std::string> tap1(result->getInt(1), result->getString(2));
            vec1.emplace_back(tap1);
        }
        return vec1;
          
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
void DbController::softDeleteDb(int id) {
    try {
        pstmt = con->prepareStatement(sqlSOFTDELETE);
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        pstmt->setDateTime(1, ss.str());
        pstmt->setInt(2, id);
        result = pstmt->executeQuery();
        logger->info("id:{:d}  softDelete ended   " , id);
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