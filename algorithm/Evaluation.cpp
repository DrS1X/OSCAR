//
// Created by 15291 on 202NCluster/2/4.
//
#include "Algo.h"

void Evaluation(path truPath, path resPath, path outPath) {
//    outPath /= "evaluation";
//    CheckFolderExist(outPath);

    const int ClusterIDOffset = 1;
    const int NC = 3;
    vector<vector<int>> M(NC + 2, vector<int>()); // unknown the number of column

    // input algorithm result and ground truth
    auto ItEnd = filesystem::directory_iterator();
    auto truIt = filesystem::directory_iterator(truPath);
    auto resIt = filesystem::directory_iterator(resPath);
    int RowEnd = M.size() - 1;
    int ColEnd = 0;
    for (; truIt != ItEnd && resIt != ItEnd; ++truIt, ++resIt) {
        Tif *tru = new Tif(Meta::DEF, (*truIt).path().string());
        Tif *res = new Tif(Meta::DEF, (*resIt).path().string());
        tru->read();
        res->read();

        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                int cid = res->get(r, c) <= 0 ? 0 : res->get(r, c) - ClusterIDOffset;
                int tid = tru->get(r, c);
                if (cid >= M[tid].size()) {
                    M[tid].resize(cid + 1, 0);
                    if (cid > ColEnd)
                        ColEnd = cid;
                }
                ++M[tid][cid];
            }
        }

        delete tru;
        delete res;
    }

    // complete the matrix M
    ++ColEnd;
    for (int r = 0; r < RowEnd; ++r) {
        M[r].resize(ColEnd + 1, 0);
        for (int c = 0; c < ColEnd; ++c) {
            M[r][ColEnd] += M[r][c];
        }
    }
    M[RowEnd].resize(ColEnd + 1, 0);

    for (int c = 0; c < M[0].size(); ++c)
        for (int r = 0; r < RowEnd; ++r)
            M[RowEnd][c] += M[r][c];

    int N = M[RowEnd][ColEnd];
    Csv csvM(outPath.string() + "\\M.csv");
    for (int r = 0; r < M.size(); ++r) {
        csvM.ofs << M[r][0];
        for (int c = 1; c < M[0].size(); ++c) {
            csvM.ofs << ',' << M[r][c];
        }
        csvM.ofs << endl;
    }

    // ARI
    double Sni_ = 0.0, Sn_j = 0.0;
    for (int r = 0; r < RowEnd; ++r)
        Sni_ += C2(M[r][ColEnd]);
    for (int c = 0; c < ColEnd; ++c)
        Sn_j += C2(M[RowEnd][c]);
    double expectedIndex = Sni_ * Sn_j / C2(N);
    double maxIndex = (Sni_ + Sn_j) / 2;
    double index = 0.0;
    for (int r = 0; r < RowEnd; ++r) {
        for (int c = 0; c < ColEnd; ++c) {
            index += C2(M[r][c]);
        }
    }
    double ARI = (index - expectedIndex) / (maxIndex - expectedIndex);

    // NMI
    double MI = 0.0;
    for (int r = 0; r < RowEnd; ++r) {
        for (int c = 0; c < ColEnd; ++c) {
            double Prc = 1.0 * M[r][c] / N;
            double Pr = 1.0 * M[r][ColEnd] / N;
            double Pc = 1.0 * M[RowEnd][c] / N;
            MI += IsZero(Prc) ? 0.0 : Prc * L2(Prc / (Pr * Pc));
        }
    }
    double Hr = 0.0, Hc = 0.0;
    for (int r = 0; r < RowEnd; ++r) {
        double Pr = 1.0 * M[r][ColEnd] / N;
        Hr += -Pr * L2(Pr);
    }
    for (int c = 0; c < ColEnd; ++c) {
        double Pc = 1.0 * M[RowEnd][c] / N;
        Hc += -Pc * L2(Pc);
    }
    double NMI = 2 * MI / (Hr + Hc);

    Csv csv(outPath.string() + "\\Evaluation.csv", "ARI,NMI");
    csv.ofs << ARI << ',' << NMI << endl;
}

vector<float> InnerEval(Cluster *BG, double datasetMean, map<int, Cluster *> &clusters) {
    BG->statistic();
    float devAvg = 0, devAvgNoBG = 0, devWeightAvg = 0;
    long nPix = 0;
    int nCls = 0;
    float CHI_1 = BG->pix * (BG->avg - datasetMean) * (BG->avg - datasetMean);
    float CHI_2 = BG->dev * BG->pix;
    float CHI;
    for (const auto &item: clusters) {
        Cluster *pc = item.second;
        if (!pc->isMerged) {
            pc->statistic();

            CHI_1 += pc->avg - datasetMean;
            CHI_2 += pc->dev * pc->pix;
            devAvg += pc->dev;
            devWeightAvg += pc->dev * pc->pix;
            nPix += pc->pix;
            ++nCls;
        }
        delete pc;
    }

    CHI_1 = CHI_1 / (nCls - 1);
    CHI_2 = CHI_2 / (nPix - nCls);
    CHI = CHI_1 / CHI_2;

    devAvgNoBG = devAvg / nCls;

    devAvg += BG->dev;
    devWeightAvg += BG->dev * BG->pix;
    nPix += BG->pix;
    nCls += 1;

    devAvg = devAvg / nCls;
    devWeightAvg = devWeightAvg / nPix;

    vector<float> res = {devAvgNoBG, devAvg, devWeightAvg, CHI};
    return res;
}