#include "RTree.h"

using namespace std;

GeoRegion GeoRegion::GLOBAL(0, Meta::DEF.nRow - 1, 0, Meta::DEF.nCol - 1);

queue<RTree *> RTree::Cache;

map<int, Cluster *> RTree::Clusters;

const int GEO_WINDOW = 3;
int DUR_THRESHOLD;
int CORE_THRESHOLD;
string OUTPUT_PATH_PID;
string OUTPUT_PATH_CID;
string OUTPUT_PATH_SHP;
int CLUSTER_ID;
float OVERLAP_THRESHOLD;
float ATTRIBUTE_THRESHOLD;

bool cmp(pair<int, float> p1, pair<int, float> p2) {
    return p1.second > p2.second;
}

vector<float> RTree::Run(float oTh, int cTh, float vTh, string inPath, string outPath) {
    vector<string> fileList;
    GetFileList(inPath, fileList);

    outPath += "\\R_" + to_string(oTh) + "_" + to_string(cTh) + "_" + to_string(vTh);
    CheckFolderExist(outPath);
    OUTPUT_PATH_PID = outPath + "\\pid";
    OUTPUT_PATH_CID = outPath + "\\cid";
    OUTPUT_PATH_SHP = outPath + "\\shp";
    CheckFolderExist(OUTPUT_PATH_PID);
    CheckFolderExist(OUTPUT_PATH_CID);
    CheckFolderExist(OUTPUT_PATH_SHP);

    CLUSTER_ID = 2;
    CORE_THRESHOLD = cTh;
    ATTRIBUTE_THRESHOLD = vTh;
    DUR_THRESHOLD = 2;
    OVERLAP_THRESHOLD = oTh;

    // evaluation background value
    double datasetMean;
    Background(inPath, &datasetMean);

    Cluster background(0);
    int windowEdge = GEO_WINDOW / 2;
    vector<pair<int, float>> coreList;
    for (int i = 0; i < fileList.size(); ++i) {
        Raster rst(Meta::DEF, fileList[i]);
        rst.read();
        RTree *tree = RTree::Create(rst.timePoint);
        vector<Poly> polyList;
        vector<pair<int, int>> neighbor;
        coreList.clear();

        // get core and sort
        for (int r = windowEdge; r < rst.meta.nRow - windowEdge; ++r) {
            for (int c = windowEdge; c < rst.meta.nCol - windowEdge; ++c) {
                float v = rst.get(r, c);
                int cnt = 1;
                for (int k = 0; k < 8; ++k) {
                    int r2 = r + Neighbor8[k][0];
                    int c2 = c + Neighbor8[k][1];
                    float v2 = rst.get(r2, c2);
                    if (fabs(v2 - v) < ATTRIBUTE_THRESHOLD)
                        ++cnt;
                }
                if (cnt >= CORE_THRESHOLD)
                    coreList.emplace_back(r * Meta::DEF.nCol + c, fabs(v - datasetMean));
            }
        }
        std::sort(coreList.begin(), coreList.end(), cmp);

        // extract region
        for (auto core: coreList) {
            int r = core.first / Meta::DEF.nCol;
            int c = core.first % Meta::DEF.nCol;
            float v = rst.get(r, c);
            if (rst.isVisited(r, c))
                continue;

            neighbor.clear();
            neighbor.emplace_back(r, c);
            for (int k = 0; k < 8; ++k) {
                int r2 = r + Neighbor8[k][0];
                int c2 = c + Neighbor8[k][1];
                float v2 = rst.get(r2, c2);
                if (!rst.isVisited(r2, c2) &&
                    fabs(v2 - v) < ATTRIBUTE_THRESHOLD)
                    neighbor.emplace_back(r2, c2);
            }

            if (neighbor.size() >= CORE_THRESHOLD) {
                Poly *pPoly = tree->DBSCAN(rst, r, c, neighbor);
                RNode *node = new RNode(pPoly, tree->timePoint);
                tree->poly2node[pPoly->id] = node;
            }
        }

//        string tmpPath = outPath + "\\Tmp_" + to_string(i);
//        tree->polyRst.write(tmpPath);

        int ignoredPoly = 0;
        for (auto kv: tree->poly2node) {
            if (kv.second->poly->range.isEqual(GeoRegion::GLOBAL)) {
                background.pix += kv.second->poly->pix;
                background.sum += kv.second->poly->sum;
                background.dev += kv.second->poly->dev;
                tree->poly2node.erase(kv.first);
                ignoredPoly = kv.first;
                break;
            }
        }

        tree->polygonize(ignoredPoly);
        for (auto kv: tree->poly2node)
            tree->insert(kv.second);

        RTree::Cache.push(tree);
        RTree::flush();
    }
    RTree::flushAll();

    // statistic dev of cluster
    vector<float> res  = InnerEval(&background, datasetMean, RTree::Clusters);

    RTree::Clusters.clear();

    return res;
}

