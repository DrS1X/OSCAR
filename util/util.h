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

#include "_const.h"
#include "model/shape.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

class opt {
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

    //long GetDsNum(CString Filename);
    static bool
    DataSpatialConvertByMean(long *pSrcBuffer,/*原数据集*/long *pTarBuffer,/*目标数据集*/long mSrcRows, long mSrcCols,
                             long mTarRows, long mTarCols, double reSize/*转换尺寸*/);

    static double CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static double CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static double CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static void
    SpatialResampleBasedOnUnevenLat(long *pOriBuffer, long *pTarBuffer, double *pOriLatBuffer, long mOriRows,
                                    long mOriCols, long mTarRows, long mTarCols, double startlog, double endlog,
                                    double startlat, double endlat);
};

#endif //CLUSTERING_UTIL_H