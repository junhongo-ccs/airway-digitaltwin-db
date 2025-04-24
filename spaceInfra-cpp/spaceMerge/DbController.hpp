#ifndef __DBCONTROLLER_HPP_INCLUDED__
#define __DBCONTROLLER_HPP_INCLUDED__

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/Statement.h>
#include <cppconn/ResultSet.h>
#include <cppconn/prepared_statement.h>
#include "../commonLib/CommonConfig.h"
#include "HoldConfig.hpp"

//! @brief Mysqlの各種コントロール(接続、検索、更新等)を行うクラス
class DbController
{
private:
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con = NULL;
    sql::Statement* stmt = NULL;
    sql::PreparedStatement* pstmt = NULL;
    sql::ResultSet* result = NULL;
    std::shared_ptr<spdlog::logger> logger;
    //const std::string TEMP_OWNER_ID = "unitTest";
    const std::string TEMP_DATA_UNIT = "sp17";
    const std::string TEMP_DATA_IO = "lasMerge_output";
    const std::string REC_ID = "lasMerge";

    //複数件検索用
    std::string sqlSELECT = "SELECT spatial_id, data_type FROM space_data_manages WHERE owner_id = ? and deleted_at is NULL "
                            "GROUP BY spatial_id, data_type HAVING count(data_type) >= 2 ";

    //明細検索用
    std::string sqlSELECT2 = "SELECT id, file_path FROM space_data_manages WHERE owner_id = ? and spatial_id = ? and data_type = ? and deleted_at is NULL ORDER BY batch_id" ;
  
    //論路削除用
    std::string sqlSOFTDELETE = "UPDATE space_data_manages SET deleted_at  = ? WHERE id = ? ";

    /*
    std::string owner_id ="";
    std::string batch_id = "";
    std::string spatial_id = "";
    std::string rec_id = "";
    std::string file_id = "";
    std::string file_path = "";
    std::string epsg_las = "";
    std::string epsg_csv = "";
    std::string data_unit = "";
    std::string data_io = "";
    std::string data_type = "";
    std::string data_status = "";
    std::string data_abcd = "";
    bool first_flag = true;
    */

public:
    //! @brief DbControllerのコンストラクタ(MySQL接続等の前処理)
    //! @param HoldConfigのアドレス
    DbController(HoldConfig);  // コンストラクタ
    ~DbController();  // デストラクタ

    //! @brief キーで複数件あるレコードを検索
    //! @param owner_id
    std::vector<std::tuple<std::string, std::string>>  selectSumDb(std::string owner_id);

    //! @brief 複数件の内容明細を検索
    //! @param owner_id、空間ID、データタイプ
    std::vector<std::tuple<int, std::string>> selectDb(std::string owner_id, std::string spatial_id, std::string data_type);

    //! @brief sqlパラメータで指定されたidのレコードのdelete日時を更新する
    //! @param id
    void softDeleteDb(int id);
};
#endif