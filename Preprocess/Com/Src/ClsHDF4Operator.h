/*
Author: ��̱� wcb892534877@icloud.com
Description:����HDF4�⺯��������HDF4���ݸ�ʽ��ң�����ݺ��Զ����ң�����ݼ�
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

	CString m_FileName;
	CString m_DataType;

private:
	//ͨ�����������ļ��������ͺ����ݼ�����
	CString m_MarineParameterName;/*
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

	BOOL SetMarineParameterName(CString m_Parameter);
	BOOL SetDatasetsType(int m_Type);

//�ж�
	bool isModis1B(CString Filename);
	bool isProduct(CString Filename);
	bool isProductOld(CString Filename);
	void* allow_memory(long datasize, int datatype);
//��ȡԭʼ���ݺ���
	bool GetFileAttList(CString Filename, CString *mAttList);  //��ȡ�ļ�ȫ�������б�
	bool GetDataSetAttList(CString File, CString DsName, CString *mAttList);//��ȡ���ݼ��µ������б�
	long GetDataSetAttNum(CString File, CString Dsnmae);
	CString GetFileAttr(CString Filename, CString AttrName); //��ȡ�ļ�����
	CString GetDataSetAttr(CString Filename, CString DsName, CString AttrName);//��ȡ���ݼ�����
	long GetFileAttNum(CString Filename);                    //��ȡ�ļ����Ը���
	CString GetFileDateTime(CString Filename);               //��ȡ�ļ�����
	CString GetChlStartTime(CString Filename);				 //��ȡҶ�����ļ���ʼʱ��
	CString GetFileProductType(CString Filename);  //���ļ��Ĳ�Ʒ���� 0:�Զ����Ʒ product��
	CString GetFileProductTypeOld(CString Filename);
	bool GetDatasetsList(CString Filename, CString *mDsList);  //��ȡ���ݼ��б�
	CString GetDatasetsNameByIndex(CString Filename, int dsIndex); //��ȡ���ݼ�����	
	bool GetDsByDsnameFROMProduct(long *pBuffer, CString Filename, CString Dsname, long mStartRow, long mRows, long mStartCol, long mCols);
	
	bool GetDsByDsnameFROMProduct(float *pBuffer, CString Filename, CString Dsname, long mStartRow, long mRows, long mStartCol, long mCols);
	bool GetDsByDsnameFROMProduct(double * pBuffer, CString Filename, CString Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols);

	int GetDsByDsname(void *pBuffer, CString Filename, CString Dsname, long &out_Rows, long &out_Cols);//��ȡ����hdf�����ݣ�������������
	
	long GetDatesetsNum(CString Filename);                         //��ȡ���ݼ�����

	double GetDataSetsScale(CString Filename, CString Dsname);     //��ȡ���ݼ�ת������
	double GetDataSetsOffsets(CString Filename, CString Dsname);   //��ȡ���ݼ���ת���ؾ�
	double GetDataSetsMissingValue(CString Filename, CString Dsname); //��ȡ���ݼ��Ķ�ʧֵ(FillValue)

	long GetDatasetsRows(CString Filename, CString Dsname);          //��ȡ���ݼ�����
	long GetDatasetsCols(CString Filename, CString Dsname);          //��ȡ���ݼ�����

	double GetDatasetsStartLog(CString Filename, CString Dsname);    //��ȡ���ݼ�����ʼ����
	double GetDatasetsStartLat(CString Filename, CString Dsname);//��ȡ���ݼ�����ʼγ��
	double GetDatasetsEndLog(CString Filename, CString Dsname);  //��ȡ���ݼ�����ֹ����
	double GetDatasetsEndLat(CString Filename, CString Dsname);   //��ȡ���ݼ�����ֹγ��

	double GetDatasetsSpatialResolution_Old(CString Filename, CString Dsname);   //��ȡ���ݼ��Ŀռ�ֱ���,�ϰ汾���ֱ���Ϊ�ַ���
	double GetDatasetsSpatialResolution_New(CString Filename, CString Dsname);   //��ȡ���ݼ��Ŀռ�ֱ��ʣ��°汾���ֱ���Ϊdouble��

	long* GetDataset(CString Filename, CString Dsname, int Dstype);//��ȡ���ݼ�
		
	//д�Զ����ļ�����ά��
	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType,
		CString m_DsName, long* pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue,double m_Resolution, CString m_Dimension=_T("2ά"));
	
	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType,
		CString m_DsName, unsigned char * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension = _T("2ά"));

	BOOL WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType, CString m_DsName, int * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension);
	
	//д�Զ����ļ���һά��
	BOOL WriteCustomHDF1DFile(CString Filename, double  m_Log,double m_Lat, CString m_ProductType, CString m_DataType, CString m_Dimension,
		CString m_DsName, long* pBuffer, double m_Scale, double m_Offset, CString m_StartTime, CString m_EndTime,long m_Number, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution);
	
	//д�Զ����ļ���һά��,������ݼ�
	BOOL WriteCustomHDF1DFile(CString  Filename, double m_Log, double m_Lat, CString  m_ProductType, CString  m_DataType,
		CString  m_Dimension, CString  *m_DsName, long **pBuffer, double *m_Scale, double m_Offset,
		CString  m_StartTime, CString  m_EndTime, long m_Number, double *m_MaxValue, double *m_MinValue,
		double *m_MeanValue, double *m_StdValue, long m_FillValue, double *m_Resolution, long count);


	//bool WriteHDFFile(CString Filename, CString m_DsName, CString m_ImageDate, CString m_Resolution, long* pBuffer, double m_StartLat, double m_EndLat, double m_StartLog, double m_EndLog, long m_Rows, long m_Cols, double m_Scale)
	//��ȡ�Զ����ļ�
	CString GetCustomDSDate(CString Filename);  //��ȡ���ݼ�������
	char* GetCustomDSDataType(char* Filename);  //��ȡ���ݼ�����������
	char* GetCustomDSDimension(char* Filename);  //��ȡ���ݼ���ά��
	double GetCustomDSLocaxtionLog(char* Filename);  //��ȡ���ݼ���λ�ã����ȣ�
	double GetCustomDSLocaxtionLat(char* Filename);  //��ȡ���ݼ���λ�ã�γ�ȣ�
	char* GetCustomDSName(char* Filename);  //��ȡ���ݼ�������


	long *GetCustom2DDataSets(char* Filename, char* m_DsName);   //��ȡ��ά���ݼ�
	long *GetCustom1DDataSets(char* Filename, char* m_DsName);   //��ȡһά���ݼ�
	long *GetCustom2DSubDataSets(char* Filename, char* m_DsName, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat); //�������뾭γ�ȶ�ȡ��ά���ݼ�
	long *GetCustom1DSubDataSets(char* Filename, char* m_DsName, char* m_StartTime, char* m_EndTime);//��������ʱ���ȡһά���ݼ�

	double GetCustomDSScale(CString Filename, CString Dsname);  //��ȡ���ݼ��ı���ϵ��
	double GetCustomDSOffset(CString Filename, CString Dsname);  //��ȡ���ݼ��Ľؾ�

	double GetCustomDSStartLog(CString Filename, CString Dsname);  //��ȡ���ݼ�����ʼ����
	double GetCustomDSEndLog(CString Filename, CString Dsname);  //��ȡ���ݼ�����ֹ����
	double GetCustomDSStartLat(CString Filename, CString Dsname);  //��ȡ���ݼ�����ʼγ��
	double GetCustomDSEndLat(CString Filename, CString Dsname);  //��ȡ���ݼ�����ֹγ��
	char* GetCustomDSStartTime(char* Filename, char* m_DsName);  //��ȡ���ݼ�����ʼʱ��
	char* GetCustomDSEndTime(char* Filename, char* m_DsName);  //��ȡ���ݼ�����ֹʱ��

	long GetCustomDSRows(CString Filename, CString Dsname);  //��ȡ���ݼ�������
	long GetCustomDSCols(CString Filename, CString Dsname);  //��ȡ���ݼ�������
	long GetCustomDSNumbers(char* Filename, char* m_DsName);  //��ȡ���ݼ��ĸ���

	double GetCustomDSMaxValue(CString Filename, CString m_DsName);  //��ȡ���ݼ������ֵ
	double GetCustomDSMinValue(char* Filename, char* m_DsName);  //��ȡ���ݼ�����Сֵ
	double GetCustomDSMeanValue(char* Filename, char* m_DsName);  //��ȡ���ݼ��ľ�ֵ
	double GetCustomDSStdValue(char* Filename, char* m_DsName);  //��ȡ���ݼ��ķ���
	double GetCustomDSFillValue(CString Filename, CString Dsname);  //��ȡ���ݼ������ֵ

	char* GetCustomDSSpaceResolution(char* Filename, char* m_DsName);  //��ȡ���ݼ��Ŀռ�ֱ���
	char* GetCustomDSTimeResolution(char* Filename, char* m_DsName);  //��ȡ���ݼ���ʱ��ֱ���

	//��Ӳ���
	void SpaceNearAnalysis(double *pBuffer, long *pTBuffer, int NearIndex, int WayIndex, long Rows, long Cols, double mFillValue, double mScale);
	void TimeNeighbourhoodAnalysis(long *pBuffer, long **temptime, int NearIndex, int WayIndex, long row, long col, double mFillValue);
	void OnTimeOrder(QStringList FileList);
	void SpaceTimeNearAnalysis(long *pBuffer, long **temptime, int TimeNearIndex, int NearIndex, int WayIndex, long mFillValue, long row, long col,double mScale);
	
	void GetDataFromSite(QString FileName, double **pBuffer, long Rows, long Cols);//��ȡվ�����ݣ��Լ�������Ϣ����Ϊ13��
	long GetDataRowsFromSite(QString FileName);//����ı��ļ�������
	long GetDataColsFromSite(QString FileName);//����ı��ļ�������
	QString *GetDataSetName(QString FileName, long Cols);
	double *GetDataSetTimeResolutionFromSite(QString FileName, long Cols);//���ʱ��ֱ���
	double *GetDataSetScaleFromSite(QString FileName, long Cols);//������ݵ�λscale

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

