//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_DATAMODEL_H
#define CLUSTERING_DATAMODEL_H


#include "sidx_impl.h"
#include "util.h"
#include "Cst.h"

using namespace std;

enum AnomalyType {Positive, Negative, None};

enum TimeUnit{Day, Mon};


class Meta {
public:
    float resolution, latLonOffset, scale = 1.0, fillValue = -9999;
    int nBand = 1, nRow, nCol, nPixel;
    float startLat, startLon, endLat, endLon;
    string projection = "GEOGCRS[\"My_GCS_WGS_1984\",DATUM[\"World Geodetic System 1984\",ELLIPSOID[\"WGS 84\",6378137,298.257223563,LENGTHUNIT[\"metre\",1]],ID[\"EPSG\",6326]],PRIMEM[\"Greenwich\",180,ANGLEUNIT[\"Degree\",0.0174532925199433]],CS[ellipsoidal,2],AXIS[\"longitude\",east,ORDER[1],ANGLEUNIT[\"Degree\",0.0174532925199433]],AXIS[\"latitude\",north,ORDER[2],ANGLEUNIT[\"Degree\",0.0174532925199433]]]";
    TimeUnit timeUnit;
    int timeScale;

    // variable field
    string date;
    float minV = FLT_MAX, maxV = FLT_MIN, mean = 0, standard = 0;
    int cnt = 0;

    static Meta DEF;

    Meta(){};

    Meta(float _resolution, int _nRow, int _nCol,float _startLat, float _startLon, float _endLat, float _endLon):
        resolution(_resolution), nRow( _nRow), nCol( _nCol),startLat( _startLat), startLon( _startLon), endLat(_endLat), endLon( _endLon)
    {
        nPixel = _nRow * _nCol;
        latLonOffset = _resolution * 0.5;
    };

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
    static const GeoRegion GLOBAL;
    int rowMin , rowMax, colMin , colMax;
public:
    GeoRegion():GeoRegion(Meta::DEF.startLat, Meta::DEF.endLat, Meta::DEF.startLon, Meta::DEF.endLon){
        rowMin = INT_MAX;
        rowMax = 0;
        colMin = INT_MAX;
        colMax = 0;
    }

    GeoRegion(double latMin, double latMax, double lonMin, double lonMax){
        double pLow[2] = {lonMin, latMin};
        double pHigh[2]= {lonMax,latMax};
        this->GeoRegion::GeoRegion(pLow, pHigh);
    }

    GeoRegion(const double* pLow, const double* pHigh):Region(pLow, pHigh, N_DIM){}

    bool checkEdge(int row, int col){
        bool check = false;
        if(row <= rowMin){
            rowMin = row;
            check = true;
        } else if(row >= rowMax){
            rowMax = row;
            check = true;
        }
        if(col <= colMin){
            colMin = col;
            check = true;
        } else if(col >= colMax){
            colMax = col;
            check = true;
        }
        return check;
    }

    // row & col To lat & lon
    void updateGeo(){
        m_pLow[1] = Meta::DEF.startLat + (rowMin + 0.5) * Meta::DEF.resolution;
        m_pHigh[1] = Meta::DEF.startLat + (rowMax + 0.5) * Meta::DEF.resolution;
        m_pLow[0] = Meta::DEF.startLon + (colMin + 0.5) * Meta::DEF.resolution;
        m_pHigh[0] = Meta::DEF.startLon + (colMax + 0.5) * Meta::DEF.resolution;
    }
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
    string dir1;//方向1
    string dir2;//方向2
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


    bool Line::sortNodes();
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
    int clusterId;
    vector<Line> lines;//所有线
    int minRow;//最小行数
    int minCol;//最小列数
    int maxRow;//最大行数
    int maxCol;//最大列数
    GeoRegion range;
    double area;//实际面积
    double length;
    double avgValue;//平均距平值
    double maxValue;//最大距平值
    double minValue;//最小距平值
    double sumValue;
    long pixelCount;
    int power;//强度
    bool isMulti;//是否是多面
    double centroidRow;//重心行号
    double centroidCol;//重心列号
    myRectangle minRec;//最小面积外包矩形
    myCircle minOutCir;//最小面积外接圆
    myCircle maxInCir;//最大面积内接圆
    chrono::time_point<chrono::system_clock> time;
public:
    Poly();

    void update(float v);

};

#endif //CLUSTERING_DATAMODEL_H
