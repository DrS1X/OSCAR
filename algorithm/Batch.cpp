//
// Created by 15291 on 2023/1/27.
//
#include "Algo.h"
#include "RTree.h"

using std::string;

void Kth(string inPath, string outPath, int T) {
    int SamplePtsNum = 100;

    vector<string> fileList;
    GetFileList(inPath, fileList);

    const int ScaleFactor = 1000;
    const int KMin = 3;
    int KMax = (2 * T + 1) * 9;
    vector<vector<unsigned short >> dat(KMax - KMin + 1, vector<unsigned short>());
    int cacheSize = 2 * T + 1;
    list<Tif *> cache;
    vector<int> neighborDis;

    for (int i = 0; i < fileList.size(); ++i) {
        Tif *tif = new Tif(Meta::DEF, fileList[i]);
        tif->read();
        cache.push_back(tif);

        if (i < cacheSize - 1)
            continue;

        list<Tif *>::iterator cur = cache.begin();
        for (int k = 0; k < T; ++k) ++cur;

        for (int r = 1; r < Meta::DEF.nRow - 1; ++r) {
            for (int c = 1; c < Meta::DEF.nCol - 1; ++c) {
                if ((*cur)->isFillValue(r, c) || (*cur)->isZero(r, c))
                    continue;

                neighborDis.clear();
                int base = (*cur)->get(r, c) * ScaleFactor;

                // centre
                bool stop = false;
                for (list<Tif *>::iterator it = cache.begin(); it != cache.end(); ++it) {
                    Tif *pTif = *it;
                    if (pTif->isFillValue(r, c)) {
                        stop = true;
                        break;
                    }

                    int v = pTif->get(r, c) * ScaleFactor;
                    int dis = abs(v - base);
                    neighborDis.push_back(dis);
                }
                if (stop)
                    continue;

                // surround
                for (list<Tif *>::iterator it = cache.begin(); it != cache.end(); ++it) {
                    Tif *pTif = *it;
                    for (int k = 0; k < 8; ++k) {
                        int r2 = r + Neighbor8[k][0];
                        int c2 = c + Neighbor8[k][1];
                        if (pTif->isFillValue(r2, c2))
                            continue;
                        int v = pTif->get(r2, c2) * ScaleFactor;
                        int dis = abs(v - base);
                        neighborDis.push_back(dis);
                    }
                }

                if (neighborDis.empty())
                    continue;

                assert(neighborDis.size() <= KMax);
                std::sort(neighborDis.begin(), neighborDis.end());
                for (int k = KMin - 1; k < neighborDis.size(); ++k) {
                    unsigned short v = neighborDis[k];
                    dat[k - (KMin - 1)].push_back(v);
                }
            }
        }

        delete cache.front();
        cache.pop_front();
    }

    // sort
    long long minDatVec = LONG_MAX;
    long long maxDatVec = 0;
    for (int i = 0; i < dat.size(); ++i) {
        std::sort(dat[i].begin(), dat[i].end());
        if (dat[i].size() < minDatVec)
            minDatVec = dat[i].size();
        if (dat[i].size() > maxDatVec)
            maxDatVec = dat[i].size();
    }

    // sample
    vector<vector<float>> sample(KMax - KMin + 1, vector<float>());
    long xStep = maxDatVec / SamplePtsNum;
    for (int k = 0; k < dat.size(); ++k) {
        for (int x = 0; x < dat[k].size(); x += xStep) {
            float v = 1.0f * dat[k][x] / ScaleFactor;
            sample[k].push_back(v);
        }
    }

    // write Kth data to csv file
    Csv K(outPath + "\\K_" + to_string(T) + "T.csv");
    // -column head
    K.ofs << "Rank";
    for (int c = KMin; c <= KMax; ++c)
        K.ofs << ',' << c << "th";
    K.ofs << std::endl;
    // -column data
    int r = 0;
    bool stop = false;
    while (!stop) {
        stop = true;
        K.ofs << r * xStep;
        for (int c = 0; c < sample.size(); ++c) {
            if (r < sample[c].size()) {
                K.ofs << ',' << setprecision(3) << fixed << sample[c][r];
                stop = false;
            }
        }
        K.ofs << std::endl;
        ++r;
    }

    // write empty threshold file
    Csv emptyThresholdFile(outPath + "\\Threshold_" + to_string(T) + "T.csv");
    emptyThresholdFile.ofs << "cTh" << ',' << "vTh" << endl;
    for (int k = KMin; k <= KMax; ++k) {
        emptyThresholdFile.ofs << k << ',' << endl;
    }
}

