#include <queue>
#include "algo.h"
#include "_const.h"
#include "Vectorization.h"

using namespace std;

POL bfs(int clusterId, Raster &rst, vector<pair<int, int>> &noZero, string outputPath) {
    Range range;
    queue<pair<int, int>> q;
    for (auto e: noZero) {
        rst.visit(e.first, e.second);
        q.push(e);
    }

    vector<Node> nodeList;
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

                if (rst.visit(r2, c2) && rst.get(r2, c2) != 0) {
                    pair<int, int> pr{r2, c2};
                    q.push(pr);
                    ++cnt;
                }
            }

            if (cnt < 8) {
                unique_ptr<Node> node;
                if (node = rst.getNode(r - 1, c - 1, range))
                    nodeList.push_back(*node);

                if (node = rst.getNode(r - 1, c, range))
                    nodeList.push_back(*node);

                if (node = rst.getNode(r, c - 1, range))
                    nodeList.push_back(*node);

                if (node = rst.getNode(r, c, range))
                    nodeList.push_back(*node);
            }

            sz--;
        }
    }

    Line line(range, nodeList);
    line.sortNodes();

    vector<Line> lineList;
    lineList.push_back(line);

    POL poly;
    poly.lines = lineList;
    poly.eventID = clusterId;
    poly.minRow = range.rowMin;
    poly.maxRow = range.rowMax;
    poly.minCol = range.colMin;
    poly.maxCol = range.colMax;
    return poly;
}

void slidingWindows(int winSize, int coreThreshold, string file, string outputPath) {
    Raster rst(file);
    vector<POL> polyList;
    int clusterId = 0;
    for (int r = winSize / 2; r < rst.rowNum - winSize / 2; r++) {
        for (int c = winSize / 2; c < rst.colNum - winSize / 2; c++) {
            if (rst.isVisited(r, c) || rst.get(r, c) == 0) continue;

            vector<pair<int, int>> noZero;
            for (int k = 0; k < 8; ++k) {
                int r2 = r + Neighbor8[k][0];
                int c2 = c + Neighbor8[k][1];
                if (rst.get(r2, c2) != 0) {
                    pair<int, int> pr{r2, c2};
                    noZero.push_back(pr);
                }
            }

            if (noZero.size() >= coreThreshold) {
                rst.visit(r, c);
                POL poly = bfs(clusterId++, rst, noZero, outputPath);
                polyList.push_back(poly);
            }
        }
    }

    gdalOpt::save(outputPath, "", "", Def.StartLog, Def.StartLat, Def.Resolution, polyList);
}

void BFS(vector<string> &fileList, string outputPath) {
    for (auto f: fileList) {
        slidingWindows(3, 8, f, outputPath);
    }
}