//
// Created by Administrator on 2022/12/6.
//

#include "Reader.h"

vector<string> ReadBatch(string inputPath, string outputPath, Meta srcMeta) {
    vector<string> fileInList;
    GetFileList(inputPath, fileInList);

    std::vector<std::string> anomalyFileList(fileInList.size());
    std::vector<std::string> resampledFileList(fileInList.size());
    if (fileInList.size() == 0) {
        cerr << "[readBatch] no inputted file." << endl;
        return anomalyFileList;
    }

    // init
    CheckFolderExist(outputPath + "\\resample");
    CheckFolderExist(outputPath + "\\anomaly");

    double ***mean = InitArr<double>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double ***stdDev = InitArr<double>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    int ***cnt = InitArr<int>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0);


    // 1. traverse all source file
    for (int k = 0; k < fileInList.size(); ++k) {
        string fName = fileInList[k];
        Tif *fileIn = new Tif(srcMeta, fName);
        if (!fileIn->read()) {
            cerr << "[readBatch] fail to read tiff file, file name: " << fName << endl;
            return anomalyFileList;
        }

        Tif *fileOut = new Tif(Meta::DEF);

        fileOut->meta.date = fileIn->meta.date;
        resampleAndStatistics(fileIn, fileOut, mean, stdDev, cnt);

        if (!fileOut->write(outputPath + RESAMPLE_PREFIX + RESAMPLE_PREFIX)) return anomalyFileList;
        resampledFileList[k] = fileOut->name;

        delete fileIn;
        delete fileOut;
    }

    // 2. mean and standard in each time unit
    for (int k = 0; k < Meta::DEF.timeScale; ++k) {
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if (cnt[k][i][j] == 0) {
                    assert(IsEqual(mean[k][i][j], 0.0));
                    assert(IsEqual(stdDev[k][i][j], 0.0));
                    continue;
                }

                mean[k][i][j] = mean[k][i][j] / cnt[k][i][j];
                double dev = stdDev[k][i][j] / cnt[k][i][j] - mean[k][i][j] * mean[k][i][j];
                if (dev > 0.0) {
                    stdDev[k][i][j] = sqrt(dev);
                } else {
                    stdDev[k][i][j] = 0.0;
                }
            }
        }
    }

    // 3. standard anomaly to remove periodic fluctuations
    for (int k = 0; k < resampledFileList.size(); ++k) {
        string fName = resampledFileList[k];
        Tif *f = new Tif(Meta::DEF, fName);
        if (!f->read()) {
            cerr << "[readBatch] fail to read tiff file, file name: " << fName << endl;
            return anomalyFileList;
        }

        int order = f->meta.getOrder();
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if (!f->isFillValue(i, j)) {
                    float v;
                    if (IsEqual(0.0, stdDev[order][i][j]))
                        v = 0.0f;
                    else
                        v = (f->get(i, j) - mean[order][i][j]) / stdDev[order][i][j];

                    f->update(i, j, v);
                }
            }
        }

        f->write(outputPath + ANOMALY_PREFIX + ANOMALY_PREFIX);
        anomalyFileList[k] = f->name;

        delete f;
    }
    delete[] mean;
    delete[] stdDev;
    delete[] cnt;
    return anomalyFileList;
}

void resampleAndStatistics(Tif *src, Tif *tar, double ***mean, double ***stdDev, int ***cnt) {
    // down sample to 1 degree resolution
    if (src->meta.startLat > tar->meta.startLat || src->meta.endLat < tar->meta.endLat) {
        cerr << "[readBatch] source raster isn't include target raster" << endl;
        return;
    }
    // cut to -60 to 60 latitude
    int offRow = (tar->meta.startLat - src->meta.startLat) / src->meta.resolution;

    int srcRows = src->meta.nRow, srcCols = src->meta.nCol;
    int tarRows = Meta::DEF.nRow, tarCols = Meta::DEF.nCol;

    float ratio = Meta::DEF.resolution / src->meta.resolution;
    int range = (int) (ratio / 2 + 0.5);
    int regionValidCntThreshold = (2 * range * 2 * range) / 3;
    int order = src->meta.getOrder();

    for (int i = 0; i < tarRows; i++) {
        for (int j = 0; j < tarCols; j++) {
            // 1. down sample
            double sum = 0;
            int regionValidCnt = 0;
            for (int k = -range; k <= range; k++) {
                for (int l = -range; l <= range; l++) {
                    int sr = (int) (i * ratio) + k + offRow;
                    int sc = (int) (j * ratio) + l;
                    if (sr >= 0 && sr < srcRows && sc >= 0 && sc < srcCols && !src->isFillValue(sr, sc)) {
                        sum += src->get(sr, sc);
                        regionValidCnt += 1;
                    }
                }
            }

            // 2. statistic
            int j2 = (j + (tarCols >> 1)) % tarCols; // -180~180 -> 0~360
            if (regionValidCnt != 0 && regionValidCnt >= regionValidCntThreshold) {
                float v = sum / regionValidCnt;
                tar->update(i, j2, v);

                // 2.1 compute mean and standard for anomaly analyze
                mean[order][i][j2] += v;
                stdDev[order][i][j2] += v * v;
                cnt[order][i][j2] += 1;
            } else {
                tar->update(i, j2, tar->meta.fillValue);
            }
        }
    }
}

