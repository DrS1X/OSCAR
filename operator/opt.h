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

    static double* GetMin_Max(double *value, long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);   //������ͬʱ���������Сֵ ����0Ϊ��Сֵ��1Ϊ���ֵ Ĭ��ȱʡֵ-9999
    static double* GetMin_Max(double *value, double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
    static double GetMeanValue(long * pBuffer,long m_Rows,long m_Cols, long DefaultValue = -9999);	 //Ĭ��ȱʡֵ-9999
    static double GetMeanValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
    static double GetStdValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);  //Ĭ��ȱʡֵ-9999
    static double GetStdValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����

    static void split(std::string& s, std::string& delim, std::vector<std::string>* ret);
    static void addArray(long *arr1, long *arr2,int mRow,int mCol);//arr1+=arr2;
    static double STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue, long *pBuffer1, long *pBuffer2, long size);
    //long GetDsNum(CString Filename);
    static bool DataSpatialConvertByMean(long *pSrcBuffer,/*ԭ���ݼ�*/long *pTarBuffer,/*Ŀ�����ݼ�*/long mSrcRows, long mSrcCols, long mTarRows, long mTarCols, double reSize/*ת���ߴ�*/);
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

    uint16 pmax, pmin;               //��ȡ���������ݵ���ֵ
    double MaxValue, MinValue;

    float32 vSlope[1], vIntercept[1];         //��ͼ������ת�����������ݵĲ���
    double vCoordinate[4];                                   //����4���ǵ�����

    int row, cols;                   //ͼ�� ��������

    int32 start[3], endge[3], dimesize[3], datatype, dsDatatype, rank;
    int32 globleattr, setattr, setdataatrr;

    //����� ���������ͺ��������ݵ�����
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
    char filename[128];	                          //��ȡmodis���ݵĻ�������

    int32 datasetsnum, attrnum;                     //���ݼ��ĸ�����ȫ�����Եĸ���

    string m_FileName;
    string m_DataType;

private:
    //ͨ�����������ļ��������ͺ����ݼ�����
    string m_MarineParameterName;/*
								  SST �����¶�
								  SSP ���潵��
								  SSS �����ζ�
								  SLA ����߶�
								  SSW ����糡
								  CHL Ҷ����
								  NPP �������������
								  LOCAL
							  */
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
    static bool meanAndStandardDeviation(vector<string> Files, double* pMeanBuffer, double* pStdBuffer);
    bool readHDF(string filename, long* buf);
    bool readHDF(string filename, int* buf);
    bool writeHDF(string Filename, Meta meta, long* pBuffer);
    bool writeHDF(string Filename, Meta meta, int* pBuffer);

    string GetFileDateTime(string Filename);               //��ȡ�ļ�����
    string GetFileProductType(string Filename);  //���ļ��Ĳ�Ʒ���� 0:�Զ����Ʒ product��
    bool GetDsByDsnameFROMProduct(long *pBuffer, string Filename, string Dsname, long mStartRow, long mRows, long mStartCol, long mCols);

    void* hdfOpt::allow_memory(long datasize, int datatype);

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
                              string m_DsName, long* pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,double m_Resolution, string m_Dimension="2ά");

    BOOL WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                              string m_DsName, unsigned char * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, string m_Dimension = "2ά");

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
    bool ReadFileByGDAL(const char* fileName); //��ȡ product HDF4�ļ�
    bool WriteGeoTiffFileByGDAL(const char* fileName, double *pMemData);//������HDF4�ļ�дΪGeoTiff
    bool Convert_GeoTiff2HDF(const char * in_fileName, const char * out_fileName, double startLat = 0, double endLat = 0, double startLog = 0, double endLog = 0);

    const char* GetProjection(const char* fileName);//��ȡGeoTiffͶӰ��Ϣ
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
    double mScale = 0.001; //תproduct HDFʱ ʹ��0.01����
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


    //long *pMemData;//���ڴ������
    double *pMemData;

    long *pGeoData;


};


#endif //CLUSTERING_OPT_H
