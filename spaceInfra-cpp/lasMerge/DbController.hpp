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
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;
    sql::ResultSet* result;
    std::shared_ptr<spdlog::logger> logger;
    //const std::string TEMP_OWNER_ID = "unitTest";
    const std::string TEMP_DATA_UNIT = "sp17";
    const std::string TEMP_DATA_IO = "lasMerge_output";
    const std::string REC_ID = "lasMerge";

    std::string sqlSELECT = "SELECT * FROM space_data_manages WHERE id = ?";

    std::string sqlSELECT2 = "SELECT * FROM space_data_manages WHERE owner_id = ? and batch_id = ? and spatial_id =? and rec_id =?";
   
    std::string sqlINSERT = "INSERT INTO space_data_manages("
        "owner_id,batch_id,spatial_id,rec_id,file_id,file_path,epsg_las,epsg_csv,data_unit,data_io,data_type,"
        "data_status,data_abcd,sp_xmin,sp_xmax,sp_ymin,sp_ymax,sp_zmin,sp_zmax,originalFileIds,points,"
        "xmin,xmax,ymin,ymax,zmin,zmax,"
        "cls0,cls1,cls2,cls3,cls4,cls5,cls6,cls7,cls8,cls9,cls10,"
        "cls11,cls12,cls13,cls14,cls15,cls16,cls17,cls18,cls19,cls20,cls99,created_at,"
        "sp_xmin_w,sp_xmax_w,sp_ymin_w,sp_ymax_w,xmin_w,xmax_w,ymin_w,ymax_w)"
        " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
        "?, ?, ?, ?, ?, ?,"
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
        "?, ?, ?, ?, ?, ?, ?, ?)";

    std::string sqlUPDATE = "UPDATE space_data_manages SET "
        "file_id = ?,file_path = ?,epsg_las = ?,epsg_csv = ?,data_unit = ?, data_io = ?, data_type = ?,data_status = ?,"
        "data_abcd = ?,sp_xmin = ?,sp_xmax = ?,sp_ymin = ?,sp_ymax = ?,sp_zmin = ?, sp_zmax = ?,originalFileIds = ?,"
        "points = ?,xmin = ?,xmax = ?,ymin = ?,ymax = ?,zmin = ?,zmax = ?,"
        "cls0 = ? ,cls1 = ? ,cls2 = ? ,cls3 = ? ,cls4 = ? ,cls5 = ? ,cls6 = ? ,cls7 = ? ,cls8 = ? ,cls9 = ? ,"
        "cls10 = ? ,cls11 = ?,cls12 = ?,cls13 = ?,cls14 = ?,cls15 = ?,cls16 = ?,cls17 = ?,cls18 = ?,cls19 = ?,"
        "cls20 = ?,cls99 = ?,updated_at  = ?,"
        "sp_xmin_w = ?,sp_xmax_w = ?,sp_ymin_w = ?,sp_ymax_w = ?,xmin_w = ?,xmax_w = ?,ymin_w = ?,ymax_w = ? "
        "WHERE owner_id = ? and batch_id = ? and spatial_id =? and rec_id =?";
   
    std::string sqlSOFTDELETE = "UPDATE space_data_manages SET deleted_at  = ? WHERE id = ? ";

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
    double sp_xmin = 0.0;
    double sp_xmax = 0.0;
    double sp_ymin = 0.0;
    double sp_ymax = 0.0;
    double sp_zmin = 0.0;
    double sp_zmax = 0.0;
    double sp_xmin_w = 0.0;
    double sp_xmax_w = 0.0;
    double sp_ymin_w = 0.0;
    double sp_ymax_w = 0.0;
    std::string originalFileIds = "";
    std::string wkABCD = "    ";
    int points = 0;
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;
    double zmin = 0.0;
    double zmax = 0.0;
    double xmin_w = 0.0;
    double xmax_w = 0.0;
    double ymin_w = 0.0;
    double ymax_w = 0.0;
    int clsCount[22] = {};
    bool first_flag = true;

public:
    //! @brief DbControllerのコンストラクタ(MySQL接続等の前処理)
    //! @param HoldConfigのアドレス
    DbController(HoldConfig);  // コンストラクタ
    ~DbController();  // デストラクタ
    //! @brief sqlのパラメータ種により処理を分岐
    //! @param jsonのsqlパラメータのリスト
    void updateDb(std::vector<std::string>);

    //! @brief sqlパラメータがgetの時にidをキーにselect&mergeする
    //! @param id
    void selectDb(std::string id);

    //! @brief sqlパラメータがinsertの時にmerge結果をinsertかupdateする
    //! @param アウトプットファイルパス
    void insertDb(std::string filePath);

    //! @brief sqlパラメータで指定されたidのレコードのdelete日時を更新する
    //! @param id
    void softDeleteDb(std::string id);
};
#endif