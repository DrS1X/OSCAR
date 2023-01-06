//
// Created by Administrator on 2022/12/6.
//

#include "Reader.h"

using std::string;

Reader::Reader(string _outputPath): outputPath(_outputPath)
{


}

Reader::~Reader(){
}


vector<string> Reader::readBatch(std::vector<std::string>& fileInList, Meta srcMeta) {
    std::vector<std::string> anomalyFileList(fileInList.size());
    std::vector<std::string> resampledFileList(fileInList.size());
    if(fileInList.size() == 0){
        cerr << "[readBatch] no inputted file." << endl;
        return anomalyFileList;
    }
    
    // init
    CheckFolderExist(outputPath + "\\resample");
    CheckFolderExist(outputPath + "\\anomaly");
    Meta::DEF.timeUnit = srcMeta.timeUnit;
    if(Meta::DEF.timeUnit == TimeUnit::Mon)
        Meta::DEF.timeScale = 12;
    else if(Meta::DEF.timeUnit == TimeUnit::Day)
        Meta::DEF.timeScale = 366;

    double*** mean = InitArr<double>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double*** stdDev = InitArr<double>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    int*** cnt = InitArr<int>(Meta::DEF.timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0);
        

    // 1. traverse all source file
    for(int k = 0; k < fileInList.size(); ++k) {
        string fName = fileInList[k];
        Tif* fileIn = new Tif(srcMeta, fName);
        if(!fileIn->read()) {
            cerr << "[readBatch] fail to read tiff file, file name: " << fName << endl;
            return anomalyFileList;
        }

        Tif* fileOut = new Tif(Meta::DEF);

        fileOut->meta.date = fileIn->meta.date;
        Reader::resampleAndStatistics(fileIn, fileOut, mean, stdDev, cnt);

        if(!fileOut->write(outputPath + RESAMPLE_PREFIX + RESAMPLE_PREFIX)) return anomalyFileList;
        resampledFileList[k] = fileOut->name;

        delete fileIn;
        delete fileOut;
    }

    // 2. mean and standard in each time unit
    for(int k = 0 ; k < Meta::DEF.timeScale; ++k) {
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if (cnt[k][i][j] == 0) {
                    assert(IsEqual(mean[k][i][j], 0.0));
                    assert(IsEqual(stdDev[k][i][j], 0.0));
                    continue;
                }

                mean[k][i][j] = mean[k][i][j] / cnt[k][i][j];
                double dev = stdDev[k][i][j] / cnt[k][i][j] - mean[k][i][j] * mean[k][i][j];
                if (dev > 0.0){
                    stdDev[k][i][j] = sqrt(dev);
                }else{
                    stdDev[k][i][j] = 0.0;
                }
            }
        }
    }

    // 3. standard anomaly to remove periodic fluctuations
    for(int k = 0; k < resampledFileList.size(); ++k) {
        string fName = resampledFileList[k];
        Tif* f = new Tif(Meta::DEF, fName);
        if(!f->read()) {
            cerr << "[readBatch] fail to read tiff file, file name: " << fName << endl;
            return anomalyFileList;
        }

        int order = f->meta.getOrder();
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if(!f->isFillValue(i,j)) {
                    float v;
                    if(IsEqual(0.0, stdDev[order][i][j]))
                        v = 0.0f;
                    else
                        v = (f->get(i, j) - mean[order][i][j]) / stdDev[order][i][j];
                    
                    f->update(i, j, v);
                }
            }
        }

        f->write(outputPath + ANOMALY_PREFIX + ANOMALY_PREFIX );
        anomalyFileList[k] = f->name;

        delete f;
    }
    delete[] mean;
    delete[] stdDev;
    delete[] cnt;
    return anomalyFileList;
}

void Reader::resampleAndStatistics(Tif* src, Tif* tar, double *** mean, double*** stdDev, int*** cnt) {
    // down sample to 1 degree resolution, cut to -60 to 60 latitude
    if(src->meta.startLat > tar->meta.startLat || src->meta.endLat < tar->meta.endLat){
        cerr << "[readBatch] source raster isn't include target raster" << endl;
        return;
    }
    int offRow = (tar->meta.startLat - src->meta.startLat) / src->meta.resolution;

    int srcRows = src->meta.nRow, srcCols = src->meta.nCol;
    int tarRows = Meta::DEF.nRow, tarCols = Meta::DEF.nCol;

    float ratio = Meta::DEF.resolution / src->meta.resolution;
    int range =  (int)(ratio / 2 + 0.5);
    int regionValidCntThreshold = (2 * range * 2 * range) / 3;
    int order = src->meta.getOrder();

    for (int i = 0;i <tarRows;i++){
        for (int j = 0;j<tarCols;j++){
            // 1. down sample
            double sum = 0;
            int regionValidCnt = 0;
            for (int k = -range; k <= range; k++){
                for (int l = -range; l <= range; l++){
                    int sr = (int)(i * ratio) + k + offRow;
                    int sc = (int)(j * ratio) + l;
                    if (sr >= 0 && sr < srcRows && sc >= 0 && sc < srcCols && !src->isFillValue(sr,sc)){
                        sum += src->get(sr,sc);
                        regionValidCnt += 1;
                    }
                }
            }

            // 2. statistic
            if (regionValidCnt != 0 && regionValidCnt >= regionValidCntThreshold) {
                float v = sum / regionValidCnt;
                tar->update(i,j, v);

                // 2.1 compute mean and standard for anomaly analyze
                mean[order][i][j] += v;
                stdDev[order][i][j] += v * v;
                cnt[order][i][j] += 1;
            }else{
                tar->update(i,j,tar->meta.fillValue);
            }
        }
    }
}

