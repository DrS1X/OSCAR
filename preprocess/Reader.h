//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_READER_H
#define CLUSTERING_READER_H

#include <vector>
#include <string>
#include <random>
#include <filesystem>
#include "util.h"
#include "Rst.h"
#include "Hdfv.h"
#include "Tif.h"
#include "Shp.h"
#include "Csv.h"

string const RESAMPLE_PREFIX = "\\resample";
string const ANOMALY_PREFIX = "\\anomaly";

vector<string> ReadBatch(std::string inputPath, string outputPath, Meta srcMeta = Meta::DEF);

bool Filter(string inputPath, string outputPath, float stdTimeOfThreshold);

bool smooth(vector<string> &fileInList, string outputPath, string fileOutPrefix);

void resampleAndStatistics(Tif *src, Tif *tar, double ***mean, double ***stdDev, int ***cnt);

void Kth(string inPath, string outPath, int T);

void SimulateData(filesystem::path inPath, filesystem::path outPath);

void Background(filesystem::path inPath, double *mean, float *time = nullptr,
                filesystem::path maskOutPath = filesystem::path());

#endif //CLUSTERING_READER_H
