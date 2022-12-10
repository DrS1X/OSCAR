#include "RTree.h"

using namespace std;

const GeoRegion GeoRegion::GLOBAL(META_DEF.endLat, META_DEF.startLat, META_DEF.startLon, META_DEF.endLon);

queue<RTree*> RTree::Cache;

int GEO_WINDOW;
int DUR_THRESHOLD;
int CORE_THRESHOLD;
int ATTRIBUTE_THRESHOLD;
string OUTPUT_PATH;
int CLUSTER_ID;
TimeUnit UNIT;
float OVERLAP_THRESHOLD;


void RTree::Run(RTreeParam p, vector<string> fileList, string outputPath) {
    CLUSTER_ID = 0;
    GEO_WINDOW = p.geoWindow;
    CORE_THRESHOLD = p.coreThreshold;
    ATTRIBUTE_THRESHOLD = p.valueThreshold / META_DEF.scale;
    OUTPUT_PATH = outputPath;
    DUR_THRESHOLD = p.durationThreshold;
    UNIT = p.unit;
    OVERLAP_THRESHOLD = p.overlapThreshold;

    for (int i = 1; i < fileList.size(); ++i) {
        Raster rst(fileList[i]);
        RTree* tree = RTree::Create(rst.timePoint);
        vector<Poly> polyList;
        int windowEdge = GEO_WINDOW / 2;
        for (int r = windowEdge; r < rst.rowNum - windowEdge; r++) {
            for (int c = windowEdge; c < rst.colNum - windowEdge; c++) {
                int v = rst.get(r, c);
                if (rst.isVisited(r, c) || v == 0) continue;

                vector<pair<int, int>> noZero;
                for (int k = 0; k < 8; ++k) {
                    int r2 = r + Neighbor8[k][0];
                    int c2 = c + Neighbor8[k][1];
                    int v2 = rst.get(r2, c2);
                    if (v2 != 0 && abs(v2 - v) < ATTRIBUTE_THRESHOLD) {
                        pair<int, int> pr{r2, c2};
                        noZero.push_back(pr);
                    }
                }

                if (noZero.size() >= CORE_THRESHOLD) {
                    rst.visit(r, c);
                    Poly* pPoly = RTree::BFS(rst, noZero);
                    RNode* node = new RNode(pPoly);
                    tree->insert(node);
                }
            }
        }

        RTree::Cache.push(tree);
        RTree::flush();
    }

    RTree::flushAll();
}

Poly* RTree::BFS(Raster &rst, vector<pair<int, int>> &noZero) {
    Line line;
    Poly* pPoly = new Poly;

    queue<pair<int, int>> q;
    for (auto e: noZero) {
        rst.visit(e.first, e.second);
        q.push(e);
        pPoly->update(rst.get(e.first, e.second));
    }

    set<Node> nodeSet;
    while (!q.empty()) {
        int sz = q.size();
        while (sz > 0) {
            pair<int, int> top = q.front();
            int r = top.first, c = top.second;
            q.pop();

            int cnt = 0;
            for (int i = 0; i < 8; ++i) {
                int r2 = r + Neighbor8[i][0];
                int c2 = c + Neighbor8[i][1];
                if (!rst.checkIndex(r2, c2))
                    continue;

                int v = rst.get(r2, c2);
                if (!rst.isVisited(r2, c2) && v != 0) {
                    // threshold
                    if(abs(v - pPoly->avgValue) > ATTRIBUTE_THRESHOLD)
                        continue;

                    rst.visit(r2, c2);
                    pPoly->update(rst.get(r2, c2));
                    pair<int, int> pr{r2, c2};
                    q.push(pr);
                    ++cnt;
                }
            }
            
            // edge
            if (cnt < 8) {
               rst.getNode(nodeSet, r - 1, c - 1, line.range);
               rst.getNode(nodeSet, r - 1, c, line.range);
               rst.getNode(nodeSet, r, c - 1, line.range);
               rst.getNode(nodeSet, r, c, line.range);
            }

            sz--;
        }
    }

    line.range.updateGeo();

    vector<Node> nodes(nodeSet.begin(), nodeSet.end());
    line.nodes = nodes;
    line.sortNodes(); // make the line of polygon edge to be clear

    vector<Line> lineList;
    lineList.push_back(line);

    pPoly->lines = lineList;
    pPoly->range = line.range;
    pPoly->minRow = line.range.rowMin;
    pPoly->maxRow = line.range.rowMax;
    pPoly->minCol = line.range.colMin;
    pPoly->maxCol = line.range.colMax;
    return pPoly;
}

RTree* RTree::Create(TP _timePoint){
    RTree* t = new RTree(_timePoint);

    return t;
}

RTree::RTree(TP _timePoint) {
    timePoint = _timePoint;
    nodeId = 0;
    maxClusterDur = 0;
    // init index
    this->prop = IndexProperty_Create();
    IndexProperty_SetIndexStorage(prop, RTStorageType::RT_Memory);
    this->idx = Index_Create(prop);
}

RTree::~RTree() {
    Index_Destroy(idx);
}

