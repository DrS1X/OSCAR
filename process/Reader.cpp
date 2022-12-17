//
// Created by Administrator on 2022/12/6.
//

#include "DataProcess.h"

using std::string;

Reader::Reader(string _prefix, int _stdTimeOfThreshold, RFileOpt *_fi, RFileOpt *_fo):
        fi(_fi), fo(_fo), stdTimeOfThreshold(_stdTimeOfThreshold), prefix(_prefix)
{
    meta = Meta::DEF;
    mean = initArr<float>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0.0F);
    standard = initArr<float>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0);
    cnt = initArr<int>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol, 0);
}

Reader::~Reader(){
    delete[] mean;
    delete[] standard;
    delete[] cnt;
}

void Reader::init(string file, TimeUnit timeUnit){
    for(int i = 0; i < timeScale; ++i){
        for(int j = 0; j < Meta::DEF.nPixel; ++j){
            mean[i][j] = 0;
            standard[i][j] = 0;
        }
    }

    meta = Meta::DEF;
    meta.timeUnit = timeUnit;
    if(timeUnit == TimeUnit::Mon)
        meta.timeScale = 12;
    else if(timeUnit == TimeUnit::Day)
        meta.timeScale = 365;

    fi->getMeta(file, meta);
}

bool Reader::readBatch(std::vector<std::string>& fileInList, TimeUnit timeUnit) {
    std::vector<std::string> fileOutList;

    if(fileInList.size() == 0){
        cerr << "[readBatch] folder is empty" << endl;
        return false;
    }

    init(fileInList[0], timeUnit);

    RFile fileIn(meta);
    RFile fileOut(Meta::DEF);

    // 1. traverse all source file
    for(auto name : fileInList){
        if(!fi->read(fileIn)) {
            return false;
        }

        resampleAndStatistics(fileIn, fileOut);
        fileOut.name = prefix + RESAMPLE_FOLDER + meta.date;
        fileOutList.push_back(fileOut.name);
        fo->write(fileOut);
    }

    // 2. mean and standard
    for(int k = 0 ; k < timeScale; ++k) {
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                if(cnt[k][i][j] == 0){
                    mean[k][i][j] = 0.0;
                    standard[k][i][j] = 0.0;
                    continue;
                }

                mean[k][i][j] = mean[k][i][j] / cnt[k][i][j];
                float tmp1 = standard[k][i][j] / cnt[k][i][j];
                float tmp2 = mean[k][i][j] * mean[k][i][j];
                if(tmp1 >= tmp2)
                    standard[k][i][j] = sqrt(tmp1 - tmp2);
                else
                    standard[k][i][j] = 0.0;
            }
        }
    }

    // 3. anomaly analyze
    RFile pos(meta);
    RFile neg(meta);
    for(auto name : fileOutList){
        if(!fo->read(fileIn)) {
            return false;
        }

        int order = fileIn.meta.getOrder();
        for (int i = 0; i < Meta::DEF.nRow; ++i) {
            for (int j = 0; j < Meta::DEF.nCol; ++j) {
                float v;
                if(fileIn.data[i][j] == fileIn.meta.fillValue)
                    v = fileIn.meta.fillValue;
                else
                    v = (fileIn.data[i][j] - mean[order][i][j]) / standard[order][i][j];
                fileOut.updateData(v, i ,j);
            }
        }
        fileOut.meta.statisticsFinish();

        splitFile(fileOut.meta.mean - stdTimeOfThreshold * fileOut.meta.standard,
                       fileOut.meta.mean + stdTimeOfThreshold * fileOut.meta.standard,
                       fileIn, pos, neg);

        pos.name = prefix + POSITIVE_FOLDER + meta.date;
        neg.name = prefix + NEGATIVE_FOLDER + meta.date;
        fo->write(pos);
        fo->write(neg);
    }

    return true;
}

void Reader::resampleAndStatistics(RFile& src, RFile& tar) {
    // 1.down sample to 1 degree resolution
    int srcRows = meta.nRow, srcCols = meta.nCol;
    int tarRows = Meta::DEF.nRow, tarCols = Meta::DEF.nCol;

    float ratio = Meta::DEF.resolution / meta.resolution;
    int offset1 =  (int)(-ratio / 2 - 0.5);
    int offset2 =  (int)(ratio / 2 + 0.5);

    for (int i = 0;i <tarRows;i++){
        for (int j = 0;j<tarCols;j++){
            // 1. down sample
            double sum = 0;
            int regionValidCnt = 0;
            int regionCnt = 0;
            for (int k = offset1; k <= offset2; k++){
                for (int l = offset1; l <= offset2; l++){
                    ++cnt;
                    int sr = (int)(i * ratio + k);
                    int sc = (int)(j * ratio + l);
                    if (sr >= 0 && sr < srcRows && sc >= 0 && sc < srcCols && src.isFillValue(sr,sc)){
                        sum += src.data[sr][sc];
                        regionValidCnt += 1;
                    }
                }
            }

            // 2. statistic
            if (regionValidCnt != 0 && regionValidCnt >= regionCnt / 3) {
                float v = sum / regionValidCnt;
                tar.data[i][j] = v;

                // 2.1 compute mean and standard for anomaly analyze
                int order = meta.getOrder();
                mean[order][i][j] += v;
                standard[order][i][j] += v * v;
                cnt[order][i][j] += 1;

                // 2.2 compute mean and standard of single image
                tar.meta.statisticsComputing(v);
            }else{
                tar.data[i][j] = tar.meta.fillValue;
            }
        }
    }
    tar.meta.statisticsFinish();
}

void Reader::splitFile(float downLimit,float upLimit, RFile file, RFile pos, RFile neg){
    file.meta.mean = 0;
    file.meta.standard = 0;
    file.meta.minV = FLT_MAX;
    file.meta.maxV = FLT_MIN;

    for(int i = 0; i < file.meta.nRow; ++i){
        for(int j = 0; j < file.meta.nCol; ++j){
            float v = file.data[i][j];
            if(v == file.meta.fillValue) continue;

            if(v > upLimit){
                pos.updateData(v, i, j);
                neg.updateData(0, i, j);
            } else if(v < downLimit){
                pos.updateData(0, i, j);
                neg.updateData(v, i, j);
            }else{
                pos.updateData(0, i, j);
                neg.updateData(0, i, j);
            }
        }
    }
    pos.meta.statisticsFinish();
    neg.meta.statisticsFinish();
}