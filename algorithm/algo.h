//
// Created by Administrator on 2022/10/12.
//

#ifndef CLUSTERING_ALGO_H
#define CLUSTERING_ALGO_H

#include <string>
#include <vector>
#include <queue>
#include "_const.h"
#include "util/util.h"
#include "hdfOpt.h"
#include "tiffOpt.h"

using namespace std;

using TP = chrono::time_point<chrono::system_clock>;
using duration_seconds = chrono::duration<float, ratio<60>>;
using duration_hours = chrono::duration<float, std::ratio<3600>>;
using duration_days = chrono::duration<float, ratio<86400>>;
using duration_weeks = chrono::duration<float, std::ratio<604800>>;
using duration_months = chrono::duration<float, std::ratio<2629746>>;
using duration_years = chrono::duration<float, std::ratio<31556952>>;

enum TimeUnit{
    Day, Mon
};

struct RTreeParam{
    int geoWindow;
    int coreThreshold;
    TimeUnit unit;
    int durationThreshold;
    float valueThreshold;
    float overlapThreshold;
};

extern int GEO_WINDOW;
extern int DUR_THRESHOLD;
extern int CORE_THRESHOLD;
extern int ATTRIBUTE_THRESHOLD;
extern string OUTPUT_PATH;
extern int CLUSTER_ID;
extern TimeUnit UNIT;
extern float OVERLAP_THRESHOLD;

class RoSTCM
{
public:
    int rsID = -1; //??????ID
    int rsclusterId = -1; //????????ID
    bool isKeyrs; //???????????
    bool Visited; //????????
    double Attribute = 0;//??????????????
    int t;//???????
    int x;
    int y;//???????
    int a;//??????????????1????????-1???????0??
    vector<int> neighborgrids; //????????????id?б?

    //RoSTCM() {}

    bool IsKey()
    {
        return this->isKeyrs;
    }

    //???ú????????
    void SetKey(bool isKeyrs)
    {
        this->isKeyrs = isKeyrs;
    }

    //???DpId????
    int GetrsID()
    {
        return this->rsID;
    }

    //????DpId????
    void SetrsId(int rsID)
    {
        this->rsID = rsID;
    }

    //GetIsVisited????
    bool isVisited()
    {
        return this->Visited;
    }

    //SetIsVisited????
    void SetVisited(bool Visited)
    {
        this->Visited = Visited;
    }

    //GetClusterId????
    long GetrsClusterId()
    {
        return this->rsclusterId;
    }

    //GetClusterId????
    void SetrsClusterId(int rsclusterId)
    {
        this->rsclusterId = rsclusterId;
    }

    //GetArrivalPoints????
    vector<int> Getneighborgrids()
    {
        return neighborgrids;
    }

    //~RoSTCM()
    //{

    //}
};

struct Param
{
    int startID;
    int endID;
    int mTempFileNum;
    int mRows;
    int mCols;
    //vector<RoSTCM> Rasterpixels;
    int drID;
    int rsclusterId;
    int minnum;

    int timesID;
    double mFillValue;
    double mScale;
    double ProgressingPer;
    int Index;
};

class DcSTMC {
private:
    int T = 2;
    int Neighborhood = 8;//???????
    int mPerNum = 200;//??ξ????????
    int rsclusterId = 2;//????
    //double NNeighbor = 0.3;//???????

    int MinNum = 100; // 400;//?????????8*((mRows*mCols)/20000)*mFileNum???????
    int CP = 15;//???????????????

    int mRows = Def.Rows;
    int mCols = Def.Cols;
    double mScale = Def.Scale;
    double mFillValue = Def.FillValue;
    Meta meta = Def;
    hdfOpt ho;

    void ExpandCluster1(vector<RoSTCM>& Rasterpixels, int drID, int rsclusterId, int row, int col, double avg, int num);
    void ClusterFilter(vector<RoSTCM>& Rasterpixels, int startID, int endID, int mTempFileNum, int size);
    void Lable(vector<RoSTCM>& Rasterpixels, int mTempFileNum);
    void OutputRasterpixels(vector<RoSTCM>& Rasterpixels, vector<string>& fileList, int mStartFileIndex, int mTempFileNum, string outputPath);
public:
    void Run(vector<string>& FileList, string outputPath, int in_mPerNum, int in_T);
};

#endif //CLUSTERING_ALGO_H
