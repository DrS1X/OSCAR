#include <queue>
#include <set>
#include "algo.h"
#include "_const.h"
#include "Vectorization.h"

using namespace std;

int WINDOW_SIZE;
int CORE_THRESHOLD;
int ATTRIBUTE_THRESHOLD;
string OUTPUT_PATH;

class Raster {
public:
    const int rowNum = Def.Rows;
    const int colNum = Def.Cols;
    const int sz = Def.Size;
private:
    int *val;
    bool *vis;

private:
    int index(int row, int col) const;

public:
    explicit Raster(string file);

    bool checkIndex(int row, int col);

    int get(int row, int col);

    void set(int row, int col, int value);

    bool isVisited(int row, int col);

    bool visit(int row, int col);

    void getNode(std::set<Node>& nodeSet, const int r, const int c, Range &range);
};


Poly bfs(int clusterId, Raster &rst, vector<pair<int, int>> &noZero) {
    Line line;
    Poly poly;

    queue<pair<int, int>> q;
    for (auto e: noZero) {
        rst.visit(e.first, e.second);
        q.push(e);
        poly.update(rst.get(e.first, e.second));
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
                    if(abs(v - poly.avgValue) > ATTRIBUTE_THRESHOLD)
                        continue;

                    rst.visit(r2, c2);
                    poly.update(rst.get(r2, c2));
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

    vector<Node> nodes(nodeSet.begin(), nodeSet.end());
    line.nodes = nodes;
    line.sortNodes();

    vector<Line> lineList;
    lineList.push_back(line);

    poly.lines = lineList;
    poly.eventID = clusterId;
    poly.minRow = line.range.rowMin;
    poly.maxRow = line.range.rowMax;
    poly.minCol = line.range.colMin;
    poly.maxCol = line.range.colMax;
    return poly;
}

void slidingWindows(string file) {
    Raster rst(file);
    vector<Poly> polyList;
    int clusterId = 0;
    int windowEdge = WINDOW_SIZE / 2;
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
                Poly poly = bfs(clusterId++, rst, noZero);
                polyList.push_back(poly);
            }
        }
    }

    gdalOpt::save(OUTPUT_PATH, "", "", Def.StartLog, Def.StartLat, Def.Resolution, polyList);
}

void BFS(const int windowSize, const int coreThreshold, const float valueThreshold, vector<string> &fileList, string outputPath) {
    WINDOW_SIZE = windowSize;
    CORE_THRESHOLD = coreThreshold;
    ATTRIBUTE_THRESHOLD = valueThreshold / Def.Scale;
    OUTPUT_PATH = outputPath;
    for (auto f: fileList) {
        slidingWindows(f);
    }
}


int Raster::index(int row, int col) const {
    return row * colNum + col;
}

Raster::Raster(string file) {
    val = new int[sz];
    vis = new bool[sz];
    memset(vis, 0, sz);
    gdalOpt::readGeoTiff(file, val);
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

void Raster::getNode(std::set<Node>& nodeSet, const int r, const int c, Range &range) {
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