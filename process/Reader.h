//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_READER_H
#define CLUSTERING_READER_H

#include <vector>
#include <string>
#include <util.h>
#include "Rst.h"
#include "Hdfv.h"
#include "Tif.h"
#include "SFileOpt.h"

class Reader {
public:
    string outputPath;
    string const RESAMPLE_PREFIX = "\\resample";
    string const ANOMALY_PREFIX = "\\anomaly";
    string const POSITIVE_PREFIX = "\\posAnomaly";
    string const NEGATIVE_PREFIX = "\\negAnomaly";
    string const POSITIVE_S_PREFIX = "\\posAnomalyS";
    string const NEGATIVE_S_PREFIX = "\\negAnomalyS";

    Reader(string _outputPath);
    vector<string> readBatch(std::vector<std::string>& fileInList, Meta srcMeta = Meta::DEF);
    pair<vector<string>,vector<string>> filter(vector<string>& anomalyFileList, float stdTimeOfThreshold);
    bool smooth(vector<string> &fileInList, string fileOutPrefix);
    ~Reader();
private:
    void resampleAndStatistics(Tif* src, Tif* tar, double *** mean, double*** stdDev, int*** cnt);
};


#endif //CLUSTERING_READER_H
