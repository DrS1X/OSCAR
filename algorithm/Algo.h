//
// Created by Administrator on 2022/10/12.
//

#ifndef CLUSTERING_ALGO_H
#define CLUSTERING_ALGO_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <cmath>
#include <memory>
#include "Reader.h"
#include "Cst.h"
#include "util.h"
#include "Tif.h"
#include "Csv.h"

using namespace std;
using filesystem::path;
using TP = chrono::time_point<chrono::system_clock>;
using duration_seconds = chrono::duration<float, ratio<60>>;
using duration_hours = chrono::duration<float, std::ratio<3600>>;
using duration_days = chrono::duration<float, ratio<86400>>;
using duration_weeks = chrono::duration<float, std::ratio<604800>>;
using duration_months = chrono::duration<float, std::ratio<2629746>>;
using duration_years = chrono::duration<float, std::ratio<31556952>>;

inline int GetDur(chrono::system_clock::time_point begin, chrono::system_clock::time_point end) {
    int dur = -1;
    switch (Meta::DEF.timeUnit) {
        case TimeUnit::Day:
            dur = 1 + chrono::duration_cast<duration_days>(end - begin).count();
            break;
        case TimeUnit::Mon:
            dur = 1 + chrono::duration_cast<duration_months>(end - begin).count();
            break;
    }
    return dur;
}

inline string GetTPStr(chrono::system_clock::time_point timePoint) {
    time_t t_ = chrono::system_clock::to_time_t(timePoint);
    char timeChars[20];
    string format;
    if (Meta::DEF.timeUnit == TimeUnit::Day) {
        format = "%Y%m%d";
    } else {
        format = "%Y%m";
    }
    std::strftime(timeChars, 20, format.c_str(), localtime(&t_));
    string str = timeChars;
    return str;
}

void DcSTCABatch(path inputPath, path outputPath, int T, int maxK, int minK, int stepK);

void RBatch(path inputPath, path outputPath, float oTh);

class Pixel {
public:
    int pid = -1;
    int cid = -1;
    bool isKey;
    bool visited;
    double Attribute = 0;
    int t;
    int x;
    int y;
    int a;
    vector<int> neighborGrids;

    //RoSTCM() {}

    bool IsKey() {
        return this->isKey;
    }

    //???ú????????
    void SetKey(bool isKeyrs) {
        this->isKey = isKeyrs;
    }


    //????DpId????
    void SetrsId(int rsID) {
        this->pid = rsID;
    }

    //GetIsVisited????
    bool isVisited() {
        return this->visited;
    }

    //SetIsVisited????
    void SetVisited(bool Visited) {
        this->visited = Visited;
    }

    //GetClusterId????
    long GetrsClusterId() {
        return this->cid;
    }

    //GetClusterId????
    void SetrsClusterId(int rsclusterId) {
        this->cid = rsclusterId;
    }

    //GetArrivalPoints????
    vector<int> Getneighborgrids() {
        return neighborGrids;
    }
};

class RNode;

class Cluster {
public :
    int id;
    double sum = 0.0;
    double avg = 0.0;
    double dev = 0.0;
    int pix = 0;
    int nPoly = 0;
    int dur;
    chrono::system_clock::time_point end;
    string endStr;
    chrono::system_clock::time_point begin;
    string beginStr;
    bool isMerged = false;
    vector<RNode *> nodes;

    Cluster(int _id) : id(_id) {
    };

    Cluster(int _id, RNode *p1);

    void merge(Cluster *another);

    void expand(float v);

    void expandRNode(RNode *node);

    void statistic();
};


class DcSTCA {
public:
    int T;
    int Neighborhood = 8;
    int batchSize = 10;
    int clusterId = 2;
    int MinNum;
    int mRows;
    int mCols;
    double mScale = 0.001;
    double mFillValue;
    double valueThreshold;
    int coreThreshold;
    Meta meta;

    map<int, Cluster *> clusters;

    vector<float> Run(string inPath, string outPath, int _T, int cTh, float vTh);

private:
    void ExpandCluster(vector<Pixel> &Rasterpixels, int drID, int clusterId, int row, int col);

    void ClusterFilter(vector<Pixel> &Rasterpixels, int startID, int endID, int mTempFileNum, int size);

    void Lable(vector<Pixel> &Rasterpixels, int mTempFileNum);

    void
    OutputRasterpixels(vector<Pixel> &Rasterpixels, vector<string> &fileList, int mStartFileIndex, int mTempFileNum,
                       string outputPath);
};


void Evaluation(path srcPath, path resPath, path outPath);

vector<float> InnerEval(Cluster *BG, double datasetMean, map<int, Cluster *> &clusters);

#endif //CLUSTERING_ALGO_H