// move polygons of a moment from memory to disk
bool RTree::flush() {
    if (RTree::Cache.size() <= DUR_THRESHOLD)
        return false;

    RTree* back = RTree::Cache.back();
    RTree* front = RTree::Cache.front();
    int diff;
    switch (UNIT) {
        case TimeUnit::Day:
            diff = chrono::duration_cast<duration_days>(back->timePoint - front->timePoint).count();
            break;
        case TimeUnit::Mon:
            diff = chrono::duration_cast<duration_months>(back->timePoint - front->timePoint).count();
            break;
        default:
            cout << "[flush] time unit is need." << endl;
            diff=0;
    }

    if(diff <= back->maxClusterDur){
//        cout << "Info: [flush] size of RTree::Cache is " + RTree::Cache.size() << endl;
        return false;
    }

    RTree::Cache.pop();

    front->save();
    delete front;
    return true;
}

void RTree::flushAll() {
    while(!RTree::Cache.empty()){
        RTree* front = RTree::Cache.front();
        RTree::Cache.pop();

        front->save();
        delete front;
    }
}

void RTree::save(){
    list<RNode*> all = this->query(GeoRegion::GLOBAL);
    vector<Poly> polyList;
    for(auto node : all){
        if(node->isDeleted)
            continue;
        polyList.push_back(*node->poly);
    }

    // get time string  of one moment
    time_t t_ = chrono::system_clock::to_time_t(this->timePoint);

    char startTimeStr[20];
    string format;
    if(UNIT == TimeUnit::Day){
        format = "%Y%m%d";
    }else {
        format = "%Y%m";
    }
    std::strftime(startTimeStr, 20, format.c_str(), localtime(&t_));

    // save to shp file-operator
    TifOpt::save(OUTPUT_PATH, startTimeStr, AnomalyType::Positive, polyList);
}

