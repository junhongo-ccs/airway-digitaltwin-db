// lasMerge.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
#pragma warning(disable : 4996)
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <vector>
#include <cassert>
#include <memory>
#include <sys/stat.h> 
#include <chrono>
#include <direct.h>
#include <regex>
#include <iomanip>
#include <liblas/liblas.hpp>
#include <cstdlib>
#include "../commonLib/CommonConfig.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "HoldConfig.hpp"
#include "DbController.hpp"

void writeMergeFiles(std::vector<std::string>, std::string, std::shared_ptr<spdlog::logger>);

int main(int argc, char* argv[])
{

    //起動引数のチェック
    if (argc == 1) {
        std::cout << "第１引数でオーナーＩＤを指定してください" << std::endl;
        return 9;
    }
    if (argc >  3) {
        std::cout << "Error! 起動引数が多すぎます!!." << std::endl;
        return 9;
    }

    //jsonPathのセット
    std::string owner_id = argv[1];
    //第２引数が--configだったらconfigPathのセット
    std::string argConfig = "";
    if (argc == 3) {
        std::string strBuff = argv[2];
        if (strBuff.find("--config=") != std::string::npos) {
            argConfig = regex_replace(argv[2], std::regex("--config="), "");
        }
    }
    std::string configPath = COMMON_CONFIG_ID;
    //--config引数がなかったらDB接続しない
    bool configExist = false;
    if (argConfig != "") {
        configPath = argConfig;
        configExist = true;
    }
    //holgConfigに各config値を格納
    HoldConfig holdconfig(configPath, configExist);
    auto logger = holdconfig.getLoggerPointer();

    logger->info("<<<<<<<<   spaceMerge app start   >>>>>>>>");
    logger->info("owner_id =  {:s} ", owner_id);
    logger->info("ConfigPath =  {:s} ", argConfig);
   
  
    try {
        //DB初期処理
        DbController Msc(holdconfig);
        //空間ID＞データタイプで複数件のレコード検索
        std::vector<std::tuple<std::string, std::string>> return1;
        return1 = Msc.selectSumDb(owner_id);
        //明細を再検索してlasをマージ＆論理削除
        for (const auto&  each  : return1) {
            std::vector<std::tuple<int, std::string>> return2;
            std::string spatial_id = std::get<0>(each);
            std::string data_type  = std::get<1>(each);
            //明細検索
            return2 = Msc.selectDb(owner_id, spatial_id, data_type);
            std::vector<std::string> vec_data_path;
            std::vector<int> vec_id;
            std::vector<std::string> vec_file_path;
            std::string first_path = "";
            bool fir_flg = true;
            //明細処理
            for (const auto & each2 : return2) {
                if (fir_flg) {
                    //先頭はマージしてファイル置換
                    first_path = std::get<1>(each2);                  
                }
                else {
                    //先頭以外は論理削除対象
                    vec_id.emplace_back(std::get<0>(each2));
                }
                //マージ対象ファイルをXX_にrenameしてバックアップ
                std::string tmp_file_path = std::get<1>(each2);
                std::filesystem::path path(tmp_file_path);
                std::string sep_path_name = path.parent_path().string();
                std::string sep_file_name = path.filename().string();
                std::string XX_tmp_file_path = sep_path_name + "/XX_" + sep_file_name;
                if (std::filesystem::is_regular_file(tmp_file_path)) {
                    std::filesystem::rename(tmp_file_path, XX_tmp_file_path);
                    //全XX_ファイルがマージ対象
                    vec_file_path.emplace_back(XX_tmp_file_path);
                }
                else {
                   logger->error("file {:s} is not exist! ", tmp_file_path);
                   throw std::exception();
                }
                fir_flg = false;
            }
            writeMergeFiles(vec_file_path, first_path, logger);
            for (int each3 : vec_id) {
                Msc.softDeleteDb(each3);
            }

        }  
    }
    catch (std::runtime_error& ex) {
        logger->error(" spaceMerge Err!! {:s} ", ex.what());
        return 9;
    }
    catch (...) {
        logger->error("  spaceMerge some Err!! ");
        return 9;
    }

    logger->info("<<<<<<<<   spaceMerge app end   >>>>>>>>");
    return 0;
}

