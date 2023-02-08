//
// Created by 15291 on 2023/1/5.
//
#include "DataModel.h"

int REGION_ID = 2;

Meta::Meta(float _resolution, float _scale, int _nRow, int _nCol, float _startLat, float _startLon, float _endLat,
           float _endLon) :
        resolution(_resolution), scale(_scale), nRow(_nRow), nCol(_nCol),
        startLat(_startLat), startLon(_startLon), endLat(_endLat), endLon(_endLon) {
    if (Meta::DEF.timeUnit == TimeUnit::Mon)
        Meta::DEF.timeScale = 12;
    else if (Meta::DEF.timeUnit == TimeUnit::Day)
        Meta::DEF.timeScale = 366;

    nPixel = _nRow * _nCol;
    latLonOffset = _resolution * 0.5;

    spatialReference = new OGRSpatialReference();
    OGRErr err = spatialReference->SetGeogCS("GCS_WGS_1984",
                                             "D_WGS_1984",
                                             "WGS_1984",
                                             SRS_WGS84_SEMIMAJOR, SRS_WGS84_INVFLATTENING,
                                             "Greenwich", 0.0,
                                             "degree", 0.0174532925199433); // SRS_UA_DEGREE_CONV

    if (err != OGRERR_NONE) {
        cerr << "[Meta] fail to set spatial reference." << endl;
    }
}

GeoRegion::GeoRegion() : GeoRegion(Meta::DEF.startLat, Meta::DEF.endLat, Meta::DEF.startLon, Meta::DEF.endLon) {
    rowMin = INT_MAX;
    rowMax = 0;
    colMin = INT_MAX;
    colMax = 0;
}

GeoRegion::GeoRegion(int _rowMin, int _rowMax, int _colMin, int _colMax) :
        rowMin(_rowMin), rowMax(_rowMax), colMin(_colMin), colMax(_colMax) {
    double pLow[2], pHigh[2];
    pLow[0] = Meta::DEF.getLon(colMin);
    pHigh[0] = Meta::DEF.getLon(colMax);
    pLow[1] = Meta::DEF.getLat(rowMax);
    pHigh[1] = Meta::DEF.getLat(rowMin);

    this->GeoRegion::GeoRegion(pLow, pHigh);
}


bool GeoRegion::checkEdge(int row, int col) {
    bool change = false;
    if (row < rowMin) {
        rowMin = row;
        change = true;
    } else if (row >= rowMax) {
        rowMax = row;
        change = true;
    }
    if (col < colMin) {
        colMin = col;
        change = true;
    } else if (col > colMax) {
        colMax = col;
        change = true;
    }
    return change;
}

// row & col To lat & lon
void GeoRegion::updateGeo() {
    m_pLow[0] = Meta::DEF.getLon(colMin);
    m_pHigh[0] = Meta::DEF.getLon(colMax);
    m_pLow[1] = Meta::DEF.getLat(rowMax);
    m_pHigh[1] = Meta::DEF.getLat(rowMin);
}

bool GeoRegion::isEqual(const GeoRegion &another) {
    if (rowMin == another.rowMin && rowMax == another.rowMax && colMin == another.colMin && colMax == another.colMax)
        return true;
    else
        return false;
}

Line::Line() {}

Line::Line(const Line &other) {
    nodes = other.nodes;
}

Line::Line(vector<Node> _nodes) : nodes(_nodes) {
}


Poly::Poly() {
    id = REGION_ID++;
    clusterId = 0;

    maxValue = DBL_MIN;
    minValue = DBL_MAX;
    sum = 0.0;
    dev = 0.0;
    pix = 0;
}

void Poly::update(int r, int c, float v) {
    range.checkEdge(r, c);

    if (v > maxValue) {
        maxValue = v;
    } else if (v < minValue) {
        minValue = v;
    }
    sum += v;
    dev += v * v;
    ++pix;
    avgValue = sum / pix;
}


Matrix::Matrix(GeoRegion &range, vector<Node> &nodeList) {
    rowOffset = range.rowMin;
    rowLen = range.rowMax - range.rowMin + 2;
    colOffset = range.colMin;
    colLen = range.colMax - range.colMin + 2;
    vector<Node *> emptyRow(colLen, nullptr);
    mat.resize(rowLen, emptyRow);
    for (int i = 0; i < nodeList.size(); ++i) {
        int nr = nodeList[i].row - rowOffset + 1, nc = nodeList[i].col - colOffset + 1;
        if (nr < 0 || nr >= rowLen || nc < 0 || nc >= colLen) {
            cout << "error: [Matrix] build matrix fail." << endl;
            break;
        }
        mat[nr][nc] = &nodeList[i];
    }
//    print();
}

void Matrix::print() {
    for (int i = 0; i < mat.size(); ++i) {
        for (int j = 0; j < mat[0].size(); ++j) {
            if (mat[i][j] == nullptr)
                cout << "  " << " ";
            else
                cout << mat[i][j]->dir1 << mat[i][j]->dir2 << " ";
        }
        cout << endl;
    }
}