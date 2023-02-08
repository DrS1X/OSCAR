//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_TIF_H
#define CLUSTERING_TIF_H

#include <gdal_priv.h>
#include <gdal.h>
#include <gdal_alg.h>
#include <iostream>
#include <unordered_map>
#include <iostream>
#include <istream>
#include <streambuf>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include "Cst.h"
#include "DataModel.h"
#include "Rst.h"
#include "Shp.h"

using std::string;

class Tif: public Rst {
public:
    static GDALDriver *driver;

    Tif();
    Tif(Meta meta);
    Tif(string fName);
    Tif(Meta _meta, string _name);
    Tif(Meta _meta, string _name, float *_data);
    ~Tif();
    bool read(string fName = "", map<string,string> *metaData = nullptr) override;
    bool write(string fName = "") override;

    bool polygonize(OGRLayer * poLayer, int iPixValField);
    void smooth();
    void mask(float threshold);
    inline void setField(GDALDataset* dataset);

    static bool readInt(string fn, int* pData, float scale = 1.0f);
    static bool writeInt(string fn, Meta& meta, int* pData);
};

#endif //CLUSTERING_TIF_H
