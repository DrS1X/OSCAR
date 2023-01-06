//
// Created by 15291 on 2023/1/5.
//
#include "DataModel.h"

int REGION_ID = 2;

Poly::Poly(){
    id = REGION_ID++;
    clusterId = 0;

    maxValue = DBL_MIN;
    minValue = DBL_MAX;
    sumValue = 0.0;
    pixelCount = 0;
}

void Poly::update(float v){
    if(v > maxValue){
        maxValue = v;
    }else if( v < minValue){
        minValue = v;
    }
    sumValue += v;
    ++pixelCount;
    avgValue = sumValue / pixelCount;
}
