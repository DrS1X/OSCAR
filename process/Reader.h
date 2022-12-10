//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_READER_H
#define CLUSTERING_READER_H

#include <vector>
#include <string>
#include <util.h>
#include "FileOperator.h"

class Reader {
public:
    FileOperator fi;
    FileOperator fo;
    Meta meta;
    int timeScale;
    float ***mean, ***standard;
    int ***cnt;

    Reader(FileOperator _fo);
    bool ReadBatch(std::vector<std::string>& fileList, TimeUnit timeUnit);
    bool ResampleAndStatistics(float **src, float **data);
    ~Reader();
private:
    static int getOrder(std::string date);
    void init(string file, TimeUnit timeUnit);
};


#endif //CLUSTERING_READER_H
