//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_RFILEOPT_H
#define CLUSTERING_RFILEOPT_H
#include <string>
#include "DataModel.h"

class RFileOpt {
public:
    virtual bool read(RFile file) = 0;
    virtual bool write(RFile file) = 0;
    virtual void getMeta(std::string fileName, Meta& meta) = 0;
};

#endif //CLUSTERING_RFILEOPT_H
