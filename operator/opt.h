//
// Created by 15291 on 2022/9/24.
//

#ifndef CLUSTERING_OPT_H
#define CLUSTERING_OPT_H

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <hdfi.h>
#include <gdal_priv.h>
#include <gdal.h>
#include "_const.h"
//gdal_wrap gdalconst_wrap geos_c ogr_wrap osr_wrap hdf mfhdf)
class opt
{
public:
    static void checkFilePath(string filePath);
    static int getDayOfYear(string fileName);
    static string generateFileName(string originFileName, string outputPath, string pre, string type, string date);
    static string generateFileName(string originFileName, string outputPath, string pre, string type);
    static string generateFileName(string originFilePath, string outputPath, string suffix);
    static void getFileList(string path, vector<string>& files);
    static void getFileList(string path, vector<string>& files, string fileType);
    static string getDate(string fileName);

    static int GetMonthFromDays(int Days);
    static int GetYearFromDays(int Days);
    static int LeapYear(int Year);

    static double* GetMin_Max(double *value, long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);   //以数组同时返回最大最小值 索引0为最小值，1为最大值 默认缺省值-9999
    static double* GetMin_Max(double *value, double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载
    static double GetMeanValue(long * pBuffer,long m_Rows,long m_Cols, long DefaultValue = -9999);	 //默认缺省值-9999
    static double GetMeanValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载
    static double GetStdValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);  //默认缺省值-9999
    static double GetStdValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//重载

    static void split(std::string& s, std::string& delim, std::vector<std::string>* ret);
    static void addArray(long *arr1, long *arr2,int mRow,int mCol);//arr1+=arr2;
    static double STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue, long *pBuffer1, long *pBuffer2, long size);
    //long GetDsNum(CString Filename);
    static bool DataSpatialConvertByMean(long *pSrcBuffer,/*原数据集*/long *pTarBuffer,/*目标数据集*/long mSrcRows, long mSrcCols, long mTarRows, long mTarCols, double reSize/*转换尺寸*/);
    static double CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
    static double CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
    static double CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);

    static void SpatialResampleBasedOnUnevenLat(long *pOriBuffer,long *pTarBuffer,double *pOriLatBuffer,long mOriRows,long mOriCols,long mTarRows,long mTarCols,double startlog,double endlog,double startlat,double endlat);
};


class hdfOpt
{
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
    uchar8* uchar8_databuffer;
    char8* char8_databuffer;
    //float64* float64_databuffer;

    float32* float32_databuffer;
    float64* float64_databuffer;

    int8  *int8_databuffer;
    uint8 *uint8_databuffer;

    int16* int16_databuffer;
    uint16* uint16_databuffer;

    int32* int32_databuffer;
    uint32* uint32_databuffer;

    int32 attr_index;
    int status;
    char filename[128];	                          //读取modis数据的基本参数

    int32 datasetsnum, attrnum;                     //数据集的个数和全局属性的个数

    string m_FileName;
    string m_DataType;

private:
    //通过窗体设置文件参数类型和数据集类型
    string m_MarineParameterName;/*
								  SST 海面温度
								  SSP 海面降雨
								  SSS 海面盐度
								  SLA 海面高度
								  SSW 海表风场
								  CHL 叶绿素
								  NPP 海洋初级生产力
								  LOCAL
							  */
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
    static bool meanAndStandardDeviation(vector<string> Files, double* pMeanBuffer, double* pStdBuffer);
    bool readHDF(string filename, long* buf);
    bool readHDF(string filename, int* buf);
    bool writeHDF(string Filename, Meta meta, long* pBuffer);
    bool writeHDF(string Filename, Meta meta, int* pBuffer);

    string GetFileDateTime(string Filename);               //读取文件日期
    string GetFileProductType(string Filename);  //读文件的产品类型 0:自定义产品 product；
    bool GetDsByDsnameFROMProduct(long *pBuffer, string Filename, string Dsname, long mStartRow, long mRows, long mStartCol, long mCols);

    void* hdfOpt::allow_memory(long datasize, int datatype);

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
                              string m_DsName, long* pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,double m_Resolution, string m_Dimension="2维");

    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, unsigned char * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, string m_Dimension = "2维");

    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType, string m_DsName, int * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, string m_Dimension);

    long* getMaxandMinFromPRES(long *DepthData, long DataNum);
    static void quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData, double *LatData, double mJuldFillValue);
    static void quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue);
    static void quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData, double mJuldFillValue);

};


class gdalOpt
{
public:
    gdalOpt();
    gdalOpt(Meta meta);
    static bool readGeoTiff(const char * in_fileName, double * pTiffData);
    static bool readGeoTiff(const string file, int* pBuffer);
    bool writeGeoTiff(string fileName, Meta meta, double *buf);
    bool writeGeoTiff(string fileName, Meta meta, int *buf);
    bool ReadFileByGDAL(const char* fileName); //读取 product HDF4文件
    bool WriteGeoTiffFileByGDAL(const char* fileName, double *pMemData);//将读入HDF4文件写为GeoTiff
    bool Convert_GeoTiff2HDF(const char * in_fileName, const char * out_fileName, double startLat = 0, double endLat = 0, double startLog = 0, double endLog = 0);

    const char* GetProjection(const char* fileName);//获取GeoTiff投影信息
    //argo
    //bool WriteVectorLineByGDAL(double X, double Y);
    //bool WriteVectorLineSHPByGDAL(long FID, QLineF Line);

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

    double  fillValue = 0.f;
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
