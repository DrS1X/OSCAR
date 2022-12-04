//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_HDF5OPT_H
#define CLUSTERING_HDF5OPT_H

// H5api_adpt.h
#ifndef H5_BUILT_AS_DYNAMIC_LIB
#define H5_BUILT_AS_DYNAMIC_LIB
#endif

#include <iostream>
#include <string>
#include <assert.h>
#include <H5Cpp.h>


class hdf5Opt {
public:
    static bool read(std::string fileName);
};


#endif //CLUSTERING_HDF5OPT_H