bool Filter(string inputPath, string outputPath, float stdTimeOfThreshold) {
    vector<string> anomalyFileList;
    GetFileList(inputPath, anomalyFileList);

    if (anomalyFileList.empty()) {
        cerr << "[Filter] no inputted file." << endl;
        return false;
    }

    outputPath += "\\anomaly_" + to_string(stdTimeOfThreshold) + "dev";
    CheckFolderExist(outputPath);

    double **totMean = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double **totSquare = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double **totStdDev = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    long **totCnt = InitArr<long>(Meta::DEF.nRow, Meta::DEF.nCol, 0L);
    double **upLimit = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double **downLimit = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);

    for (int k = 0; k < anomalyFileList.size(); ++k) {
        Tif *f = new Tif(Meta::DEF, anomalyFileList[k]);
        if (!f->read()) {
            return false;
        }

        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if (!f->isFillValue(i, j)) {
                    totMean[i][j] += f->get(i, j);
                    totCnt[i][j] += 1;
                    if (totCnt[i][j] < 0) {
                        cerr << "[Filter] count of pixel is overflow at totCnt["
                             << i << "][" << j << "]." << endl;
                        return false;
                    }
                }
            }
        }
        delete f;
    }

    const double PREVENT_OVERFLOW = 1.0e13;
    for (int k = 0; k < anomalyFileList.size(); ++k) {
        Tif *f = new Tif(Meta::DEF, anomalyFileList[k]);
        if (!f->read()) {
            return false;
        }

        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if (!f->isFillValue(i, j)) {
                    float v = f->get(i, j);
                    totSquare[i][j] += v * v;
                    if (totSquare[i][j] > PREVENT_OVERFLOW) {
                        assert(totCnt[i][j] > 0);
                        totStdDev[i][j] += totSquare[i][j] / totCnt[i][j];
                        totSquare[i][j] = 0.0;
                    }
                }
            }
        }
        delete f;
    }


    // 4. total mean and standard
    for (int i = 0; i < Meta::DEF.nRow; ++i) {
        for (int j = 0; j < Meta::DEF.nCol; ++j) {
            if (totCnt[i][j] == 0) {
                assert(IsEqual(totMean[i][j], 0.0));
                assert(IsEqual(totStdDev[i][j], 0.0));
                continue;
            }

            totMean[i][j] = totMean[i][j] / totCnt[i][j];
            totStdDev[i][j] += totSquare[i][j] / totCnt[i][j];
            double dev = totStdDev[i][j] - totMean[i][j] * totMean[i][j];
            if (dev > 0.0) {
                totStdDev[i][j] = sqrt(dev);
            } else {
                totStdDev[i][j] = 0.0;
            }

            upLimit[i][j] = totMean[i][j] + stdTimeOfThreshold * totStdDev[i][j];
            downLimit[i][j] = totMean[i][j] - stdTimeOfThreshold * totStdDev[i][j];
        }
    }

    // 5. filter little value
    for (int k = 0; k < anomalyFileList.size(); ++k) {
        string fName = anomalyFileList[k];
        Tif *f = new Tif(Meta::DEF, fName);
        if (!f->read()) {
            return false;
        }

        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                float v = f->get(i, j);
                if (v < upLimit[i][j] && v > downLimit[i][j]) {
                    f->update(i, j, 0.0f);
                }
            }
        }

        filesystem::path p(fName);

        f->write(outputPath + "\\" + p.filename().string());

        delete f;
    }
    return true;
}