void writeMergeFiles(std::vector<std::string> vecInputFiles, std::string outputFilePath, std::shared_ptr<spdlog::logger> logger)
{
    try
    {
    std::ofstream ofs(outputFilePath, std::ios::out | std::ios::binary);
    if ( !ofs ) {
        logger->error("outfile {:s} can not open! ", outputFilePath);
        std::cout << "outfile " << outputFilePath << "  can not open!" << std::endl;
        throw std::exception();

    }

    int aRecCount[5] = { 0, 0, 0, 0, 0 };
    double xmin = 999999999999.9, ymin = 999999999999.9, zmin = 999999999999.9,
        xmax = -999999999999.9, ymax = -999999999999.9, zmax = -999999999999.9;
    int pointCount = 0;
    //header情報取得のため最初のファイルだけを読む
    std::string inputFileName1 = vecInputFiles.at(0);
    std::ifstream ifs1(inputFileName1, std::ios::in | std::ios::binary);
    if (!ifs1) {
        logger->error("infile{:s} can not open!", inputFileName1);
        throw std::exception();
    }
    liblas::Reader reader1(ifs1);
    liblas::Header const& header_in = reader1.GetHeader();
    liblas::Header header_out;
    liblas::SpatialReference srs = header_in.GetSRS();
    //srs.SetFromUserInput(toCrs);	//TODO 出力SRS設定　GDAL　が必要　https://mf-atelier.sakura.ne.jp/mf-atelier2/gdal_ogr_api_tips/
    header_out.SetSRS(srs);
    header_out.SetDataFormatId(header_in.GetDataFormatId());
    header_out.SetScale(header_in.GetScaleX(), header_in.GetScaleY(), header_in.GetScaleZ());
    header_out.SetOffset(header_in.GetOffsetX(), header_in.GetOffsetY(), header_in.GetOffsetZ());
    liblas::Writer writer(ofs, header_out);
    ifs1.close();

    for (int i = 0; i < vecInputFiles.size(); i++) {
        //inputファイル毎の明細の読み書き
        int inCount = 0;
        std::ifstream ifs(vecInputFiles.at(i), std::ios::in | std::ios::binary);
        if (!ifs) {
            logger->error("infile{:s} can not open!", vecInputFiles.at(i));
            throw std::exception();
        }
        liblas::Reader reader(ifs);

        while (reader.ReadNextPoint()) {
            liblas::Point const& p = reader.GetPoint();
            double x = p.GetX();
            double y = p.GetY();
            double z = p.GetZ();
            if (x > xmax) {
                xmax = x;
            }
            if (y > ymax) {
                ymax = y;
            }
            if (z > zmax) {
                zmax = z;
            }
            if (x < xmin) {
                xmin = x;
            }
            if (y < ymin) {
                ymin = y;
            }
            if (z < zmin) {
                zmin = z;
            }
            liblas::Point p_out(&header_out);
            p_out.SetCoordinates(x, y, z);
            p_out.SetColor(p.GetColor());
            p_out.SetClassification(p.GetClassification());
            writer.WritePoint(p_out);
            aRecCount[p.GetReturnNumber()]++;
            pointCount++;
            inCount++;
        }
        logger->info("{:s} input count = {:d} ", vecInputFiles.at(i), inCount);
        ifs.close();
    }
    header_out.SetMin(xmin, ymin, zmin);
    header_out.SetMax(xmax, ymax, zmax);
    //std::cout << "min = " << xmin << "  " << ymin << "  " << zmin << std::endl;
    //std::cout << "max = " << xmax << "  " << ymax << "  " << zmax << std::endl;
    header_out.SetPointRecordsCount(pointCount);
    header_out.SetPointRecordsByReturnCount(0, aRecCount[0]);
    header_out.SetPointRecordsByReturnCount(1, aRecCount[1]);
    header_out.SetPointRecordsByReturnCount(2, aRecCount[2]);
    header_out.SetPointRecordsByReturnCount(3, aRecCount[3]);
    header_out.SetPointRecordsByReturnCount(4, aRecCount[4]);
    writer.SetHeader(header_out);
    writer.WriteHeader();
    ofs.close();
    logger->info("{:s} output count =  {:d} ", outputFilePath, pointCount);
    }
    catch (std::exception const& e)
    {
        logger->error("Error: {:s} ", e.what());
        exit(9);
    }

}

