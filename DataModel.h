//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_DATAMODEL_H
#define CLUSTERING_DATAMODEL_H

#include <array>
#include <algorithm>
#include <ogr_spatialref.h>
#include "sidx_impl.h"
#include "Cst.h"

using namespace std;

enum TimeUnit{Day, Mon};


class Meta {
public:
    float resolution, latLonOffset, scale = 1.0, fillValue = -9999;
    int nBand = 1, nRow, nCol, nPixel;
    float startLat, startLon, endLat, endLon;
    OGRSpatialReference *spatialReference;
    TimeUnit timeUnit = TimeUnit::Mon;
    int timeScale;
    bool isSimulated = false;

    // variable field
    string date;
    float minV = FLT_MAX, maxV = FLT_MIN, mean = 0, standard = 0;
    int cnt = 0;

    static Meta DEF;

    Meta(){};

    Meta(float _resolution, float _scale, int _nRow, int _nCol,float _startLat, float _startLon, float _endLat, float _endLon);

    int getOrder(){
        // keep the place for February 29
        static const int DayAccumulate[13] = {-1,30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

        int order = -1;
        if(date == ""){
            cerr << "[getOrder] date is null." << endl;
            return order;
        }
        if(timeUnit == TimeUnit::Mon){
            order = stoi(date.substr(4, 2)) - 1;
        }else if(timeUnit == TimeUnit::Day){
            int month = stoi(date.substr(4, 2));
            int day = stoi(date.substr(6, 2));
            order = DayAccumulate[month - 1] + day;
            if(order >= 366){
                cerr << "[getOrder] order of year greater than 365. month: " << month << ", day: " << day << endl;
                return order;
            }
        }
        return order;
    }

    inline double getLat(int r){
        return (nRow - r - 1) * resolution + startLat;
    }

    inline double getLon(int c){
        return (c + 1) * resolution + startLon;
    }
};

class GeoRegion : public SpatialIndex::Region {
public:
    static GeoRegion GLOBAL;
    int rowMin , rowMax, colMin , colMax;
public:
    GeoRegion();

    GeoRegion(int _rowMin , int _rowMax,int _colMin , int _colMax);

    GeoRegion(const double* pLow, const double* pHigh):Region(pLow, pHigh, N_DIM){}

    bool checkEdge(int row, int col);

    // row & col To lat & lon
    void updateGeo();

    bool isEqual(const GeoRegion& another);
};

class Node {
private:
    map<string, string> DirTurn_RightTopAndLeftBot = {
            {"b", "r"},
            {"t", "l"},
            {"l", "t"},
            {"r", "b"}
    };
    map<string, string> DirTurn_LeftTopAndRightBot = {
            {"b", "l"},
            {"t", "r"},
            {"l", "b"},
            {"r", "t"}
    };
    map<string, string> DirTurn = {
            {"b","t"},
            {"t","b"},
            {"l","r"},
            {"r","l"}
    };
public:
    int id;//唯一标识
    int type;//结点类型
    int row;
    int col;
    string dir1;
    string dir2;
    string outDir;//线进入方向
    bool isVisited;
    int power;//强度
    int eventID;
public:
    Node() {}

    Node(int _nodeType, int _row, int _col) :
            type(_nodeType),
            row(_row),
            col(_col) {
        isVisited = false;

        switch (type) {
            case NodeType_LeftTop:
                dir1 = "l";//左方向
                dir2 = "t";//上方向
                break;
            case NodeType_RightTop:
                dir1 = "t";
                dir2 = "r";
                break;
            case NodeType_LeftBot:
                dir1 = "b";
                dir2 = "l";
                break;
            case NodeType_RightBot:
                dir1 = "r";
                dir2 = "b";
                break;
            case NodeType_TopBot:
                dir1 = "t";
                dir2 = "b";
                break;
            case NodeType_LeftRight:
                dir1 = "l";
                dir2 = "r";
                break;
            default:
                dir1 = "n";
                dir2 = "n";
        }
    }

    bool operator< (const Node& another) const{
        if(this->row != another.row){
            return this->row < another.row;
        }else{
            return this->col < another.col;
        }
    }

