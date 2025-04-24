#ifndef COORDSEGMENTATION_H
#define COORDSEGMENTATION_H

#include <string>
#include <vector>
#include <memory>
#include <proj.h>

struct HeaderInfo {
    int ncols;
    int nrows;
    int xllcorner;
    int yllcorner;
    int cellsize;
    int NODATA_value;
};

class CoordSegmentation {
public:
    CoordSegmentation(int zone, bool isNorthern, std::string confirmLasEpsg);
    ~CoordSegmentation();
    void run(const std::string& inputFile, const std::string& outputFile);

private:
    PJ_CONTEXT* C;
    PJ* P;
    void transformLatLngToUtm(double lat, double lng, int& x, int& y);
    std::vector<std::string> readFile(const std::string& filePath);
    HeaderInfo parseHeader(const std::vector<std::string>& data);
    std::vector<std::string> clipData(const std::vector<std::string>& data, const HeaderInfo& headerInfo,
        double xMin, double xMax, double yMin, double yMax);
    HeaderInfo calculateClippedHeader(const HeaderInfo& headerInfo,
        double xMin, double xMax, double yMin, double yMax);
    void writeClippedData(const std::string& outputFile,
        const std::vector<std::string>& clippedData,
        const HeaderInfo& newHeader);
};

#endif // COORDSEGMENTATION_H