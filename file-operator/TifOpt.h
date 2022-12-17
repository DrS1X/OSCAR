//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_TIFOPT_H
#define CLUSTERING_TIFOPT_H

#include <gdal_priv.h>
#include <gdal.h>
#include <iostream>
#include "_const.h"
#include "DataModel.h"
#include "RFileOpt.h"

using std::string;

class TifOpt:public RFileOpt {
public:
    GDALDriver *geoTiffDriver;

    TifOpt();
    bool read(RFile file) override;
    bool write(RFile file) override;
    void getMeta(string fileName, Meta& meta) override;

    static bool readFlatten(string file, int* pData);
    static bool readFlatten(string file, double* pData);
    static bool writeFlatten(string file, Meta meta, int * pData);

    //TODO scale
};

#endif //CLUSTERING_TIFOPT_H
