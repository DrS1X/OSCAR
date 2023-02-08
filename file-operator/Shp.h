//
// Created by 15291 on 2022/12/15.
//

#ifndef CLUSTERING_SHP_H
#define CLUSTERING_SHP_H

#include <gdal_priv.h>
#include <gdal.h>
#include <string>
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <cpl_conv.h>
#include "DataModel.h"

using std::string;
class Shp {
public:
    static GDALDriver *driver;
    static void createFields(OGRLayer *poLayer);
    static void setFields(Poly* polygon, string time, OGRFeature *poFeature);
    static void write(string outPath, string startTime, string abnormalType, vector<Poly>& polygons);
};


#endif //CLUSTERING_SHP_H
