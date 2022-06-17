/*
Author: 伍程斌 wcb892534877@icloud.com
Description:利用HDF4库函数，处理HDF4数据格式的遥感数据和自定义的遥感数据集
Date: 2018-03
*/

#include "hdf.h"
#include "dfsd.h"
#include "mfhdf.h"
#include "atlstr.h"
#include <QString>
#include "qfile.h"
#include <QTextStream>
#include "CONST.h"

#pragma once


class CClsHDF4Operator
{
public:
	CClsHDF4Operator();
	CClsHDF4Operator(Meta meta);
	CClsHDF4Operator(CString, int);
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

	CString m_FileName;
	CString m_DataType;

private:
	//通过窗体设置文件参数类型和数据集类型
	CString m_MarineParameterName;/*
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

	BOOL SetMarineParameterName(CString m_Parameter);
	BOOL SetDatasetsType(int m_Type);

//判断
	bool isModis1B(CString Filename);
	bool isProduct(CString Filename);
	bool isProductOld(CString Filename);
	void* allow_memory(long datasize, int datatype);
//读取原始数据函数
	bool GetFileAttList(CString Filename, CString *mAttList);  //读取文件全局属性列表
	bool GetDataSetAttList(CString File, CString DsName, CString *mAttList);//读取数据集下的属性列表
	long GetDataSetAttNum(CString File, CString Dsnmae);
	CString GetFileAttr(CString Filename, CString AttrName); //读取文件属性
	CString GetDataSetAttr(CString Filename, CString DsName, CString AttrName);//读取数据集属性
	long GetFileAttNum(CString Filename);                    //读取文件属性个数
	CString GetFileDateTime(CString Filename);               //读取文件日期
	CString GetChlStartTime(CString Filename);				 //读取叶绿素文件起始时间
	CString GetFileProductType(CString Filename);  //读文件的产品类型 0:自定义产品 product；
	CString GetFileProductTypeOld(CString Filename);
	bool GetDatasetsList(CString Filename, CString *mDsList);  //读取数据集列表
	CString GetDatasetsNameByIndex(CString Filename, int dsIndex); //读取数据集名称	
	bool GetDsByDsnameFROMProduct(long *pBuffer, CString Filename, CString Dsname, long mStartRow, long mRows, long mStartCol, long mCols);
	
	bool GetDsByDsnameFROMProduct(float *pBuffer, CString Filename, CString Dsname, long mStartRow, long mRows, long mStartCol, long mCols);
	bool GetDsByDsnameFROMProduct(double * pBuffer, CString Filename, CString Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols);

	int GetDsByDsname(void *pBuffer, CString Filename, CString Dsname, long &out_Rows, long &out_Cols);//读取所有hdf的数据，返回数据类型
	
	long GetDatesetsNum(CString Filename);                         //读取数据集个数

	double GetDataSetsScale(CString Filename, CString Dsname);     //读取数据集转换因子
	double GetDataSetsOffsets(CString Filename, CString Dsname);   //读取数据集的转换截距
	double GetDataSetsMissingValue(CString Filename, CString Dsname); //读取数据集的丢失值(FillValue)

	long GetDatasetsRows(CString Filename, CString Dsname);          //读取数据集行数
	long GetDatasetsCols(CString Filename, CString Dsname);          //读取数据集行数

	double GetDatasetsStartLog(CString Filename, CString Dsname);    //读取数据集的起始经度
	double GetDatasetsStartLat(CString Filename, CString Dsname);//读取数据集的起始纬度
	double GetDatasetsEndLog(CString Filename, CString Dsname);  //读取数据集的终止经度
	double GetDatasetsEndLat(CString Filename, CString Dsname);   //读取数据集的终止纬度

	double GetDatasetsSpatialResolution_Old(CString Filename, CString Dsname);   //读取数据集的空间分辨率,老版本，分辨率为字符型
	double GetDatasetsSpatialResolution_New(CString Filename, CString Dsname);   //读取数据集的空间分辨率，新版本，分辨率为double型

	long* GetDataset(CString Filename, CString Dsname, int Dstype);//读取数据集
		
	//写自定义文件（二维）
	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType,
		CString m_DsName, long* pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,double m_Resolution, CString m_Dimension=_T("2维"));
	
	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType,
		CString m_DsName, unsigned char * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension = _T("2维"));

	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType, CString m_DsName, int * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension);
	
	//写自定义文件（一维）
	BOOL WriteCustomHDF1DFile(CString Filename, double  m_Log,double m_Lat, CString m_ProductType, CString m_DataType, CString m_Dimension,
		CString m_DsName, long* pBuffer, double m_Scale, double m_Offset, CString m_StartTime, CString m_EndTime,long m_Number, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution);
	
	//写自定义文件（一维）,多个数据集
	BOOL WriteCustomHDF1DFile(CString  Filename, double m_Log, double m_Lat, CString  m_ProductType, CString  m_DataType,
		CString  m_Dimension, CString  *m_DsName, long **pBuffer, double *m_Scale, double m_Offset,
		CString  m_StartTime, CString  m_EndTime, long m_Number, double *m_MaxValue, double *m_MinValue,
		double *m_MeanValue, double *m_StdValue, long m_FillValue, double *m_Resolution, long count);


	//bool WriteHDFFile(CString Filename, CString m_DsName, CString m_ImageDate, CString m_Resolution, long* pBuffer, double m_StartLat, double m_EndLat, double m_StartLog, double m_EndLog, long m_Rows, long m_Cols, double m_Scale)
	//读取自定义文件
	CString GetCustomDSDate(CString Filename);  //读取数据集的日期
	char* GetCustomDSDataType(char* Filename);  //读取数据集的数据类型
	char* GetCustomDSDimension(char* Filename);  //读取数据集的维度
	double GetCustomDSLocaxtionLog(char* Filename);  //读取数据集的位置（经度）
	double GetCustomDSLocaxtionLat(char* Filename);  //读取数据集的位置（纬度）
	char* GetCustomDSName(char* Filename);  //读取数据集的名称


	long *GetCustom2DDataSets(char* Filename, char* m_DsName);   //读取二维数据集
	long *GetCustom1DDataSets(char* Filename, char* m_DsName);   //读取一维数据集
	long *GetCustom2DSubDataSets(char* Filename, char* m_DsName, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat); //根据输入经纬度读取二维数据集
	long *GetCustom1DSubDataSets(char* Filename, char* m_DsName, char* m_StartTime, char* m_EndTime);//根据输入时间读取一维数据集

	double GetCustomDSScale(CString Filename, CString Dsname);  //读取数据集的比例系数
	double GetCustomDSOffset(CString Filename, CString Dsname);  //读取数据集的截距

	double GetCustomDSStartLog(CString Filename, CString Dsname);  //读取数据集的起始经度
	double GetCustomDSEndLog(CString Filename, CString Dsname);  //读取数据集的终止经度
	double GetCustomDSStartLat(CString Filename, CString Dsname);  //读取数据集的起始纬度
	double GetCustomDSEndLat(CString Filename, CString Dsname);  //读取数据集的终止纬度
	char* GetCustomDSStartTime(char* Filename, char* m_DsName);  //读取数据集的起始时间
	char* GetCustomDSEndTime(char* Filename, char* m_DsName);  //读取数据集的终止时间

	long GetCustomDSRows(CString Filename, CString Dsname);  //读取数据集的行数
	long GetCustomDSCols(CString Filename, CString Dsname);  //读取数据集的列数
	long GetCustomDSNumbers(char* Filename, char* m_DsName);  //读取数据集的个数

	double GetCustomDSMaxValue(CString Filename, CString m_DsName);  //读取数据集的最大值
	double GetCustomDSMinValue(char* Filename, char* m_DsName);  //读取数据集的最小值
	double GetCustomDSMeanValue(char* Filename, char* m_DsName);  //读取数据集的均值
	double GetCustomDSStdValue(char* Filename, char* m_DsName);  //读取数据集的方差
	double GetCustomDSFillValue(CString Filename, CString Dsname);  //读取数据集的填充值

	char* GetCustomDSSpaceResolution(char* Filename, char* m_DsName);  //读取数据集的空间分辨率
	char* GetCustomDSTimeResolution(char* Filename, char* m_DsName);  //读取数据集的时间分辨率

	//添加部分
	void SpaceNearAnalysis(double *pBuffer, long *pTBuffer, int NearIndex, int WayIndex, long Rows, long Cols, double mFillValue, double mScale);
	void TimeNeighbourhoodAnalysis(long *pBuffer, long **temptime, int NearIndex, int WayIndex, long row, long col, double mFillValue);
	void OnTimeOrder(QStringList FileList);
	void SpaceTimeNearAnalysis(long *pBuffer, long **temptime, int TimeNearIndex, int NearIndex, int WayIndex, long mFillValue, long row, long col,double mScale);
	
	void GetDataFromSite(QString FileName, double **pBuffer, long Rows, long Cols);//读取站点数据，以及行列信息，列为13列
	long GetDataRowsFromSite(QString FileName);//获得文本文件的行数
	long GetDataColsFromSite(QString FileName);//获得文本文件的列数
	QString *GetDataSetName(QString FileName, long Cols);
	double *GetDataSetTimeResolutionFromSite(QString FileName, long Cols);//获得时间分辨率
	double *GetDataSetScaleFromSite(QString FileName, long Cols);//获得数据单位scale

	void DisCretization_One(double *startdata, double *enddata, long Cols, int WayIndex, int Number, int StdIndex);
	void DisCretization_Two(double *startdata, double *enddata, long FileNum, double mFillValue, int WayIndex, int Number, int StdIndex);
	void SpaceDisCretization(long *pBuffer, double *pResultBuffer, long Rows, long Cols, double mFillValue, int WayIndex, int Number, int StdIndex);

	//
	void GetNakeDataFromTXT(QString FileName, double **pBuffer, long Rows, long Cols);
	long GetDataRowsFromTXT(QString FileName);
	long GetDataColsFromTXT(QString FileName);
	bool WriteDataToCSV(CString FileName, double *pBuffer, long mRows, long mCols);

	//Argo
	bool WriteHDFFileFromCSV(CString FileName, CString mDsName, long *pBuffer, long mRows, long mCols, QString mDsDate, double mSpatialRes);
	QStringList getFoderNamesFromFolder(QString path);
	QStringList getFileNamesFromFolder(QString path);
	long getCycleDepthData(long CycleID, long *CycleIndex, long *DepthData, long mDepthIndex,long mRows, long mCols, long *mStartID, long *mEndID, double mFillValue);
	bool getCycleLonLat(long *CycleIndex, long CycleID, double *LonData, double *LatData, long mStartID, long mEndID, double mFillValue, long mRows, long mCols, double *LatStart, double *LatEnd, double *LongStart, double *LongEnd);
	bool getCycleYMDandDurTime(long *CycleIndex, long CycleID, double *TimeData, long mStartID, long mEndID, double mFillValue, long mRows, long mCols, long *Year, long *Month, long *Day, double *DurTime);
	bool getCycleTempandPasl(double *temp, double *psal, double *ProfDepth, long Depth, double mProfRows, double mProfCols, double *Temp, double *Salinity, double mProfFillValue);
	long* getMaxandMinFromPRES(long *DepthData, long DataNum);
	static void quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData, double *LatData, double mJuldFillValue);
	static void quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue);
	static void quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData, double mJuldFillValue);
};