Poly *RTree::DBSCAN(Raster &rst, int r0, int c0, vector<pair<int, int>> neighbor) {
    Poly *pPoly = new Poly;
    queue<pair<int, int>> q;
    for (auto p: neighbor) {
        flagPixel(rst, pPoly, p.first, p.second);
        q.push(p);
    }

    char flag[8];
    while (!q.empty()) {
        pair<int, int> top = q.front();
        int r = top.first, c = top.second;
        q.pop();
        ::memset(flag, 0, 8);
        int reachableCnt = 1;

        for (int i = 0; i < 8; ++i) {
            int r1 = r + Neighbor8[i][0];
            int c1 = c + Neighbor8[i][1];
            int r2 = r + Neighbor8[(i + 1) % 8][0];
            int c2 = c + Neighbor8[(i + 1) % 8][1];
            int r3 = r + Neighbor8[(i + 2) % 8][0];
            int c3 = c + Neighbor8[(i + 2) % 8][1];

            if (!rst.checkIndex(r2, c2) ||
                fabs(rst.get(r2, c2) - pPoly->avg) > ATTRIBUTE_THRESHOLD)
                continue;

            flag[(i + 1) % 8] = 'r';
            ++reachableCnt;

            if (rst.isVisited(r2, c2) ||
                i % 2 == 0 &&
                rst.checkIndex(r1, c1) &&
                rst.checkIndex(r3, c3) &&
                (fabs(pPoly->avg - rst.get(r1, c1)) > ATTRIBUTE_THRESHOLD ||
                 rst.isVisited(r1, c1) && polyRst.get(r1, c1) != pPoly->id) &&
                (fabs(pPoly->avg - rst.get(r3, c3)) > ATTRIBUTE_THRESHOLD ||
                 rst.isVisited(r3, c3) && polyRst.get(r3, c3) != pPoly->id))
                continue;

            flag[(i + 1) % 8] = 'v';
        }

        if (reachableCnt >= CORE_THRESHOLD) {
            for (int i = 0; i < 8; ++i) {
                if (flag[i] != 'v')
                    continue;
                int r2 = r + Neighbor8[i][0], c2 = c + Neighbor8[i][1];
                q.emplace(r2, c2);
                flagPixel(rst, pPoly, r2, c2);
            }
        }
    }

    pPoly->range.updateGeo();

    for (int r = pPoly->range.rowMin; r <= pPoly->range.rowMax; ++r) {
        for (int c = pPoly->range.colMin; c <= pPoly->range.colMax; ++c) {
            if (polyRst.get(r, c) != pPoly->id) {
                pPoly->sumBG += rst.get(r, c);
                pPoly->nPixBG += 1;
            }
        }
    }

    return pPoly;
}

bool RTree::polygonize(int ignoredPolyId) {
    // mask
    if (ignoredPolyId > 0) {
        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                if (IsEqual(polyRst.get(r, c), ignoredPolyId))
                    polyRst.update(r, c, 0.0f);
            }
        }
    }

    // raster and shape file name
    string fileName = "poly_" + timePointStr;
    polyRst.name = OUTPUT_PATH_PID + "\\" + fileName + TIF_SUFFIX;
    string shpFileName = OUTPUT_PATH_SHP + "\\" + fileName + SHP_SUFFIX;

    poShp = Shp::driver->Create(shpFileName.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    poLayer = poShp->CreateLayer(shpFileName.c_str(), Meta::DEF.spatialReference, wkbPolygon, NULL);
    Shp::createFields(poLayer);
    if (!polyRst.polygonize(poLayer, 1)) {
        cerr << "[save] the transform from raster to shp is fail." << endl;
        return false;
    }

    // update feature of layer of shape file
    OGRFeature *feature;
    while (feature = poLayer->GetNextFeature()) {
        int polyID = feature->GetFieldAsInteger(1);
        RNode *node = poly2node[polyID];
        assert(node != NULL);
        node->setFeature(feature);
    }

    return true;
}

void RTree::flagPixel(Raster &rst, Poly *poly, int r, int c) {
    rst.visit(r, c);
    poly->update(r, c, rst.get(r, c));

    assert(polyRst.isZero(r, c));
    polyRst.update(r, c, poly->id);
}

RTree *RTree::Create(TP _timePoint) {
    RTree *t = new RTree(_timePoint);

    return t;
}

