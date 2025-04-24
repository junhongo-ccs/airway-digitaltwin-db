#ifndef __MYSQLCONTROLLER_HPP_INCLUDED__
#define __MYSQLCONTROLLER_HPP_INCLUDED__

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/Statement.h>
#include <cppconn/ResultSet.h>
#include <cppconn/prepared_statement.h>
#include "CommonConfig.h"
#include "MakeSpatialIdList.h"

//! @brief Mysqlの各種コントロール(接続、検索、更新等)を行うクラス
class MysqlController
{
private:
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt_s;
    sql::PreparedStatement* pstmt_d;
    sql::PreparedStatement* pstmt_i;
    sql::PreparedStatement* pstmt_u;
    sql::ResultSet* result;
    std::shared_ptr<spdlog::logger> logger;
    //const std::string TEMP_OWNER_ID = "unitTest";
   // const std::string TEMP_DATA_UNIT = "sp17";
   // const std::string TEMP_DATA_IO = "Csvsp2Space_output";
   // const std::string REC_ID = "Csvsp2Space";

   
    std::string sqlSELECT = "SELECT * FROM average_population WHERE from_datetime = ? and hour =? and holiday_flg =? and mesh_area =?";

    std::string sqlDELETE = "DELETE FROM average_population WHERE from_datetime = ? and hour =? and holiday_flg =? and mesh_area =?";

    std::string sqlINSERT = "INSERT INTO average_population("
            "spatial_id,from_datetime,to_datetime,"
            "hour,holiday_flg,mesh_area,city_code,stay_average_population,move_average_population,"
            "stay_average_population_spatial,move_average_population_spatial,created_at,updated_at)"
            " VALUES(? , ? , CONCAT (LAST_DAY(?),' 23:59:59'), ? , ? , ? , ? , ? , ? , ? , ? ,NOW(), NOW())";

    std::string sqlUPDATE = "UPDATE average_population SET "
        "city_code = ?,stay_average_population = ?,move_average_population = ?, "
        "stay_average_population_spatial = ?,move_average_population_spatial = ?,updated_at = NOW() "
        "WHERE spatial_id = ? and from_datetime = ? and hour =? and holiday_flg =? and mesh_area =?";

public:
//! @brief MysqlControllerのコンストラクタ(MySQL接続等の前処理)
//! @param HoldConfigのアドレス
//!
    MysqlController(std::string btDbHost, std::string btDbName, std::string btDbUser, std::string btDbPass, std::shared_ptr<spdlog::logger> logger);  // コンストラクタ
    ~MysqlController();  // デストラクタ

//! @brief MysqlControllerのコンストラクタ(MySQL接続等の前処理)
//! @param ownerId
//! @param batchId
//! @param インデックスファイル1行のリスト
//! @param アウトファイル名(XX無)
//! @param アウトプットファイルフルパス
//! @param ポイント数
//! @param classfication別ポイント数
//! @param InLasEpsg
//! @param CsvEpsg
//! @param PROJ(座標変換用)
//!
//! 
    //void MysqlSoftDelete(int pointCloudId);
    bool MysqlInsert(std::vector<std::string> vecList, double stay, double move);

};
#endif