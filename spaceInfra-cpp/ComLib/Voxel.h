#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include "../../SpatialId/src/spatial_api.h"
#include <spdlog/spdlog.h>
//#include "../../SpatialId/src/common/spatial_point.h"
//#include "../../SpatialId/src/common/spatial_polygons.h"
//#include "CommonUtil.h"
//#include <liblas/liblas.hpp>
//#include <laszip/laszip_api.h>


namespace ComLib {
    class Voxel
    {
        std::shared_ptr<spdlog::logger> logger;
        std::mutex mutex;

        std::string spatialId;   // 空間ID
        std::string voxelFilePath;
        int lasEpsg;
        int globalZoom;
        int localZoom;
        spatialid::Zfxy baseLocalZfxy;  // ローカル空間ID(北西下)
        int lonIndexSize;  // ローカル空間IDに分割した際の経度方向数
        int latIndexSize;  // ローカル空間IDに分割した際の緯度方向数
        int altIndexSize;  // ローカル空間IDに分割した際の高さ数
        int lonIndexByte;  // 経度方向のデータ保持に必要なバイト数
        int planeSize;  // 1平面（経度x緯度）のボクセルデータサイズ[bytes]
        bool needCheckInner = false;
        double lasMinX = INT_MAX, lasMaxX = INT_MIN;
        double lasMinY = INT_MAX, lasMaxY = INT_MIN;
        double lasMinZ = INT_MAX, lasMaxZ = INT_MIN;
        int lasPointCount = 0;
        double defaultLasScale = 0.0001;
        //bool useScaleZ = false;
        double scaleZ = 0.0001;

        std::map<int, std::unique_ptr<unsigned char[]>> voxelData;
        void checkInnerVoxel(const std::unordered_set<std::string>& spatialIdList);
        void setBit(int altIndex, int lonIndex, int latIndex) {
            std::lock_guard<std::mutex> lock(mutex);

            if (voxelData.find(altIndex) == voxelData.end()) {
                unsigned char* pBuf = new unsigned char[planeSize]();
                voxelData[altIndex].reset(pBuf);
            }


            int bytePos = latIndex * lonIndexByte + (lonIndex >> 3);
            int bitPos = lonIndex & 0x7;
            voxelData[altIndex].get()[bytePos] |= (1 << bitPos);
        }

        void setVoxelBuf(const spatialid::Zfxy& zfxy) {
            int altIndex = zfxy.alt_index - baseLocalZfxy.alt_index;
            int lonIndex = zfxy.lon_index - baseLocalZfxy.lon_index;
            int latIndex = zfxy.lat_index - baseLocalZfxy.lat_index;

            setBit(altIndex, lonIndex, latIndex);
        }
    protected:
        virtual void loadExistedData();

    public:
        Voxel(const std::string& spatialId, int localZoom, int lasEpsg, std::string& voxelFilePath, 
            const std::string& loggerName,bool needCheckInner = false);
        virtual ~Voxel() {
            resetAll();
            ////明示的にリセット
            //for (int i = 0; i < altIndexSize; i++) {
            //    if (voxelData.find(i) != voxelData.end()) {
            //        voxelData[i].reset();
            //    }
            //}
        };

        void resetAll() {
            //明示的にリセット
            for (int i = 0; i < altIndexSize; i++) {
                if (voxelData.find(i) != voxelData.end()) {
                    voxelData[i].reset();
                }
            }
            voxelData.clear();
        }

        void addPolygon(std::vector<spatialid::Triangle>& triangles, spatialid::CRS crs);
        void addLocalZoomSpatialIds(std::unordered_set<std::string>& localZoomSpatialIds);
        virtual void addFile(std::string& filePath);

        void toFile(int color = 0xff2000) {
            toFile(defaultLasScale, color);
        }

        void toFile(double lasScale, int color = 0xff2000);
        void toLas(/*std::string& outputLazFileName, int epsg, */ double scale = 0.0001, int color = 0xff2000);
        void las2laz(std::string& lasFileName, std::string& lazFileName);

        //std::string calcHash();
        //std::string calcHashEx();

        bool  testPlane(int altIndex) {
            return (voxelData.find(altIndex) != voxelData.end());
        }

        bool testBit(int lonIndex, int latIndex, int altIndex) {
            if (voxelData.find(altIndex) != voxelData.end()) {
                unsigned char* plane = voxelData[altIndex].get();
                uint64_t offset = (uint64_t)lonIndexByte * latIndex + (lonIndex / 8);
                int bit_shift = lonIndex % 8;
                if ((plane[offset] & (1 << bit_shift)) != 0) {
                    return true;
                }
            }

            return false;
        }

        int getLonIndexSize() {
            return lonIndexSize;
        }
        int getLatIndexSize() {
            return latIndexSize;
        }
        int getAltIndexSize() {
            return altIndexSize;
        }
        int getLonIndexBytes() {
            return lonIndexByte;
        }
        int getLocalZoom() {
            return localZoom;
        }
        std::string getSpatialId() {
            return spatialId;
        }

        spatialid::Zfxy& getBaseLocalZfxy() {
            return baseLocalZfxy;
        }

        std::string getVoxelFilePath() {
            return voxelFilePath;
        }

        std::map<int, std::unique_ptr<unsigned char[]>>& getVoxelData() {
            return voxelData;
        }

        int getPlaneSize() {
            return planeSize;
        }

        double getDataSize() {
            double tmpSize = voxelData.size() * (double)planeSize / (2<<20);
            return tmpSize;
        }

        void getLasMinMax(double& minX, double& maxX, double& minY, double& maxY,
            double& minZ, double& maxZ) {
            minX = lasMinX;
            maxX = lasMaxX;
            minY = lasMinY;
            maxY = lasMaxY;
            minZ = lasMinZ;
            maxZ = lasMaxZ;
        }

        int getLasPointCount() {
            return lasPointCount;
        }

        void setScaleZ(double value) {
            scaleZ = value;
        }
    };


};
