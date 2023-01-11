//
// Created by Administrator on 2022/12/6.
//

#ifndef CLUSTERING_RST_H
#define CLUSTERING_RST_H
#include <string>
#include "DataModel.h"

class Rst{
public:
    Meta meta = Meta::DEF;
    string name = "";
    float* data = nullptr; // length = nRow * nCol

    Rst(){
        meta = Meta::DEF;
        init();
    };

    Rst(Meta _meta, string _name, float* _data):
            meta(_meta), name(_name), data(_data){};

    Rst(Meta _meta, string _name): meta(_meta), name(_name){
        init();
    };

    Rst(Meta _meta): meta(_meta){
        init();
    };

    Rst(string _name): name(_name){
        meta = Meta::DEF;
    };

    void init(){
        data = new float[meta.nRow * meta.nCol];
        for(int i = 0; i < meta.nRow; ++i)
            for(int j = 0; j < meta.nCol; ++j)
                data[i * meta.nCol + j] = 0.0F;
    }

    int index(int row, int col)  {
        return row * meta.nCol + col;
    };

    bool checkIndex(int row, int col) {
        if (row < 0 || row >= meta.nRow || col < 0 || col >= meta.nCol) {
            return false;
        } else {
            return true;
        }
    }

    bool isFillValue(int r, int c){
        float scaledFillValue = IsEqual(meta.scale, 1.0f) ? meta.fillValue : meta.fillValue * meta.scale;
        return IsEqual(data[index(r, c)], scaledFillValue);
    };

    inline bool isZero(int r, int c){
        return IsEqual(data[index(r, c)], 0.0f);
    };

    virtual float get(int row, int col) {
        if(!checkIndex(row, col)) {
            cerr << "[get] out of data range. row:"<< row << ", col:" << col << endl;
            return meta.fillValue;
        }else
            return data[index(row, col)];
    };

    void update(int r, int c,float v){
        if(IsEqual(v, meta.fillValue) && !IsEqual(meta.scale,1.0f))
            v *= meta.scale;
        data[index(r,c)] = v;
    };

    virtual ~Rst(){
        delete[] data;
    }

    virtual bool read(string fName = "")  = 0;
    virtual bool write(string fName = "") = 0;
};

#endif //CLUSTERING_RST_H
