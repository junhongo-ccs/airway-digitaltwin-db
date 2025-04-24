// lasMerge.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
#pragma warning(disable : 4996)
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
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
#include <filesystem>
#include "CommonConfig.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "HoldConfig.hpp"
#include "convertLas.h"
#include <proj.h>

void writeMergeFiles(std::vector<std::string>, std::string, std::string, std::string, std::shared_ptr<spdlog::logger>);
void initializeTransform(std::string, std::string);
void transformPoint(double&, double&);

PJ_CONTEXT* ctx;
PJ* transform;

int main(int argc, char* argv[])
{

    //起動引数のチェック
    if (argc == 1) {
        std::cout << "第１引数でjsonファイルのフルパスを指定してください" << std::endl;
        return 9;
    }
    if (argc > 6) {
        std::cout << "Error! 起動引数が多すぎます!!." << std::endl;
        return 9;
    }

    //jsonPathのセット
    std::string jsonPath = argv[1];
    //入力EPSG
    std::string inputEPSG = "";
	//出力EPSG
    std::string outputEPSG = "";

    // オプション引数の解析
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--iepsg" && i + 1 < argc) {
            inputEPSG = argv[++i];
        }
        else if (arg == "--oepsg" && i + 1 < argc) {
            outputEPSG = argv[++i];
        }
        else {
            std::cout << "Error! 不明なオプション: " << arg << std::endl;
            return 9;
        }
    }

    //holgConfigに各config値を格納
    HoldConfig holdconfig;
    auto logger = holdconfig.getLoggerPointer();

    logger->info("<<<<<<<<   lazMerge app start   >>>>>>>>");
    logger->info("jsonPath =  {:s} ", jsonPath);
    if (!inputEPSG.empty()) {
        logger->info("inputEPSG  =  {:s} ", inputEPSG);
    }
    if (!outputEPSG.empty()) {
        logger->info("outputEPSG =  {:s} ", outputEPSG);
    }

	if (!inputEPSG.empty() && !outputEPSG.empty()) {
		initializeTransform(inputEPSG, outputEPSG);
	}
   
    try {
        std::ifstream ifs(jsonPath);
        if (!ifs) {
            logger->error("MergeJson  {:s} can not open! ", jsonPath);
            return 9;
        }
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);

        auto& execPoint = doc["exec"];
        
        for (size_t idx = 0; idx < execPoint.Size(); ++idx) {
            std::vector<std::string> vecInputFiles;
            logger->info("{:d} turn  ", idx + 1);
            for (auto itr = execPoint[idx]["input"].Begin(); itr != execPoint[idx]["input"].End(); ++itr) {
                vecInputFiles.push_back(itr->GetString());
            }
            //マージ読み書き
            writeMergeFiles(vecInputFiles, execPoint[idx]["output"].GetString(), inputEPSG, outputEPSG, logger);
        }

		if (!inputEPSG.empty() && !outputEPSG.empty()) {
			proj_destroy(transform);
			proj_context_destroy(ctx);
		}
    }
    catch (std::runtime_error& ex) {
        logger->error(" lazMerge Err!! {:s} ", ex.what());
        return 9;
    }
    catch (...) {
        logger->error("  lazMerge some Err!! ");
        return 9;
    }

    logger->info("<<<<<<<<   lazMerge app end   >>>>>>>>");
    return 0;
}


void initializeTransform(std::string inputEpsg, std::string outputEpsg) {
    // PROJの初期化
    PJ_CONTEXT* ctx;
    //PJ* transform;
    ctx = proj_context_create();
    if (ctx == nullptr) {
        throw std::runtime_error("Failed to create PROJ context");
    }

    // 入出力EPSGコードを設定
    std::string src_utm = "EPSG:" + inputEpsg;

    std::string target_epsg = "EPSG:" + outputEpsg;

    transform = proj_create_crs_to_crs(ctx, src_utm.c_str(), target_epsg.c_str(), NULL);
    if (transform == nullptr) {
        proj_context_destroy(ctx);
        throw std::runtime_error("Failed to create transformation");
    }
}


