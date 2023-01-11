//
// Created by Administrator on 2022/11/2.
//

#ifndef CLUSTERING_RTREE_H
#define CLUSTERING_RTREE_H

#include <chrono>
#include <time.h>
#include <sidx_impl.h>
#include <sidx_api.h>
#include "Algo.h"
#include "util.h"
#include "Cst.h"
#include "Tif.h"
#include "SFileOpt.h"

using std::string;

class Raster: public Tif {
public:
    TP timePoint;
private:
    bool *vis;

private:
    chrono::time_point<chrono::system_clock> getTimePoint(string fileName);

public:
    Raster(Meta _meta, string _fileName);

    ~Raster();

    float get(int row, int col);

    bool isVisited(int row, int col);

    bool visit(int row, int col);

    void getNode(std::set<Node>& nodeSet, const int r, const int c, GeoRegion &range);

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
    int nodeId = 2;
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
