//! @file AppMain.cpp
//! @brief メインファイル

#include <time.h>
//#include <ogrsf_frmts.h>
#include "Config.h"
#include <spdlog/spdlog.h>
#include "CommonUtil.h"
#include "Converter.h"
//#ifdef _WIN32
//#include <windows.h>
//#endif


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

    unsigned long handle = 0L;

    try {

        logger->info("<<<<<<<<<<<<< MakeAreaVoxel 処理開始(thredCount:{0}) >>>>>>>>>>>>", config.getThreadCount());

//#ifdef _WIN32
//        handle = (unsigned long)::CreateMutexA(NULL, FALSE, (LPCSTR)"makeAreaVoxel");
//        if (::GetLastError() == ERROR_ALREADY_EXISTS)
//        {
//            throw std::runtime_error("MakeAreaVoxelが既に起動されています");
//        }
//
//#endif

 
        //GDALAllRegister();

        Converter converter;
        converter.convert();

        clock_t t_end = clock();

        logger->debug("++ 経過時間：{:.2f}(秒)", (double)(t_end - t_start) / CLOCKS_PER_SEC);
    }
    catch (std::bad_alloc& ex) {
        logger->error("メモリが足りません");
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

    logger->info("<<<<<<<<<<<<< MakeAreaVoxel 処理終了 経過時間:{0}(秒) >>>>>>>>>>>>", (double)(clock() - t_start) / CLOCKS_PER_SEC);

    //SetConsoleOutputCP(savedCp);

//#ifdef _WIN32
//    if (handle > 0L) {
//        ::CloseHandle((HANDLE)handle);
//    }
//#endif

    return ret;
}
