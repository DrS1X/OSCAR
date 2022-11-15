//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_OPT_H
#define CLUSTERING_OPT_H

#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <array>
#include <string>
#include <chrono>
#include <hdfi.h>
#include <gdal_priv.h>
#include <gdal.h>
#include <sidx_api.h>
#include "Region.h"
#include "_const.h"

using namespace std;

class GeoRegion : public SpatialIndex::Region {
public:
    static const GeoRegion GLOBAL;
    static const int N_DIM = 2;
    int rowMin , rowMax, colMin , colMax ;
public:
    GeoRegion():Region(){
        rowMin = INT_MAX;
        rowMax = 0;
        colMin = INT_MAX;
        colMax = 0;
    }

    GeoRegion(double latMin, double latMax, double lonMin, double lonMax){
        double pLow[2];
        double pHigh[2];
        pLow[0] = lonMin;
        pHigh[0] = lonMax;
        pLow[1] = latMin;
        pHigh[1] = latMax;
        Region(pLow, pHigh, N_DIM);
    }

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
    bool updateGeo(){
        m_pHigh[1] = Def.StartLat - (rowMin + 0.5) * Def.Resolution;
        m_pLow[1] = Def.StartLat - (rowMax + 0.5) * Def.Resolution;
        m_pLow[0] = Def.StartLon + (colMin + 0.5) * Def.Resolution;
        m_pHigh[0] = Def.StartLon + (colMax + 0.5) * Def.Resolution;
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
    Poly(){
        maxValue = DBL_MIN;
        minValue = DBL_MAX;
        sumValue = 0.0;
        pixelCount = 0;
    }

    void update(float v){
        if(v > maxValue){
            maxValue = v;
        }else if( v < minValue){
            minValue = v;
        }
        sumValue += v;
        ++pixelCount;
        avgValue = sumValue / pixelCount;
    }

};

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

class hdfOpt {
public:
    hdfOpt();

    hdfOpt(Meta meta);

    hdfOpt(string, int);

public:
    Meta meta;

    uint16 pmax, pmin;               //读取的物理数据的最值
    double MaxValue, MinValue;

    float32 vSlope[1], vIntercept[1];         //由图像数据转换成物理数据的参数
    double vCoordinate[4];                                   //定义4个角点坐标

    int row, cols;                   //图像 的行列数

    int32 start[3], endge[3], dimesize[3], datatype, dsDatatype, rank;
    int32 globleattr, setattr, setdataatrr;

    //定义的 的数据类型和属性数据的类型
    uchar8 *uchar8_databuffer;
    char8 *char8_databuffer;
    //float64* float64_databuffer;

    float32 *float32_databuffer;
    float64 *float64_databuffer;

    int8 *int8_databuffer;
    uint8 *uint8_databuffer;

    int16 *int16_databuffer;
    uint16 *uint16_databuffer;

    int32 *int32_databuffer;
    uint32 *uint32_databuffer;

    int32 attr_index;
    int status;
    char filename[128];                              //读取modis数据的基本参数

    int32 datasetsnum, attrnum;                     //数据集的个数和全局属性的个数

    string m_FileName;
    string m_DataType;

private:
    //通过窗体设置文件参数类型和数据集类型
    string m_MarineParameterName;
    int m_DatesetsType;
    /*
    0：全球2维南北太平洋中间;
    1:全球2维南北太平洋分开;
    2:全球2维南北对调太平洋中间;
    3:全球2维南北对调太平洋分开;
    4:全球3维南北太平洋中间
    5：全球3维南北太平洋分开
    6：全球3维南北对调太平洋中间
    7：全球3维南北对调太平洋分开
    8：区域2维
    9：区域3维
    */
public:

    static Meta getHDFMeta(string templateFile);

    static bool meanAndStandardDeviation(vector<string> Files, double *pMeanBuffer, double *pStdBuffer);

    bool readHDF(string filename, long *buf);

    bool readHDF(string filename, int *buf);

    bool writeHDF(string Filename, Meta meta, long *pBuffer);

    bool writeHDF(string Filename, Meta meta, int *pBuffer);

