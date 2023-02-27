//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_UTIL_H
#define CLUSTERING_UTIL_H


#include <io.h>
#include <direct.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <direct.h>
#include <stdio.h>
#include <float.h>
#include <regex>
#include <filesystem>
#include "Cst.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

template<class T>
inline T*** InitArr(int x, int y, int z, T initValue){
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
inline T** InitArr(int x, int y, T initValue){
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

inline bool IsEqual(float a, float b) {
    return fabs(a - b) < std::numeric_limits<float>::epsilon();
}

inline bool IsEqual(double a, double b) {
    return fabs(a - b) < std::numeric_limits<double>::epsilon();
}

inline bool IsZero(float a){
    return IsEqual(a, 0.0f);
}

inline long C2(int x){
    return x * (x - 1) / 2;
}

inline double L2(double x){
    return x < std::numeric_limits<double>::epsilon() ? 0.0 : log2(x);
}


inline bool CheckFolderExist(std::filesystem::path folder, bool cover = true){
    std::error_code err;
    bool exist = false;
    if (std::filesystem::exists(folder)){
        exist = true;
        if(!cover)
            return exist;
        if(!std::filesystem::remove_all(folder,err)){
            std::cerr << "[CheckFolderExist] fail to remove the old folder: " << folder
                      << ", err code: " << err.message() << endl;
            return exist;
        }
    }

    if(!std::filesystem::create_directories(folder, err)) {
        std::cerr << "[CheckFolderExist] fail to create new folder " << folder
                  << ", err code: " << err.message() << endl;
    }
    return exist;
}

inline void GetFileList(string path, vector<string>& files)
{
    for (const auto & file : std::filesystem::recursive_directory_iterator(path)) {
        if(file.is_directory())
            continue;
        string fileName = file.path().string();
        int point = fileName.find_last_of('.');
        if(fileName.substr(point, 4) == ".xml")
            continue;
        files.push_back(fileName);
    }
}

inline string GetDate(string fileName) {
    static const std::regex pattern("19|20[0-9]{2}[0-1][0-9]([0-3][0-9]|)");

    string date = "";
    int idx = fileName.find_last_of("\\");
    if(idx == -1)
        idx = fileName.find_last_of("/");
    if(idx != -1)
        fileName = fileName.substr(idx + 1);

    std::smatch result;
    string::const_iterator iter_begin = fileName.cbegin();
    string::const_iterator iter_end = fileName.cend();
    if (regex_search(iter_begin, iter_end,result, pattern)){
        date = fileName.substr(result[0].first - iter_begin,result[0].second - result[0].first);
    }else{
        cout << "[GetDate] failed to match date from file name: " <<fileName<< endl;
    }

    return date;
}

inline string GenerateFileName(string originFileName, string outputPath, string pre, string type) {
    string date = GetDate(originFileName);
    string folder = outputPath + "\\";

    string mOutFileName = folder + pre + date + type;
    return mOutFileName;
}

inline string GenerateFileName(string originFilePath, string outputPath, string suffix) {
    string fileName = originFilePath.substr(originFilePath.find_last_of("\\"),
                                            originFilePath.find_last_of(".") - originFilePath.find_last_of("\\"));
    string folder = outputPath + "\\";

    string outFileName = folder + fileName + suffix;
    return outFileName;
}

#endif //CLUSTERING_UTIL_H