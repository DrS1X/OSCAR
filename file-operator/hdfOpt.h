//
// Created by Administrator on 2022/12/4.
//

#ifndef CLUSTERING_HDFOPT_H
#define CLUSTERING_HDFOPT_H

#include <hdfi.h>
using std::string;

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
    0：全球2Dimesion南北太平洋中间;
    1:全球2Dimesion南北太平洋分开;
    2:全球2Dimesion南北对调太平洋中间;
    3:全球2Dimesion南北对调太平洋分开;
    4:全球3维南北太平洋中间
    5：全球3维南北太平洋分开
    6：全球3维南北对调太平洋中间
    7：全球3维南北对调太平洋分开
    8：区域2Dimesion
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
    bool GetDsByDsnameFROMProduct(long *pBuffer, string Filename, string Dsname, long mStartRow, long mRows, long mStartCol, long mCols);

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
    bool WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, long *pBuffer, double m_Scale, double m_Offset, double m_StartLog,
                              double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols,
                              double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2Dimesion");

    bool WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, unsigned char *pBuffer, double m_Scale, double m_Offset,
                              double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows,
                              long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                              long m_FillValue, double m_Resolution, string m_Dimension = "2Dimesion");

    bool WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType, string m_DsName,
                         int *pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog,
                         double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue,
                         double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,
                         double m_Resolution, string m_Dimension);

    long *getMaxandMinFromPRES(long *DepthData, long DataNum);

    static void quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData,
                     double *LatData, double mJuldFillValue);

    static void quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue);

    static void quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData, double mJuldFillValue);

};

#endif //CLUSTERING_HDFOPT_H