    string GetFileDateTime(string Filename);               //读取文件日期
    string GetFileProductType(string Filename);  //读文件的产品类型 0:自定义产品 product；
    bool
    GetDsByDsnameFROMProduct(long *pBuffer, string Filename, string Dsname, long mStartRow, long mRows, long mStartCol,
                             long mCols);

    void *hdfOpt::allow_memory(long datasize, int datatype);

    long GetDatesetsNum(string Filename);

    double GetDataSetsScale(string Filename, string Dsname);     //读取数据集转换因子
    double GetDataSetsOffsets(string Filename, string Dsname);   //读取数据集的转换截距
    double GetDataSetsMissingValue(string Filename, string Dsname); //读取数据集的丢失值(FillValue)

    long GetDatasetsRows(string Filename, string Dsname);          //读取数据集行数
    long GetDatasetsCols(string Filename, string Dsname);          //读取数据集行数

    double GetDatasetsStartLog(string Filename, string Dsname);    //读取数据集的起始经度
    double GetDatasetsStartLat(string Filename, string Dsname);//读取数据集的起始纬度
    double GetDatasetsEndLog(string Filename, string Dsname);  //读取数据集的终止经度
    double GetDatasetsEndLat(string Filename, string Dsname);   //读取数据集的终止纬度

    double GetDatasetsSpatialResolution_New(string Filename, string Dsname);   //读取数据集的空间分辨率，新版本，分辨率为double型


    //写自定义文件（二维）
    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, long *pBuffer, double m_Scale, double m_Offset, double m_StartLog,
                              double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols,
                              double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2维");

    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, unsigned char *pBuffer, double m_Scale, double m_Offset,
                              double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows,
                              long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2维");

    BOOL
    WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType, string m_DsName,
                         int *pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog,
                         double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue,
                         double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,
                         double m_Resolution, string m_Dimension);

    long *getMaxandMinFromPRES(long *DepthData, long DataNum);

    static void
    quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData,
                     double *LatData, double mJuldFillValue);

    static void quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue);

    static void
    quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData, double mJuldFillValue);

};

enum AnomalyType {Positive, Negative};

class gdalOpt {
public:
    gdalOpt();

    gdalOpt(Meta meta);

    static bool readGeoTiff(const char *in_fileName, double *pTiffData);

    static bool readGeoTiff(const string file, int *pBuffer);

    static void save(string outputPath,string startTime, AnomalyType anomalyType, vector<Poly>& polygons);

    static void save(string outPath, string startTime, string abnormalType, const double startLog, const double startLat,
                     const double resolution, vector<Poly>& polygons);

    bool writeGeoTiff(string fileName, Meta meta, double *buf);

    bool writeGeoTiff(string fileName, Meta meta, int *buf);

    bool ReadFileByGDAL(const char *fileName); //读取 product HDF4文件
    bool WriteGeoTiffFileByGDAL(const char *fileName, double *pMemData);//将读入HDF4文件写为GeoTiff
    bool Convert_GeoTiff2HDF(const char *in_fileName, const char *out_fileName, double startLat = 0, double endLat = 0,
                             double startLog = 0, double endLog = 0);

    const char *GetProjection(const char *fileName);//获取GeoTiff投影信息

public:

    long rowOff; //row offset
    long colOff;

    long iRow;
    long iCol;

    GDALDriver *geoTiffDriver;
    GDALDataset *pDataSet;
    GDALDataset *geoTiffDataset;

    double startLog = 0.f;
    double startLat = 0.f;
    double endLog = 0.f;
    double endLat = 0.f;
    double mScale = 0.001; //转product HDF时 使用0.01即可
    double mOffsets = 0.f;

    double maxValue = 0.f;
    double minValue = 0.f;
    double meanValue = 0.f;
    double stdValue = 0.f;

    int Cols = 0;
    int Rows = 0;

    double fillValue = 0.f;
    double dsResolution = 1.f;

    string dataType = "";
    string productType = "";
    string dimension = "";
    string imgDate = "";
    string dsName = "";
    string projection = "";


    //long *pMemData;//用于存放数据
    double *pMemData;

    long *pGeoData;

};

#endif //CLUSTERING_OPT_H