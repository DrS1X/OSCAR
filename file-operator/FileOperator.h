//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_FILEOPERATOR_H
#define CLUSTERING_FILEOPERATOR_H
#include <string>
#include "DataModel.h"

class FileOperator {
public:
    virtual bool read(RFile file) {};
    virtual bool write(RFile file) {};
    virtual void getMeta(std::string fileName, Meta& meta);
};

#endif //CLUSTERING_FILEOPERATOR_H
