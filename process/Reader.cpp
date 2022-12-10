//
// Created by Administrator on 2022/12/6.
//

#include "DataProcess.h"

using std::string;

Reader::Reader(FileOperator _fo): fi(_fo){
    mean = initArr<float>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol);
    standard = initArr<float>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol);
    cnt = initArr<int>(timeScale, Meta::DEF.nRow, Meta::DEF.nCol);
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

    fi.getMeta(file, meta);
}

bool Reader::ReadBatch(std::vector<std::string>& fileList, TimeUnit timeUnit) {
    if(fileList.size() == 0){
        cerr << "[ReadFile] no file" << endl;
        return false;
    }

    init(fileList[0], timeUnit);

    float** src = initArr<float>(meta.nRow, meta.nCol);
    float** data = initArr<float>(Meta::DEF.nRow, Meta::DEF.nCol);

    for(auto f : fileList){
        if(!fi.read(f, meta, src)) {
            delete[] src;
            delete[] data;
            return false;
        }

        Reader::ResampleAndStatistics(src, data);

        string fileNameOut = "D:\\prData\\ori-mon-" + meta.date + ".tiff";

       ho.writeHDF(fileNameOut, h4Meta, dataInt);
    }

    // mean and standard
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


    delete[] src;
    delete[] data;
    return true;
}

bool Reader::ResampleAndStatistics(float **src, float** data) {
    // 1.down sample to 1 degree resolution
    int srcRows = meta.nRow, srcCols = meta.nCol;
    int tarRows = Meta::DEF.nRow, tarCols = Meta::DEF.nCol;

    float ratio = Meta::DEF.resolution / meta.resolution;
    int offset1 =  (int)(-ratio / 2 - 0.5);
    int offset2 =  (int)(ratio / 2 + 0.5);

    for (int i = 0;i <tarRows;i++)
    {
        for (int j = 0;j<tarCols;j++)
        {
            // 1. Down sample
            double sum = 0;
            int regionValidCnt = 0;
            int regionCnt = 0;
            for (int k = offset1; k <= offset2; k++)
            {
                for (int l = offset1; l <= offset2; l++)
                {
                    ++cnt;
                    int sr = (int)(i * ratio + k);
                    int sc = (int)(j * ratio + l);
                    if (sr >= 0 && sr < srcRows && sc >= 0 && sc < srcCols && !isFillValue(src[sr][sc]))
                    {
                        sum += src[sr][sc];
                        regionValidCnt += 1;
                    }
                }
            }

            if (regionValidCnt != 0 && regionValidCnt >= regionCnt / 3) {
                data[i][j] = sum / regionValidCnt;

                // 2. compute mean and standard for anomaly analyze
                int order = Reader::getOrder(meta.date);
                mean[order][i][j] += data[i][j];
                standard[order][i][j] += data[i][j] * data[i][j];
                cnt[order][i][j] += 1;
            }else{
                data[i][j] = FILL_VAL;
            }
        }
    }

    //
}

int Reader::getOrder(string date){
    static const int DayAccumulate[12] = {31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

    int order = -1;
    if(UNIT == TimeUnit::Mon){
        order = stoi(date.substr(4, 2)) - 1;
    }else if(UNIT == TimeUnit::Day){
        int month = stoi(date.substr(4, 2));
        int day = stoi(date.substr(6, 2));

        if (month == 1)
            order = day - 1;
        else
            order = DayAccumulate[month - 2] + day - 1;
    }
    return order;
}