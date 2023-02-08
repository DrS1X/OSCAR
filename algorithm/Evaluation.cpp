//
// Created by 15291 on 202NCluster/2/4.
//
#include "Algo.h"

void Evaluation(path truPath, path resPath, path outPath) {
//    outPath /= "evaluation";
//    CheckFolderExist(outPath);

    const int NC = 3;
    vector<vector<int>> M(NC + 2, vector<int>(NC + 2));
    int end = M.size() - 1;

    auto ItEnd = filesystem::directory_iterator();
    auto truIt = filesystem::directory_iterator(truPath);
    auto resIt = filesystem::directory_iterator(resPath);
    for (; truIt != ItEnd && resIt != ItEnd; ++truIt, ++resIt) {
        Tif *tru = new Tif(Meta::DEF, (*truIt).path().string());
        Tif *res = new Tif(Meta::DEF, (*resIt).path().string());
        tru->read();
        res->read();

        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                int cid = res->get(r, c) <= 0 ? 0 : r / (Meta::DEF.nRow / NC) + 1;
                int tid = tru->get(r, c);
                ++M[tid][cid];
            }
        }

        delete tru;
        delete res;
    }
    for (int r = 0; r < end; ++r)
        for (int c = 0; c < M[0].size() - 1; ++c)
            M[r][M[0].size() - 1] += M[r][c];
    for (int c = 0; c < M[0].size(); ++c)
        for (int r = 0; r < end; ++r)
            M[end][c] += M[r][c];
    int N = M[end][M[0].size() - 1];
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
    for (int r = 0; r < end; ++r)
        Sni_ += C2(M[r][M[0].size() - 1]);
    for (int c = 0; c < M[0].size() - 1; ++c)
        Sni_ += C2(M[end][c]);
    double expectedIndex = Sni_ * Sn_j / C2(N);
    double maxIndex = (Sni_ + Sn_j) / 2;
    double index = 0.0;
    for (int r = 0; r < end; ++r) {
        for (int c = 0; c < M[0].size() - 1; ++c) {
            index += C2(M[r][c]);
        }
    }
    double ARI = (index - expectedIndex) / (maxIndex - expectedIndex);

    // NMI
    double MI = 0.0;
    for (int r = 0; r < M.size() - 1; ++r) {
        for (int c = 0; c < M[0].size() - 1; ++c) {
            double Prc = 1.0 * M[r][c] / N;
            double Pr = 1.0 * M[r][end] / N;
            double Pc = 1.0 * M[end][c] / N;
            MI += Prc * L2(Prc / (Pr * Pc));
        }
    }
    double Hr = 0.0, Hc = 0.0;
    for (int r = 0; r < M.size(); ++r) {
        double Pr = 1.0 * M[r][end] / N;
        Hr += -Pr * L2(Pr);
    }
    for (int c = 0; c < M[0].size(); ++c) {
        double Pc = 1.0 * M[end][c] / N;
        Hc += -Pc * L2(Pc);
    }
    double NMI = MI / sqrt(Hr * Hc);

    vector<double> ARIs(NC + 1);
    vector<double> NMIs(NC + 1);
    for (int i = 0; i <= NC; ++i) {
        int TP = M[i][i];
        int FP = M[end][i] - TP;
        int FN = M[i][end] - TP;
        int TN = N - TP - FP - FN;

        /*
           0   1
        0 TP  FN
        1 FP  TN
                */

        double ei = 1.0 * (C2(TP + FN) + C2(FP + TN)) * (C2(TP + FP) + C2(FN + TN)) / C2(N);
        double maxi = (C2(TP + FN) + C2(FP + TN) + C2(TP + FP) + C2(FN + TN)) / 2.0;
        double ind = C2(TP) + C2(FN) + C2(FP) + C2(TN);
        ARIs[i] = (ind - ei) / (maxi - ei);

        double p0_ = 1.0 * (TP + FN) / N;
        double p1_ = 1 - p0_;
        double p_0 = 1.0 * (TP + FP) / N;
        double p_1 = 1 - p_0;
        double hr = -p0_ * L2(p0_) - p1_ * L2(p1_);
        double hc = -p_0 * L2(p_0) - p_1 * L2(p_1);

        double p00 = 1.0 * TP / N;
        double p01 = 1.0 * FN / N;
        double p10 = 1.0 * FP / N;
        double p11 = 1.0 * TN / N;

        double mi = p00 * L2(p00 / p0_ / p_0) +
                    p01 * L2(p01 / p0_ / p_1) +
                    p10 * L2(p10 / p1_ / p_0) +
                    p11 * L2(p11 / p1_ / p_1);

        NMIs[i] = mi / sqrt(hr * hc);
    }

    Csv csv(outPath.string() + "\\Evaluation.csv", "cid,ARI,NMI");
    for (int i = 0; i < NC + 1; ++i) {
        csv.ofs << to_string(i) << ',' <<
                to_string(ARIs[i]) << ',' <<
                to_string(NMIs[i]) << endl;
    }
    csv.ofs << "Total," << ARI << ','<< NMI;
}