pair<vector<string>,vector<string>> Reader::filter(vector<string>& anomalyFileList, float stdTimeOfThreshold){
    vector<string> posLs(anomalyFileList.size());
    vector<string> negLs(anomalyFileList.size());
    pair<vector<string>,vector<string>> lsPr{posLs,negLs};

    if(anomalyFileList.empty()){
        cerr << "[filter] no inputted file." << endl;
        return lsPr;
    }

    CheckFolderExist(outputPath + "\\posAnomaly");
    CheckFolderExist(outputPath + "\\negAnomaly");
    double ** totMean = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double ** totSquare = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double ** totStdDev = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    long ** totCnt = InitArr<long>(Meta::DEF.nRow, Meta::DEF.nCol, 0L);
    double ** upLimit = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);
    double ** downLimit = InitArr<double>(Meta::DEF.nRow, Meta::DEF.nCol, 0.0);

    for(int k = 0; k < anomalyFileList.size(); ++k) {
        Tif *f = new Tif(Meta::DEF, anomalyFileList[k]);
        if (!f->read()) {
            return lsPr;
        }

        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if(!f->isFillValue(i,j)) {
                    totMean[i][j] += f->get(i, j);
                    totCnt[i][j] += 1;
                    if(totCnt[i][j] < 0){
                        cerr << "[filter] count of pixel is overflow at totCnt["
                        << i << "]["<<j<<"]."<<endl;
                        return lsPr;
                    }
                }
            }
        }
        delete f;
    }

    static const double PREVENT_OVERFLOW = 1.0e13;
    for(int k = 0; k < anomalyFileList.size(); ++k) {
        Tif *f = new Tif(Meta::DEF, anomalyFileList[k]);
        if (!f->read()) {
            return lsPr;
        }

        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if(!f->isFillValue(i,j)) {
                    float v = f->get(i, j);
                    totSquare[i][j] += v * v;
                    if(totSquare[i][j] > PREVENT_OVERFLOW){
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
            if(totCnt[i][j] == 0){
                assert(IsEqual(totMean[i][j], 0.0));
                assert(IsEqual(totStdDev[i][j], 0.0));
                continue;
            }

            totMean[i][j] = totMean[i][j] / totCnt[i][j];
            totStdDev[i][j] += totSquare[i][j] / totCnt[i][j];
            double dev = totStdDev[i][j] - totMean[i][j] * totMean[i][j];
            if(dev > 0.0){
                totStdDev[i][j] = sqrt(dev);
            }else{
                totStdDev[i][j] = 0.0;
            }

            upLimit[i][j] = totMean[i][j] + stdTimeOfThreshold * totStdDev[i][j];
            downLimit[i][j] = totMean[i][j] - stdTimeOfThreshold * totStdDev[i][j];
        }
    }

    // 5. filter little value
   for(int k = 0; k < anomalyFileList.size(); ++k) {
        string fName = anomalyFileList[k];
        Tif *f = new Tif(Meta::DEF, fName);
        if (!f->read()) {
            return lsPr;
        }

        Tif* pos = new Tif(f->meta);
        Tif* neg = new Tif(f->meta);
        pos->meta.date = f->meta.date;
        neg->meta.date = f->meta.date;

        for(int i = 0; i < Meta::DEF.nRow; ++i){
            for(int j = 0; j < Meta::DEF.nCol; ++j){
                if(f->isFillValue(i,j)){
                    pos->update(i, j, Meta::DEF.fillValue);
                    neg->update(i, j, Meta::DEF.fillValue);
                    continue;
                }

                float v = f->get(i,j);
                if(v > upLimit[i][j]){
                    pos->update( i, j,v);
                    neg->update( i, j,0.0f);
                }else if(v < downLimit[i][j]){
                    pos->update(i, j,0.0f);
                    neg->update(i, j,v);
                }else{
                    pos->update(i, j,0.0f);
                    neg->update(i, j,0.0f);
                }
            }
        }

        pos->write(outputPath + POSITIVE_PREFIX + POSITIVE_PREFIX);
        neg->write(outputPath + NEGATIVE_PREFIX + NEGATIVE_PREFIX);

        lsPr.first[k] = pos->name;
        lsPr.second[k] = neg->name;

        delete f;
        delete pos;
        delete neg;
    }
    return lsPr;
}

bool Reader::smooth(vector<string> &fileInList, string fileOutPrefix){
    if(fileInList.empty()){
        cerr << "[smooth] no inputted file." << endl;
        return false;
    }

    CheckFolderExist(outputPath + fileOutPrefix);
    for(int i = 0 ; i < fileInList.size(); ++i){
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