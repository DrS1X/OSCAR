//
// Created by 15291 on 2023/1/5.
//
#include "DataModel.h"

int REGION_ID = 2;


GeoRegion::GeoRegion():GeoRegion(Meta::DEF.startLat, Meta::DEF.endLat, Meta::DEF.startLon, Meta::DEF.endLon){
    rowMin = INT_MAX;
    rowMax = 0;
    colMin = INT_MAX;
    colMax = 0;
}

GeoRegion::GeoRegion(double latMin, double latMax, double lonMin, double lonMax){
    double pLow[2] = {lonMin, latMin};
    double pHigh[2]= {lonMax,latMax};
    this->GeoRegion::GeoRegion(pLow, pHigh);
}


bool GeoRegion::checkEdge(int row, int col){
    bool change = false;
    if(row < rowMin){
        rowMin = row;
        change = true;
    } else if(row >= rowMax){
        rowMax = row;
        change = true;
    }
    if(col < colMin){
        colMin = col;
        change = true;
    } else if(col > colMax){
        colMax = col;
        change = true;
    }
    return change;
}

// row & col To lat & lon
void GeoRegion::updateGeo(){
    m_pLow[0] = Meta::DEF.getLon(colMin);
    m_pHigh[0] = Meta::DEF.getLon(colMax);
    m_pLow[1] = Meta::DEF.getLat(rowMax);
    m_pHigh[1] = Meta::DEF.getLat(rowMin);
}


Line::Line(){}

Line::Line(const Line &other){
    nodes = other.nodes;
}

Line::Line(vector<Node> _nodes):nodes(_nodes){
}


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


Matrix::Matrix(GeoRegion &range, vector<Node> &nodeList) {
    rowOffset = range.rowMin;
    rowLen = range.rowMax - range.rowMin + 2;
    colOffset = range.colMin;
    colLen = range.colMax - range.colMin + 2;
    vector<Node*> emptyRow(colLen, nullptr);
    mat.resize(rowLen, emptyRow);
    for (int i = 0; i < nodeList.size(); ++i) {
        int nr = nodeList[i].row - rowOffset + 1, nc = nodeList[i].col - colOffset + 1;
        if(nr < 0 || nr >= rowLen || nc < 0 || nc >= colLen){
            cout << "error: [Matrix] build matrix fail." << endl;
            break;
        }
        mat[nr][nc] = &nodeList[i];
    }
//    print();
}

void Matrix::print(){
    for(int i = 0; i < mat.size(); ++i){
        for(int j = 0; j < mat[0].size(); ++j){
            if(mat[i][j] == nullptr)
                cout << "  " << " ";
            else
                cout << mat[i][j]->dir1 << mat[i][j]->dir2 << " ";
        }
        cout << endl;
    }
}