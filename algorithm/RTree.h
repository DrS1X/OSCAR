//
// Created by Administrator on 2022/11/2.
//

#ifndef CLUSTERING_RTREE_H
#define CLUSTERING_RTREE_H

#include <chrono>
#include <sidx_impl.h>
#include <time.h>
#include "algo.h"
#include "util/util.h"
#include "_const.h"
#include "tiffOpt.h"
#include "hdfOpt.h"

using std::string;

class Raster {
public:
    const int rowNum = Def.Rows;
    const int colNum = Def.Cols;
    const int sz = Def.Size;
    TP timePoint;
private:
    int *val;
    bool *vis;

private:
    int index(int row, int col) const;
    chrono::time_point<chrono::system_clock> getTimePoint(string fileName);

public:
    explicit Raster(string file);

    ~Raster();

    bool checkIndex(int row, int col);

    int get(int row, int col);

    void set(int row, int col, int value);

    bool isVisited(int row, int col);

    bool visit(int row, int col);

    void getNode(std::set<Node>& nodeSet, const int r, const int c, GeoRegion &range);

};

class Matrix{
private:
    vector<vector<Node *>> mat;
    int rowOffset, rowLen, colOffset, colLen;
public:
    Matrix(GeoRegion& range, vector<Node>& nodeList){
        rowOffset = range.rowMin;
        rowLen = range.rowMax - range.rowMin + 2;
        colOffset = range.colMin;
        colLen = range.colMax - range.colMin + 2;
        vector<Node*> emptyRow(colLen, nullptr);
        mat.resize(rowLen, emptyRow);
        for (int i = 0; i < nodeList.size(); ++i) {
            int nr = nodeList[i].row - rowOffset + 1, nc = nodeList[i].col - colOffset + 1;
            if(nr < 0 || nr >= rowLen || nc < 0 || nc >= colLen){
                cout << "error: [Matrix] build matrix fail." << endl;
                break;
            }
            mat[nr][nc] = &nodeList[i];
        }
        //print();
    }

    void print(){
        for(int i = 0; i < mat.size(); ++i){
            for(int j = 0; j < mat[0].size(); ++j){
                if(mat[i][j] == nullptr)
                    cout << "  " << " ";
                else
                    cout << mat[i][j]->dir1 << mat[i][j]->dir2 << " ";
            }
            cout << endl;
        }
    }

    Node* get(int row, int col){
        return mat[row + 1][col + 1];
    }
};


class RNode {
public:
    int id;
    Poly *poly;
    list<RNode *> prev;
    list<RNode *> next;
    int dur;
    bool isDeleted;

    RNode(Poly *_pPoly);
    void mergeCluster();
};

class RTree{
private:
    IndexPropertyH prop;
    IndexH idx;
    int nodeId;
    chrono::system_clock::time_point timePoint;
    int maxClusterDur;

public:
    static queue<RTree*> Cache;
    static void Run(RTreeParam p,  vector<string> fileList, string outputPath);
    static Poly* BFS(Raster &rst, vector<pair<int, int>> &noZero);
    static bool flush();
    static void flushAll();
    static RTree* Create(TP _timePoint);


    RTree(){};
    RTree(TP _timePoint);
    void insert(RNode* node);
    list<RNode*> RTree::query(GeoRegion range) const;
    void save();
    ~RTree();
};

#endif //CLUSTERING_RTREE_H
