//
// Created by Administrator on 2022/10/12.
//

#ifndef CLUSTERING_ALGO_H
#define CLUSTERING_ALGO_H

#include <string>
#include <vector>
#include "_const.h"
#include "opt.h"

using namespace std;

void BFS(const int windowSize, const int coreThreshold, const float valueThreshold, vector<string> &fileList, string outputPath);

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
