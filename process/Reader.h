//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_READER_H
#define CLUSTERING_READER_H

#include <vector>
#include <string>
#include <util.h>
#include "RFileOpt.h"
#include "SFileOpt.h"

class Reader {
public:
    int stdTimeOfThreshold;
    RFileOpt *fi, *fo;
    Meta meta;
    int timeScale;
    float ***mean, ***standard;
    int ***cnt;
    string prefix;
    string const RESAMPLE_FOLDER = "resample-";
    string const POSITIVE_FOLDER = "posAnomaly-";
    string const NEGATIVE_FOLDER = "negAnomaly-";


    Reader(string _prefix, int _stdTimeOfThreshold, RFileOpt* _fi, RFileOpt* _fo);
    bool readBatch(std::vector<std::string>& fileInList, TimeUnit timeUnit);
    void resampleAndStatistics(RFile& src, RFile& tar);
    void splitFile(float downLimit,float upLimit, RFile file, RFile pos, RFile neg);

    ~Reader();
private:
    void init(string file, TimeUnit timeUnit);
};


#endif //CLUSTERING_READER_H