RTree::RTree(TP _timePoint) {
    timePoint = _timePoint;
    maxClusterDur = 0;
    // init index
    this->prop = IndexProperty_Create();
    IndexProperty_SetIndexStorage(prop, RTStorageType::RT_Memory);
    this->idx = Index_Create(prop);

    timePointStr = GetTPStr(timePoint);
}

RTree::~RTree() {
    list<RNode *> all = this->query(GeoRegion::GLOBAL);
    for (auto node: all) {
        delete node;
    }

    Index_Destroy(idx);
}

// move polygons of a moment from memory to disk
bool RTree::flush() {
    if (RTree::Cache.size() <= DUR_THRESHOLD)
        return false;

    RTree *back = RTree::Cache.back();
    RTree *front = RTree::Cache.front();
    int diff = GetDur(front->timePoint, back->timePoint);

    if (diff <= back->maxClusterDur) {
        return false;
    }

    RTree::Cache.pop();

    front->save();
    delete front;
    return true;
}

void RTree::flushAll() {
    while (!RTree::Cache.empty()) {
        RTree *front = RTree::Cache.front();
        RTree::Cache.pop();

        front->save();
        delete front;
    }
}

void RTree::save() {
    for (auto kv: poly2node) {
        RNode *node = kv.second;
        OGRFeature *feature = kv.second->pFeat;

        if (!node->cluster || node->cluster->dur < DUR_THRESHOLD) {
            GIntBig fid = feature->GetFID();
            OGRFeature::DestroyFeature(feature);
            poLayer->DeleteFeature(fid);
        } else {
            Shp::setFields(node->poly, timePointStr, feature);
            poLayer->SetFeature(feature);
            OGRFeature::DestroyFeature(feature);
        }
    }

    // write raster of cluster id
    for (int r = 0; r < Meta::DEF.nRow; ++r) {
        for (int c = 0; c < Meta::DEF.nCol; ++c) {
            int polyID = polyRst.get(r,c);
            if(IsZero(polyID))
                continue;

            polyRst.update(r,c,poly2node[polyID]->poly->cid);
        }
    }
    polyRst.meta.date = timePointStr;
    polyRst.write(OUTPUT_PATH_CID + "\\cid");

    GDALClose(GDALDataset::ToHandle(poShp));
}

void RTree::insert(RNode *node) {
    node->id = ++nodeId;
    node->mergeCluster();
    if (node->cluster && node->cluster->dur > this->maxClusterDur)
        this->maxClusterDur = node->cluster->dur;

    GeoRegion &range = node->poly->range;
    double pdMin[N_DIM] = {range.getLow(0), range.getLow(1)};
    double pdMax[N_DIM] = {range.getHigh(0), range.getHigh(1)};

    uint8_t *pData = (uint8_t *) malloc(sizeof(RNode *));
    memcpy(pData, &node, sizeof(RNode *));

    Index_InsertData(idx,
                     nodeId,
                     pdMin,
                     pdMax,
                     N_DIM,
                     pData,
                     sizeof(RNode *));
}

list<RNode *> RTree::query(GeoRegion range) const {
    list<RNode *> result;

    // range query
    double pdMin[N_DIM] = {range.getLow(0), range.getLow(1)};
    double pdMax[N_DIM] = {range.getHigh(0), range.getHigh(1)};
    if (pdMin[0] > pdMax[0] || pdMin[1] > pdMax[1])
        cerr << "[query] range is in invalid." << endl;

    IndexItemH **items = (IndexItemH **) malloc(sizeof(IndexItemH *));
    uint64_t *nResults = (uint64_t *) malloc(sizeof(uint64_t));
    // type of items is ***SpatialIndex::IData
    Index_Intersects_obj(idx, pdMin, pdMax, N_DIM, items, nResults);

    // parse result
    uint64_t *length = (uint64_t *) malloc(sizeof(uint64_t));
    uint8_t **data = (uint8_t **) malloc(sizeof(uint8_t *));

    for (int i = 0; i < *nResults; ++i) {
        IndexItemH item = *(*items + i);
        IndexItem_GetData(item, data, length);

        if (*length != sizeof(RNode *)) {
            cout << "[query] length of byte is wrong. length=" << *length << endl;
        }

        RNode *pN = (RNode *) malloc(sizeof(RNode *));
        memcpy(&pN, *data, sizeof(RNode *));
        result.push_back(pN);
    }

    free(items);
    free(nResults);
    free(length);
    free(data);

    return result;
}


RNode::RNode(Poly *_pPoly, chrono::system_clock::time_point _timePoint) {
    this->poly = _pPoly;
    this->timePoint = _timePoint;
    this->TPStr = GetTPStr(_timePoint);
}

