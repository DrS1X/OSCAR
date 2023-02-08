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

void DcSTCABatch(bool isDcSTCA, string inputPath, string outputPath, int T, int maxK, int minK, int stepK) {
    if (minK == 0) minK = 4;
    if (maxK == 0) maxK = isDcSTCA ? (2 * T + 1) * 9 : 9;

    // Kth
    filesystem::path parent(inputPath);
    parent = parent.parent_path();
    string csvPath = parent.string();
    csvPath += "\\Threshold_";
    vector<vector<string>> K(maxK - minK + 1, vector<string>(2));
    csvPath += isDcSTCA ? to_string(T) : "0";
    csvPath += "T.csv";
    if(!Csv::Read(csvPath, K, maxK - minK + 1, 2))
        return;
    map<int, float> KV;
    for (const auto &r: K) {
        int key = atoi(r[0].c_str());
        float val = atof(r[1].c_str());
        KV[key] = val;
    }

    if (isDcSTCA)
        outputPath += "\\DcSTCA_Batch_" + to_string(T) + "T";
    else
        outputPath += "\\R_Batch_" + to_string(T) + "T";
    CheckFolderExist(outputPath);

    string csvOutPath = isDcSTCA ? outputPath + "\\DcSTCA_Res_" + to_string(T) + "T.csv" :
                     outputPath + "\\R_Res_" + to_string(T) + "T.csv";
    Csv csv(csvOutPath, "cTh,vTh,avgDev,time");
    for (int k = minK; k <= maxK; k += stepK) {
        float avgDev;

        std::chrono::steady_clock::time_point bef = std::chrono::steady_clock::now();

        if (isDcSTCA) {
            DcSTCA a;
            avgDev = a.Run(inputPath, outputPath, T, k, KV[k]);
        } else {
            avgDev = RTree::Run(T, k, KV[k], inputPath, outputPath);
        }

        std::chrono::steady_clock::time_point aft = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(aft - bef);

        csv.ofs << k << ',' << KV[k] << ',' << avgDev << ',' << time_span.count() << endl;
    }
}


Cluster::Cluster(int _id, double _sum, double _dev, int _pix): id(_id), sum(_sum), dev(_dev), pix(_pix){
    avg = sum / pix;
}

Cluster::Cluster(const Cluster &another) {
    id = another.id;
    sum = another.sum;
    avg = another.avg;
    dev = another.dev;
    pix = another.pix;
}

void Cluster::expand(float v) {
    sum += v;
    dev += v * v;
    ++pix;
    avg = sum / pix;
}

void Cluster::expandBatch(double _sum, double _dev, int _pix) {
    sum += _sum;
    dev += _dev;
    pix += _pix;
    avg = sum / pix;
}

void Cluster::statistic() {
    assert(pix > 0);
    double x = dev / pix - avg * avg;
    dev = x > 0.0 ? sqrt(x) : 0.0;
}