void DcSTCABatch(path inputPath, path outputPath, int T, int maxK, int minK, int stepK) {
    if (minK == 0) minK = 3;
    if (maxK == 0) maxK = 9 * (2 * T + 1);

    // Kth
    path csvInPath(inputPath);
    csvInPath = csvInPath.parent_path().append("Threshold_" + to_string(T) + "T.csv");
    vector<vector<string>> K(2, vector<string>());
    if (!Csv::Read(csvInPath.string(), K))
        return;
    map<int, float> KV;
    for (int r = 0; r < K[0].size(); ++r) {
        int key = atoi(K[0][r].c_str());
        float val = atof(K[1][r].c_str());
        KV[key] = val;
    }

    outputPath.append("DcSTCA_Batch_" + to_string(T) + "T");
    CheckFolderExist(outputPath);

    path csvOutPath(outputPath);
    csvOutPath.append("DcSTCA_Res_" + to_string(T) + "T.csv");
    Csv csv(csvOutPath.string(), "cTh,vTh,devAvgNoBG,devAvg,devWeightAvg,CHI,time");
    for (int k = minK; k <= maxK; k += stepK) {
        if (KV[k] == 0) {
            cerr << "[DcSTCABatch] " + to_string(k) + " Kth parameter no found.";
            exit(1);
        }

        std::chrono::steady_clock::time_point bef = std::chrono::steady_clock::now();

        DcSTCA a;
        vector<float> res = a.Run(inputPath.string(), outputPath.string(), T, k, KV[k]);

        std::chrono::steady_clock::time_point aft = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(aft - bef);

        csv.ofs << k << ',' << KV[k] << ',' << res[0] << ',' << res[1] << ',' << res[2] << ',' << res[3] << ','
                << time_span.count() << endl;
    }
}

void RBatch(path inputPath, path outputPath, float oTh) {
    const int minK = 3;
    const int maxK = 9;

    // Kth
    path csvInPath(inputPath);
    csvInPath = csvInPath.parent_path().append("Threshold_0T.csv");
    vector<vector<string>> K(2, vector<string>());
    if (!Csv::Read(csvInPath.string(), K))
        return;
    map<int, float> KV;
    for (int r = 0; r < K[0].size(); ++r) {
        int key = atoi(K[0][r].c_str());
        float val = atof(K[1][r].c_str());
        KV[key] = val;
    }

    outputPath.append("R_Batch_" + to_string(oTh) + "oTh");
    CheckFolderExist(outputPath);

    path csvOutPath(outputPath);
    csvOutPath.append("R_Res_" + to_string(oTh) + "oTh.csv");
    Csv csv(csvOutPath.string(), "cTh,vTh,devAvgNoBG,devAvg,devWeightAvg,CHI,time");
    for (int k = minK; k <= maxK; k += 1) {
        if (KV[k] == 0) {
            cerr << "[DcSTCABatch] " + to_string(k) + " Kth parameter no found.";
            exit(1);
        }

        auto res = RTree::Run(oTh, k, KV[k], inputPath.string(), outputPath.string());

        csv.ofs << k << ',' << KV[k] << ',' << res[0] << ',' << res[1] << ',' << res[2] << ',' << res[3] << ',' << res[4] << endl;
    }
}


Cluster::Cluster(int _id, RNode *node) : id(_id) {
    nodes = vector<RNode *>(0);
    node->poly->cid = id;
    sum = node->poly->sum;
    dev = node->poly->dev;
    pix = node->poly->pix;
    nodes.push_back(node);
    begin = node->timePoint;
    beginStr = GetTPStr(begin);
    end = node->timePoint;
    endStr = GetTPStr(end);
    dur = 1;
    nPoly = 1;
    avg = sum / pix;
}

void Cluster::merge(Cluster *another) {
    another->isMerged = true;
    sum += another->sum;
    dev += another->dev;
    pix += another->pix;
    nPoly += another->nPoly;

    if (begin > another->begin) {
        begin = another->begin;
        beginStr = GetTPStr(begin);
        dur = GetDur(begin, end);
    }
    if (end < another->end) {
        end = another->end;
        endStr = GetTPStr(end);
        dur = GetDur(begin, end);
    }

    for (auto &item: another->nodes) {
        item->cluster = this;
        item->poly->cid = id;
        nodes.push_back(item);
    }
    avg = sum / pix;
}

void Cluster::expand(float v) {
    sum += v;
    dev += v * v;
    ++pix;
    avg = sum / pix;
}

void Cluster::expandRNode(RNode *node) {
    sum += node->poly->sum;
    dev += node->poly->dev;
    pix += node->poly->pix;
    ++nPoly;
    nodes.push_back(node);
    node->cluster = this;
    node->poly->cid = this->id;

    if (node->timePoint > end) {
        end = node->timePoint;
        endStr = GetTPStr(end);
        dur = GetDur(begin, end);
    }
    if (node->timePoint < begin) {
        begin = node->timePoint;
        beginStr = GetTPStr(begin);
        dur = GetDur(begin, end);
    }
    avg = sum / pix;
}

void Cluster::statistic() {
    assert(pix > 0);
    avg = sum / pix;
    dev = dev / pix - avg * avg;
//    dev = x > 0.0 ? sqrt(x) : 0.0;
}
