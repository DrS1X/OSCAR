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
#include "Shp.h"

using std::string;

class Raster : public Tif {
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

};

class RNode {
public:
    int id;
    Cluster *cluster = nullptr;
    Poly *poly;
    OGRFeature *pFeat;
    OGRGeometry *pGeom;
    double area;
    chrono::system_clock::time_point timePoint;
    string TPStr;

    RNode(Poly *_pPoly, chrono::system_clock::time_point _timePoint);

    void mergeCluster();

    bool setFeature(OGRFeature *_pFeat);

    ~RNode();
};

class RTree {
private:
    IndexPropertyH prop;
    IndexH idx;
    int nodeId = 2;
    chrono::system_clock::time_point timePoint;
    string timePointStr;
    int maxClusterDur = 1;
    Tif polyRst;
    map<int, RNode *> poly2node;
    GDALDataset *poShp;
    OGRLayer *poLayer;

public:
    static map<int, Cluster *> Clusters;
    static queue<RTree *> Cache;

    static vector<float> Run(float oTh, int cTh, float vTh, string inPath, string outPath);

    static bool flush();

    static void flushAll();

    static RTree *Create(TP _timePoint);

    RTree() {};

    RTree(TP _timePoint);

    inline void flagPixel(Raster &rst, Poly *poly, int r, int c);

    Poly *DBSCAN(Raster &rst, int r, int c, vector<pair<int, int>> neighbor);

    bool polygonize(int ignoredPolyId);

    void insert(RNode *node);

    list<RNode *> RTree::query(GeoRegion range) const;

    void save();

    ~RTree();
};

#endif //CLUSTERING_RTREE_H