bool smooth(vector<string> &fileInList, string outputPath, string fileOutPrefix) {
    if (fileInList.empty()) {
        cerr << "[smooth] no inputted file." << endl;
        return false;
    }

    CheckFolderExist(outputPath + fileOutPrefix);
    for (int i = 0; i < fileInList.size(); ++i) {
        Tif *f = new Tif(Meta::DEF, fileInList[i]);
        if (!f->read()) {
            return false;
        }

        f->smooth();
        f->write(outputPath + fileOutPrefix + fileOutPrefix);
        delete f;
    }

    return true;
}

void Background(filesystem::path inPath, double *mean, float *time, filesystem::path maskOutPath) {
    double avg = 0.0, dev = 0.0;
    long cnt = 0;
    map<string, string> metaData;
    metaData["Mean"] = "";
    metaData["StdDev"] = "";
    for (const auto &f: filesystem::directory_iterator(inPath)) {
        Tif *t = new Tif(Meta::DEF);
        t->read(f.path().string(), &metaData);

        double mean0, stdDev0;
        mean0 = ::atof(metaData["Mean"].c_str());
        avg += mean0 * Meta::DEF.nPixel;
        stdDev0 = ::atof(metaData["StdDev"].c_str());
        dev += (stdDev0 * stdDev0 + mean0 * mean0) * Meta::DEF.nPixel;

        /*double mean1 = 0.0, stdDev1 = 0.0;
        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                float v = t->get(r,c);
                mean1 += v;
                stdDev1 += v * v;
            }
        }*/

        ++cnt;
        delete t;
    }

    cnt *= Meta::DEF.nPixel;
    avg = avg / cnt;
    dev = dev / cnt - avg * avg;
    dev = dev <= 0.0 ? 0.0 : sqrt(dev);
    if (mean) *mean = avg;


    if (time == nullptr || maskOutPath.empty())
        return;

    double up = avg + (*time) * dev;
    double down = avg - (*time) * dev;
    for (const auto &f: filesystem::directory_iterator(inPath)) {
        Tif *t = new Tif(Meta::DEF);
        t->read(f.path().string());

        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                float v = t->get(r, c);
                if (v > down && v < up)
                    t->update(r, c, 0.0f);
            }
        }

        string fn = maskOutPath.string() + "\\" + f.path().filename().string();
        t->name = fn;
        t->write();
        delete t;
    }
}

void SimulateData(filesystem::path inPath, filesystem::path outPath) {

    Meta::DEF.nRow = 60;
    Meta::DEF.nCol = 20;
    Meta::DEF.startLat = 0;
    Meta::DEF.endLat = 60;
    Meta::DEF.startLon = 0;
    Meta::DEF.endLon = 20;
    Meta::DEF.nPixel = 60 * 20;

    filesystem::path srcPath(outPath), truthPath(outPath);
    srcPath /= "src";
    truthPath /= "truth";
    CheckFolderExist(srcPath);
    CheckFolderExist(outPath);

    std::vector<std::string> files;
    GetFileList(inPath.string(), files);

    // 从epoch（1970年1月1日00:00:00 UTC）开始经过的纳秒数，unsigned类型会截断这个值
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    // 第一个参数为高斯分布的平均值，第二个参数为标准差
    std::normal_distribution<float> distribution(0.0, 1.0);

    for (const auto &file: std::filesystem::directory_iterator(inPath)) {
        filesystem::path fileName(file.path().filename());
        fileName.replace_extension(".tiff");
        srcPath /= fileName;
        truthPath /= fileName;
        Tif *src = new Tif(Meta::DEF, srcPath.string());
        Tif *truth = new Tif(Meta::DEF, truthPath.string());

        vector<vector<string>> csvDat(Meta::DEF.nRow, vector<string>(Meta::DEF.nCol));
        Csv::Read(file.path().string(), csvDat, Meta::DEF.nRow, Meta::DEF.nCol);
        for (int r = 0; r < Meta::DEF.nRow; ++r) {
            for (int c = 0; c < Meta::DEF.nCol; ++c) {
                float v = 0.0f;
                if (!csvDat[r][c].empty()) {
                    v = std::atof(csvDat[r][c].c_str());
                    if (v >= 3)
                        truth->update(r, c, r / 20 + 1);
                }

                float rand = distribution(generator);
                v += rand;
                src->update(r, c, v);
            }
        }

        src->write();
        truth->write();
        delete src;
        delete truth;
    }
}