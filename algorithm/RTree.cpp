#include "RTree.h"

using namespace std;

const GeoRegion GeoRegion::GLOBAL(Meta::DEF.startLat, Meta::DEF.endLat,
                                  Meta::DEF.startLon, Meta::DEF.endLon);

queue<RTree*> RTree::Cache;

int GEO_WINDOW;
int DUR_THRESHOLD;
int CORE_THRESHOLD;
int ATTRIBUTE_THRESHOLD;
string OUTPUT_PATH;
int CLUSTER_ID;
float OVERLAP_THRESHOLD;
float ATTRIBUTE_RATIO_THRESHOLD;


void RTree::Run(RTreeParam p, vector<string> fileList, string outputPath) {
    CheckFolderExist(outputPath);

    CLUSTER_ID = 2;
    GEO_WINDOW = p.geoWindow;
    CORE_THRESHOLD = p.coreThreshold;
    ATTRIBUTE_THRESHOLD = p.valueThreshold;
    OUTPUT_PATH = outputPath;
    DUR_THRESHOLD = p.durationThreshold;
    Meta::DEF.timeUnit = p.unit;
    OVERLAP_THRESHOLD = p.overlapThreshold;
    ATTRIBUTE_RATIO_THRESHOLD = 0.3;

    for (int i = 0; i < fileList.size(); ++i) {
        Raster rst(Meta::DEF,fileList[i]);
        rst.read();
        RTree* tree = RTree::Create(rst.timePoint);
        vector<Poly> polyList;
        int windowEdge = GEO_WINDOW / 2;
        for (int r = windowEdge; r < rst.meta.nRow - windowEdge; r++) {
            for (int c = windowEdge; c < rst.meta.nCol - windowEdge; c++) {
                float v = rst.get(r, c);
                if (rst.isVisited(r, c) || IsEqual(v,0.0f)) 
                    continue;

                vector<pair<int, int>> noZero;
                for (int k = 0; k < 8; ++k) {
                    int r2 = r + Neighbor8[k][0];
                    int c2 = c + Neighbor8[k][1];
                    float v2 = rst.get(r2, c2);
                    if (!IsEqual(v2,0.0f) && fabs(v2 - v) < ATTRIBUTE_THRESHOLD) {
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

            // mask
            for (int i = 0; i < 8; i += 2) {
                int r1 = r + Neighbor8[i][0];
                int c1 = c + Neighbor8[i][1];
                int r2 = r + Neighbor8[(i + 1) % 8][0];
                int c2 = c + Neighbor8[(i + 1) % 8][1];
                int r3 = r + Neighbor8[(i + 2) % 8][0];
                int c3 = c + Neighbor8[(i + 2) % 8][1];

                if (!rst.checkIndex(r1, c1) || !rst.checkIndex(r2, c2) ||
                        !rst.checkIndex(r3, c3))
                    continue;

                if(IsZero(rst.get(r1,c1)) && !IsZero(rst.get(r2,c2)) &&
                    IsZero(rst.get(r3,c3))){
                    rst.update(r2,c2,0.0f);
                }
            }

            int cnt = 0;
            for (int i = 0; i < 8; ++i) {
                int r2 = r + Neighbor8[i][0];
                int c2 = c + Neighbor8[i][1];
                if (!rst.checkIndex(r2, c2))
                    continue;

                float v = rst.get(r2, c2);
                if (!rst.isVisited(r2, c2) && !IsZero(v)) {
                    if(fabs(v - pPoly->avgValue) > ATTRIBUTE_THRESHOLD)
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
               rst.getNode(nodeSet, r - 1, c - 1, pPoly->range);
               rst.getNode(nodeSet, r - 1, c, pPoly->range);
               rst.getNode(nodeSet, r, c - 1, pPoly->range);
               rst.getNode(nodeSet, r, c, pPoly->range);
            }

            sz--;
        }
    }

    pPoly->range.updateGeo();

    vector<Node> nodes(nodeSet.begin(), nodeSet.end());
    Line line(nodes);
    line.range = pPoly->range;
    line.sortNodes(); // make the line of polygon edge to be clear

    pPoly->lines.push_back(line);
    return pPoly;
}

RTree* RTree::Create(TP _timePoint){
    RTree* t = new RTree(_timePoint);

    return t;
}

RTree::RTree(TP _timePoint) {
    timePoint = _timePoint;
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
    switch (Meta::DEF.timeUnit) {
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
    if(Meta::DEF.timeUnit == TimeUnit::Day){
        format = "%Y%m%d";
    }else {
        format = "%Y%m";
    }
    std::strftime(startTimeStr, 20, format.c_str(), localtime(&t_));
    string st(startTimeStr);
    // save to shp file-operator
    SFileOpt::write(OUTPUT_PATH, st , "positive", polyList);
}

void RTree::insert(RNode *node) {
    node->id = ++nodeId;
    node->mergeCluster();
    if(node->dur > this->maxClusterDur)
        this->maxClusterDur = node->dur;

    GeoRegion& range = node->poly->range;
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
    int maxDur = 0;
    for (auto it = overlap.begin(); it != overlap.end(); ++it) {
        RNode* pNode = *it;
        double prevArea = (pNode->poly->range).getArea();
        double intersectArea = this->poly->range.getIntersectingArea(pNode->poly->range);
        double valDiff = fabs(this->poly->avgValue - pNode->poly->avgValue);
        double greaterVal = this->poly->avgValue > pNode->poly->avgValue ? this->poly->avgValue : pNode->poly->avgValue;

        if(intersectArea / prevArea < OVERLAP_THRESHOLD &&
            intersectArea / area < OVERLAP_THRESHOLD ||
            valDiff / greaterVal > ATTRIBUTE_RATIO_THRESHOLD){
            continue;
        }

        prev.push_back(pNode);

        if (pNode->dur > maxDur)
            maxDur = pNode->dur;

        if(pNode->isDeleted)
            keep.push(pNode);
    }
    if(maxDur > 0)
        this->dur = maxDur + 1;

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
        this->poly->clusterId = CLUSTER_ID++;
    else {
        if(this->prev.front()->poly->clusterId == 0){
            this->poly->clusterId = CLUSTER_ID++;
            this->prev.front()->poly->clusterId = this->poly->clusterId;
        }else
            this->poly->clusterId = this->prev.front()->poly->clusterId;

        if (this->prev.size() > 1){
            queue<RNode *, list<RNode *>> cluster(prev);

            while (!cluster.empty()) {
                RNode *top = cluster.front();
                cluster.pop();
                top->poly->clusterId = this->poly->clusterId;
                for (auto prevNode: top->prev) {
                    if (prevNode->poly->clusterId != this->poly->clusterId)
                        cluster.push(prevNode);
                }
            }
        }
    }
}

Raster::Raster(Meta _meta, string _fileName):Tif(_meta, _fileName) {
    vis = new bool[meta.nPixel];
    memset(vis, 0, meta.nPixel);

    timePoint = getTimePoint(_fileName);
}

Raster::~Raster(){
    delete[] vis;
}

TP Raster::getTimePoint(string fileName){
    string dateStr = GetDate(fileName);

    tm tm_ = {0};
    tm_.tm_isdst = 0; // 非夏令时。
    tm_.tm_sec = 0;
    tm_.tm_min = 0;
    tm_.tm_hour = 0;
    tm_.tm_mday = 1;

    int year, month;
    year = stoi(dateStr.substr(0,4));
    month = stoi(dateStr.substr(4,2));

    tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。

     if(Meta::DEF.timeUnit == TimeUnit::Day){
         tm_.tm_mday = stoi(fileName.substr(6,2));
     }

    time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式，将tm时间转换为秒时间

    TP tp = std::chrono::system_clock::from_time_t(t_);
    return tp;
}

float Raster::get(int row, int col) {
    if(row < 0 || col < 0 || row >= meta.nRow || col >= meta.nCol) // just for the edge process of getNode()
        return 0;
    else
        return data[index(row, col)];
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
    int r1 = r, r2 = r, r3 = r + 1, r4 = r + 1;
    int c1 = c, c2 = c + 1, c3 = c, c4 = c + 1;
    float v1 = get(r1, c1), v2 = get(r2, c2), v3 = get(r3, c3), v4 = get(r4, c4);

    if (IsZero(v1) && !IsZero(v2) && !IsZero(v3) && !IsZero(v4) || !IsZero(v1) && IsZero(v2) && IsZero(v3) && IsZero(v4)) {
        nodeSet.emplace(NodeType_LeftTop, r, c);
    } else if (!IsZero(v1) && IsZero(v2) && !IsZero(v3) && !IsZero(v4) || IsZero(v1) && !IsZero(v2) && IsZero(v3) && IsZero(v4)) {
        nodeSet.emplace(NodeType_RightTop, r, c);
    } else if (!IsZero(v1) && !IsZero(v2) && IsZero(v3) && !IsZero(v4) || IsZero(v1) && IsZero(v2) && !IsZero(v3) && IsZero(v4)) {
        nodeSet.emplace(NodeType_LeftBot, r, c);
    } else if (!IsZero(v1) && !IsZero(v2) && !IsZero(v3) && IsZero(v4) || IsZero(v1) && IsZero(v2) && IsZero(v3) && !IsZero(v4)) {
        nodeSet.emplace(NodeType_RightBot, r, c);
    } else if(!IsZero(v1) && !IsZero(v4) && IsZero(v2) && IsZero(v3)){
        nodeSet.emplace(NodeType_LeftTopAndRightBot, r, c);
    } else if(IsZero(v1) && IsZero(v4) && !IsZero(v2) && !IsZero(v3)){
        nodeSet.emplace(NodeType_RightTopAndLeftBot, r, c);
    } else if (!IsZero(v1) && IsZero(v2) && !IsZero(v3) && IsZero(v4) || IsZero(v1) && !IsZero(v2) && IsZero(v3) && !IsZero(v4)) {
        nodeSet.emplace(NodeType_TopBot, r, c);
    } else if (!IsZero(v1) && !IsZero(v2) && IsZero(v3) && IsZero(v4) || IsZero(v1) && IsZero(v2) && !IsZero(v3) && !IsZero(v4)) {
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
            cerr << "[sortNodes] out of range." << endl;
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


