//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_UTIL_H
#define CLUSTERING_UTIL_H


#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <array>
#include <string>
#include <chrono>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <float.h>
#include <regex>

#include "_const.h"
#include "DataModel.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

template<class T>
inline T*** initArr(int x, int y, int z){
    T*** arr;
    arr = new T**[x];
    for(int i = 0; i < x; ++i){
        arr[x] = new T*[y];
        for(int j = 0; j < y; ++j){
            arr[x][y] = new T[z];
        }
    }
    return arr;
}

template<class T>
inline T** initArr(int x, int y){
    T** arr;
    arr = new T*[x];
    for(int i = 0; i < x; ++i){
        arr[x] = new T[y];
    }
    return arr;
}

static inline bool isEqual(double a, double b) {
    return fabs(a - b) < std::numeric_limits<double>::epsilon();
}

static inline bool isEqual(float a, float b) {
    return fabs(a - b) < std::numeric_limits<float>::epsilon();
}

static inline bool isFillValue(float v){
    return isEqual(v, FILL_VAL);
}


class util {
public:

    static void checkFilePath(string filePath);

    static int getDayOfYear(string fileName);

    static string generateFileName(string originFileName, string outputPath, string pre, string type, string date);

    static string generateFileName(string originFileName, string outputPath, string pre, string type);

    static string generateFileName(string originFilePath, string outputPath, string suffix);

    static void getFileList(string path, vector<string> &files);

    static void getFileList(string path, vector<string> &files, string fileType);

    static string getDate(string fileName);

    static int GetMonthFromDays(int Days);

    static int GetYearFromDays(int Days);

    static int LeapYear(int Year);

    static double *GetMin_Max(double *value, long *pBuffer, long m_Rows, long m_Cols,
                              long DefaultValue = -9999);   //以数组同时返回最大最小值 索引0为最小值，1为最大值 默认缺省值-9999
    static double *
    GetMin_Max(double *value, double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载
    static double GetMeanValue(long *pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);     //默认缺省值-9999
    static double GetMeanValue(double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载
    static double GetStdValue(long *pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);  //默认缺省值-9999
    static double GetStdValue(double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载

    static void split(std::string &s, std::string &delim, std::vector<std::string> *ret);

    static void addArray(long *arr1, long *arr2, int mRow, int mCol);//arr1+=arr2;
    static double
    STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue,
                  long *pBuffer1, long *pBuffer2, long size);

    static double CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static double CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static double CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static void
    SpatialResampleBasedOnUnevenLat(long *pOriBuffer, long *pTarBuffer, double *pOriLatBuffer, long mOriRows,
                                    long mOriCols, long mTarRows, long mTarCols, double startlog, double endlog,
                                    double startlat, double endlat);
};

#endif //CLUSTERING_UTIL_H