RNode::~RNode() {
    delete poly;
}

// update duration and cluster id following prev overlap region
void RNode::mergeCluster() {
    if (RTree::Cache.empty())
        return;

    // get overlap polygon(RNode) between this and previous
    RTree *prevTree = RTree::Cache.back();
    list<RNode *> overlap = prevTree->query(poly->range);
    if (overlap.empty()) {
        return;
    }

    // filter overlap polygon
    vector<RNode *> connectivity;
    float maxIntersectRatio = 0.0;
    RNode *maxIntersectNode = this;
    for (auto it = overlap.begin(); it != overlap.end(); ++it) {
        RNode *pn = *it;

        // intersection area
        if (!this->pGeom->Overlaps(pn->pGeom) &&
            !this->pGeom->Contains(pn->pGeom) &&
            !pn->pGeom->Contains(this->pGeom))
            continue;
        OGRGeometry *poGeom3 = this->pGeom->Intersection(pn->pGeom);
        if (poGeom3 == NULL || !poGeom3->IsValid()) {
            cerr << "[mergeCluster] fail to get poGeom3." << endl;
            return;
        }
        double intersectArea = poGeom3->toPolygon()->get_Area();

        if (intersectArea / pn->area < OVERLAP_THRESHOLD &&
            intersectArea / area < OVERLAP_THRESHOLD ||
            //                intersectArea <= CORE_THRESHOLD ||
            fabs(poly->avg - pn->poly->avg) > ATTRIBUTE_THRESHOLD /*||
                pn->cluster &&
                fabs(poly->avg - pn->cluster->avg) > ATTRIBUTE_THRESHOLD*/) {
            continue;
        }

        connectivity.push_back(pn);

        double ratio = intersectArea / this->area;
        if (pn->cluster && ratio > maxIntersectRatio) {
            maxIntersectRatio = ratio;
            maxIntersectNode = pn;
        }
    }

    if (connectivity.empty())
        return;

    if (maxIntersectNode == this) {
        // all prev node is not cluster
        Cluster *newCls = new Cluster(CLUSTER_ID, this);
        this->cluster = newCls;
        RTree::Clusters[newCls->id] = newCls;
        ++CLUSTER_ID;

        for (auto pn: connectivity) {
            newCls->expandRNode(pn);
        }
    } else {
        // merge cluster based the largest prev node
        Cluster *base = maxIntersectNode->cluster;
        base->expandRNode(this);
        for (auto pn: connectivity) {
            if (pn->cluster == base)
                continue;
            if (!pn->cluster) {
                pn->poly->cid = base->id;
                base->expandRNode(pn);
            } else {
                // merge prev cluster to base cluster
                base->merge(pn->cluster);
            }
        }
    }
}

bool RNode::setFeature(OGRFeature *_pFeat) {
    pFeat = _pFeat;
    pGeom = this->pFeat->GetGeometryRef();
    if (pGeom == NULL || !pGeom->IsValid()) {
        cerr << "[setFeature] fail to get Geometry." << endl;
        return false;
    }
    area = pGeom->toPolygon()->get_Area();
}

Raster::Raster(Meta _meta, string _fileName) : Tif(_meta, _fileName) {
    vis = new bool[meta.nPixel];
    memset(vis, 0, meta.nPixel);

    timePoint = getTimePoint(_fileName);
}

Raster::~Raster() {
    delete[] vis;
}

TP Raster::getTimePoint(string fileName) {
    string dateStr = GetDate(fileName);

    tm tm_ = {0};
    tm_.tm_isdst = 0; // 非夏令时。
    tm_.tm_sec = 0;
    tm_.tm_min = 0;
    tm_.tm_hour = 0;
    tm_.tm_mday = 1;

    int year, month;
    year = stoi(dateStr.substr(0, 4));
    month = stoi(dateStr.substr(4, 2));

    tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。

    if (Meta::DEF.timeUnit == TimeUnit::Day) {
        tm_.tm_mday = stoi(dateStr.substr(6, 2));
    }

    time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式，将tm时间转换为秒时间

    TP tp = std::chrono::system_clock::from_time_t(t_);
    return tp;
}

float Raster::get(int row, int col) {
    if (row < 0 || col < 0 || row >= meta.nRow || col >= meta.nCol) // just for the edge process of getNode()
        return 0;
    else
        return data[index(row, col)];
}

bool Raster::isVisited(int row, int col) {
    int id = index(row, col);
    return vis[id];
}

bool Raster::visit(int row, int col) {
    int id = index(row, col);
    if (vis[id])
        return false;
    else {
        vis[id] = true;
        return true;
    }
}