    string getNextDirection(string direction) {
        if (type == NodeType_RightTopAndLeftBot) {
            return DirTurn_RightTopAndLeftBot.at(direction);
        } else if(type == NodeType_LeftTopAndRightBot){
            return DirTurn_LeftTopAndRightBot.at(direction);
        }else {
            return dir1 != DirTurn.at(direction) ? dir1 : dir2;
        }
    }
};

class Line {
public:
    int id;//唯一标识
    vector<Node> nodes;//所有点
    int minRow;//最小行数
    int minCol;//最小列数
    int maxRow;//最大行数
    int maxCol;//最大列数
    GeoRegion range;
    int type;//线类型，0为外环，1为内环
    int power;//强度
    int eventID;//事件id
    double length;//长度

    map<string, int> RowMove = {
            {"l", 0},
            {"r", 0},
            {"t", -1},
            {"b", 1}
    };
    map<string, int> ColMove = {
            {"l", -1},
            {"r", 1},
            {"t", 0},
            {"b", 0}
    };

    Line();

    Line( const Line &other);

    Line(vector<Node> _nodes);

};

class myCircle {
public:
    double x;//圆心x坐标
    double y;//圆心y坐标
    double r;//圆半径

    myCircle(double _x, double _y, double _r) {
        x = _x;
        y = _y;
        r = _r;
    }

    myCircle() {
        x = 0.0;
        y = 0.0;
        r = 0.0;
    }
};

class myRectangle {
public:
    double minX;
    double maxX;
    double minY;
    double maxY;
    double area;

    array<double, 2> p1;
    array<double, 2> p2;
    array<double, 2> p3;
    array<double, 2> p4;

    double length;//矩形长度
    double width;//矩形宽度

    myRectangle(double m_minX, double m_maxX, double m_minY, double m_maxY, double m_area) {
        minX = m_minX;
        maxX = m_maxX;
        minY = m_minY;
        maxY = m_maxY;
        area = m_area;

        length = m_maxX - m_minX;
        width = m_maxY - m_minY;
        if (width > length) {//长宽交换
            double temp = length;
            length = width;
            width = temp;
        }

        p1[0] = minX;
        p1[1] = minY;
        p2[0] = maxX;
        p2[1] = minY;
        p3[0] = minX;
        p3[1] = maxY;
        p4[0] = maxX;
        p4[1] = maxY;
    }

    myRectangle() {
        minX = DBL_MIN;
        maxX = DBL_MAX;
        minY = DBL_MIN;
        maxY = DBL_MAX;
        area = DBL_MAX;

        length = DBL_MAX;
        width = DBL_MIN;

        p1[0] = minX;
        p1[1] = minY;
        p2[0] = maxX;
        p2[1] = minY;
        p3[0] = minX;
        p3[1] = maxY;
        p4[0] = maxX;
        p4[1] = maxY;
    }

    myRectangle(array<double, 2> m_p1, array<double, 2> m_p2, array<double, 2> m_p3, array<double, 2> m_p4,
                double m_length, double m_width, double m_area) {
        area = m_area;

        p1[0] = m_p1[0];
        p1[1] = m_p1[1];
        p2[0] = m_p2[0];
        p2[1] = m_p2[1];
        p3[0] = m_p3[0];
        p3[1] = m_p3[1];
        p4[0] = m_p4[0];
        p4[1] = m_p4[1];

        length = m_length;
        width = m_width;

        array<double, 4> m_xa{m_p1[0], m_p2[0], m_p3[0], m_p4[0]};
        array<double, 4> m_ya{m_p1[1], m_p2[1], m_p3[1], m_p4[1]};

        maxX = *max_element(m_xa.begin(), m_xa.end());
        minX = *min_element(m_xa.begin(), m_xa.end());
        minY = *min_element(m_ya.begin(), m_ya.end());
        maxY = *max_element(m_ya.begin(), m_ya.end());
    }
};

class Poly {
public:
    int id;
    int cid;
    vector<Line> lines;
    GeoRegion range;
    double area;
    double avg;//平均距平值
    double maxValue;//最大距平值
    double minValue;//最小距平值
    double sum;
    double dev;
    long pix;
    double sumBG;
    long nPixBG;
public:
    Poly();

    void update(int r, int c, float v);

};


class Matrix{
private:
    vector<vector<Node *>> mat;
    int rowOffset, rowLen, colOffset, colLen;
public:
    Matrix(GeoRegion& range, vector<Node>& nodeList);

    void print();

    inline Node* get(int row, int col){
        return mat[row + 1][col + 1];
    }
};


#endif //CLUSTERING_DATAMODEL_H
