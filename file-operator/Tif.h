//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_TIF_H
#define CLUSTERING_TIF_H

#include <gdal_priv.h>
#include <gdal.h>
#include <iostream>
#include <unordered_map>
#include "Cst.h"
#include "DataModel.h"
#include "Rst.h"

using std::string;

class Tif: public Rst {
public:
    static GDALDriver *driver;

    Tif(Meta meta);
    Tif(string fName);
    Tif(Meta _meta, string _name);
    Tif(Meta _meta, string _name, float *_data);
    ~Tif();
    bool read(string fName = "") override;
    bool write(string fName = "") override;

    void smooth();

    static bool readInt(string fn, int* pData, float scale = 0.001);
    static bool writeInt(string fn, Meta& meta, int* pData);

};

#endif //CLUSTERING_TIF_H
