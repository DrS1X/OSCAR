#include "RTree.h"

using namespace std;

const GeoRegion GeoRegion::GLOBAL(0, Meta::DEF.nRow - 1, 0, Meta::DEF.nCol - 1);

queue<RTree *> RTree::Cache;

map<int, Cluster *> RTree::Clusters;

const int GEO_WINDOW = 3;
int DUR_THRESHOLD;
int CORE_THRESHOLD;
string OUTPUT_PATH_RST;
string OUTPUT_PATH_SHP;
int CLUSTER_ID;
float OVERLAP_THRESHOLD;
float ATTRIBUTE_THRESHOLD;

bool cmp(pair<int, float> p1, pair<int, float> p2) {
    return p1.second > p2.second;
}

float RTree::Run(int _T, int cTh, float vTh, string inPath, string outPath) {
    vector<string> fileList;
    GetFileList(inPath, fileList);

    outPath += "\\R_" + to_string(_T) + "_" + to_string(cTh) + "_" + to_string(vTh);
    CheckFolderExist(outPath);
    OUTPUT_PATH_RST = outPath + "\\rst";
    CheckFolderExist(OUTPUT_PATH_RST);
    OUTPUT_PATH_SHP = outPath + "\\shp";
    CheckFolderExist(OUTPUT_PATH_SHP);

    CLUSTER_ID = 2;
    CORE_THRESHOLD = cTh;
    ATTRIBUTE_THRESHOLD = vTh;
    DUR_THRESHOLD = 2 * _T + 1;
    OVERLAP_THRESHOLD = 0.5;

    // evaluation background value
    double datasetMean;
    Background(inPath, &datasetMean);

    int windowEdge = GEO_WINDOW / 2;
    for (int i = 0; i < fileList.size(); ++i) {
        Raster rst(Meta::DEF, fileList[i]);
        rst.read();
        RTree *tree = RTree::Create(rst.timePoint);
        vector<Poly> polyList;
        vector<pair<int, int>> neighbor;
        vector<pair<int, float>> coreList;

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
                RNode *node = new RNode(pPoly);
                tree->poly2node[pPoly->id] = node;
            }
        }

