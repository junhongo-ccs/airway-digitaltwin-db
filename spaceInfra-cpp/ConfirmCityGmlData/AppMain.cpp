//! @file AppMain.cpp
//! @brief メインファイル

#include <time.h>
#include "Config.h"
#include <spdlog/spdlog.h>
#include "CommonUtil.h"
#include "ConfirmHandler.h"

//! @brief メイン関数
//! 
//! @param argc 引数の数
//! @param **argv 引数が格納されている配列
int main(int argc, char** argv)
{
    int ret = 0;

    //UINT savedCp = GetConsoleOutputCP();
    //UINT defCp = GetACP();
    //if (savedCp != defCp) {
    //    SetConsoleOutputCP(defCp);
    //}


    clock_t t_start = clock();

    //Config
    Config& config = Config::getInstance();
    config.loadConfig(argc, argv);
    auto logger = spdlog::get(config.getLoggerName());

    try {


        logger->info("<<<<<<<<<<<<< ConfirmCityGmlData 処理開始(バッチID:{0}) >>>>>>>>>>>>", config.getBatchId());


        //確定処理
        ConfirmHandler handler;
        ret = handler.execute();

        clock_t t_end = clock();

        logger->debug("++ 経過時間：{:.2f}(秒)", (double)(t_end - t_start) / CLOCKS_PER_SEC);
    }
    catch (std::bad_alloc& ex) {
        logger->error("メモリが足りません{0}");
        ret = 9;
    }
    catch (std::runtime_error& ex) {
        logger->error(ex.what());
        ret = 9;
    }
    catch (...) {
        logger->error("予期せねエラーが発生しました");
        ret = 9;
    }

    logger->trace("+++ ret:{0}", ret);
    logger->info("<<<<<<<<<<<<< ConfirmCityGmlData 処理終了 経過時間:{0}(秒)(バッチID:{1}) >>>>>>>>>>>>",
        (double)(clock() - t_start) / CLOCKS_PER_SEC, config.getBatchId());

    //SetConsoleOutputCP(savedCp);

    return ret;
}
