//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_UTIL_H
#define CLUSTERING_UTIL_H


#include <io.h>
#include <direct.h>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <array>
#include <string>
#include <chrono>
#include <direct.h>
#include <stdio.h>
#include <float.h>
#include <regex>

#include "_const.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

template<class T>
inline T*** initArr(int x, int y, int z, T initValue){
    T*** arr;
    arr = new T**[x];
    for(int i = 0; i < x; ++i){
        arr[i] = new T*[y];
        for(int j = 0; j < y; ++j){
            arr[i][j] = new T[z];
            for(int k = 0; k < z; ++k){
                arr[i][j][k] = initValue;
            }
        }
    }
    return arr;
}

template<class T>
inline T** initArr(int x, int y, T initValue){
    T** arr;
    arr = new T*[x];
    for(int i = 0; i < x; ++i){
        arr[i] = new T[y];
        for(int j = 0; j < y; ++j){
            arr[i][j] = initValue;
        }
    }
    return arr;
}

bool CheckFolderExist(string folder);

static inline bool isEqual(double a, double b) {
    return fabs(a - b) < std::numeric_limits<double>::epsilon();
}

inline bool isEqual(float a, float b) {
    return fabs(a - b) < std::numeric_limits<float>::epsilon();
}

string GetDate(string fileName);

class util {
public:

    static void checkFilePath(string filePath);

    static string generateFileName(string originFileName, string outputPath, string pre, string type);

    static string generateFileName(string originFilePath, string outputPath, string suffix);

    static void getFileList(string path, vector<string> &files, string fileType);

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
};

#endif //CLUSTERING_UTIL_H