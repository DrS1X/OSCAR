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

void BFS(vector<string> &fileList, string outputPath);

class Raster {
public:
    const int rowNum = Def.Rows;
    const int colNum = Def.Cols;
    const int sz = Def.Size;
private:

    int *buffer;
    bool *vis;

private:

    int index(int row, int col) {
        return row * colNum + col;
    }

public:
    explicit Raster(string file) {
        buffer = new int[sz];
        vis = new bool[sz];
        memset(vis, 0, sz);
        gdalOpt::readGeoTiff(file, buffer);
    }

    bool checkIndex(int row, int col) {
        if (row < 0 || row >= rowNum || col < 0 || col >= colNum) {
            return false;
        } else {
            return true;
        }
    }

    int get(int row, int col) {
        if(row < 0 || col < 0 || row >= rowNum || col >= colNum) // just for the edge process of getNode()
            return 0;
        else
            return buffer[index(row, col)];
    }

    void set(int row, int col, int value) {
        buffer[index(row, col)] = value;
    }

    bool isVisited(int row, int col){
        int id = index(row, col);
        return vis[id];
    }

    bool visit(int row, int col) {
        int id = index(row, col);
        if (vis[id])
            return false;
        else {
            vis[id] = true;
            return true;
        }
    }
    
    unique_ptr<Node> getNode(int r, int c, Range &range) {
        unique_ptr<Node> n;
        int r1 = r, r2 = r, r3 = r + 1, r4 = r + 1;
        int c1 = c, c2 = c + 1, c3 = c, c4 = c + 1;
        int v1 = get(r1, c1), v2 = get(r2, c2), v3 = get(r3, c3), v4 = get(r4, c4);

        if (v1 == 0 && v2 != 0 && v3 != 0 && v4 != 0 || v1 != 0 && v2 == 0 && v3 == 0 && v4 == 0) {
            n = make_unique<Node>(NodeType_LeftTop, r, c);
        } else if (v1 != 0 && v2 == 0 && v3 != 0 && v4 != 0 || v1 == 0 && v2 != 0 && v3 == 0 && v4 == 0) {
            n = make_unique<Node>(NodeType_RightTop, r, c);
        } else if (v1 != 0 && v2 != 0 && v3 == 0 && v4 != 0 || v1 == 0 && v2 == 0 && v3 != 0 && v4 == 0) {
            n = make_unique<Node>(NodeType_LeftBot, r, c);
        } else if (v1 != 0 && v2 != 0 && v3 != 0 && v4 == 0 || v1 == 0 && v2 == 0 && v3 == 0 && v4 != 0) {
            n = make_unique<Node>(NodeType_RightBot, r, c);
        } else if(v1 != 0 && v4 != 0 && v2 == 0 && v3 == 0){
            n = make_unique<Node> (NodeType_LeftTopAndRightBot, r, c);
        } else if(v1 == 0 && v4 == 0 && v2 != 0 && v3 != 0){
            n = make_unique<Node> (NodeType_RightTopAndLeftBot, r, c);
        } else if (v1 != 0 && v2 == 0 && v3 != 0 && v4 == 0 || v1 == 0 && v2 != 0 && v3 == 0 && v4 != 0) {
            n = make_unique<Node>(NodeType_TopBot, r, c);
        } else if (v1 != 0 && v2 != 0 && v3 == 0 && v4 == 0 || v1 == 0 && v2 == 0 && v3 != 0 && v4 != 0) {
            n = make_unique<Node>(NodeType_LeftRight, r, c);
        }

        range.checkEdge(r, c);
        return n;
    }
};

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
