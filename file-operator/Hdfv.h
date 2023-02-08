//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_HDFV_H
#define CLUSTERING_HDFV_H

// H5api_adpt.h
#ifndef H5_BUILT_AS_DYNAMIC_LIB
#define H5_BUILT_AS_DYNAMIC_LIB
#endif

#include <string>
#include <assert.h>
#include <unordered_map>
#include <hdf5.h>
#include <H5Cpp.h>

#include "util.h"
#include "Rst.h"
#include "DataModel.h"
#include "Cst.h"

using std::string;

class Hdfv : public Rst{
public:
    string groupName = H5_GROUP_NAME, datasetName = H5_DATASET_NAME;
    H5::DataType dataType;

    Hdfv(string fileName,string _groupName, string _datasetName);

    Hdfv(string fileName);

    static std::unordered_map<string, string> parseAttribute(char* attText);

    bool read(string fName = "", map<string,string> *metaData = nullptr)  override;

    bool write(string fName = "") override;
};

#endif //CLUSTERING_HDFV_H
