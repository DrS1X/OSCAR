//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_OPT_H
#define CLUSTERING_OPT_H

#include <iostream>
#include <math.h>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <hdfi.h>
#include <gdal_priv.h>
#include <gdal.h>
#include "_const.h"
//gdal_wrap gdalconst_wrap geos_c ogr_wrap osr_wrap hdf mfhdf)

using namespace std;

class Range {
public:
    int rowMin , rowMax, colMin , colMax ;
public:
    Range(){
        rowMin = INT_MAX;
        rowMax = 0;
        colMin = INT_MAX;
        colMax = 0;
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

    pair<double, double> getGeoCoordinates(){
        pair<double, double> pr;
        return pr;
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
    int id;//Ψһ��ʶ
    int type;//�������
    int row;//����
    int col;//ά��
    string dir1;//����1
    string dir2;//����2
    string outDir;//�߽��뷽��
    bool isUsed;//�Ƿ�ʹ��
    int power;//ǿ��
    int eventID;
public:
    Node() {}

    Node(int _nodeType, int _row, int _col) :
            type(_nodeType),
            row(_row),
            col(_col) {
        isUsed = false;

        switch (type) {
            case NodeType_LeftTop:
                dir1 = "l";//����
                dir2 = "t";//�Ϸ���
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

class Matrix{
private:
    vector<vector<Node *>> mat;
    int rowOffset, rowLen, colOffset, colLen;
public:
    Matrix(Range& range, vector<Node>& nodeList){
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
        print();
    }

    void print(){
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

    Node* get(int row, int col){
        return mat[row + 1][col + 1];
    }
};

class Line {
public:
    int id;//Ψһ��ʶ
    vector<Node> nodes;//���е�
    int minRow;//��С����
    int minCol;//��С����
    int maxRow;//�������
    int maxCol;//�������
    Range range;
    int type;//�����ͣ�0Ϊ�⻷��1Ϊ�ڻ�
    int power;//ǿ��
    int eventID;//�¼�id
    double length;//����

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

public:
    Line(){}

    Line(const Range& _range, const vector<Node>& nodeList){
        minRow = _range.rowMin;
        maxRow = _range.rowMax;
        minCol = _range.colMin;
        maxCol = _range.colMax;
        range = _range;

        nodes = nodeList;
    }

    bool sortNodes(){
        int rowOffset = range.rowMin;
        int rowLen = range.rowMax - range.rowMin + 1;
        int colOffset = range.colMin;
        int colLen = range.colMax - range.colMin + 1;
        Node *head = nullptr;

        // build mat of region
        for (int i = 0; i < nodes.size(); ++i) {
            if (head == nullptr && nodes[i].type != NodeType_LeftTopAndRightBot &&
                    nodes[i].type != NodeType_RightTopAndLeftBot) {
                head = &nodes[i];
                break;
            }
        }

        Matrix mat(range, nodes);

        // search the line of edge
        Node *cur = head;
        string direction = cur->dir1;
        int r = cur->row - rowOffset, c = cur->col - colOffset;
        vector<Node> newNodeList;
        newNodeList.push_back(*head);
        int cnt = 0;
        do {
            r += RowMove.at(direction);
            c += ColMove.at(direction);
            if(r < -1 || c < -1 || r >= rowLen || c >= colLen){
                cout << "error: [sortNodes] out of range." << endl;
                return false;
            }

            if (mat.get(r, c) == nullptr)
                continue;

            cur = mat.get(r, c);
            if(cur->isUsed){
                newNodeList.push_back(*cur);
                break;
            }

            if(cur->type != NodeType_LeftTopAndRightBot && cur->type != NodeType_RightTopAndLeftBot) {
                cur->isUsed = true;
                newNodeList.push_back(*cur);
            }
            direction = cur->getNextDirection(direction);
            cnt++;
        } while (head != cur || cnt >= nodes.size());

        nodes = newNodeList;
        Matrix tmp(range, nodes); // just view the matrix of nodes
        return true;
    }

};

class myCircle {
public:
    double x;//Բ��x����
    double y;//Բ��y����
    double r;//Բ�뾶

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

    double length;//���γ���
    double width;//���ο��

    myRectangle(double m_minX, double m_maxX, double m_minY, double m_maxY, double m_area) {
        minX = m_minX;
        maxX = m_maxX;
        minY = m_minY;
        maxY = m_maxY;
        area = m_area;

        length = m_maxX - m_minX;
        width = m_maxY - m_minY;
        if (width > length) {//������
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

class POL {
public:
    int id;//Ψһ��ʶ
    int eventID;//�¼�id
    vector<Line> lines;//������
    int minRow;//��С����
    int minCol;//��С����
    int maxRow;//�������
    int maxCol;//�������
    double area;//ʵ�����
    double avgValue;//ƽ����ƽֵ
    double volume;//��ƽֵ���
    double maxValue;//����ƽֵ
    double minValue;//��С��ƽֵ
    int power;//ǿ��
    bool isMulti;//�Ƿ��Ƕ���
    double length;//�ܳ�
    double coreRow;//�����к�
    double coreCol;//�����к�
    myRectangle minRec;//��С����������
    myCircle minOutCir;//��С������Բ
    myCircle maxInCir;//�������ڽ�Բ
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
                              long DefaultValue = -9999);   //������ͬʱ���������Сֵ ����0Ϊ��Сֵ��1Ϊ���ֵ Ĭ��ȱʡֵ-9999
    static double *
    GetMin_Max(double *value, double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
    static double GetMeanValue(long *pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);     //Ĭ��ȱʡֵ-9999
    static double GetMeanValue(double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
    static double GetStdValue(long *pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);  //Ĭ��ȱʡֵ-9999
    static double GetStdValue(double *pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����

    static void split(std::string &s, std::string &delim, std::vector<std::string> *ret);

    static void addArray(long *arr1, long *arr2, int mRow, int mCol);//arr1+=arr2;
    static double
    STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue,
                  long *pBuffer1, long *pBuffer2, long size);

    //long GetDsNum(CString Filename);
    static bool
    DataSpatialConvertByMean(long *pSrcBuffer,/*ԭ���ݼ�*/long *pTarBuffer,/*Ŀ�����ݼ�*/long mSrcRows, long mSrcCols,
                             long mTarRows, long mTarCols, double reSize/*ת���ߴ�*/);

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

    uint16 pmax, pmin;               //��ȡ���������ݵ���ֵ
    double MaxValue, MinValue;

    float32 vSlope[1], vIntercept[1];         //��ͼ������ת�����������ݵĲ���
    double vCoordinate[4];                                   //����4���ǵ�����

    int row, cols;                   //ͼ�� ��������

    int32 start[3], endge[3], dimesize[3], datatype, dsDatatype, rank;
    int32 globleattr, setattr, setdataatrr;

    //����� ���������ͺ��������ݵ�����
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
    char filename[128];                              //��ȡmodis���ݵĻ�������

    int32 datasetsnum, attrnum;                     //���ݼ��ĸ�����ȫ�����Եĸ���

    string m_FileName;
    string m_DataType;

private:
    //ͨ�����������ļ��������ͺ����ݼ�����
    string m_MarineParameterName;
    int m_DatesetsType;
    /*
    0��ȫ��2ά�ϱ�̫ƽ���м�;
    1:ȫ��2ά�ϱ�̫ƽ��ֿ�;
    2:ȫ��2ά�ϱ��Ե�̫ƽ���м�;
    3:ȫ��2ά�ϱ��Ե�̫ƽ��ֿ�;
    4:ȫ��3ά�ϱ�̫ƽ���м�
    5��ȫ��3ά�ϱ�̫ƽ��ֿ�
    6��ȫ��3ά�ϱ��Ե�̫ƽ���м�
    7��ȫ��3ά�ϱ��Ե�̫ƽ��ֿ�
    8������2ά
    9������3ά
    */
public:

    static Meta getHDFMeta(string templateFile);

    static bool meanAndStandardDeviation(vector<string> Files, double *pMeanBuffer, double *pStdBuffer);

    bool readHDF(string filename, long *buf);

    bool readHDF(string filename, int *buf);

    bool writeHDF(string Filename, Meta meta, long *pBuffer);

    bool writeHDF(string Filename, Meta meta, int *pBuffer);

    string GetFileDateTime(string Filename);               //��ȡ�ļ�����
    string GetFileProductType(string Filename);  //���ļ��Ĳ�Ʒ���� 0:�Զ����Ʒ product��
    bool
    GetDsByDsnameFROMProduct(long *pBuffer, string Filename, string Dsname, long mStartRow, long mRows, long mStartCol,
                             long mCols);

    void *hdfOpt::allow_memory(long datasize, int datatype);

    long GetDatesetsNum(string Filename);

    double GetDataSetsScale(string Filename, string Dsname);     //��ȡ���ݼ�ת������
    double GetDataSetsOffsets(string Filename, string Dsname);   //��ȡ���ݼ���ת���ؾ�
    double GetDataSetsMissingValue(string Filename, string Dsname); //��ȡ���ݼ��Ķ�ʧֵ(FillValue)

    long GetDatasetsRows(string Filename, string Dsname);          //��ȡ���ݼ�����
    long GetDatasetsCols(string Filename, string Dsname);          //��ȡ���ݼ�����

    double GetDatasetsStartLog(string Filename, string Dsname);    //��ȡ���ݼ�����ʼ����
    double GetDatasetsStartLat(string Filename, string Dsname);//��ȡ���ݼ�����ʼγ��
    double GetDatasetsEndLog(string Filename, string Dsname);  //��ȡ���ݼ�����ֹ����
    double GetDatasetsEndLat(string Filename, string Dsname);   //��ȡ���ݼ�����ֹγ��

    double GetDatasetsSpatialResolution_New(string Filename, string Dsname);   //��ȡ���ݼ��Ŀռ�ֱ��ʣ��°汾���ֱ���Ϊdouble��


    //д�Զ����ļ�����ά��
    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, long *pBuffer, double m_Scale, double m_Offset, double m_StartLog,
                              double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols,
                              double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2ά");

    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, unsigned char *pBuffer, double m_Scale, double m_Offset,
                              double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows,
                              long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2ά");

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

class gdalOpt {
public:
    gdalOpt();

    gdalOpt(Meta meta);

    static bool readGeoTiff(const char *in_fileName, double *pTiffData);

    static bool readGeoTiff(const string file, int *pBuffer);

    static void
    save(string outPath, string startTime, string AbnormalType, const double startLog, const double startLat,
         const double resolution, vector<POL> polygons);

    bool writeGeoTiff(string fileName, Meta meta, double *buf);

    bool writeGeoTiff(string fileName, Meta meta, int *buf);

    bool ReadFileByGDAL(const char *fileName); //��ȡ product HDF4�ļ�
    bool WriteGeoTiffFileByGDAL(const char *fileName, double *pMemData);//������HDF4�ļ�дΪGeoTiff
    bool Convert_GeoTiff2HDF(const char *in_fileName, const char *out_fileName, double startLat = 0, double endLat = 0,
                             double startLog = 0, double endLog = 0);

    const char *GetProjection(const char *fileName);//��ȡGeoTiffͶӰ��Ϣ

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
    double mScale = 0.001; //תproduct HDFʱ ʹ��0.01����
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


    //long *pMemData;//���ڴ������
    double *pMemData;

    long *pGeoData;


};

#endif //CLUSTERING_OPT_H