//        string tmpPath = outPath + "\\Tmp_" + to_string(i);
//        tree->polyRst.write(tmpPath);

        int ignoredPoly = 0;
        for (auto kv: tree->poly2node) {
            if (kv.second->poly->range.isEqual(GeoRegion::GLOBAL)) {
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
    float devAvg = 0.0f;
    int cCnt = 0;
    for (const auto &item: RTree::Clusters) {
        Cluster *pc = item.second;
        pc->statistic();
        devAvg += pc->dev;
        ++cCnt;

        delete pc;
    }
    devAvg = devAvg / cCnt;
    RTree::Clusters.clear();
    return devAvg;
}

Poly *RTree::DBSCAN(Raster &rst, int r0, int c0, vector<pair<int, int>> neighbor) {
    Poly *pPoly = new Poly;
    queue<pair<int, int>> q;
    for (auto p: neighbor) {
        flagPixel(rst, pPoly, p.first, p.second);
        q.push(p);
    }

    while (!q.empty()) {
        pair<int, int> top = q.front();
        int r = top.first, c = top.second;
        q.pop();
//        neighbor.clear();
        for (int i = 0; i < 8; ++i) {
            int r1 = r + Neighbor8[i][0];
            int c1 = c + Neighbor8[i][1];
            int r2 = r + Neighbor8[(i + 1) % 8][0];
            int c2 = c + Neighbor8[(i + 1) % 8][1];
            int r3 = r + Neighbor8[(i + 2) % 8][0];
            int c3 = c + Neighbor8[(i + 2) % 8][1];

            float v = rst.get(r2, c2);
            if (rst.checkIndex(r2, c2) &&
                !rst.isVisited(r2, c2) &&
                fabs(v - pPoly->avgValue) <= ATTRIBUTE_THRESHOLD) {
                if (i % 2 == 0 &&
                    rst.checkIndex(r1, c1) &&
                    rst.checkIndex(r3, c3) &&
                    (fabs(pPoly->avgValue - rst.get(r1, c1)) > ATTRIBUTE_THRESHOLD ||
                     rst.isVisited(r1, c1) && polyRst.get(r1, c1) != pPoly->id) &&
                    (fabs(pPoly->avgValue - rst.get(r3, c3)) > ATTRIBUTE_THRESHOLD ||
                     rst.isVisited(r3, c3) && polyRst.get(r3, c3) != pPoly->id))
                    continue;

//                neighbor.emplace_back(r2, c2);
                q.emplace(r2, c2);
                flagPixel(rst, pPoly, r2, c2);
            }
        }

        /*if (neighbor.size() + 1 >= CORE_THRESHOLD) {
            for (int i = 0; i < neighbor.size(); ++i) {
                q.push(neighbor[i]);
                flagPixel(rst, pPoly, neighbor[i].first, neighbor[i].second);
            }
        }*/
    }

    pPoly->range.updateGeo();

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
    polyRst.name = OUTPUT_PATH_RST + "\\" + fileName + TIF_SUFFIX;
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
        node->pFeat = feature;
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

    // get time string  of one moment
    time_t t_ = chrono::system_clock::to_time_t(this->timePoint);
    char timeChars[20];
    string format;
    if (Meta::DEF.timeUnit == TimeUnit::Day) {
        format = "%Y%m%d";
    } else {
        format = "%Y%m";
    }
    std::strftime(timeChars, 20, format.c_str(), localtime(&t_));
    timePointStr = timeChars;
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
    int diff;
    switch (Meta::DEF.timeUnit) {
        case TimeUnit::Day:
            diff = chrono::duration_cast<duration_days>(back->timePoint - front->timePoint).count();
            break;
        case TimeUnit::Mon:
            diff = chrono::duration_cast<duration_months>(back->timePoint - front->timePoint).count();
            break;
        default:
            cout << "[flush] time unit is need." << endl;
            diff = 0;
    }

    if (diff <= back->maxClusterDur) {
//        cout << "Info: [flush] size of RTree::Cache is " + RTree::Cache.size() << endl;
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

        if (node->isDeleted) {
            GIntBig fid = feature->GetFID();
            OGRFeature::DestroyFeature(feature);
            poLayer->DeleteFeature(fid);
        } else {
            Shp::setFields(node->poly, timePointStr, feature);
            poLayer->SetFeature(feature);
            OGRFeature::DestroyFeature(feature);

            int cid = node->poly->clusterId;
            if (RTree::Clusters.find(cid) == Clusters.end())
                RTree::Clusters[cid] = new Cluster(cid, node->poly->sum, node->poly->dev, node->poly->pix);

            RTree::Clusters[cid]->expandBatch(node->poly->sum, node->poly->dev, node->poly->pix);


        }
    }
    GDALClose(poShp);
}

void RTree::insert(RNode *node) {
    node->id = ++nodeId;
    node->mergeCluster();
    if (node->dur > this->maxClusterDur)
        this->maxClusterDur = node->dur;

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


RNode::RNode(Poly *_pPoly) {
    this->poly = _pPoly;
    this->isDeleted = true;
    this->dur = 1;
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

    // get area of this RNode
    OGRGeometry *poGeom = this->pFeat->GetGeometryRef();
    if (poGeom == NULL || !poGeom->IsValid()) {
        cerr << "[mergeCluster] fail to get poGeom." << endl;
        return;
    }
    OGRPolygon *poPoly = poGeom->toPolygon();
    double area = poPoly->get_Area();

    // filter overlap polygon
    queue<RNode *> keep;
    // get max dur of previous
    int maxDur = 0;
    float maxArea = area;
    RNode *maxPrevNode;
    for (auto it = overlap.begin(); it != overlap.end(); ++it) {
        RNode *pn = *it;

        // intersection area
        double prevArea, intersectArea;
        OGRGeometry *poGeom2, *poGeom3;
        OGRPolygon *poPoly2, *poPoly3;
        poGeom2 = pn->pFeat->GetGeometryRef();
        if (poGeom2 == NULL || !poGeom2->IsValid()) {
            cerr << "[mergeCluster] fail to get poGeom2." << endl;
            return;
        }
        if (!poGeom->Overlaps(poGeom2) &&
            !poGeom->Contains(poGeom2) &&
            !poGeom2->Contains(poGeom))
            continue;
        poGeom3 = poGeom->Intersection(poGeom2);
        if (poGeom3 == NULL || !poGeom3->IsValid()) {
            cerr << "[mergeCluster] fail to get poGeom3." << endl;
            return;
        }
        poPoly2 = poGeom2->toPolygon();
        poPoly3 = poGeom3->toPolygon();
        prevArea = poPoly2->get_Area();
        intersectArea = poPoly3->get_Area();

        // attribute difference
        double valDiff = fabs(poly->avgValue - pn->poly->avgValue);

        if (/*intersectArea / prevArea < OVERLAP_THRESHOLD &&
            intersectArea / area < OVERLAP_THRESHOLD ||*/
                intersectArea <= CORE_THRESHOLD ||
                valDiff > ATTRIBUTE_THRESHOLD) {
            continue;
        }

        prev.push_back(pn);

        if (prevArea > maxArea) {
            maxArea = prevArea;
            maxPrevNode = pn;
        }

        if (pn->dur > maxDur)
            maxDur = pn->dur;

        if (pn->isDeleted)
            keep.push(pn);
    }
    if (maxDur > 0)
        this->dur = maxDur + 1;

    // update duration
    if (dur >= DUR_THRESHOLD) {
        //keep this cluster to save
        isDeleted = false;
        while (!keep.empty()) {
            RNode *top = keep.front();
            top->isDeleted = false;
            keep.pop();
            for (auto prevNode: top->prev) {
                keep.push(prevNode);
            }
        }
    }

    // update cluste id
    if (this->prev.size() == 0)
        return;


    if (this->prev.front()->poly->clusterId == 0) {
        this->poly->clusterId = CLUSTER_ID++;
        this->prev.front()->poly->clusterId = this->poly->clusterId;
    } else
        this->poly->clusterId = this->prev.front()->poly->clusterId;

    if (this->prev.size() > 1) {
        queue<RNode *, list<RNode *>> cluster(prev);

        while (!cluster.empty()) {
            RNode *top = cluster.front();
            cluster.pop();
            if (top->poly->clusterId != poly->clusterId) {
                delete RTree::Clusters[top->poly->clusterId];
                RTree::Clusters.erase(top->poly->clusterId);

                top->poly->clusterId = poly->clusterId;

                RTree::Clusters[poly->clusterId]->expandBatch(
                        top->poly->sum, top->poly->dev, top->poly->pix);
            }
            for (auto prevNode: top->prev) {
                if (prevNode->poly->clusterId != poly->clusterId)
                    cluster.push(prevNode);
            }
        }
    }
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