void RTree::insert(RNode *node) {
    node->id = ++nodeId;
    node->mergeCluster();
    if(node->dur > this->maxClusterDur)
        this->maxClusterDur = node->dur;

    GeoRegion range = node->poly->range;
    double pdMin[N_DIM] = {range.getLow(0), range.getLow(1)};
    double pdMax[N_DIM] = {range.getHigh(0), range.getHigh(1)};

    uint8_t *pData = (uint8_t*) malloc(sizeof(RNode*));
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
    if(pdMin[0] > pdMax[0] || pdMin[1] > pdMax[1])
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

        RNode *pN = (RNode* )malloc(sizeof(RNode*));
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

// update duration and cluster id following prev overlap region
void RNode::mergeCluster() {
    if (RTree::Cache.empty())
        return;

    // get overlap polygon(RNode) between this and previous
    RTree* prevTree = RTree::Cache.back();
    list<RNode*> overlap = prevTree->query(this->poly->range);
    if (overlap.empty()) {
        return;
    }

    double area = this->poly->range.getArea();
    queue<RNode *> keep;
    // get max dur of previous
    for (auto it = overlap.begin(); it != overlap.end(); ++it) {
        RNode* pNode = *it;
        double prevArea = (pNode->poly->range).getArea();
        double intersectArea = this->poly->range.getIntersectingArea(pNode->poly->range);

        if(intersectArea / prevArea < OVERLAP_THRESHOLD && intersectArea / area < OVERLAP_THRESHOLD){
            continue;
        }

        prev.push_back(pNode);

        if (pNode->dur > this->dur)
            this->dur = pNode->dur;

        if(pNode->isDeleted)
            keep.push(pNode);
    }
    ++this->dur;

    // update duration
    if(this->dur >= DUR_THRESHOLD){
        //keep this cluster to save
        this->isDeleted = false;
        while(!keep.empty()){
            RNode* top = keep.front();
            top->isDeleted = false;
            keep.pop();
            for(auto prevNode: top->prev){
                keep.push(prevNode);
            }
        }
    }

    // update cluste id
    if(this->prev.size() == 0)
        this->poly->clusterId = ++CLUSTER_ID;
    else if(this->prev.size() == 1)
        this->poly->clusterId = (this->prev).front()->poly->clusterId;
    else {
        this->poly->clusterId = (this->prev).front()->poly->clusterId;

        queue<RNode *, list<RNode*>> cluster(prev);
        cluster.pop();

        while(!cluster.empty()){
            RNode* top = cluster.front();
            cluster.pop();
            top->poly->clusterId = this->poly->clusterId;
            for(auto prevNode: top->prev){
                if(prevNode->poly->clusterId != this->poly->clusterId)
                    cluster.push(prevNode);
            }
        }
    }

}

int Raster::index(int row, int col) const {
    return row * colNum + col;
}

Raster::Raster(string file) {
    val = new int[sz];
    vis = new bool[sz];
    memset(vis, 0, sz);
    TifOpt::readGeoTiff(file, val);

    timePoint = getTimePoint(file);
}

Raster::~Raster(){
    delete[] val;
    delete[] vis;
}

TP Raster::getTimePoint(string fileName){
    int startIndex = fileName.size() - 12;
    tm tm_ = {0};
    tm_.tm_isdst = 0;                          // 非夏令时。
    tm_.tm_sec = 0;
    tm_.tm_min = 0;
    tm_.tm_hour = 0;
    tm_.tm_mday = 1;

    int year, month;
    year = stoi(fileName.substr(startIndex,4));
    month = stoi(fileName.substr(startIndex + 4,2));

    tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。

     if(UNIT == TimeUnit::Day){
         tm_.tm_mday = stoi(fileName.substr(startIndex + 6,2));
     }

    time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式，将tm时间转换为秒时间

    TP tp = std::chrono::system_clock::from_time_t(t_);
    return tp;
}

bool Raster::checkIndex(int row, int col) {
    if (row < 0 || row >= rowNum || col < 0 || col >= colNum) {
        return false;
    } else {
        return true;
    }
}

int Raster::get(int row, int col) {
    if(row < 0 || col < 0 || row >= rowNum || col >= colNum) // just for the edge process of getNode()
        return 0;
    else
        return val[index(row, col)];
}

void Raster::set(int row, int col, int value) {
    val[index(row, col)] = value;
}

bool Raster::isVisited(int row, int col){
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

void Raster::getNode(std::set<Node>& nodeSet, const int r, const int c, GeoRegion &range) {
    Node* n;
    int r1 = r, r2 = r, r3 = r + 1, r4 = r + 1;
    int c1 = c, c2 = c + 1, c3 = c, c4 = c + 1;
    int v1 = get(r1, c1), v2 = get(r2, c2), v3 = get(r3, c3), v4 = get(r4, c4);

    if (v1 == 0 && v2 != 0 && v3 != 0 && v4 != 0 || v1 != 0 && v2 == 0 && v3 == 0 && v4 == 0) {
        nodeSet.emplace(NodeType_LeftTop, r, c);
    } else if (v1 != 0 && v2 == 0 && v3 != 0 && v4 != 0 || v1 == 0 && v2 != 0 && v3 == 0 && v4 == 0) {
        nodeSet.emplace(NodeType_RightTop, r, c);
    } else if (v1 != 0 && v2 != 0 && v3 == 0 && v4 != 0 || v1 == 0 && v2 == 0 && v3 != 0 && v4 == 0) {
        nodeSet.emplace(NodeType_LeftBot, r, c);
    } else if (v1 != 0 && v2 != 0 && v3 != 0 && v4 == 0 || v1 == 0 && v2 == 0 && v3 == 0 && v4 != 0) {
        nodeSet.emplace(NodeType_RightBot, r, c);
    } else if(v1 != 0 && v4 != 0 && v2 == 0 && v3 == 0){
        nodeSet.emplace(NodeType_LeftTopAndRightBot, r, c);
    } else if(v1 == 0 && v4 == 0 && v2 != 0 && v3 != 0){
        nodeSet.emplace(NodeType_RightTopAndLeftBot, r, c);
    } else if (v1 != 0 && v2 == 0 && v3 != 0 && v4 == 0 || v1 == 0 && v2 != 0 && v3 == 0 && v4 != 0) {
        nodeSet.emplace(NodeType_TopBot, r, c);
    } else if (v1 != 0 && v2 != 0 && v3 == 0 && v4 == 0 || v1 == 0 && v2 == 0 && v3 != 0 && v4 != 0) {
        nodeSet.emplace(NodeType_LeftRight, r, c);
    }

    range.checkEdge(r, c);
}

bool Line::sortNodes(){
    int rowOffset = range.rowMin;
    int rowLen = range.rowMax - range.rowMin + 1;
    int colOffset = range.colMin;
    int colLen = range.colMax - range.colMin + 1;
    Node *head = nullptr;

    // build mat of region
    for (int i = 0; i < nodes.size(); ++i) {
        if (head == nullptr && nodes[i].type != NodeType_LeftTopAndRightBot &&
            nodes[i].type != NodeType_RightTopAndLeftBot) {
            head = &nodes[i];
            break;
        }
    }

    Matrix mat(range, nodes);

    // search the line of edge
    vector<Node> newNodeList;
    Node *cur = head;
    string direction = cur->dir1;
    int r = cur->row - rowOffset, c = cur->col - colOffset;
    cur->isVisited = true;
    newNodeList.push_back(*head);
    int cnt = 0;
    do {
        r += RowMove.at(direction);
        c += ColMove.at(direction);
        if(r < -1 || c < -1 || r >= rowLen || c >= colLen){
            cout << "error: [sortNodes] out of range." << endl;
            return false;
        }

        if (mat.get(r, c) == nullptr)
            continue;

        cur = mat.get(r, c);
        if(cur->isVisited){
            newNodeList.push_back(*cur);
            break;
        }

        if(cur->type != NodeType_LeftTopAndRightBot && cur->type != NodeType_RightTopAndLeftBot) {
            cur->isVisited = true;
            newNodeList.push_back(*cur);
        }
        direction = cur->getNextDirection(direction);
        cnt++;
    } while (head != cur || cnt >= nodes.size());

    nodes = newNodeList;
    Matrix tmp(range, nodes); // just view the matrix of nodes
    return true;
}