// 点群を変換する
void transformPoint(double& x, double& y) {
	//std::cout << "x = " << x << "  y = " << y << std::endl;
    PJ_COORD input_coord;
	input_coord.v[0] = y;   // 緯度
    input_coord.v[1] = x;   // 経度
	input_coord.v[2] = 0.0; // 高さ
    input_coord.v[3] = HUGE_VAL;
    PJ_COORD output_coord = proj_trans(transform, PJ_FWD, input_coord);
    // エラーチェック
    if (proj_errno(transform) != 0) {
        int err = proj_errno(transform);
        const char* err_msg = proj_errno_string(err);
        std::cerr << "座標変換エラー: " << err_msg << std::endl;
        y = x = std::numeric_limits<double>::quiet_NaN();
        return;
    }
    x = output_coord.v[0];
    y = output_coord.v[1];
}

void writeMergeFiles(std::vector<std::string> vecInputFiles, std::string outputFilePath, std::string inputEPSG, std::string outputEPSG, std::shared_ptr<spdlog::logger> logger)
{
    try
    {
        std::string inputFileName1 = vecInputFiles.at(0);

        int path_i = outputFilePath.find_last_of("/") + 1;
        std::string outputPathname = outputFilePath.substr(0, path_i + 1);
        std::string tmpLasFileName = outputPathname + "XX_tmpfile.las";
        std::string tmpLasFileName2 = outputPathname + "XX_tmpfile2.las";
        convertLas(inputFileName1, tmpLasFileName);
        std::ofstream ofs(tmpLasFileName2, std::ios::out | std::ios::binary);
        if ( !ofs ) {
            logger->error("outfile {:s} can not open! ", tmpLasFileName2);
            std::cout << "outfile " << tmpLasFileName2 << "  can not open!" << std::endl;
            throw std::exception();

        }

        int aRecCount[5] = { 0, 0, 0, 0, 0 };
        double xmin = 999999999999.9, ymin = 999999999999.9, zmin = 999999999999.9,
            xmax = -999999999999.9, ymax = -999999999999.9, zmax = -999999999999.9;
        int pointCount = 0;
        //header情報取得のため最初のファイルだけを読む
   
        std::ifstream ifs1(tmpLasFileName, std::ios::in | std::ios::binary);
        if (!ifs1) {
            logger->error("infile{:s} can not open!", inputFileName1);
            throw std::exception();
        }
        liblas::Reader reader1(ifs1);
        liblas::Header const& header_in = reader1.GetHeader();
        liblas::Header header_out;
        liblas::SpatialReference srs;

        //srs.SetFromUserInput(toCrs);	//TODO 出力SRS設定　GDAL　が必要　https://mf-atelier.sakura.ne.jp/mf-atelier2/gdal_ogr_api_tips/
        header_out.SetSRS(srs);
        header_out.SetDataFormatId(header_in.GetDataFormatId());
		// TODO 暫定でx, yのlasScaleを0.01に設定
        header_out.SetScale(0.01, 0.01, header_in.GetScaleZ());
        liblas::Writer writer(ofs, header_out);
        ifs1.close();
        std::filesystem::remove(tmpLasFileName);

        for (int i = 0; i < vecInputFiles.size(); i++) {
            //inputファイル毎の明細の読み書き
            int inCount = 0;
            convertLas(vecInputFiles.at(i), tmpLasFileName);
            std::ifstream ifs(tmpLasFileName, std::ios::in | std::ios::binary);
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

                // EPSG変換
			    if (!inputEPSG.empty() && !outputEPSG.empty()) {
				    transformPoint(x, y);
			    }

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
            std::filesystem::remove(tmpLasFileName);
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
        //convertLas(tmpLasFileName2, outputFilePath);
        std::filesystem::rename(tmpLasFileName2, outputFilePath);
        logger->info("{:s} output count =  {:d} ", outputFilePath, pointCount);
    }
    catch (std::exception const& e)
    {
        logger->error("Error: {:s} ", e.what());
        exit(9);
    }

}

