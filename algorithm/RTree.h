//
// Created by Administrator on 2022/11/2.
//

#ifndef CLUSTERING_RTREE_H
#define CLUSTERING_RTREE_H

#include <chrono>
#include "opt.h"
#include "algo.h"

class Matrix{
private:
    vector<vector<Node *>> mat;
    int rowOffset, rowLen, colOffset, colLen;
public:
    Matrix(Range& range, vector<Node>& nodeList){
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
    int mergeCluster();
};

class RTree{
private:
    static const int N_DIM = 2;
    IndexPropertyH prop;
    IndexH idx;
    int nodeId;
    chrono::time_point<chrono::system_clock> time;
    int maxClusterDur;

public:
    static queue<RTree*> Cache;
    static void Run(RTreeParam p,  vector<string> fileList, string outputPath);
    static Poly* BFS(Raster &rst, vector<pair<int, int>> &noZero) ;
    static bool flush();
    static TP getCacheEarliestTime();

    RTree();
    void insert(RNode* node);
    vector<RNode*> RTree::query(Range range) const;
    ~RTree();
};

#endif //CLUSTERING_RTREE_H
