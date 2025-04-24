
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <bitset>
#include <regex>
#include <liblas/liblas.hpp>
#include <zlib.h>
#include <laszip/laszip_api.h>
#include "convertLas.h"


void convertLas(std::string& fromFileName, std::string& toFileName) {
    //clock_t ts = clock();

    //std::string lazFileName = std::regex_replace(lasFileName, std::regex("\\.las"), ".laz");
    if (fromFileName == toFileName) {
        throw std::runtime_error("LASファイルの形式が間違っています");
    }
    //std::cout << "lazFile:" << lazFileName << std::endl;

    //create reader
    laszip_POINTER laszip_reader;
    if (laszip_create(&laszip_reader)) {
        throw std::runtime_error("failed to create laszip reader");
    }

    //open reader
    laszip_BOOL is_compressed = 0;
    if (laszip_open_reader(laszip_reader, fromFileName.c_str(), &is_compressed))
    {
        throw std::runtime_error("failed to open laszip reader(" + fromFileName + ")");
    }
    //std::cout << "is_compressed:" << is_compressed << std::endl;

    //get a ponter to the header
    laszip_header* header;

    if (laszip_get_header_pointer(laszip_reader, &header))
    {
        throw std::runtime_error("failed to get the header pointer from laszip reader");
    }

    // how many points does the file have
    laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

    //logger->trace("polygon:{0}, points in las: {1}", polygonInd, npoints);

    // get a pointer to the points that will be read
    laszip_point* point;

    if (laszip_get_point_pointer(laszip_reader, &point))
    {
        throw std::runtime_error("failed to get point pointer from laszip reader");
    }

    // create the writer
    laszip_POINTER laszip_writer;
    if (laszip_create(&laszip_writer))
    {
        throw std::runtime_error("failed to create laszip writer");
    }

    // initialize the header for the writer using the header of the reader
    if (laszip_set_header(laszip_writer, header))
    {
        throw std::runtime_error("failed to set header for laszip writer");
    }

    // open the writer
    laszip_BOOL compress = (is_compressed ? false : true);

    if (laszip_open_writer(laszip_writer, toFileName.c_str(), compress))
    {
        throw std::runtime_error("failed to open laszip writer");
    }

    // read the points
    laszip_I64 p_count = 0;
    while (p_count < npoints)
    {
        // read a point
        if (laszip_read_point(laszip_reader))
        {
            throw std::runtime_error("failed to read point");
        }

        // copy the point
        if (laszip_set_point(laszip_writer, point))
        {
            throw std::runtime_error("failed to set point");
        }

        // write the point

        if (laszip_write_point(laszip_writer))
        {
            throw std::runtime_error("failed to write point");
        }

        p_count++;
    }

    // close the writer
    if (laszip_close_writer(laszip_writer))
    {
        throw std::runtime_error("failed to close laszip writer");
    }

    // destroy the writer
    if (laszip_destroy(laszip_writer))
    {
        throw std::runtime_error("failed to destroy laszip writer");
    }

    // close the reader
    if (laszip_close_reader(laszip_reader))
    {
        throw std::runtime_error("failed to close laszip reader");
    }

    // destroy the reader

    if (laszip_destroy(laszip_reader))
    {
        throw std::runtime_error("failed to destroy laszip reader");
    }

    ////LASファイルの削除
    //if (!config.needKeepLas() && std::filesystem::exists(lasFileName)) {
    //    bool rc = false;
    //    for (int i = 0; i < 3; i++) {
    //        try {
    //            std::filesystem::remove(lasFileName);
    //            rc = true;
    //            break;
    //        }
    //        catch (...) {
    //            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //        }
    //    }
    //    if (!rc) {
    //        logger->warn("削除エラー：{1}", lasFileName);
    //    }
    //}

    //logger->debug("+++  las2laz::polygon:{0}, p_count:{1}, elapsed time:{2:.2f}", polygonInd, p_count, (double)(clock() - ts) / CLOCKS_PER_SEC);
}
