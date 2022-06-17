#include "ClsHDF4Operator.h"
#include <ClsGeneralOperator.h>
#include <QMessageBox>
#include <QFileDialog>
#include "CONST.h"

CClsHDF4Operator::CClsHDF4Operator() {};

CClsHDF4Operator::CClsHDF4Operator(CString m_MarineParameterName,int m_DatesetsType)
{
	this->m_MarineParameterName = m_MarineParameterName;
	this->m_DatesetsType = m_DatesetsType;
}

CClsHDF4Operator::CClsHDF4Operator(Meta meta) {
	this->meta = meta;
}

Meta CClsHDF4Operator::getHDFMeta(string templateFile) {
	Meta meta;

	string strTemp = templateFile;
	CString tName = strTemp.c_str();

	CClsHDF4Operator pReadHDF;

	CString DsName = Def.DataSetName.c_str();

	meta.Resolution = pReadHDF.GetDatasetsSpatialResolution_New(tName, DsName);
	meta.Scale = pReadHDF.GetDataSetsScale(tName, DsName);

	meta.MissingValue = pReadHDF.GetDataSetsMissingValue(tName, DsName);

	meta.Rows = pReadHDF.GetDatasetsRows(tName, DsName);
	meta.Cols = pReadHDF.GetDatasetsCols(tName, DsName);
	meta.Size = meta.Rows * meta.Cols;

	meta.StartLog = pReadHDF.GetDatasetsStartLog(tName, DsName);
	meta.StartLat = pReadHDF.GetDatasetsStartLat(tName, DsName);
	meta.EndLog = pReadHDF.GetDatasetsEndLog(tName, DsName);
	meta.EndLat = pReadHDF.GetDatasetsEndLat(tName, DsName);
	//USES_CONVERSION;
	//meta.ProductType = W2A(pReadHDF.GetFileProductType(tName));
	meta.Offset = pReadHDF.GetDataSetsOffsets(tName, DsName);

	return meta;
}

bool CClsHDF4Operator::meanAndStandardDeviation(vector<string> Files, double* pMeanBuffer, double* pStdBuffer) {
	int FileNum = Files.size();
	CClsHDF4Operator ReadHDF;
	CString templateFile = Files[0].c_str();
	CString mDsName = Def.DataSetName.c_str();
	long mRows = ReadHDF.GetDatasetsRows(templateFile, mDsName);
	long mCols = ReadHDF.GetDatasetsCols(templateFile, mDsName);
	double mMissingValue = ReadHDF.GetDataSetsMissingValue(templateFile, mDsName);
	double mScale = ReadHDF.GetDataSetsScale(templateFile, mDsName);

	for (long j = 0; j < mRows * mCols; j++)
	{
		pMeanBuffer[j] = 0.0;
		pStdBuffer[j] = 0.0;
	}

	int* pValueNum = new int[mRows * mCols];
	for (long i = 0; i < mRows * mCols; ++i)
		pValueNum[i] = 0;

	//开始计算均值
	for (long i = 0; i<FileNum; i++)
	{
		CString tStr = Files[i].c_str();
		//定义缓冲区,存储区域均值
		long *pBuffer = NULL;
		pBuffer = (long*)malloc(mRows*mCols * sizeof(long));

		//读取数据
		if (!ReadHDF.GetDsByDsnameFROMProduct(pBuffer, tStr, mDsName, 0, mRows, 0, mCols))
		{
			//释放内存
			free(pBuffer);
			return false;
		}

		//int serialNum = CClsGeneralOperator::getDayOfYear(Files[i]);
		//计算
		for (long j = 0; j<mRows*mCols; j++)
		{
			if (pBuffer[j] != (long)mMissingValue)
			{
				pMeanBuffer[j] += ((double)pBuffer[j] * mScale);
				pStdBuffer[j] += pow(((double)pBuffer[j] * mScale), 2.0);
				pValueNum[j] += 1;
			}
		}

		//释放内存
		free(pBuffer);
	}

	//计算任意格点处均值\标准化方差	
	for (long j = 0; j<mRows*mCols; j++)
	{
		if (pValueNum[j] != 0)
		{
			pMeanBuffer[j] = ((double)pMeanBuffer[j] / pValueNum[j]);  //  ???????
			double tV1 = (double)pStdBuffer[j] / pValueNum[j];
			double tV2 = pow(pMeanBuffer[j], 2.0);
			if (tV1 >= tV2)
				pStdBuffer[j] = sqrt(tV1 - tV2);
			else
				pStdBuffer[j] = 0.0;
		}
		else
		{
			pMeanBuffer[j] = 0.0;
			pStdBuffer[j] = 0.0;
		}
	}
	delete[] pValueNum;
	return true;
}

bool CClsHDF4Operator::writeHDF(string Filename, Meta meta, long * pBuffer) {
	double *Max_Min = new double[2];
	Max_Min = CClsGeneralOperator::GetMin_Max(Max_Min, pBuffer, meta.Rows, meta.Cols);
	double mMaxValue = Max_Min[1];
	double mMinValue = Max_Min[0];
	double mMeanValue = CClsGeneralOperator::GetMeanValue(pBuffer, meta.Rows, meta.Cols);
	double mStdValue = CClsGeneralOperator::GetStdValue(pBuffer, meta.Rows, meta.Cols);
	delete Max_Min;

	CString Date = meta.Date.c_str();

	if(!this->WriteCustomHDF2DFile(Filename.c_str(), Date, Def.ProductType.c_str(), "0",
		Def.DataSetName.c_str(), pBuffer, meta.Scale, meta.Offset, meta.StartLog, meta.EndLog, meta.StartLat, meta.EndLat,
		meta.Rows, meta.Cols, mMaxValue, mMinValue, mMeanValue, mStdValue, meta.MissingValue, meta.Resolution, "2维"))
		return false;

	return true;
}

bool CClsHDF4Operator::writeHDF(string Filename, Meta meta, int * buf) {
	double sum = 0;
	//获取最大值、最小值
	double MaxValue = DBL_MIN, MinValue = DBL_MAX;
	int count = 0;
	for (int j = 0; j < meta.Size; j++)
	{
		if (buf[j] > MaxValue)
			MaxValue = buf[j];
		if (buf[j] < MinValue)
			MinValue = buf[j];
		sum += buf[j];
		count++;
	}
	//获取平均值
	double MeanValue = sum / count;
	sum = 0;
	for (int j = 0; j < meta.Size; j++)
	{
		sum += ((double)buf[j] - MeanValue) * ((double)buf[j] - MeanValue);
	}
	//获取方差
	double StdValue = sqrt(sum / count);

	CString Date = meta.Date.c_str();

	if (!this->WriteCustomHDF2DFile(Filename.c_str(), Date, Def.ProductType.c_str(), "0",
		Def.DataSetName.c_str(), buf, meta.Scale, meta.Offset, meta.StartLog, meta.EndLog, meta.StartLat, meta.EndLat,
		meta.Rows, meta.Cols, MaxValue, MinValue, MeanValue, StdValue, meta.MissingValue, meta.Resolution, "2维"))
		return false;

	return true;
}

bool CClsHDF4Operator::readHDF(string fileName, long* buf) {
	return this->GetDsByDsnameFROMProduct(buf, fileName.c_str(), Def.DataSetName.c_str(), 0, Def.Rows, 0, Def.Cols);
}

bool CClsHDF4Operator::readHDF(string fileName, int* buf) {
	//获取文件的id
	const char* lpsz = fileName.c_str();
	int fid = SDstart(lpsz, DFACC_READ);

	//找数据集
	const char* tagChar = Def.DataSetName.c_str();
	int32 tagIndex = SDnametoindex(fid, tagChar);

	//从id中获取文件中数据集的id号
	int	sid = SDselect(fid, tagIndex);
	//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	int a = datatype;
	int b = rank;
	int c = attrnum;
	long t = 0;
	//读取数据
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;

	endge[0] = Def.Rows;
	endge[1] = Def.Cols;
	endge[2] = 1;

	if(datatype != 24){
		cout << "The datatype of file is not int32!" << endl;
		return false;
	}

	status = SDreaddata(sid, start, NULL, endge, buf);

	status = SDendaccess(sid);
	status = SDend(fid);

	return true;
}

BOOL CClsHDF4Operator::SetMarineParameterName(CString m_Parameter)
{
	return 0;
}

BOOL CClsHDF4Operator::WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType, CString m_DsName, int * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension)
{
	//成像时间     DFNT_CHAR8
	//产品类型     DFNT_CHAR8
	//数据类型     DFNT_CHAR8 
	//纬度	       DFNT_CHAR8 
	//数据集名     DFNT_INT32
	//比例系数     DFNT_FLOAT64
	//比例截距     DFNT_FLOAT64
	//起始经度     DFNT_FLOAT64
	//终止经度     DFNT_FLOAT64
	//起始纬度     DFNT_FLOAT64 
	//终止纬度     DFNT_FLOAT64
	//行数	       DFNT_UINT16,
	//列数	       DFNT_UINT16
	//最大值       DFNT_FLOAT64
	//最小值       DFNT_FLOAT64
	//均值         DFNT_FLOAT64 
	//方差         DFNT_FLOAT64
	//空值         DFNT_UINT16 
	//空间分辨率   DFNT_ FLOAT64

	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_CREATE);
	if (fid == -1)return false;

	int32 startWrite[2], dimesizeWrite[2];

	//创建数据集id
	dimesizeWrite[0] = m_Rows;
	dimesizeWrite[1] = m_Cols;

	const char* tagChar = W2A((LPCTSTR)m_DsName.GetBuffer(m_DsName.GetLength()));
	int sid = SDcreate(fid, tagChar, DFNT_INT32, 2, dimesizeWrite); //数据类型
	if (sid == -1)
	{
		//关闭打开hdf文件			
		status = SDend(fid);
		return false;
	}

	//写入数据集
	startWrite[0] = 0;
	startWrite[1] = 0;

	status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP)pBuffer);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入属性信息
	//数据集日期
	tagChar = W2A((LPCTSTR)m_ImageDate.GetBuffer(m_ImageDate.GetLength()));
	status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据类型信息
	tagChar = W2A((LPCTSTR)m_DataType.GetBuffer(m_DataType.GetLength()));
	status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//产品类型信息
	tagChar = "Product";
	status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//维度信息
	tagChar = W2A((LPCTSTR)m_Dimension.GetBuffer(m_Dimension.GetLength()));
	status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入数据集的属性信息
	//转换比例和截距
	status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	double mOffsets = 0.00;
	status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//起始经纬度
	status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的行列数
	status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//填充值
	status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的最大值和最小值
	/*double minValue, maxValue;
	double  *tempValue = new double[2];
	tempValue = CClsGeneralOperator::GetMin_Max(pBuffer, m_Rows, m_Cols);*/
	//minValue = tempValue[0]; m_MaxValue = tempValue[1];

	status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//数据集的最大值和最小值
	status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//写入数据集分辨率
	status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);
	return true;
}

BOOL CClsHDF4Operator::WriteCustomHDF2DFile(CString  Filename, CString  m_ImageDate, 
	                                        CString  m_ProductType, CString  m_DataType, 
	                                        CString  m_DsName, long * pBuffer, double m_Scale, 
	                                        double m_Offset, double m_StartLog, double m_EndLog, 
	                                        double m_StartLat, double m_EndLat, long m_Rows, 
	                                        long m_Cols, double m_MaxValue, double m_MinValue, 
	                                        double m_MeanValue, double m_StdValue, long m_FillValue, 
	                                        double m_Resolution, CString  m_Dimension)
{
	//成像时间     DFNT_CHAR8
	//产品类型     DFNT_CHAR8
	//数据类型     DFNT_CHAR8 
	//纬度	       DFNT_CHAR8 
	//数据集名     DFNT_INT32
	//比例系数     DFNT_FLOAT64
	//比例截距     DFNT_FLOAT64
	//起始经度     DFNT_FLOAT64
	//终止经度     DFNT_FLOAT64
	//起始纬度     DFNT_FLOAT64 
	//终止纬度     DFNT_FLOAT64
	//行数	       DFNT_UINT16,
	//列数	       DFNT_UINT16
	//最大值       DFNT_FLOAT64
	//最小值       DFNT_FLOAT64
	//均值         DFNT_FLOAT64 
	//方差         DFNT_FLOAT64
	//空值         DFNT_UINT16 
	//空间分辨率   DFNT_ FLOAT64

	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_CREATE);
	if (fid == -1)return false;

	int32 startWrite[2], dimesizeWrite[2];

	//创建数据集id
	dimesizeWrite[0] = m_Rows;
	dimesizeWrite[1] = m_Cols;

	const char* tagChar = W2A((LPCTSTR)m_DsName.GetBuffer(m_DsName.GetLength()));
	int sid = SDcreate(fid, tagChar, DFNT_INT32, 2, dimesizeWrite);
	if (sid == -1)
	{
		//关闭打开hdf文件			
		status = SDend(fid);
		return false;
	}

	//写入数据集
	startWrite[0] = 0;
	startWrite[1] = 0;

	//统一缺省值为-9999
	double dFactory = 1;// 0.001 / m_Scale;
	for (long i = 0; i < m_Rows*m_Cols; i++)
	{
		if (pBuffer[i] <= -9998)
		{
			pBuffer[i] = -9999;
		}		
	}


	//所有product的Scale统一为0.01
	//FillValue 统一为-9999
	//m_Scale = 0.001;
	m_FillValue = -9999;

	m_MeanValue /= dFactory;
	m_MaxValue /= dFactory;
	m_MinValue /= dFactory;
	m_StdValue /= dFactory;

	for (long i = 0; i < m_Rows*m_Cols; i++)
	{
		if (pBuffer[i] != m_FillValue)
		{
			pBuffer[i] /= dFactory;
		}
	}

	status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP)pBuffer);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入属性信息
	//数据集日期
	tagChar = W2A((LPCTSTR)m_ImageDate.GetBuffer(m_ImageDate.GetLength()));
	status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据类型信息
	tagChar = W2A((LPCTSTR)m_DataType.GetBuffer(m_DataType.GetLength()));
	status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//产品类型信息
	tagChar = "Product";
	status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//维度信息
	tagChar = W2A((LPCTSTR)m_Dimension.GetBuffer(m_Dimension.GetLength()));
	status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入数据集的属性信息
	//转换比例和截距
	status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//起始经纬度
	status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的行列数
	status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//填充值
	status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的最大值和最小值
	//double minValue, maxValue;
	//double  *tempValue = new double[2];
	//tempValue = CClsGeneralOperator::GetMin_Max(pBuffer, m_Rows, m_Cols);
	//minValue = tempValue[0]; m_MaxValue = tempValue[1];

	status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//数据集的平均值和标准差
	status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//写入数据集分辨率
	status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);
	return true;
}

BOOL CClsHDF4Operator::WriteCustomHDF2DFile(CString Filename, CString m_ImageDate, CString m_ProductType, CString m_DataType, CString m_DsName, unsigned char * pBuffer, double m_Scale, double m_Offset, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution, CString m_Dimension)
{
	//成像时间     DFNT_CHAR8
	//产品类型     DFNT_CHAR8
	//数据类型     DFNT_CHAR8 
	//纬度	       DFNT_CHAR8 
	//数据集名     DFNT_INT32
	//比例系数     DFNT_FLOAT64
	//比例截距     DFNT_FLOAT64
	//起始经度     DFNT_FLOAT64
	//终止经度     DFNT_FLOAT64
	//起始纬度     DFNT_FLOAT64 
	//终止纬度     DFNT_FLOAT64
	//行数	       DFNT_UINT16,
	//列数	       DFNT_UINT16
	//最大值       DFNT_FLOAT64
	//最小值       DFNT_FLOAT64
	//均值         DFNT_FLOAT64 
	//方差         DFNT_FLOAT64
	//空值         DFNT_UINT16 
	//空间分辨率   DFNT_ FLOAT64

	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_CREATE);
	if (fid == -1)return false;

	int32 startWrite[2], dimesizeWrite[2];

	//创建数据集id
	dimesizeWrite[0] = m_Rows;
	dimesizeWrite[1] = m_Cols;

	const char* tagChar = W2A((LPCTSTR)m_DsName.GetBuffer(m_DsName.GetLength()));
	int sid = SDcreate(fid, tagChar, DFNT_INT32, 2, dimesizeWrite);
	if (sid == -1)
	{
		//关闭打开hdf文件			
		status = SDend(fid);
		return false;
	}

	//写入数据集
	startWrite[0] = 0;
	startWrite[1] = 0;

	status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP)pBuffer);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入属性信息
	//数据集日期
	tagChar = W2A((LPCTSTR)m_ImageDate.GetBuffer(m_ImageDate.GetLength()));
	status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据类型信息
	tagChar = W2A((LPCTSTR)m_DataType.GetBuffer(m_DataType.GetLength()));
	status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//产品类型信息
	tagChar = "Product";
	status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//维度信息
	tagChar = W2A((LPCTSTR)m_Dimension.GetBuffer(m_Dimension.GetLength()));
	status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入数据集的属性信息
	//转换比例和截距
	status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	double mOffsets = 0.00;
	status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//起始经纬度
	status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的行列数
	status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//填充值
	status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的最大值和最小值
	/*double minValue, maxValue;
	double  *tempValue = new double[2];
	tempValue = CClsGeneralOperator::GetMin_Max(pBuffer, m_Rows, m_Cols);*/
	//minValue = tempValue[0]; m_MaxValue = tempValue[1];

	status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//数据集的最大值和最小值
	status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//写入数据集分辨率
	status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);
	return true;
}

BOOL CClsHDF4Operator::WriteCustomHDF1DFile(CString  Filename, double m_Log, double m_Lat, CString  m_ProductType, CString  m_DataType, 
	                                        CString  m_Dimension, CString  m_DsName, long * pBuffer, double m_Scale, double m_Offset, 
	                                        CString  m_StartTime, CString  m_EndTime, long m_Number, double m_MaxValue, double m_MinValue, 
	                                        double m_MeanValue, double m_StdValue, long m_FillValue, double m_Resolution)
{
	//经度     DFNT_FLOAT64
	//纬度     DFNT_FLOAT64
	//产品类型     DFNT_CHAR8
	//数据类型     DFNT_CHAR8 
	//数据集名     DFNT_INT32
	//比例系数     DFNT_FLOAT64
	//比例截距     DFNT_FLOAT64
	//起始时间     DFNT_CHAR8
	//终止时间     DFNT_CHAR8
	//序列数       DFNT_UINT16
	//最大值       DFNT_FLOAT64
	//最小值       DFNT_FLOAT64
	//均值         DFNT_FLOAT64 
	//方差         DFNT_FLOAT64
	//空值         DFNT_UINT16
	//时间分辨率   DFNT_CHAR8

	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_CREATE);
	if (fid == -1)return false;

	int32 startWrite[1], dimesizeWrite[1];

	//创建数据集id
	dimesizeWrite[0] = m_Number;
	//dimesizeWrite[1] = m_Cols;

	const char* tagChar = W2A((LPCTSTR)m_DsName.GetBuffer(m_DsName.GetLength()));
	int sid = SDcreate(fid, tagChar, DFNT_INT32, 1, dimesizeWrite);
	if (sid == -1)
	{
		//关闭打开hdf文件			
		status = SDend(fid);
		return false;
	}

	//写入数据集
	startWrite[0] = 0;
	//startWrite[1] = 0;

	status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP)pBuffer);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入属性信息
	//数据集起止日期
	tagChar = W2A((LPCTSTR)m_StartTime.GetBuffer(m_StartTime.GetLength()));
	status = SDsetattr(sid, "StartTime", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	tagChar = W2A((LPCTSTR)m_EndTime.GetBuffer(m_EndTime.GetLength()));
	status = SDsetattr(sid, "EndTime", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据类型信息
	tagChar = W2A((LPCTSTR)m_DataType.GetBuffer(m_DataType.GetLength()));
	status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//产品类型信息
	tagChar = "Product";
	status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//维度信息
	tagChar = W2A((LPCTSTR)m_Dimension.GetBuffer(m_Dimension.GetLength()));
	status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//写入数据集的属性信息
	//转换比例和截距
	status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	double mOffsets = 0.00;
	status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//经纬度
	status = SDsetattr(fid, "Log", DFNT_FLOAT64, 1, &m_Log);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(fid, "Lat", DFNT_FLOAT64, 1, &m_Lat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的序列数
	status = SDsetattr(sid, "Number", DFNT_UINT16, 1, &m_Number);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//填充值
	status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据集的最大值和最小值
	//double minValue, maxValue;
	//double  *tempValue = new double[2];
	//tempValue = CClsGeneralOperator::GetMin_Max(pBuffer, m_Rows, m_Cols);
	//minValue = tempValue[0]; m_MaxValue = tempValue[1];

	status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//数据集的平均值和标准差
	status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}
	//写入数据集分辨率
	status = SDsetattr(sid, "TimeResolution", DFNT_FLOAT64, 1, &m_Resolution);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);
	return true;
}

BOOL CClsHDF4Operator::WriteCustomHDF1DFile(CString  Filename, double m_Log, double m_Lat, CString  m_ProductType, CString  m_DataType,
	CString  m_Dimension, CString  *m_DsName, long **pBuffer, double *m_Scale, double m_Offset,
	CString  m_StartTime, CString  m_EndTime, long m_Number, double *m_MaxValue, double *m_MinValue,
	double *m_MeanValue, double *m_StdValue, long m_FillValue, double *m_Resolution,long count)
{
	//经度     DFNT_FLOAT64
	//纬度     DFNT_FLOAT64
	//产品类型     DFNT_CHAR8
	//数据类型     DFNT_CHAR8 
	//数据集名     DFNT_INT32
	//比例系数     DFNT_FLOAT64
	//比例截距     DFNT_FLOAT64
	//起始时间     DFNT_CHAR8
	//终止时间     DFNT_CHAR8
	//序列数       DFNT_UINT16
	//最大值       DFNT_FLOAT64
	//最小值       DFNT_FLOAT64
	//均值         DFNT_FLOAT64 
	//方差         DFNT_FLOAT64
	//空值         DFNT_UINT16
	//时间分辨率   DFNT_CHAR8

	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_CREATE);
	if (fid == -1)return false;

	int32 startWrite[1], dimesizeWrite[1];

	//创建数据集id
	dimesizeWrite[0] = m_Number;
	startWrite[0] = 0;
	//dimesizeWrite[1] = m_Cols;
	const char* tagChar;
	int sid;
	for (long i = 0; i < count; i++)
	{
		tagChar = W2A((LPCTSTR)m_DsName[i].GetBuffer(m_DsName[i].GetLength()));
		sid = SDcreate(fid, tagChar, DFNT_INT32, 1, dimesizeWrite);
		if (sid == -1)
		{
			//关闭打开hdf文件			
			status = SDend(fid);
			return false;
		}
		//写入数据集
		status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP)pBuffer[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//写入属性信息
		//数据集起止日期
		tagChar = W2A((LPCTSTR)m_StartTime.GetBuffer(m_StartTime.GetLength()));
		status = SDsetattr(sid, "StartTime", DFNT_CHAR8, 50, (VOIDP)tagChar);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		tagChar = W2A((LPCTSTR)m_EndTime.GetBuffer(m_EndTime.GetLength()));
		status = SDsetattr(sid, "EndTime", DFNT_CHAR8, 50, (VOIDP)tagChar);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//写入数据集的属性信息
		//转换比例和截距
		status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}

		double mOffsets = 0.00;
		status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//数据集的序列数
		status = SDsetattr(sid, "Number", DFNT_UINT16, 1, &m_Number);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//填充值
		status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//数据集的最大值和最小值
		//double minValue, maxValue;
		//double  *tempValue = new double[2];
		//tempValue = CClsGeneralOperator::GetMin_Max(pBuffer, m_Rows, m_Cols);
		//minValue = tempValue[0]; m_MaxValue = tempValue[1];

		status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}

		status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//数据集的平均值和标准差
		status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}

		status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
		//写入数据集分辨率
		status = SDsetattr(sid, "TimeResolution", DFNT_FLOAT64, 1, &m_Resolution[i]);
		if (status == -1)
		{
			//关闭打开hdf文件
			status = SDendaccess(sid);
			status = SDend(fid);
			return false;
		}
	}

	//经纬度
	status = SDsetattr(fid, "Log", DFNT_FLOAT64, 1, &m_Log);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	status = SDsetattr(fid, "Lat", DFNT_FLOAT64, 1, &m_Lat);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//数据类型信息
	tagChar = W2A((LPCTSTR)m_DataType.GetBuffer(m_DataType.GetLength()));
	status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//产品类型信息
	tagChar = "Product";
	status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//维度信息
	tagChar = W2A((LPCTSTR)m_Dimension.GetBuffer(m_Dimension.GetLength()));
	status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP)tagChar);
	if (status == -1)
	{
		//关闭打开hdf文件
		status = SDendaccess(sid);
		status = SDend(fid);
		return false;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);
	return true;
}

CString CClsHDF4Operator::GetCustomDSDate(CString Filename)
{
	CString YYYYMMDDD;
	m_DataType = this->GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)  //MODIS1B
	{
		CString tStr = CClsGeneralOperator::GetFileName(Filename);

		int Year, Day;
		int pos = tStr.Find(_T("."));
		CString yStr = tStr.Mid(pos + 2, 4);

		USES_CONVERSION;
		const char* lpy = W2A((LPCTSTR)yStr.GetBuffer(yStr.GetLength()));
		Year = atoi(lpy);

		CString mStr = tStr.Mid(pos + 6, 3);
		const char* lpm = W2A((LPCTSTR)mStr.GetBuffer(mStr.GetLength()));
		Day = atoi(lpm);

		YYYYMMDDD = CClsGeneralOperator::GetYMDFromYD(Year, Day);

		CString tTime = tStr.Mid(pos + 10, 4);

		YYYYMMDDD = YYYYMMDDD + _T("   UTD Time: ") + tTime;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)  //产品
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		CString ss;


		const char* tagChar = "ImageDate";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, char8_databuffer);
		ss = char8_databuffer;
		ss = ss.Left(24);
		YYYYMMDDD = ss;
		//以上三行的原始代码是：YYYYMMDDD = char8_databuffer;
		free(char8_databuffer);

		status = SDend(fid);
	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //产品
	{

		CString tStr = CClsGeneralOperator::GetFileName(Filename);
		int Year, Day;
		CString yStr = tStr.Mid(1, 4);

		USES_CONVERSION;
		const char* lpy = W2A((LPCTSTR)yStr.GetBuffer(yStr.GetLength()));
		Year = atoi(lpy);

		CString mStr = tStr.Mid(5, 3);
		const char* lpm = W2A((LPCTSTR)mStr.GetBuffer(mStr.GetLength()));
		Day = atoi(lpm);

		YYYYMMDDD = CClsGeneralOperator::GetYMDFromYD(Year, Day);

		CString tTime = tStr.Mid(8, 4);

		YYYYMMDDD = YYYYMMDDD + _T("   UTD Time: ") + tTime;

	}
	else//其它操作接口
	{
	}


	return YYYYMMDDD;
}

char * CClsHDF4Operator::GetCustomDSDataType(char * Filename)
{
	return nullptr;
}

char * CClsHDF4Operator::GetCustomDSDimension(char * Filename)
{
	return nullptr;
}

double CClsHDF4Operator::GetCustomDSLocaxtionLog(char * Filename)
{
	return 0.0;
}

double CClsHDF4Operator::GetCustomDSLocaxtionLat(char * Filename)
{
	return 0.0;
}

char * CClsHDF4Operator::GetCustomDSName(char * Filename)
{
	return nullptr;
}

long * CClsHDF4Operator::GetCustom2DDataSets(char * Filename, char * m_DsName)
{
	return nullptr;
}

long * CClsHDF4Operator::GetCustom1DDataSets(char * Filename, char * m_DsName)
{
	return nullptr;
}

long * CClsHDF4Operator::GetCustom2DSubDataSets(char * Filename, char * m_DsName, double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat)
{
	return nullptr;
}

long * CClsHDF4Operator::GetCustom1DSubDataSets(char * Filename, char * m_DsName, char * m_StartTime, char * m_EndTime)
{
	return nullptr;
}

double CClsHDF4Operator::GetCustomDSScale(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);	
	m_DataType = GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "Scale";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		tValue = float64_databuffer[0];
		free(float64_databuffer);
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}


	SDendaccess(sid);
	status = SDend(fid);

	return tValue;
}

double CClsHDF4Operator::GetCustomDSOffset(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "Offsets";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		tValue = float64_databuffer[0];
		free(float64_databuffer);
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}
	SDendaccess(sid);
	status = SDend(fid);
	return tValue;
}

double CClsHDF4Operator::GetCustomDSStartLog(CString Filename, CString Dsname)
{
	double mStartLog;
	//获取文件类型
	m_DataType = GetFileProductTypeOld(Filename);
	if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);

		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		const char* tagChar = "StartLog";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (status == -1)
		{
			status = SDendaccess(sid);
			status = SDend(fid);
			return NULL;
		}

		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		mStartLog = float64_databuffer[0];

		free(float64_databuffer);

		status = SDendaccess(sid);
		status = SDend(fid);

		return mStartLog;

	}
	else       //目只对产品类型进行处理，其他不做处理
	{
		return NULL;

	}
	return NULL;
}

double CClsHDF4Operator::GetCustomDSEndLog(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* lpszDs = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, lpszDs);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) != NULL) //产品数据
	{
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("不支持的数据类型!"));
	}

	const char* tagChar = "EndLog";
	tagIndex = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

	float64_databuffer = NULL;
	float64_databuffer = (float64*)allow_memory(globleattr, datatype);
	status = SDreadattr(sid, tagIndex, float64_databuffer);
	tValue = float64_databuffer[0];
	free(float64_databuffer);

	SDendaccess(sid);
	status = SDend(fid);

	return tValue;
}

double CClsHDF4Operator::GetCustomDSStartLat(CString Filename, CString Dsname)
{
	double mStartLat;
	//获取文件类型
	m_DataType = GetFileProductTypeOld(Filename);
	if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);

		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		const char* tagChar = "StartLat";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (status == -1)
		{
			status = SDendaccess(sid);
			status = SDend(fid);
			return NULL;
		}

		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		mStartLat = float64_databuffer[0];

		free(float64_databuffer);

		status = SDendaccess(sid);
		status = SDend(fid);
		return mStartLat;

	}
	else       //目只对产品类型进行处理，其他不做处理
	{
		return NULL;


	}
	return NULL;
}

double CClsHDF4Operator::GetCustomDSEndLat(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* lpszDs = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, lpszDs);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) != NULL) //产品数据
	{

		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("不支持的数据类型!"));
		return 0.0;
	}

	const char* tagChar = "EndLat";
	tagIndex = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

	float64_databuffer = NULL;
	float64_databuffer = (float64*)allow_memory(globleattr, datatype);
	status = SDreadattr(sid, tagIndex, float64_databuffer);
	tValue = float64_databuffer[0];
	free(float64_databuffer);

	SDendaccess(sid);
	status = SDend(fid);

	return tValue;
}

char * CClsHDF4Operator::GetCustomDSStartTime(char * Filename, char * m_DsName)
{
	return nullptr;
}

char * CClsHDF4Operator::GetCustomDSEndTime(char * Filename, char * m_DsName)
{
	return nullptr;
}

long CClsHDF4Operator::GetCustomDSRows(CString Filename, CString Dsname)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum < 0)    //文件中没有数据集 
	{
		status = SDend(fid);
		return false;
	}

	m_DataType = this->GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)   //MODIS1B
	{

		const char* tagChar1 = "EV_250_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);
		if (tagIndex < 0)
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);

			if (tagIndex < 0)
			{
				const char* tagChar3 = "EV_1KM_RefSB";
				tagIndex = SDnametoindex(fid, tagChar3);
			}
		}

		//从id中获取文件中数据集的id号
		int sid = SDselect(fid, tagIndex);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[0];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)
	{
		const char* tagChar = "Number of Scan Lines";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, int32_databuffer);

		long Rows = int32_databuffer[0];
		free(int32_databuffer);

		status = SDend(fid);

		return Rows;
	}
	else if (m_DataType.CompareNoCase(_T("OSISAF")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 4);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}

	else//其它操作接口，此处主要处理Chl数据
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
		long tRows = dimesize[0];
		status = SDendaccess(sid);
		status = SDend(fid);
		return tRows;
	}
}

long CClsHDF4Operator::GetCustomDSCols(CString Filename, CString Dsname)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum < 0)    //文件中没有数据集 
	{
		status = SDend(fid);
		return false;
	}

	m_DataType = this->GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)   //MODIS1B
	{

		const char* tagChar1 = "EV_250_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);
		if (tagIndex < 0)
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);

			if (tagIndex < 0)
			{
				const char* tagChar3 = "EV_1KM_RefSB";
				tagIndex = SDnametoindex(fid, tagChar3);
			}
		}

		//从id中获取文件中数据集的id号
		int sid = SDselect(fid, tagIndex);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[2];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)
	{
		const char* tagChar = "Pixels per Scan Line";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, int32_databuffer);

		long Cols = int32_databuffer[0];
		free(int32_databuffer);

		status = SDend(fid);
		return Cols;
	}
	else if (m_DataType.CompareNoCase(_T("OSISAF")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 4);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tCols = dimesize[2];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tCols;

	}
	else//其它操作接口，此处主要处理Chl数据
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
}

long CClsHDF4Operator::GetCustomDSNumbers(char * Filename, char * m_DsName)
{
	return 0;
}

double CClsHDF4Operator::GetCustomDSMaxValue(CString Filename, CString m_DsName)
{
	double value = 0;
	long mCutRows, mCutCols;
	mCutRows = CClsHDF4Operator::GetDatasetsRows(Filename, m_DsName);
	mCutCols = CClsHDF4Operator::GetDatasetsCols(Filename, m_DsName);
	
	//CString mDsDate = CClsHDF4Operator::GetFileDateTime(Filename);

	long *pTBuffer = NULL;
	pTBuffer = (long*)malloc(sizeof(long)*mCutRows*mCutCols);

	//double *pBuffer = NULL;
	//pBuffer = (double*)malloc(sizeof(double)*mCutRows*mCutCols);
	
	for (long m = 0;m<mCutRows;m++)
	{
		for (long n = 0;n<mCutCols;n++)
		{
			long temp = pTBuffer[m*mCutCols + n];
			if (temp >= value)
			{
				value = temp;
			}
			//if (temp != (long)mMissingValue)
			//{
			//	double value = log10((double)temp*mScale);
			//	if (value <= 2.0 && value >= -4.0)
			//		pBuffer[m*mCutCols + n] = value;
			//	else
			//		pBuffer[m*mCutCols + n] = (double)mMissingValue;
			//}
			//else
			//	pBuffer[m*mCutCols + n] = (double)mMissingValue;
		}
	}
	return value;
}

double CClsHDF4Operator::GetCustomDSMinValue(char * Filename, char * m_DsName)
{
	return 0.0;
}

double CClsHDF4Operator::GetCustomDSMeanValue(char * Filename, char * m_DsName)
{
	return 0.0;
}

double CClsHDF4Operator::GetCustomDSStdValue(char * Filename, char * m_DsName)
{
	return 0.0;
}

double CClsHDF4Operator::GetCustomDSFillValue(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductTypeOld(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "FillValue";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		int16_databuffer = NULL;
		int16_databuffer = (int16*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, int16_databuffer);
		tValue = (double)int16_databuffer[0];
		free(int16_databuffer);
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}


	SDendaccess(sid);
	status = SDend(fid);
	return tValue;
}

char * CClsHDF4Operator::GetCustomDSSpaceResolution(char * Filename, char * m_DsName)
{
	return nullptr;
}

char * CClsHDF4Operator::GetCustomDSTimeResolution(char * Filename, char * m_DsName)
{
	return nullptr;
}

BOOL CClsHDF4Operator::SetDatasetsType(int m_Type)
{
	return 0;
}

bool CClsHDF4Operator::isModis1B(CString Filename)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	const char* tagChar = "HDFEOSVersion";

	//index = SDfindattr(sid, scf);

	int32 tagIndex=SDfindattr(fid, tagChar);

	if (tagIndex < 0)
	{
		status = SDend(fid);
		return false;
	}

	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

	CString tagStr;

	if (status == -1)
	{
		status = SDend(fid);
		return false;
	}
	else
	{
		switch (datatype) {
		case 3:
		{
			uchar8_databuffer = NULL;
			uchar8_databuffer = (uchar8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uchar8_databuffer);
			tagStr = uchar8_databuffer;
			free(uchar8_databuffer);
			break;
		}
		case 4:
		{
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr + 1, datatype);
			status = SDreadattr(fid, tagIndex, char8_databuffer);
			tagStr = char8_databuffer;

			//	tagStr.Format("%s",char8_databuffer);
			free(char8_databuffer);
			break;
		}

		default:
			status = SDend(fid);
			return false;
		}
	}


	status = SDend(fid);

	tagStr = tagStr.Trim();
	//目前只对该版本的MODIS数据进行处理
	if (tagStr.CompareNoCase(_T("HDFEOS_V2.9")) == NULL)
	{
		m_DataType = _T("MODIS1B");
		return true;
	}
	else
		return false;
}

bool CClsHDF4Operator::isProduct(CString Filename)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	const char* tagChar = "ProductType";

	//index = SDfindattr(sid,scf);
	int32 tagIndex = SDfindattr(fid, tagChar);

	if (tagIndex < 0)
	{
		status = SDend(fid);
		return false;
	}

	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

	CString tagStr;

	if (status == -1)
	{
		status = SDend(fid);
		return false;
	}
	else
	{
		switch (datatype) {
		case 3:
		{
			uchar8_databuffer = NULL;
			uchar8_databuffer = (uchar8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uchar8_databuffer);
			tagStr = uchar8_databuffer;
			free(uchar8_databuffer);
			break;
		}
		case 4:
		{
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, char8_databuffer);
			tagStr = char8_databuffer;

			//	tagStr.Format("%s",char8_databuffer);
			free(char8_databuffer);
			break;
		}

		default:
			status = SDend(fid);
			return false;
		}
	}

	status = SDend(fid);

	tagStr = tagStr.Trim();
	if (tagStr.CompareNoCase(_T("Product")) == NULL)
	{
		m_DataType = _T("Product");
		return true;
	}
	else
		return false;
}

bool CClsHDF4Operator::isProductOld(CString Filename)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	const char* tagChar = "DataType";

	//index = SDfindattr(sid,scf);
	int32 tagIndex = SDfindattr(fid, tagChar);

	if (tagIndex < 0)
	{
		status = SDend(fid);
		return false;
	}

	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

	CString tagStr;

	if (status == -1)
	{
		status = SDend(fid);
		return false;
	}
	else
	{
		switch (datatype) {
		case 3:
		{
			uchar8_databuffer = NULL;
			uchar8_databuffer = (uchar8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uchar8_databuffer);
			tagStr = uchar8_databuffer;
			free(uchar8_databuffer);
			break;
		}
		case 4:
		{
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, char8_databuffer);
			tagStr = char8_databuffer;

			//	tagStr.Format("%s",char8_databuffer);
			free(char8_databuffer);
			break;
		}

		default:
			status = SDend(fid);
			return false;
		}
	}

	status = SDend(fid);

	tagStr = tagStr.Trim();
	if (tagStr.CompareNoCase(_T("Product")) == NULL)
	{
		m_DataType = _T("Product");
		return true;
	}
	else
		return false;
}

void * CClsHDF4Operator::allow_memory(long datasize, int datatype)
{
	switch (datatype) {
	case 3:
		return((uchar8 *)calloc(datasize, sizeof(uchar8)));
		break;
	case 4:
		return((char8 *)calloc(datasize, sizeof(char8)));
		break;
	case 5:
		return((float32 *)calloc(datasize, sizeof(float32)));
		break;
	case 6:
		return((float64 *)calloc(datasize, sizeof(float64)));
		break;
	case 20:
		return((int8 *)calloc(datasize, sizeof(int8)));
		break;
	case 21:
		return((uint8 *)calloc(datasize, sizeof(uint8)));
		break;
	case 22:
		return((int16 *)calloc(datasize, sizeof(int16)));
		break;
	case 23:
		return((uint16 *)calloc(datasize, sizeof(uint16)));
		break;
	case 24:
		return((int32 *)calloc(datasize, sizeof(int32)));
		break;
	case 25:
		return((uint32 *)calloc(datasize, sizeof(uint32)));
		break;
	default:
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("不存在的数据类型!"));
		return NULL;
		break;
	}
}

bool CClsHDF4Operator::GetFileAttList(CString Filename, CString * mAttList)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	for (int i = 0; i < attrnum; i++)
	{
		status = SDattrinfo(fid, i, filename, &datatype, &globleattr);
		*(mAttList + i) = filename;
	}
	
	status = SDend(fid);
	return true;
}

bool CClsHDF4Operator::GetDataSetAttList(CString Filename, CString DsName, CString * mAttList)
{
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);
	int status = SDfileinfo(fid, &datasetsnum, &attrnum);
	int sid = SDselect(fid, SDnametoindex(fid,W2A((LPCTSTR)DsName.GetBuffer(DsName.GetLength()))));
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	for (int i = 0; i < attrnum; i++)
	{
		status = SDattrinfo(sid, i, filename, &datatype, &globleattr);
		*(mAttList + i) = filename;
	}
	if (status < 0)return false;
	else
	{
		return true;
	}
}

long CClsHDF4Operator::GetDataSetAttNum(CString Filename, CString DsName)
{
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);
	int status = SDfileinfo(fid, &datasetsnum, &attrnum);
	int sid = SDselect(fid, SDnametoindex(fid, W2A((LPCTSTR)DsName.GetBuffer(DsName.GetLength()))));
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	return attrnum;
}

CString CClsHDF4Operator::GetFileAttr(CString Filename, CString AttrName)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);
	CString CAttrName = AttrName;
	const char *tagChar = W2A((LPCTSTR)AttrName.GetBuffer(CAttrName.GetLength()));

	//const char* tagChar = W2A((LPCTSTR)Attrname.GetBuffer(Attrname.GetLength()));
	int32 tagIndex = SDfindattr(fid, tagChar);

	if (tagIndex < 0)
	{
		return "";
	}

	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

	CString tagStr;

	if (status != -1)
	{
		switch (datatype) {
		case 3:
		{
			uchar8_databuffer = NULL;
			uchar8_databuffer = (uchar8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uchar8_databuffer);
			tagStr = uchar8_databuffer;
			free(uchar8_databuffer);
			break;
		}
		case 4:
		{
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, char8_databuffer);
			tagStr = char8_databuffer;
			free(char8_databuffer);
			break;
		}
		case 5:
			float32_databuffer = NULL;
			float32_databuffer = (float32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, float32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%f", *(float32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%f", *float32_databuffer);

			free(float32_databuffer);
			break;
		case 6:
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, float64_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%f", *(float64_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%f", *float64_databuffer);

			free(float64_databuffer);

			break;
		case 20:
			int8_databuffer = NULL;
			int8_databuffer = (int8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int8_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int8_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int8_databuffer);
			free(int8_databuffer);

			break;
		case 21:
			uint8_databuffer = NULL;
			uint8_databuffer = (uint8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint8_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint8_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint8_databuffer);

			free(uint8_databuffer);

			break;
		case 22:
			int16_databuffer = NULL;
			int16_databuffer = (int16*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int16_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int16_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int16_databuffer);

			free(int16_databuffer);

			break;
		case 23:
			uint16_databuffer = NULL;
			uint16_databuffer = (uint16*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint16_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint16_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint16_databuffer);

			free(uint16_databuffer);

			break;
		case 24:
			int32_databuffer = NULL;
			int32_databuffer = (int32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int32_databuffer);

			free(int32_databuffer);

			break;
		case 25:
			uint32_databuffer = NULL;
			uint32_databuffer = (uint32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint32_databuffer);

			free(uint32_databuffer);

			break;

		default:
			return ""; 
		}
	}
	status = SDend(fid);
	return tagStr;
}

CString CClsHDF4Operator::GetDataSetAttr(CString Filename, CString DsName, CString AttrName)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fileid = SDstart(lpsz, DFACC_READ);
	CString CAttrName = AttrName;
	const char *tagChar = W2A((LPCTSTR)AttrName.GetBuffer(CAttrName.GetLength()));

	int fid = SDselect(fileid, SDnametoindex(fileid,W2A((LPCTSTR)DsName.GetBuffer(DsName.GetLength()))));

	//const char* tagChar = W2A((LPCTSTR)Attrname.GetBuffer(Attrname.GetLength()));
	int32 tagIndex = SDfindattr(fid, tagChar);

	if (tagIndex < 0)
	{
		return "";
	}

	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

	CString tagStr;

	if (status != -1)
	{
		switch (datatype) {
		case 3:
		{
			uchar8_databuffer = NULL;
			uchar8_databuffer = (uchar8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uchar8_databuffer);
			tagStr = uchar8_databuffer;
			free(uchar8_databuffer);
			break;
		}
		case 4:
		{
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, char8_databuffer);
			tagStr = char8_databuffer;
			free(char8_databuffer);
			break;
		}
		case 5:
			float32_databuffer = NULL;
			float32_databuffer = (float32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, float32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%f", *(float32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%f", *float32_databuffer);

			free(float32_databuffer);
			break;
		case 6:
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, float64_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%f", *(float64_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%f", *float64_databuffer);

			free(float64_databuffer);

			break;
		case 20:
			int8_databuffer = NULL;
			int8_databuffer = (int8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int8_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int8_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int8_databuffer);
			free(int8_databuffer);

			break;
		case 21:
			uint8_databuffer = NULL;
			uint8_databuffer = (uint8*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint8_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint8_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint8_databuffer);

			free(uint8_databuffer);

			break;
		case 22:
			int16_databuffer = NULL;
			int16_databuffer = (int16*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int16_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int16_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int16_databuffer);

			free(int16_databuffer);

			break;
		case 23:
			uint16_databuffer = NULL;
			uint16_databuffer = (uint16*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint16_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint16_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint16_databuffer);

			free(uint16_databuffer);

			break;
		case 24:
			int32_databuffer = NULL;
			int32_databuffer = (int32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, int32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(int32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *int32_databuffer);

			free(int32_databuffer);

			break;
		case 25:
			uint32_databuffer = NULL;
			uint32_databuffer = (uint32*)allow_memory(globleattr, datatype);
			status = SDreadattr(fid, tagIndex, uint32_databuffer);

			if (globleattr > 1)
			{
				for (int m = 0; m < globleattr; m++)
				{
					CString str;
					str.Format(L"%d", *(uint32_databuffer + m));
					tagStr += str + _T("|");
				}
			}
			else
				tagStr.Format(L"%d", *uint32_databuffer);

			free(uint32_databuffer);

			break;

		default:
			return "";
		}
	}
	status = SDend(fid);
	return tagStr;
}

long CClsHDF4Operator::GetFileAttNum(CString Filename)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	status = SDend(fid);
	return attrnum;
}

CString CClsHDF4Operator::GetFileDateTime(CString Filename)
{
	CString YYYYMMDDD;
	m_DataType = this->GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)  //MODIS1B
	{
		CString tStr = CClsGeneralOperator::GetFileName(Filename);

		int Year, Day;
		int pos = tStr.Find(_T("."));
		CString yStr = tStr.Mid(pos + 2, 4);

		USES_CONVERSION;
		const char* lpy = W2A((LPCTSTR)yStr.GetBuffer(yStr.GetLength()));
		Year = atoi(lpy);

		CString mStr = tStr.Mid(pos + 6, 3);
		const char* lpm = W2A((LPCTSTR)mStr.GetBuffer(mStr.GetLength()));
		Day = atoi(lpm);

		YYYYMMDDD = CClsGeneralOperator::GetYMDFromYD(Year, Day);

		CString tTime = tStr.Mid(pos + 10, 4);

		YYYYMMDDD = YYYYMMDDD + _T("   UTD Time: ") + tTime;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)  //产品
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		CString ss;


		const char* tagChar = "ImageDate";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, char8_databuffer);
		ss = char8_databuffer;
		ss = ss.Left(24);
		YYYYMMDDD = ss;
		//以上三行的原始代码是：YYYYMMDDD = char8_databuffer;
		free(char8_databuffer);

		status = SDend(fid);
	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //产品
	{
		
		CString tStr = CClsGeneralOperator::GetFileName(Filename);
		int Year, Day;
		CString yStr = tStr.Mid(1, 4);

		USES_CONVERSION;
		const char* lpy = W2A((LPCTSTR)yStr.GetBuffer(yStr.GetLength()));
		Year = atoi(lpy);

		CString mStr = tStr.Mid(5, 3);
		const char* lpm = W2A((LPCTSTR)mStr.GetBuffer(mStr.GetLength()));
		Day = atoi(lpm);

		YYYYMMDDD = CClsGeneralOperator::GetYMDFromYD(Year, Day);

		CString tTime = tStr.Mid(8, 4);

		YYYYMMDDD = YYYYMMDDD + _T("   UTD Time: ") + tTime;

	}
	else//其它操作接口
	{
	}


	return YYYYMMDDD;
}

CString CClsHDF4Operator::GetChlStartTime(CString Filename)
{

	CString START_TIME = "";
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);


	const char* tagChar = "Start Time";
	int32 tagIndex = SDfindattr(fid, tagChar);
	status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);
	int a = datatype;
	uint32_databuffer = NULL;
	uint32_databuffer = (uint32*)allow_memory(globleattr, datatype);
	status = SDreadattr(fid, tagIndex, uint32_databuffer);
	START_TIME.Format(L"%d", *(uint32_databuffer));
	free(uint32_databuffer);

	status = SDend(fid);
	return START_TIME;
}
//ProductType
CString CClsHDF4Operator::GetFileProductType(CString Filename)
{
	CString tempStr = Filename.Right(3);
	if (tempStr.CompareNoCase(_T("hdf")) == NULL)    //hdf文件
	{
		bool isModis = this->isModis1B(Filename);
		if (isModis)
			return _T("MODIS1B");
		else
		{
			bool isPrd = isProduct(Filename);
			if (isPrd)
				return _T("Product");
			else
				//其它操作接口
				return "";
		}
		return "";
	}
	else if (tempStr.CompareNoCase(_T("ASC")) == NULL)  //asc 文件
	{
		return _T("ASC");

	}
	else if (tempStr.CompareNoCase(_T("LAC")) == NULL)  //MODIS 2级产品数据
	{
		return _T("LAC");
	}
	else
	{
		tempStr = GetFileAttr(Filename, _T("title"));
		int index = tempStr.Find(_T("OSI SAF"));
		if (index != -1)
		{
			return _T("OSISAF");
		}
		index = tempStr.Find(_T("FLK"));
		if (index != -1)
		{
			return _T("FLK");
		}
		index = tempStr.Find(_T("MSLA"));
		if (index != -1)
		{
			return _T("MSLA");
		}
		{
			return "";
		}
		//others
	}
}

CString CClsHDF4Operator::GetFileProductTypeOld(CString Filename)
{
	CString tempStr = Filename.Right(3);
	if (tempStr.CompareNoCase(_T("hdf")) == NULL)    //hdf文件
	{
		bool isModis = this->isModis1B(Filename);
		if (isModis)
			return _T("MODIS1B");
		else
		{
			bool isPrd = isProductOld(Filename);
			if (isPrd)
				return _T("Product");
			else
				//其它操作接口
				return "";
		}
		return "";
	}
	else if (tempStr.CompareNoCase(_T("ASC")) == NULL)  //asc 文件
	{
		return _T("ASC");

	}
	else if (tempStr.CompareNoCase(_T("LAC")) == NULL)  //MODIS 2级产品数据
	{
		return _T("LAC");
	}
	else
	{
		tempStr = GetFileAttr(Filename, _T("title"));
		int index = tempStr.Find(_T("OSI SAF"));
		if (index != -1)
		{
			return _T("OSISAF");
		}
		index = tempStr.Find(_T("FLK"));
		if (index != -1)
		{
			return _T("FLK");
		}
		index = tempStr.Find(_T("MSLA"));
		if (index != -1)
		{
			return _T("MSLA");
		}
		{
			return "";
		}
		//others
	}
}

bool CClsHDF4Operator::GetDatasetsList(CString Filename, CString *mDsList)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)//MODIS1B数据
	{
		const char* tagChar1 = "EV_1KM_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);

		if (tagIndex >= 0)
		{
			//1km的modis数据
			//添加反射率数据集
			*mDsList = _T("Reflectance DataSets");
			for (int i = 1; i < 3; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM from 250m");
				*(mDsList + i) = str;
			}
			for (int i = 3; i < 8; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM from 500m");
				*(mDsList + i) = str;
			}
			for (int i = 8; i < 13; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM");
				*(mDsList + i) = str;
			}

			CString tempStr = _T("Band13lo_1KM");
			*(mDsList + 13) = tempStr;
			tempStr = _T("Band13hi_1KM");
			*(mDsList + 14) = tempStr;
			tempStr = _T("Band14lo_1KM");
			*(mDsList + 15) = tempStr;
			tempStr = _T("Band14hi_1KM");
			*(mDsList + 16) = tempStr;

			for (int i = 15; i < 20; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM");
				*(mDsList + i + 2) = str;
			}

			tempStr = _T("Band26_1KM");
			*(mDsList + 22) = tempStr;

			//添加辐射率数据集
			*(mDsList + 23) = _T("Radiance DataSets");
			for (int i = 1; i < 3; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM from 250m");
				*(mDsList + i + 23) = str;
			}
			for (int i = 3; i < 8; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM from 500m");
				*(mDsList + i + 23) = str;
			}
			for (int i = 8; i < 13; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM");
				*(mDsList + i + 23) = str;
			}

			tempStr = _T("Band13lo_1KM");
			*(mDsList + 13 + 23) = tempStr;
			tempStr = _T("Band13hi_1KM");
			*(mDsList + 14 + 23) = tempStr;
			tempStr = _T("Band14lo_1KM");
			*(mDsList + 15 + 23) = tempStr;
			tempStr = _T("Band14hi_1KM");
			*(mDsList + 16 + 23) = tempStr;

			for (int i = 15; i < 20; i++)
			{
				CString str;
				str.Format(L"%d", i);
				str = _T("Band") + str + _T("_1KM");
				*(mDsList + i + 2 + 23) = str;
			}

			tempStr = _T("Band26_1KM");
			*(mDsList + 22 + 23) = tempStr;

			//添加发射率数据集
			*(mDsList + 46) = _T("Emissive DataSets");
			for (int i = 20; i < 37; i++)
			{
				if (i < 26)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_1KM");
					*(mDsList + i + 27) = str;
				}
				if (i > 26)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_1KM");
					*(mDsList + i + 26) = str;
				}
			}
		}
		else
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);
			if (tagIndex >= 0)
			{
				//500m的modis数据
				//获取反射率数据集
				*mDsList = _T("Reflectance DataSets");
				for (int i = 1; i < 3; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_1KM from 250m");
					*(mDsList + i) = str;
				}
				for (int i = 3; i < 8; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_500m");
					*(mDsList + i) = str;
				}

				//获取辐射率数据集
				*(mDsList + 8) = _T("Radiance DataSets");
				for (int i = 1; i < 3; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_1KM from 250m");
					*(mDsList + i + 8) = str;
				}
				for (int i = 3; i < 8; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_500m ");
					*(mDsList + i + 8) = str;
				}

			}
			else
			{
				//250m的modis数据
				//获取反射率数据集
				*mDsList = _T("Reflectance DataSets");
				for (int i = 1; i < 3; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_250m ");
					*(mDsList + i) = str;
				}
				//获取辐射率数据集
				*(mDsList + 3) = _T("Radiance DataSets");
				for (int i = 1; i < 3; i++)
				{
					CString str;
					str.Format(L"%d", i);
					str = _T("Band") + str + _T("_250m ");
					*(mDsList + i + 3) = str;
				}
			}
		}
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		//遍历数据集
		for (int i = 0; i < datasetsnum; i++)
		{
			int sid = SDselect(fid, i);
			status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
			*(mDsList + i) = filename;

			SDendaccess(sid);
		}

	}
	else
	{
		for (int i = 0; i < datasetsnum; i++)
		{
			int sid = SDselect(fid, i);
			status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
			*(mDsList + i) = filename;

			SDendaccess(sid);
		}

	}

	status = SDend(fid);
	return true;
}

CString CClsHDF4Operator::GetDatasetsNameByIndex(CString Filename, int dsIndex)
{
	CString dsName;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum <= dsIndex)
		return "";

	int sid = SDselect(fid, dsIndex);
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	dsName = filename;

	SDendaccess(sid);
	status = SDend(fid);

	return dsName;
}

bool CClsHDF4Operator::GetDsByDsnameFROMProduct(long * pBuffer, CString Filename, CString Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//找数据集
	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);

	//从id中获取文件中数据集的id号
	int	sid = SDselect(fid, tagIndex);
	//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	int a = datatype;
	int b = rank;
	int c = attrnum;
	long t = 0;
	//读取数据
	start[0] = mStartRow;
	start[1] = mStartCol;
	start[2] = 0;

	endge[0] = m_Rows;
	endge[1] = m_Cols;
	endge[2] = 1;

	//读取数据集
	switch (datatype) {
	case 3:
		uchar8_databuffer = NULL;
		uchar8_databuffer = (uchar8*)allow_memory(sizeof(uchar8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uchar8_databuffer);

		free(uchar8_databuffer);

		break;
	case 4:
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(sizeof(char8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, char8_databuffer);

		free(char8_databuffer);

		break;
	case 5:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, float32_databuffer);
		free(float32_databuffer);

		break;
	case 6:
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(sizeof(float64)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, float64_databuffer);

		free(float64_databuffer);

		break;
	case 20:
	{
		int8_databuffer = NULL;
		int8_databuffer = (int8*)allow_memory(sizeof(int8)*m_Rows*m_Cols, datatype);

		start[2] = 0;
		start[1] = 0;
		start[0] = 0;
		endge[0] = 1;
		endge[1] = m_Rows;
		endge[2] = m_Cols;

		status = SDreaddata(sid, start, NULL, endge, int8_databuffer);

		//获取数据比例
		//			double scale = GetScales(fileName,dsName);

		for (int i = 0; i< m_Rows*m_Cols; i++)
		{
			pBuffer[i] = int8_databuffer[i];
			/*int8 temp = pBuffer[i];
			if (temp ==-1)
			pBuffer[i] = 1000;
			else if(temp ==0)
			pBuffer[i] = -9999;
			else
			pBuffer[i]=(int8_databuffer[i]*scale)*1000+0.5 ;*/
		}

		free(int8_databuffer);
		break;
	}
	case 21:
		uint8_databuffer = NULL;
		uint8_databuffer = (uint8*)allow_memory(sizeof(uint8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint8_databuffer);

		free(uint8_databuffer);

		break;
	case 22:
		int16_databuffer = NULL;
		int16_databuffer = (int16*)allow_memory(sizeof(int16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, int16_databuffer);


		free(int16_databuffer);

		break;
	case 23:
		uint16_databuffer = NULL;
		uint16_databuffer = (uint16*)allow_memory(sizeof(uint16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint16_databuffer);


		free(uint16_databuffer);

		break;
	case 24:
		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(sizeof(int32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		t = pBuffer[193];
		free(int32_databuffer);

		break;
	case 25:
		uint32_databuffer = NULL;
		uint32_databuffer = (uint32*)allow_memory(sizeof(uint32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint32_databuffer);

		free(uint32_databuffer);

		break;

	case 144:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, float32_databuffer);
		free(float32_databuffer);

		break;

	default:
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("数据类型有误!"));
		break;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);

	return true;
}

int CClsHDF4Operator::GetDsByDsname(void * pBuffer, CString Filename, CString Dsname,long &out_Rows, long &out_Cols)
{
	long mStartRow = 0;
	long mStartCol = 0;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//找数据集
	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);

	//从id中获取文件中数据集的id号
	int	sid = SDselect(fid, tagIndex);
	//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	long m_Rows = dimesize[0];
	long m_Cols = dimesize[1];
	out_Rows = m_Rows;
	out_Cols = m_Cols;

	//读取数据
	start[0] = mStartRow;
	start[1] = mStartCol;
	start[2] = 0;

	endge[0] = m_Rows;
	endge[1] = m_Cols;
	endge[2] = 1;

	//读取数据集
	switch (datatype) {
	case 3:
		pBuffer = NULL;
		pBuffer = (uchar8*)allow_memory(sizeof(uchar8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uchar8_databuffer);

		break;
	case 4:
		pBuffer = NULL;
		pBuffer = (char8*)allow_memory(sizeof(char8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(char8_databuffer);

		break;
	case 5:
		pBuffer = NULL;
		pBuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);

		break;
	case 6:
		pBuffer = NULL;
		pBuffer = (float64*)allow_memory(sizeof(float64)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(float64_databuffer);

		break;
	case 20:
	{
		pBuffer = NULL;
		pBuffer = (int8*)allow_memory(sizeof(int8)*m_Rows*m_Cols, datatype);

		start[2] = 0;
		start[1] = 0;
		start[0] = 0;
		endge[0] = 1;
		endge[1] = m_Rows;
		endge[2] = m_Cols;

		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		//获取数据比例
		//			double scale = GetScales(fileName,dsName);
		free(int8_databuffer);
		break;
	}
	case 21:
		pBuffer = NULL;
		pBuffer = (uint8*)allow_memory(sizeof(uint8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uint8_databuffer);

		break;
	case 22:
		pBuffer = NULL;
		pBuffer = (int16*)allow_memory(sizeof(int16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);


		free(int16_databuffer);

		break;
	case 23:
		pBuffer = NULL;
		pBuffer = (uint16*)allow_memory(sizeof(uint16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);


		free(uint16_databuffer);

		break;
	case 24:
		pBuffer = NULL;
		pBuffer = (int32*)allow_memory(sizeof(int32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		break;
	case 25:
		pBuffer = NULL;
		pBuffer = (uint32*)allow_memory(sizeof(uint32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uint32_databuffer);

		break;

	case 144:
		pBuffer = NULL;
		pBuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);
		break;
	default:
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("数据类型有误!"));
		break;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);

	return datatype;
}

bool CClsHDF4Operator::GetDsByDsnameFROMProduct(float * pBuffer, CString Filename, CString Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//找数据集
	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);

	//从id中获取文件中数据集的id号
	int	sid = SDselect(fid, tagIndex);
	//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	int a = datatype;
	int b = rank;
	int c = attrnum;
	//读取数据
	start[0] = mStartRow;
	start[1] = mStartCol;
	start[2] = 0;

	endge[0] = m_Rows;
	endge[1] = m_Cols;
	endge[2] = 1;

	//读取数据集
	//datatype = 22;//修改方法
	switch (datatype) {
	case 3:
		uchar8_databuffer = NULL;
		uchar8_databuffer = (uchar8*)allow_memory(sizeof(uchar8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uchar8_databuffer);

		break;
	case 4:
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(sizeof(char8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, char8_databuffer);

		free(char8_databuffer);

		break;
	case 5:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);

		break;
	case 6:
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(sizeof(float64)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, float64_databuffer);

		free(float64_databuffer);

		break;
	case 20:
	{
		int8_databuffer = NULL;
		int8_databuffer = (int8*)allow_memory(sizeof(int8)*m_Rows*m_Cols, datatype);

		start[2] = 0;
		start[1] = 0;
		start[0] = 0;
		endge[0] = 1;
		endge[1] = m_Rows;
		endge[2] = m_Cols;

		status = SDreaddata(sid, start, NULL, endge, int8_databuffer);

		//获取数据比例
		//			double scale = GetScales(fileName,dsName);

		for (int i = 0; i< m_Rows*m_Cols; i++)
		{
			pBuffer[i] = int8_databuffer[i];
			/*int8 temp = pBuffer[i];
			if (temp ==-1)
			pBuffer[i] = 1000;
			else if(temp ==0)
			pBuffer[i] = -9999;
			else
			pBuffer[i]=(int8_databuffer[i]*scale)*1000+0.5 ;*/
		}

		free(int8_databuffer);
		break;
	}
	case 21:
		uint8_databuffer = NULL;
		uint8_databuffer = (uint8*)allow_memory(sizeof(uint8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint8_databuffer);

		free(uint8_databuffer);

		break;
	case 22:
		int16_databuffer = NULL;
		int16_databuffer = (int16*)allow_memory(sizeof(int16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);


		free(int16_databuffer);

		break;
	case 23:
		uint16_databuffer = NULL;
		uint16_databuffer = (uint16*)allow_memory(sizeof(uint16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint16_databuffer);


		free(uint16_databuffer);

		break;
	case 24:
		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(sizeof(int32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(int32_databuffer);

		break;
	case 25:
		uint32_databuffer = NULL;
		uint32_databuffer = (uint32*)allow_memory(sizeof(uint32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uint32_databuffer);

		break;

	case 144:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);

		break;

	default:
		QMessageBox::warning(NULL, QStringLiteral("错误"), QStringLiteral("数据类型有误"));
		break;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);

	return true;
}

bool CClsHDF4Operator::GetDsByDsnameFROMProduct(double * pBuffer, CString Filename, CString Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//找数据集
	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);

	//从id中获取文件中数据集的id号
	int	sid = SDselect(fid, tagIndex);
	//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
	int a = datatype;
	int b = rank;
	int c = attrnum;
	//读取数据
	start[0] = mStartRow;
	start[1] = mStartCol;
	start[2] = 0;

	endge[0] = m_Rows;
	endge[1] = m_Cols;
	endge[2] = 1;

	//读取数据集
	//datatype = 22;//修改方法
	switch (datatype) {
	case 3:
		uchar8_databuffer = NULL;
		uchar8_databuffer = (uchar8*)allow_memory(sizeof(uchar8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uchar8_databuffer);

		break;
	case 4:
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(sizeof(char8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, char8_databuffer);

		free(char8_databuffer);

		break;
	case 5:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);

		break;
	case 6:
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(sizeof(float64)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, float64_databuffer);

		free(float64_databuffer);

		break;
	case 20:
	{
		int8_databuffer = NULL;
		int8_databuffer = (int8*)allow_memory(sizeof(int8)*m_Rows*m_Cols, datatype);

		start[2] = 0;
		start[1] = 0;
		start[0] = 0;
		endge[0] = 1;
		endge[1] = m_Rows;
		endge[2] = m_Cols;

		status = SDreaddata(sid, start, NULL, endge, int8_databuffer);

		//获取数据比例
		//			double scale = GetScales(fileName,dsName);

		for (int i = 0; i< m_Rows*m_Cols; i++)
		{
			pBuffer[i] = int8_databuffer[i];
			/*int8 temp = pBuffer[i];
			if (temp ==-1)
			pBuffer[i] = 1000;
			else if(temp ==0)
			pBuffer[i] = -9999;
			else
			pBuffer[i]=(int8_databuffer[i]*scale)*1000+0.5 ;*/
		}

		free(int8_databuffer);
		break;
	}
	case 21:
		uint8_databuffer = NULL;
		uint8_databuffer = (uint8*)allow_memory(sizeof(uint8)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint8_databuffer);

		free(uint8_databuffer);

		break;
	case 22:
		int16_databuffer = NULL;
		int16_databuffer = (int16*)allow_memory(sizeof(int16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);


		free(int16_databuffer);

		break;
	case 23:
		uint16_databuffer = NULL;
		uint16_databuffer = (uint16*)allow_memory(sizeof(uint16)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, uint16_databuffer);


		free(uint16_databuffer);

		break;
	case 24:
		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(sizeof(int32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(int32_databuffer);

		break;
	case 25:
		uint32_databuffer = NULL;
		uint32_databuffer = (uint32*)allow_memory(sizeof(uint32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);

		free(uint32_databuffer);

		break;

	case 144:
		float32_databuffer = NULL;
		float32_databuffer = (float32*)allow_memory(sizeof(float32)*m_Rows*m_Cols, datatype);
		status = SDreaddata(sid, start, NULL, endge, pBuffer);
		free(float32_databuffer);

		break;

	default:
		QMessageBox::warning(NULL, QStringLiteral("错误"), QStringLiteral("数据类型有误"));
		break;
	}

	//关闭打开hdf文件
	status = SDendaccess(sid);
	status = SDend(fid);

	return true;
}

long CClsHDF4Operator::GetDatesetsNum(CString Filename)
{
	m_DataType = GetFileProductType(Filename);

	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));

	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum < 0)    //文件中没有数据集 
		return false;

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)//MODIS1B数据
	{
		const char* tagChar1 = "EV_1KM_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);

		if (tagIndex >= 0)
		{
			status = SDend(fid);
			return 63;
		}
		else
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);
			if (tagIndex >= 0)
			{
				status = SDend(fid);
				return 16;
			}
			else
			{
				status = SDend(fid);
				return 6;
			}
		}
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		status = SDend(fid);
		return datasetsnum;
	}

	else if (m_DataType.CompareNoCase(_T("FLK")) == NULL)
	{
		status = SDend(fid);
		return datasetsnum;

	}
	else if (m_DataType.CompareNoCase(_T("MSLA")) == NULL)
	{
		status = SDend(fid);
		return datasetsnum;

	}

	else
	{
		//	AfxMessageBox(_T("不支持的数据格式"));
		status = SDend(fid);
		return datasetsnum;
		//others
	}

	status = SDend(fid);
	return NULL;
}

double CClsHDF4Operator::GetDataSetsScale(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "Scale";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (datatype == 4) {
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, char8_databuffer);
			CString mResStr = char8_databuffer;
			free(char8_databuffer);
			//tValue = _ttof(mResStr);

			const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
			tValue = atof(lpstr);
		}
		else {
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, float64_databuffer);
			tValue = float64_databuffer[0];
			free(float64_databuffer);
		}
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}


	SDendaccess(sid);
	status = SDend(fid);

	return tValue;
}

double CClsHDF4Operator::GetDataSetsOffsets(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "Offsets";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (datatype == 4) {
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, char8_databuffer);
			CString mResStr = char8_databuffer;
			free(char8_databuffer);
			//double tValue = _ttof(mResStr);
			const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
			tValue = atof(lpstr);
		}
		else
		{
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, float64_databuffer);
			tValue = float64_databuffer[0];
			free(float64_databuffer);
		}
		//float64_databuffer = NULL;
		//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		//status = SDreadattr(sid, tagIndex, float64_databuffer);
		//tValue = float64_databuffer[0];
		//free(float64_databuffer);
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}
	SDendaccess(sid);
	status = SDend(fid);
	return tValue;
}

double CClsHDF4Operator::GetDataSetsMissingValue(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* tagChar = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, tagChar);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) == NULL) //产品数据
	{
		const char* tagChar = "FillValue";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (datatype == 4) {
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, char8_databuffer);
			CString mResStr = char8_databuffer;
			free(char8_databuffer);
			//double tValue = _ttof(mResStr);
			const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
			tValue = atof(lpstr);
		}
		else {
			int16_databuffer = NULL;
			int16_databuffer;
			int64_t *fiilValue = (int64_t*)allow_memory(globleattr, datatype);

			long *fill = new long(globleattr);
			status = SDreadattr(sid, tagIndex, fill);
			tValue = (double)fill[0];
			free(fiilValue);
		}
	}
	else if (m_DataType.CompareNoCase(_T("MOIDS1B")) == NULL)  //MODIS1B
	{

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)  //MODIS2
	{
	}
	else
	{
		//其他
	}
	SDendaccess(sid);
	status = SDend(fid);
	return tValue;
}

long CClsHDF4Operator::GetDatasetsRows(CString Filename, CString Dsname)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum < 0)    //文件中没有数据集 
	{
		status = SDend(fid);
		return false;
	}

	m_DataType = this->GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)   //MODIS1B
	{

		const char* tagChar1 = "EV_250_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);
		if (tagIndex < 0)
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);

			if (tagIndex < 0)
			{
				const char* tagChar3 = "EV_1KM_RefSB";
				tagIndex = SDnametoindex(fid, tagChar3);
			}
		}

		//从id中获取文件中数据集的id号
		int sid = SDselect(fid, tagIndex);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[0];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)
	{
		const char* tagChar = "Number of Scan Lines";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, int32_databuffer);

		long Rows = int32_databuffer[0];
		free(int32_databuffer);

		status = SDend(fid);

		return Rows;
	}
	else if (m_DataType.CompareNoCase(_T("OSISAF")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 4);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}

	else//其它操作接口，此处主要处理Chl数据
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
		long tRows = dimesize[0];
		status = SDendaccess(sid);
		status = SDend(fid);
		return tRows;
	}
}

long CClsHDF4Operator::GetDatasetsCols(CString Filename, CString Dsname)
{
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	//获取文件中数据集的个数 
	attrnum = 0;
	status = SDfileinfo(fid, &datasetsnum, &attrnum);

	if (datasetsnum < 0)    //文件中没有数据集 
	{
		status = SDend(fid);
		return false;
	}

	m_DataType = this->GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("MODIS1B")) == NULL)   //MODIS1B
	{

		const char* tagChar1 = "EV_250_RefSB";
		int32 tagIndex = SDnametoindex(fid, tagChar1);
		if (tagIndex < 0)
		{
			const char* tagChar2 = "EV_500_RefSB";
			tagIndex = SDnametoindex(fid, tagChar2);

			if (tagIndex < 0)
			{
				const char* tagChar3 = "EV_1KM_RefSB";
				tagIndex = SDnametoindex(fid, tagChar3);
			}
		}

		//从id中获取文件中数据集的id号
		int sid = SDselect(fid, tagIndex);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[2];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
	else if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;

	}
	else if (m_DataType.CompareNoCase(_T("LAC")) == NULL)
	{
		const char* tagChar = "Pixels per Scan Line";
		int32 tagIndex = SDfindattr(fid, tagChar);
		status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

		int32_databuffer = NULL;
		int32_databuffer = (int32*)allow_memory(globleattr, datatype);
		status = SDreadattr(fid, tagIndex, int32_databuffer);

		long Cols = int32_databuffer[0];
		free(int32_databuffer);

		status = SDend(fid);
		return Cols;
	}
	else if (m_DataType.CompareNoCase(_T("OSISAF")) == NULL)
	{
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 4);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tCols = dimesize[2];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tCols;

	}
	else//其它操作接口，此处主要处理Chl数据
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);
		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		attrnum = 0;
		status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

		long tRows = dimesize[1];

		status = SDendaccess(sid);
		status = SDend(fid);

		return tRows;
	}
}

double CClsHDF4Operator::GetDatasetsStartLog(CString Filename, CString Dsname)
{
	double mStartLog;
	//获取文件类型
	m_DataType = GetFileProductType(Filename);
	if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);

		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		const char* tagChar = "StartLog";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (status == -1)
		{
			status = SDendaccess(sid);
			status = SDend(fid);
			return NULL;
		}
		
		//经过C#程序输出的hdf
		if (datatype == 4) {
			double tempNum;
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, char8_databuffer);
			CString mResStr = char8_databuffer;
			free(char8_databuffer);
			const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
			tempNum = atof(lpstr);
			status = SDendaccess(sid);
			status = SDend(fid);

			return tempNum;
		}
		//经过本QT程序输出的hdf
		else {
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, float64_databuffer);
			mStartLog = float64_databuffer[0];

			free(float64_databuffer);

			status = SDendaccess(sid);
			status = SDend(fid);

			return mStartLog;
		}

		//float64_databuffer = NULL;
		//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		//status = SDreadattr(sid, tagIndex, float64_databuffer);
		//mStartLog = float64_databuffer[0];

		//free(float64_databuffer);

		//status = SDendaccess(sid);
		//status = SDend(fid);

		//return mStartLog;

	}
	else       //目只对产品类型进行处理，其他不做处理
	{
		return NULL;

	}
	return NULL;
}

double CClsHDF4Operator::GetDatasetsStartLat(CString Filename, CString Dsname)
{
	double mStartLat;
	//获取文件类型
	m_DataType = GetFileProductType(Filename);
	if (m_DataType.CompareNoCase(_T("Product")) == NULL)
	{
		//获取文件的id
		USES_CONVERSION;
		const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
		int fid = SDstart(lpsz, DFACC_READ);

		//假定所有数据集的空间分辨率相等，从第一个数据集中获取
		int sid = SDselect(fid, 0);
		//获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
		const char* tagChar = "StartLat";
		int32 tagIndex = SDfindattr(sid, tagChar);
		status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

		if (status == -1)
		{
			status = SDendaccess(sid);
			status = SDend(fid);
			return NULL;
		}
		
		//经过C#程序输出的hdf
		if (datatype == 4) {
			double tempNum;
			char8_databuffer = NULL;
			char8_databuffer = (char8*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, char8_databuffer);
			CString mResStr = char8_databuffer;
			free(char8_databuffer);
			const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
			tempNum = atof(lpstr);
			status = SDendaccess(sid);
			status = SDend(fid);

			return tempNum;
		}//经过本QT程序输出的hdf
		else {
			float64_databuffer = NULL;
			float64_databuffer = (float64*)allow_memory(globleattr, datatype);
			status = SDreadattr(sid, tagIndex, float64_databuffer);
			mStartLat = float64_databuffer[0];

			free(float64_databuffer);

			status = SDendaccess(sid);
			status = SDend(fid);
			return mStartLat;
		}
		//float64_databuffer = NULL;
		//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		//status = SDreadattr(sid, tagIndex, float64_databuffer);
		//mStartLat = float64_databuffer[0];

		//free(float64_databuffer);

		//status = SDendaccess(sid);
		//status = SDend(fid);
		//return mStartLat;

	}
	else       //目只对产品类型进行处理，其他不做处理
	{
		return NULL;


	}
	return NULL;
}

double CClsHDF4Operator::GetDatasetsEndLog(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* lpszDs = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, lpszDs);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) != NULL) //产品数据
	{
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("不支持的数据类型!"));
	}

	const char* tagChar = "EndLog";
	tagIndex = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);
	
	//经过C#程序输出的hdf
	if (datatype == 4) {
		double tempNum;
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, char8_databuffer);
		CString mResStr = char8_databuffer;
		free(char8_databuffer);
		const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
		tempNum = atof(lpstr);
		status = SDendaccess(sid);
		status = SDend(fid);

		return tempNum;
	}//经过本QT程序输出的hdf
	else {
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		tValue = float64_databuffer[0];
		free(float64_databuffer);

		SDendaccess(sid);
		status = SDend(fid);

		return tValue;
	}

	//float64_databuffer = NULL;
	//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
	//status = SDreadattr(sid, tagIndex, float64_databuffer);
	//tValue = float64_databuffer[0];
	//free(float64_databuffer);

	//SDendaccess(sid);
	//status = SDend(fid);

	//return tValue;
}

double CClsHDF4Operator::GetDatasetsEndLat(CString Filename, CString Dsname)
{
	double tValue;
	//获取文件的id
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)Filename.GetBuffer(Filename.GetLength()));
	int fid = SDstart(lpsz, DFACC_READ);

	const char* lpszDs = W2A((LPCTSTR)Dsname.GetBuffer(Dsname.GetLength()));
	int32 tagIndex = SDnametoindex(fid, lpszDs);
	int sid = SDselect(fid, tagIndex);

	m_DataType = GetFileProductType(Filename);

	if (m_DataType.CompareNoCase(_T("Product")) != NULL) //产品数据
	{

		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("不支持的数据类型!"));
		return 0.0;
	}

	const char* tagChar = "EndLat";
	tagIndex = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

	//经过C#程序输出的hdf
	if (datatype == 4) {
		double tempNum;
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, char8_databuffer);
		CString mResStr = char8_databuffer;
		free(char8_databuffer);
		const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
		tempNum = atof(lpstr);
		status = SDendaccess(sid);
		status = SDend(fid);

		return tempNum;
	}//经过本QT程序输出的hdf
	else {
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, tagIndex, float64_databuffer);
		tValue = float64_databuffer[0];
		free(float64_databuffer);

		SDendaccess(sid);
		status = SDend(fid);

		return tValue;
	}
	//float64_databuffer = NULL;
	//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
	//status = SDreadattr(sid, tagIndex, float64_databuffer);
	//tValue = float64_databuffer[0];
	//free(float64_databuffer);

	//SDendaccess(sid);
	//status = SDend(fid);

	//return tValue;
}

double CClsHDF4Operator::GetDatasetsSpatialResolution_Old(CString mFileName, CString DsName)
{
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)mFileName.GetBuffer(mFileName.GetLength()));

	int fid;
	if ((fid = SDstart(lpsz, DFACC_READ)) == -1)
	{
		status = SDend(fid);
		return 0.0;
	}

	//获取文件中数据集的个数 
	status = SDfileinfo(fid, &datasetsnum, &attrnum);
	if (datasetsnum < 0)
	{
		status = SDend(fid);
		return 0.0;
	}

	//获取数据集索引，并选择数据集
	const char* lpszDs = W2A((LPCTSTR)DsName.GetBuffer(DsName.GetLength()));
	int index = SDnametoindex(fid, lpszDs);

	int sid = SDselect(fid, index);
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

	//循环获取数据集的基本信息
	CString tempStr;
	const char* tagChar = "DSResolution";
	index = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, index, filename, &datatype, &globleattr);

	char8_databuffer = NULL;
	char8_databuffer = (char8*)allow_memory(globleattr, datatype);
	status = SDreadattr(sid, index, char8_databuffer);
	tempStr = char8_databuffer;

	//float64_databuffer = NULL;
	//float64_databuffer = (float64*)allow_memory(globleattr, datatype);
	//status = SDreadattr(sid, index, float64_databuffer);
	//tempStr.Format(_T("%lf"), float64_databuffer[0]);

	
	free(char8_databuffer);

	double tempNum;
	const char* lpstr = W2A((LPCTSTR)tempStr.GetBuffer(tempStr.GetLength()));
	tempNum = atof(lpstr);


	status = SDendaccess(sid);
	status = SDend(fid);

	return tempNum;

}

double CClsHDF4Operator::GetDatasetsSpatialResolution_New(CString mFileName, CString DsName)
{
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)mFileName.GetBuffer(mFileName.GetLength()));

	int fid;
	if ((fid = SDstart(lpsz, DFACC_READ)) == -1)
	{
		status = SDend(fid);
		return 0.0;
	}

	//获取文件中数据集的个数 
	status = SDfileinfo(fid, &datasetsnum, &attrnum);
	if (datasetsnum < 0)
	{
		status = SDend(fid);
		return 0.0;
	}

	//获取数据集索引，并选择数据集
	const char* lpszDs = W2A((LPCTSTR)DsName.GetBuffer(DsName.GetLength()));
	int index = SDnametoindex(fid, lpszDs);

	int sid = SDselect(fid, index);
	status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

	//循环获取数据集的基本信息
	CString tempStr;
	const char* tagChar = "DSResolution";
	index = SDfindattr(sid, tagChar);
	status = SDattrinfo(sid, index, filename, &datatype, &globleattr);

	double tempNum;
	
	//经过C#程序输出的hdf
	if (datatype == 4) {
		char8_databuffer = NULL;
		char8_databuffer = (char8*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, index, char8_databuffer);
		CString mResStr = char8_databuffer;
		free(char8_databuffer);
		//double tempNum = _ttof(mResStr);
		const char* lpstr = W2A((LPCTSTR)mResStr.GetBuffer(mResStr.GetLength()));
		tempNum = atof(lpstr);

		status = SDendaccess(sid);
		status = SDend(fid);

		return tempNum;
	}//经过本QT程序输出的hdf
	else {
		float64_databuffer = NULL;
		float64_databuffer = (float64*)allow_memory(globleattr, datatype);
		status = SDreadattr(sid, index, float64_databuffer);
		tempStr.Format(_T("%lf"), float64_databuffer[0]);
		free(float64_databuffer);

		const char* lpstr = W2A((LPCTSTR)tempStr.GetBuffer(tempStr.GetLength()));
		tempNum = atof(lpstr);

		status = SDendaccess(sid);
		status = SDend(fid);

		return tempNum;
	}
}

long * CClsHDF4Operator::GetDataset(CString Filename, CString Dsname, int Dstype)
{
	return nullptr;
}

void CClsHDF4Operator::SpaceNearAnalysis(double *pBuffer, long *pTBuffer, int NearIndex, int WayIndex, long Rows, long Cols, double mFillValue, double mScale)
{
	int t = NearIndex + 1;
	switch (WayIndex)
	{
	case 0://max
	{
		for (int j = 0; j < Rows; j++)
		{
			for (int k = 0; k < Cols; k++)
			{
				//邻域之外
				if (j<t || j>Rows - t - 1 || k<t || k>Cols - t - 1)
				{
					pBuffer[j*Cols + k] = pTBuffer[j*Cols + k] * mScale;
				}
				//邻域内
				else
				{
					int tValue = -10000;
					int tValueNum = 0;
					for (int m = j - t; m <= j + t; m++)
					{
						for (int n = k - t; n <= k + t; n++)
						{
							long temp = pTBuffer[m*Cols + n];
							if (temp != mFillValue)
							{
								tValueNum++;
								if (temp >= tValue)
								{
									tValue = temp;
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[j*Cols + k] = mFillValue;
					}
					else
					{
						pBuffer[j*Cols + k] = tValue*mScale;
					}
				}
			}
		}
	}
	break;
	case 1://min
	{
		for (int j = 0; j < Rows; j++)
		{
			for (int k = 0; k < Cols; k++)
			{
				//邻域之外
				if (j<t || j>Rows - t - 1 || k<t || k>Cols - t - 1)
				{
					pBuffer[j*Cols + k] = pTBuffer[j*Cols + k] * mScale;
				}
				//邻域内
				else
				{
					int tValue = 1000000;
					int tValueNum = 0;
					for (int m = j - t; m <= j + t; m++)
					{
						for (int n = k - t; n <= k + t; n++)
						{
							long temp = pTBuffer[m*Cols + n];
							if (temp != mFillValue)
							{
								tValueNum++;
								if (temp <= tValue)
								{
									tValue = temp;
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[j*Cols + k] = mFillValue;
					}
					else
					{
						pBuffer[j*Cols + k] = tValue*mScale;
					}
				}
			}
		}
	}
	break;
	case 2://mean
	{
		for (int j = 0; j < Rows; j++)
		{
			for (int k = 0; k < Cols; k++)
			{
				//邻域之外
				if (j<t || j>Rows - t - 1 || k<t || k>Cols - t - 1)
				{
					pBuffer[j*Cols + k] = pTBuffer[j*Cols + k] * mScale;
				}
				//邻域内
				else
				{
					int count = 0;
					long sum = 0;
					for (int m = j - t; m <= j + t; m++)
					{
						for (int n = k - t; n <= k + t; n++)
						{
							long temp = pTBuffer[m*Cols + n];
							if (temp != mFillValue)
							{
								count++;
								sum += temp;
							}
						}
					}
					if (count == 0)
					{
						pBuffer[j*Cols + k] = mFillValue;
					}
					else
					{
						pBuffer[j*Cols + k] = sum*1.0*mScale / count;
					}
				}
			}
		}
	}
	break;
	case 3://std
	{
		for (int j = 0; j < Rows; j++)
		{
			for (int k = 0; k < Cols; k++)
			{
				//邻域之外
				if (j<t || j>Rows - t - 1 || k<t || k>Cols - t - 1)
				{
					pBuffer[j*Cols + k] = pTBuffer[j*Cols + k] * mScale;
				}
				//邻域内
				else
				{
					int count = 0;
					long sum = 0;
					double mean = 0;
					for (int m = j - t; m <= j + t; m++)
					{
						for (int n = k - t; n <= k + t; n++)
						{
							long temp = pTBuffer[m*Cols + n];
							if (temp != mFillValue)
							{
								count++;
								sum += temp;
							}
						}
					}
					if (count == 0)
					{
						pBuffer[j*Cols + k] = mFillValue;
					}
					else
					{
						mean = sum*1.0*mScale / count;
						double stdsum = 0;
						for (int m = j - t; m <= j + t; m++)
						{
							for (int n = k - t; n <= k + t; n++)
							{
								long temp = pTBuffer[m*Cols + n];
								if (temp != mFillValue)
								{
									stdsum = stdsum + (temp*1.0 * mScale - mean)*(temp*1.0 * mScale - mean);
								}
							}
						}
						pBuffer[j*Cols + k] = stdsum / count;
						//////
					}
				}
			}
		}
	}
	break;
	default:
		break;
	}
}

void CClsHDF4Operator::TimeNeighbourhoodAnalysis(long *pBuffer, long **temptime, int NearIndex, int WayIndex, long Row, long Col, double mFillValue)
{
	int t = NearIndex + 1;
	switch (WayIndex)
	{
	case 0://max
	{
		long max = 0;
		long count = 0;
		for (long x = 0; x < Row*Col; x++)
		{
			for (long y = 0; y < 2 * t + 1; y++)
			{
				long temp = temptime[y][x];
				if (temp != mFillValue)
				{
					count++;
					if (temp > max)
					{
						max = temp;
					}
				}
			}
			if (count == 0)
			{
				max = mFillValue;
			}
			pBuffer[x] = max;
			max = 0;
			count = 0;
		}
		break;
	}
	case 1://min
	{
		long min = 1000000;
		long count = 0;
		for (long x = 0; x < Row*Col; x++)
		{
			for (long y = 0; y < 2 * t + 1; y++)
			{
				long temp = temptime[y][x];
				if (temp != mFillValue)
				{
					count++;
					if (temp < min)
					{
						min = temp;
					}
				}
			}
			if (count == 0)
			{
				min = mFillValue;
			}
			pBuffer[x] = min;
			min = 1000000;
		}
		break;
	}
	case 2://mean
	{
		long sum = 0;
		long count = 0;
		for (long x = 0; x < Row*Col; x++)
		{
			for (long y = 0; y < 2 * t + 1; y++)
			{
				long temp = temptime[y][x];
				if (temp != mFillValue)
				{
					sum = sum + temp;
					count++;
				}
			}
			if (count == 0)
			{
				pBuffer[x] = mFillValue;
			}
			else
			{
				pBuffer[x] = sum*1.0 / count;
				sum = 0;
				count = 0;
			}
		}
		break;
	}
	case 3://std
	{
		long sum = 0;
		long count = 0;
		for (long x = 0; x < Row*Col; x++)
		{
			for (long y = 0; y < 2 * t + 1; y++)
			{
				long temp = temptime[y][x];
				if (temp != mFillValue)
				{
					sum = sum + temp;
					count++;
				}
			}
			if (count == 0)
			{
				pBuffer[x] = mFillValue;
			}
			else
			{
				double Tsum = 0;
				for (long y = 0; y < 2 * t + 1; y++)
				{
					long temp = temptime[y][x];
					if (temp != mFillValue)
					{
						Tsum = Tsum + (temp*1.0 - sum*1.0 / count)*(temp*1.0 - sum*1.0 / count);
					}
				}
				pBuffer[x] = Tsum / count;
			}
		}
		break;
	}
	}
}

void CClsHDF4Operator::OnTimeOrder(QStringList FileList)
{
	long number = FileList.count();
	CClsGeneralOperator *p = new CClsGeneralOperator();
	CClsHDF4Operator *q = new CClsHDF4Operator();
	CString str1, date1, str2, date2;
	for (long i = 0; i < number - 1; i++)
	{
		for (long j = 0; j < number - 1 - i; j++)
		{
			str1 = p->QStrToCStr(FileList.at(j));
			date1 = q->GetFileDateTime(str1);
			str2 = p->QStrToCStr(FileList.at(j+1));
			date2 = q->GetFileDateTime(str2);
			if (date1 > date2)
			{
				FileList.swap(j + 1, j);
			}
		}
	}
}

void CClsHDF4Operator::SpaceTimeNearAnalysis(long *pBuffer, long **temptime, int TimeNearIndex, int NearIndex,int WayIndex, long mFillValue,long Row, long Col,double mScale)
{
	int t = TimeNearIndex + 1;//时间邻域
	long ***tempbuffer = new long**[2 * t + 1];
	for (long x = 0; x < 2 * t + 1; x++)
	{
		tempbuffer[x] = new long*[Row];
	}
	for (long x = 0; x < 2 * t + 1; x++)
	{
		for (long y = 0; y < Row; y++)
		{
			tempbuffer[x][y] = new long[Col];
		}
	}
	for (long m = 0; m < 2 * t + 1; m++)
	{
		for (long x = 0; x < Row; x++)
		{
			for (long y = 0; y < Col; y++)
			{
				tempbuffer[m][x][y] = temptime[m][x*Col + y];
			}
		}
	}
	
	////
	int t1 = NearIndex + 1;//空间邻域
	switch (WayIndex)
	{
	case 0://max
	{
		for (long x = 0; x < Row; x++)
		{
			for (long y = 0; y < Col; y++)
			{
				//邻域之外
				if (x<t1 || x>Row - t1 - 1 || y<t1 || y>Col - t1 - 1)
				{
					pBuffer[x*Col + y] = tempbuffer[t + 1][x][y];
				}
				///邻域内
				else
				{
					int tValue = -1000000;
					int tValueNum = 0;
					for (long z = 0; z < 2 * t + 1; z++)
					{
						for (int m = x - t1; m <= x + t1; m++)
						{
							for (int n = y - t1; n <= y + t1; n++)
							{
								long temp = tempbuffer[z][m][n];
								if (temp != mFillValue)
								{
									tValueNum++;
									if (temp >= tValue)
									{
										tValue = temp;
									}
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[x*Col + y] = mFillValue;
					}
					else
					{
						pBuffer[x*Col + y] = tValue;
					}
				}
			}
		}
		break;
	}
	case 1://min
	{
		for (long x = 0; x < Row; x++)
		{
			for (long y = 0; y < Col; y++)
			{
				//邻域之外
				if (x<t1 || x>Row - t1 - 1 || y<t1 || y>Col - t1 - 1)
				{
					pBuffer[x*Col + y] = tempbuffer[t + 1][x][y];
				}
				///邻域内
				else
				{
					int tValue = 1000000;
					int tValueNum = 0;
					for (long z = 0; z < 2 * t + 1; z++)
					{
						for (int m = x - t1; m <= x + t1; m++)
						{
							for (int n = y - t1; n <= y + t1; n++)
							{
								long temp = tempbuffer[z][m][n];
								if (temp != mFillValue)
								{
									tValueNum++;
									if (temp <= tValue)
									{
										tValue = temp;
									}
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[x*Col + y] = mFillValue;
					}
					else
					{
						pBuffer[x*Col + y] = tValue;
					}
				}
			}
		}
		break;
	}
	case 2://mean
	{
		for (long x = 0; x < Row; x++)
		{
			for (long y = 0; y < Col; y++)
			{
				//邻域之外
				if (x<t1 || x>Row - t1 - 1 || y<t1 || y>Col - t1 - 1)
				{
					pBuffer[x*Col + y] = tempbuffer[t + 1][x][y];
				}
				///邻域内
				else
				{
					double sum = 0;
					int tValueNum = 0;
					for (long z = 0; z < 2 * t + 1; z++)
					{
						for (int m = x - t1; m <= x + t1; m++)
						{
							for (int n = y - t1; n <= y + t1; n++)
							{
								long temp = tempbuffer[z][m][n];
								if (temp != mFillValue)
								{
									tValueNum++;
									sum = sum + temp;
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[x*Col + y] = mFillValue;
					}
					else
					{
						pBuffer[x*Col + y] = sum / tValueNum;
					}
				}
			}
		}
		break;
	}
	case 3://std
	{
		for (long x = 0; x < Row; x++)
		{
			for (long y = 0; y < Col; y++)
			{
				//邻域之外
				if (x<t1 || x>Row - t1 - 1 || y<t1 || y>Col - t1 - 1)
				{
					pBuffer[x*Col + y] = tempbuffer[t + 1][x][y];
				}
				///邻域内
				else
				{
					double sum = 0, stdSum = 0;
					int tValueNum = 0;
					for (long z = 0; z < 2 * t + 1; z++)
					{
						for (int m = x - t1; m <= x + t1; m++)
						{
							for (int n = y - t1; n <= y + t1; n++)
							{
								long temp = tempbuffer[z][m][n];
								if (temp != mFillValue)
								{
									tValueNum++;
									sum = sum + temp;
								}
							}
						}
					}
					if (tValueNum == 0)
					{
						pBuffer[x*Col + y] = mFillValue;
					}
					else
					{
						for (long z = 0; z < 2 * t + 1; z++)
						{
							for (int m = x - t1; m <= x + t1; m++)
							{
								for (int n = y - t1; n <= y + t1; n++)
								{
									long temp = tempbuffer[z][m][n];
									if (temp != mFillValue)
									{
										stdSum = stdSum + (temp - sum / tValueNum)*(temp - sum / tValueNum);
									}
								}
							}
						}
						pBuffer[x*Col + y] = stdSum / tValueNum;
					}
				}
			}
		}
		break;
	}
	}
	////释放内存
	for (long x = 0; x < 2 * t + 1; x++)
	{
		for (long y = 0; y < Row; y++)
		{
			delete[] tempbuffer[x][y];
		}
	}
	for (long x = 0; x < 2 * t + 1; x++)
	{
		delete[] tempbuffer[x];
	}
	delete[] tempbuffer;
}

//以下4个接口为站点数据操作函数
void CClsHDF4Operator::GetDataFromSite(QString FileName, double **pBuffer, long Rows,long Cols)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return;
	}
	
	for (long i = 0; i < Rows + 3; i++)
	{
		if (i < 3)//跳过前3行
		{
			char str[1000];
			fscanf(fp, "%s", &str);
			fscanf(fp, "\n");
			continue;
		}
		for (long j = 0; j < Cols; j++)
		{
			fscanf(fp, "%lf", &pBuffer[i - 3][j]);
			fscanf(fp, ",");
		}
		//double a = pBuffer[i][5];
		fscanf(fp, "\n");
	}
	fclose(fp);
}

long CClsHDF4Operator::GetDataRowsFromSite(QString FileName)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	long count = 0;
	char str[1000];
	while (fgets(str, 1000, fp) != NULL)
	{
		count++;
	}
	return count - 3;//减去前3行（表头、时间分辨率、单位scale）
}

long CClsHDF4Operator::GetDataColsFromSite(QString FileName)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	char str[1000];
	long count = 0;
	bool split = false;
	QString strline;
	while (fgets(str, 1000, fp) != NULL)
	{
		if (!split)//第一行为表头，读取第二行
		{
			split = true;
			continue;

		}
		strline = str;
		long a = strline.count();
		for (long i = 0; i < strline.count(); i++)
		{
			if (strline.at(i) != "," && split)
			{
				count++;
				split = false;
			}
			else if (strline.at(i) == ",")
			{
				split = true;
			}
		}
		break;
	}
	return count;
}



QString *CClsHDF4Operator::GetDataSetName(QString FileName,long Cols)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	char str[1000];
	long count = 0;
	bool split = true;
	QString strline;
	QString *strTitle = new QString[Cols];
	while (fgets(str, 1000, fp) != NULL)
	{
		strline = str;
		for (long i = 0; i < strline.count(); i++)
		{
			if (strline.at(i) != ",")
			{
				strTitle[count] = strTitle[count] + strline.at(i);
			}
			else if (strline.at(i) == ",")
			{
				QString str = strTitle[count];
				count++;
			}
		}
		break;
	}
	return strTitle;
}

//////
double *CClsHDF4Operator::GetDataSetTimeResolutionFromSite(QString FileName, long Cols)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	char str[1000];
	long count = 0;
	QString strline;
	QString *strTimeResolution = new QString[Cols];
	long line = 1;
	while (fgets(str, 1000, fp) != NULL)
	{
		if (line != 2)//第二行为时间分辨率
		{
			line++;
			continue;
		}
		strline = str;
		for (long i = 0; i < strline.count(); i++)
		{
			if (strline.at(i) != ",")
			{
				strTimeResolution[count] = strTimeResolution[count] + strline.at(i);
			}
			else if (strline.at(i) == ",")
			{
				//QString str = strTitle[count];
				count++;
			}
		}
		break;
	}
	double *TimeResolution = new double[Cols];
	for (long i = 0; i < Cols; i++)
	{
		TimeResolution[i] = strTimeResolution[i].toDouble();
	}

	return TimeResolution;
}

double *CClsHDF4Operator::GetDataSetScaleFromSite(QString FileName, long Cols)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	char str[1000];
	long count = 0;
	QString strline;
	QString *strScale = new QString[Cols];
	long line = 1;
	while (fgets(str, 1000, fp) != NULL)
	{
		if (line != 3)//第3行为单位,scale
		{
			line++;
			continue;
		}
		strline = str;
		for (long i = 0; i < strline.count(); i++)
		{
			if (strline.at(i) != ",")
			{
				strScale[count] = strScale[count] + strline.at(i);
			}
			else if (strline.at(i) == ",")
			{
				count++;
			}
		}
		break;
	}
	double *Scale = new double[Cols];
	for (long i = 0; i < Cols; i++)
	{
		Scale[i] = strScale[i].toDouble();
	}

	return Scale;
}

void CClsHDF4Operator::SpaceDisCretization(long *pBuffer,double *pResultBuffer,long Rows,long Cols,double mFillValue,int WayIndex,int Number,int StdIndex)
{
	CClsGeneralOperator *pOp = new CClsGeneralOperator();
	switch (WayIndex)
	{
	case 3://均值
	{
		//double MeanValue = pOp->GetMeanValue(pBuffer, Rows, Cols, mFillValue);
		//MeanValue = abs(MeanValue);
		//for (long i = 0; i < Rows*Cols; i++)
		//{
		//	double temp = pBuffer[i];
		//	if (temp != mFillValue)
		//	{
		//		temp = abs(temp);
		//		long count = 0;//奇数量
		//		if (Number % 2 == 0)//偶数量
		//			count = 1;
		//		for (long j = 0; j < Number / 2; j++)
		//		{
		//			if (temp < MeanValue*(j + 1) / 2)
		//			{
		//				if (pBuffer[i] < 0)
		//				{
		//					pResultBuffer[i] = count * (-1);
		//					break;
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count;
		//					break;
		//				}
		//			}
		//			else
		//			{
		//				count++;
		//			}
		//			if (count == Number / 2 && Number % 2 == 1)
		//			{
		//				if (pBuffer[i] < 0)
		//				{
		//					pResultBuffer[i] = count * (-1);
		//					break;
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count;
		//					break;
		//				}
		//			}
		//			if (count == Number / 2 + 1 && Number % 2 == 0)
		//			{
		//				if (pBuffer[i] < 0)
		//				{
		//					pResultBuffer[i] = count * (-1) + 1;
		//					break;
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count - 1;
		//					break;
		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		pResultBuffer[i] = mFillValue;
		//	}
		//}
		break;
	}
	case 0://标准差
	{
		double MeanValue = pOp->GetMeanValue(pBuffer, Rows, Cols, mFillValue);
		double StdValue = pOp->GetStdValue(pBuffer, Rows, Cols, mFillValue);
		switch (StdIndex)
		{
		case -1:
			break;
		case 0:
		{
			StdValue = StdValue / 2.0;
			break;
		}
		case 1:
			break;
		case 2:
			StdValue = StdValue * 2.0;
			break;
		case 3:
			StdValue = StdValue * 3.0;
			break;
		default:
			break;
		}
		double *extreme = new double[Number - 1];
		int *odd = new int[Number];
		int *even = new int[Number];
		for (long i = 0; i < Number; i++)
		{
			if (Number % 2 == 1)
			{
				if (i < Number / 2)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
				if (i > Number / 2)
					extreme[i - 1] = MeanValue - (Number / 2 - i)*StdValue;
			}
			else
			{
				if (i < Number - 1)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
			}
			odd[i] = i - Number / 2;//奇数量离散化值
			if (i < Number / 2)
				even[i] = (-1)*(i + 1);//偶数量离散化值
			else
				even[i] = (i + 1) - Number / 2;
		}
		for (long i = 0; i < Rows*Cols; i++)
		{
			double temp = pBuffer[i];
			long count = 0;
			if (temp != mFillValue)
			{
				for (long j = 0; j < Number - 1; j++)
				{
					if (temp < extreme[j])
					{
						if (Number % 2 == 1)
							pResultBuffer[i] = odd[j];
						else
							pResultBuffer[i] = even[j];
						break;
					}
					else
						count++;
				}
				if (count == Number - 1)
				{
					if (Number % 2 == 1)
						pResultBuffer[i] = odd[count];
					else
						pResultBuffer[i] = even[count];
				}
			}
			else
			{
				pResultBuffer[i] = mFillValue;
			}
		}
		//for (long i = 0; i < Rows*Cols; i++)
		//{
		//	double temp = pBuffer[i];
		//	if (temp != mFillValue)
		//	{
		//		//temp = abs(temp);
		//		long count = 0;//奇数量
		//		if (Number % 2 == 0)//偶数量
		//			count = 1;
		//		for (long j = 0; j < Number / 2; j++)
		//		{
		//			if (temp < MeanValue + StdValue*(j + 1))
		//			{
		//				if (pBuffer[i] < MeanValue)
		//				{
		//					pResultBuffer[i] = count * (-1);
		//					break;
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count;
		//					break;
		//				}
		//			}
		//			else
		//			{
		//				count++;
		//			}
		//			if (count == Number / 2 && Number % 2 == 1)
		//			{
		//				if (pBuffer[i] < MeanValue)
		//				{
		//					pResultBuffer[i] = count * (-1);
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count;
		//				}
		//			}
		//			if (count == Number / 2 + 1 && Number % 2 == 0)
		//			{
		//				if (pBuffer[i] < MeanValue)
		//				{
		//					pResultBuffer[i] = count * (-1) + 1;
		//				}
		//				else
		//				{
		//					pResultBuffer[i] = count - 1;
		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		pResultBuffer[i] = mFillValue;
		//	}
		//}
		break;
	}
	case 1://等间隔
	{
		double *Max_Min = new double[2];
		Max_Min = pOp->GetMin_Max(Max_Min, pBuffer, Rows, Cols, mFillValue);
		MaxValue = Max_Min[1];
		MinValue = Max_Min[0];
		double dSpace = (MaxValue*1.0 - MinValue*1.0) / Number;
		
		for (long j = 0; j < Rows*Cols; j++)
		{
			double temp = pBuffer[j];
			if (temp != mFillValue)
			{
				for (long i = 0; i < Number; i++)
				{
					double Right = MinValue*1.0 + dSpace*(i + 1);
					if (i == Number - 1)
						Right = MaxValue + 1;
					if (temp < Right)
					{
						pResultBuffer[j] = i + 1;
						break;
					}
				}
			}
			else
			{
				pResultBuffer[j] = mFillValue;
			}
		}
		break;
	}
	default:
		break;
	}
}

void CClsHDF4Operator::DisCretization_One(double *startdata, double *enddata, long Cols,int WayIndex, int Number, int StdIndex)
{
	CClsGeneralOperator *pOp = new CClsGeneralOperator();
	switch (WayIndex)
	{
	case 3://均值
	{

		break;
	}
	case 0://标准差
	{
		double StdValue = pOp->GetStdValue(startdata, 1, Cols);
		switch (StdIndex)
		{
		case -1:
			break;
		case 0:
		{
			StdValue = StdValue / 2.0;
			break;
		}
		case 1:
			break;
		case 2:
			StdValue = StdValue * 2.0;
			break;
		case 3:
			StdValue = StdValue * 3.0;
			break;
		default:
			break;
		}
		double MeanValue = pOp->GetMeanValue(startdata, 1, Cols);
		double *extreme = new double[Number - 1];
		int *odd = new int[Number];
		int *even = new int[Number];
		for (long i = 0; i < Number; i++)
		{
			if (Number % 2 == 1)
			{
				if (i < Number / 2)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
				if (i > Number / 2)
					extreme[i - 1] = MeanValue - (Number / 2 - i)*StdValue;
			}
			else
			{
				if (i < Number - 1)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
			}
			odd[i] = i - Number / 2;//奇数量离散化值
			if (i < Number / 2)
				even[i] = (-1)*(i + 1);//偶数量离散化值
			else
				even[i] = (i + 1) - Number / 2;
		}
		for (long i = 0; i < Cols; i++)
		{
			double temp = startdata[i];
			long count = 0;
			for (long j = 0; j < Number - 1; j++)
			{
				if (temp < extreme[j])
				{
					if (Number % 2 == 1)
						enddata[i] = odd[j];
					else
						enddata[i] = even[j];
					break;
				}
				else
					count++;
			}
			if (count == Number - 1)
			{
				if (Number % 2 == 1)
					enddata[i] = odd[count];
				else
					enddata[i] = even[count];
			}
		}
		break;
	}
	case 1://等间隔
	{
		long count = 0;
		MaxValue = 0;
		MinValue = 1000000;
		for (long i = 0; i < Cols; i++)
		{
			double temp = startdata[i];
			if (temp > MaxValue)
				MaxValue = temp;
			if (temp < MinValue)
				MinValue = temp;
		}
		double dSpace = (MaxValue*1.0 - MinValue*1.0) / Number;
		for (long j = 0; j < Cols; j++)
		{
			double temp = startdata[j];
			for (long i = 0; i < Number; i++)
			{
				double Right = MinValue*1.0 + dSpace*(i + 1);
				if (i == Number - 1)
					Right = MaxValue + 1;
				if (temp < Right)
				{
					enddata[j] = i + 1;
					break;
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void CClsHDF4Operator::DisCretization_Two(double *startdata, double *enddata, long FileNum,double mFillValue, int WayIndex, int Number, int StdIndex)
{
	CClsGeneralOperator *pOp = new CClsGeneralOperator();
	switch (WayIndex)
	{
	case 3://均值
	{

		break;
	}
	case 0://标准差
	{
		double StdValue = pOp->GetStdValue(startdata, 1, FileNum, mFillValue);
		if (StdValue == mFillValue)
		{
			for (long i = 0; i < FileNum; i++)
			{
				enddata[i] = mFillValue;
			}
			return;
		}
		switch (StdIndex)
		{
		case -1:
			break;
		case 0:
		{
			StdValue = StdValue / 2.0;
			break;
		}
		case 1:
			break;
		case 2:
			StdValue = StdValue * 2.0;
			break;
		case 3:
			StdValue = StdValue * 3.0;
			break;
		default:
			break;
		}
		double MeanValue = pOp->GetMeanValue(startdata, 1, FileNum, mFillValue);
		double *extreme = new double[Number - 1];
		int *odd = new int[Number];
		int *even = new int[Number];
		for (long i = 0; i < Number; i++)
		{
			if (Number % 2 == 1)
			{
				if (i < Number / 2)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
				if (i > Number / 2)
					extreme[i - 1] = MeanValue - (Number / 2 - i)*StdValue;
			}
			else
			{
				if (i < Number - 1)
					extreme[i] = MeanValue - (Number / 2 - i)*StdValue;
			}
			odd[i] = i - Number / 2;//奇数量离散化值
			if (i < Number / 2)
				even[i] = (-1)*(i + 1);//偶数量离散化值
			else
				even[i] = (i + 1) - Number / 2;
		}
		for (long i = 0; i < FileNum; i++)
		{
			double temp = startdata[i];
			long count = 0;
			if (temp != mFillValue)
			{
				for (long j = 0; j < Number - 1;j++)
				{
					if (temp < extreme[j])
					{
						if (Number % 2 == 1)
							enddata[i] = odd[j];
						else
							enddata[i] = even[j];
						break;
					}
					else
						count++;
				}
				if (count == Number - 1)
				{
					if (Number % 2 == 1)
						enddata[i] = odd[count];
					else
						enddata[i] = even[count];
				}
			}
			else
			{
				enddata[i] = mFillValue;
			}
		}
		//for (long i = 0; i < FileNum; i++)
		//{
		//	long count = 0;
		//	if (Number % 2 == 0)
		//		count = 1;
		//	double temp = startdata[i];
		//	if (temp != mFillValue)
		//	{
		//		for (long j = 0; j < Number / 2; j++)
		//		{
		//			if (temp <= MeanValue + (j + 1)*StdValue)
		//			{
		//				if (temp < MeanValue)
		//				{
		//					enddata[i] = count*(-1);
		//					break;
		//				}
		//				else
		//				{
		//					enddata[i] = count;
		//					break;
		//				}
		//			}
		//			else
		//			{
		//				count++;
		//			}
		//			////奇数
		//			if (count == Number / 2 && Number % 2 == 1)
		//			{
		//				if (temp < MeanValue)
		//					enddata[i] = count*(-1);
		//				else
		//					enddata[i] = count;
		//			}
		//			////偶数
		//			if (count == Number / 2 + 1 && Number % 2 == 0)
		//			{
		//				if (temp < MeanValue)
		//					enddata[i] = count*(-1) + 1;
		//				else
		//					enddata[i] = count - 1;
		//			}
		//		}
		//	}
		//	else
		//	{
		//		enddata[i] = mFillValue;
		//	}
		//}
		break;
	}
	case 1://等间隔
	{
		long count = 0;
		MaxValue = 0;
		MinValue = 1000000;
		for (long i = 0; i < FileNum; i++)
		{
			double temp = startdata[i];
			if (temp != mFillValue)
			{
				if (temp > MaxValue)
					MaxValue = temp;
				if (temp < MinValue)
					MinValue = temp;
			}
			else
			{
				count++;
			}
		}
		if (count == FileNum)//全为缺省值
		{
			for (long i = 0; i < FileNum; i++)
			{
				enddata[i] = mFillValue;
			}
			return;
		}
		double dSpace = (MaxValue*1.0 - MinValue*1.0) / Number;
		for (long j = 0; j < FileNum; j++)
		{
			double temp = startdata[j];
			if (temp != mFillValue)
			{
				for (long i = 0; i < Number; i++)
				{
					double Right = MinValue*1.0 + dSpace*(i + 1);
					if (i == Number - 1)
						Right = MaxValue + 1;
					if (temp < Right)
					{
						enddata[j] = i + 1;
						break;
					}
				}
			}
			else
			{
				enddata[j] = mFillValue;
			}
		}
		break;
	}
	default:
		break;
	}
}

//txt为裸数据,以逗号（英文）分隔
void CClsHDF4Operator::GetNakeDataFromTXT(QString FileName, double **pBuffer, long Rows, long Cols)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return;
	}

	for (long i = 0; i < Rows; i++)
	{
		for (long j = 0; j < Cols; j++)
		{
			fscanf(fp, "%lf", &pBuffer[i][j]);
			fscanf(fp, ",");
		}
		fscanf(fp, "\n");
	}
	fclose(fp);
}

long CClsHDF4Operator::GetDataRowsFromTXT(QString FileName)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	long count = 0;
	char str[1000];
	while (fgets(str, 1000, fp) != NULL)
	{
		count++;
	}
	return count;
}

long CClsHDF4Operator::GetDataColsFromTXT(QString FileName)
{
	string tempStr = FileName.toLocal8Bit().data();

	FILE *fp;
	if ((fp = fopen((char*)tempStr.c_str(), "rt")) == NULL)
	{
		QMessageBox::information(NULL, QStringLiteral("失败"), QStringLiteral("无法打开文件！"));
		return 0;
	}
	char str[2000];
	long count = 0;
	bool split = true;
	QString strline;
	while (fgets(str, 2000, fp) != NULL)
	{
		strline = str;
		for (long i = 0; i < strline.count(); i++)
		{
			if (strline.at(i) != "," && split)
			{
				count++;
				split = false;
			}
			else if (strline.at(i) == ",")
			{
				split = true;
			}
		}
		break;
	}
	return count;
}

bool CClsHDF4Operator::WriteDataToCSV(CString FileName, double *pBuffer, long mRows, long mCols)
{
	////写入文件
	//USES_CONVERSION;
	//const char* lpsz = W2A((LPCTSTR)FileName.GetBuffer(FileName.GetLength()));
	//FILE *outFile;
	//errno_t err;
	//if ((err = fopen_s(&outFile, lpsz, "wb")) != 0)
	//{
	//	QMessageBox::information(NULL, QStringLiteral("警告"), QStringLiteral("打开文件失败!"));
	//	return  false;
	//}
	//for (long i = 0; i<mRows; i++)
	//{
	//	for (long j = 0; j<mCols; j++)
	//	{
	//		fprintf(outFile, "%lf ", pBuffer[i*mCols + j]);
	//		fprintf(outFile, "%s ", ",");
	//	}
	//	fprintf(outFile, "\n");
	//}
	////关闭文件
	//fclose(outFile);
	
	QFile csvFile(CClsGeneralOperator::CStrToQStr(FileName));
	if (csvFile.open(QIODevice::ReadWrite))
	{
		QTextStream stream(&csvFile);
		for (int i = 0; i<mRows; i++)
		{
			for (int j = 0; j<mCols; j++)
			{
				stream << pBuffer[i*mCols + j] << ",";
			}
			stream << "\n";
		}
	}
	csvFile.close();
	return true;
}

//Argo
QStringList CClsHDF4Operator::getFoderNamesFromFolder(QString path)
{
	QDir dir(path);
	path = path + "/";
	QStringList FileList;
	QStringList  dir_list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dir_list.count(); i++)
	{
		QString str = path + dir_list.at(i);
		FileList << str;
	}
	dir_list.clear();
	return FileList;
}

QStringList CClsHDF4Operator::getFileNamesFromFolder(QString path)
{
	QDir dir(path);
	path = path + "/";
	QStringList FileList;
	QStringList nameFilters;
	nameFilters << "*.nc";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);//文件名
	for (int i = 0; i < files.count(); i++)
	{
		QString str = path + files.at(i);
		FileList << str;
	}
	files.clear();
	nameFilters.clear();
	return FileList;
}

long CClsHDF4Operator::getCycleDepthData(long CycleID, long *CycleIndex, long *DepthData,long mDepthIndex, long mRows, long mCols, long *mStartID, long *mEndID, double mFillValue)
{
	//PRES范围初始值，有文献指出800-1500也属于,有待完善
	long mMaxValue = 0, mMinValue = 0;
	switch (mDepthIndex)
	{
	case 0: {
		mMaxValue = 150;
		mMinValue = 50;
		break;
	}
	case 1: {
		mMaxValue = 600;
		mMinValue = 400;
		break;
	}
	case 2: {
		mMaxValue = 900;
		mMinValue = 700;
		break;
	}
	case 3: {
		mMaxValue = 1100;
		mMinValue = 900;
		break;
	}
	case 4: {
		mMaxValue = 1600;
		mMinValue = 1400;
		break;
	}
	case 5: {
		mMaxValue = 2100;
		mMinValue = 1900;
		break;
	}
	case 6: {
		long datacount = 0;
		for (int i = 0; i < mRows*mCols; i++)
		{
			double tempvalue = DepthData[i];
			if (CycleIndex[i] == CycleID && DepthData[i] != mFillValue && !std::isnan(tempvalue))
			{
				datacount++;
			}
		}
		if (datacount == 0)
			return mFillValue;
		long *MaxMin = new long[2];
		long *Data = new long[datacount];
		datacount = 0;
		for (int i = 0; i < mRows*mCols; i++)
		{
			double tempvalue = DepthData[i];
			if (CycleIndex[i] == CycleID && DepthData[i] != mFillValue && !std::isnan(tempvalue))
			{
				Data[datacount] = DepthData[i];
				datacount++;
			}
		}
		MaxMin = getMaxandMinFromPRES(Data, datacount);
		mMaxValue = MaxMin[0];
		mMinValue = MaxMin[1];
		delete[] MaxMin;
		delete[] Data;
		break;
	}
	default:
		break;
	}
	/////////
	long Depth = 0, count = 0;
	for (int i = 0; i < mRows*mCols; i++)
	{
		if (CycleIndex[i] == CycleID)
		{
			if (DepthData[i] >= mMinValue && DepthData[i] <= mMaxValue)
			{
				//记录起始ID
				if (i <= *mStartID)
					*mStartID = i;
				if (i >= *mEndID)
					*mEndID = i;

				count++;
				continue;
			}
			if (count != 0 && DepthData[i] != mFillValue)
				break;
		}
		else
			continue;
	}
	//一个都没有
	if (count == 0)
		return mFillValue;
	//只有一个
	if (*mStartID == *mEndID)
		return DepthData[*mStartID];

	//n个
	count = 0;
	for (int i = *mStartID; i <= *mEndID; i++)
	{
		if (DepthData[i] != mFillValue)
		{
			count++;
			Depth += DepthData[i];
		}
	}
	return Depth / count;
}

bool CClsHDF4Operator::getCycleLonLat(long *CycleIndex, long CycleID, double *LonData, double *LatData, long mStartID, long mEndID, double mFillValue, long mRows, long mCols, double *LatStart, double *LatEnd, double *LongStart, double *LongEnd)
{
	//往外推经纬度时只能推一个周期内的
	int check = 0;
	//获取起始经纬度

	//if (mStartID == 0)
	//	mStartID++;
	//for (int i = mStartID - 1; i > -1; i--)
	//{
	//	if (LonData[i] != mFillValue && LatData[i] != mFillValue && CycleIndex[i] == CycleID)
	//	{
	//		*LongStart = LonData[i];
	//		*LatStart = LatData[i];
	//		check++;
	//		break;
	//	}
	//}
	//if (check == 0)
	//{
	//	if (CycleID == 0)
	//	{
	//		*LongStart = mFillValue;
	//		*LatStart = mFillValue;
	//		*LongEnd = mFillValue;
	//		*LatEnd = mFillValue;
	//		return false;
	//	}
	//	for (int i = 0; i < mRows*mCols - 1; i++)
	//	{
	//		if (CycleIndex[i] == CycleID - 1 && CycleIndex[i + 1] != CycleID - 1)
	//		{
	//			for (int j = i; j > -1; j--)
	//			{
	//				if (LonData[j] != mFillValue && LatData[j] != mFillValue && CycleIndex[j] == CycleID - 1)
	//				{
	//					*LongStart = LonData[j];
	//					*LatStart = LatData[j];
	//					check++;
	//					break;
	//				}
	//				//没找到时，提前跳出循环
	//				if (CycleIndex[j] != CycleID - 1)
	//					break;
	//			}
	//			if (check == 0)
	//			{
	//				*LongStart = mFillValue;
	//				*LatStart = mFillValue;
	//				*LongEnd = mFillValue;
	//				*LatEnd = mFillValue;
	//				return false;
	//			}
	//			break;
	//		}
	//	}
	//	if (check == 0)
	//	{
	//		*LongStart = mFillValue;
	//		*LatStart = mFillValue;
	//		*LongEnd = mFillValue;
	//		*LatEnd = mFillValue;
	//		return false;
	//	}
	//}

	if (CycleID == 0)
	{
		*LongStart = mFillValue;
		*LatStart = mFillValue;
		*LongEnd = mFillValue;
		*LatEnd = mFillValue;
		return false;
	}
	for (int i = 0; i < mRows*mCols - 1; i++)
	{
		if (CycleIndex[i] == CycleID - 1 && CycleIndex[i + 1] != CycleID - 1)
		{
			for (int j = i; j > -1; j--)
			{
				if (LonData[j] != mFillValue && LatData[j] != mFillValue && CycleIndex[j] == CycleID - 1)
				{
					*LongStart = LonData[j];
					*LatStart = LatData[j];
					check++;
					break;
				}
				//没找到时，提前跳出循环
				if (CycleIndex[j] != CycleID - 1)
					break;
			}
			if (check == 0)
			{
				*LongStart = mFillValue;
				*LatStart = mFillValue;
				*LongEnd = mFillValue;
				*LatEnd = mFillValue;
				return false;
			}
			break;
		}
	}
	if (std::isnan(*LongStart) || std::isnan(*LatStart))
	{
		*LongStart = mFillValue;
		*LatStart = mFillValue;
		*LongEnd = mFillValue;
		*LatEnd = mFillValue;
		return false;
	}
	
	//获取终止经纬度

	//if (mEndID == mRows*mCols - 1)
	//	mEndID--;
	//for (int i = mEndID + 1; i < mRows*mCols; i++)
	//{
	//	if (LonData[i] != mFillValue && LatData[i] != mFillValue && CycleIndex[i] == CycleID)
	//	{
	//		*LongEnd = LonData[i];
	//		*LatEnd = LatData[i];
	//		check++;
	//		break;
	//	}
	//}
	//if (check == 1)
	//{
	//	long maxID = -1;
	//	for (int i = 0; i < mRows*mCols; i++)
	//	{
	//		if (CycleIndex[i] > maxID)
	//			maxID = CycleIndex[i];
	//	}
	//	if (CycleID == maxID)
	//	{
	//		*LongStart = mFillValue;
	//		*LatStart = mFillValue;
	//		*LongEnd = mFillValue;
	//		*LatEnd = mFillValue;
	//		return false;
	//	}
	//	for (int i = 0; i < mRows*mCols - 1; i++)
	//	{
	//		if (CycleIndex[i] != CycleID + 1 && CycleIndex[i + 1] == CycleID + 1)
	//		{
	//			for (int j = i + 1; j < mRows*mCols; j++)
	//			{
	//				if (LonData[j] != mFillValue && LatData[j] != mFillValue && CycleIndex[j] == CycleID + 1)
	//				{
	//					*LongEnd = LonData[j];
	//					*LatEnd = LatData[j];
	//					check++;
	//					break;
	//				}
	//				//没找到时，提前跳出循环
	//				if (CycleIndex[j] != CycleID + 1)
	//					break;
	//			}
	//			if (check != 2)
	//			{
	//				*LongStart = mFillValue;
	//				*LatStart = mFillValue;
	//				*LongEnd = mFillValue;
	//				*LatEnd = mFillValue;
	//				return false;
	//			}
	//			break;
	//		}
	//	}
	//}

	long maxID = -1;
	for (int i = 0; i < mRows*mCols; i++)
	{
		if (CycleIndex[i] > maxID)
			maxID = CycleIndex[i];
	}
	if (CycleID == maxID)
	{
		*LongStart = mFillValue;
		*LatStart = mFillValue;
		*LongEnd = mFillValue;
		*LatEnd = mFillValue;
		return false;
	}
	for (int i = 0; i < mRows*mCols - 1; i++)
	{
		if (CycleIndex[i] != CycleID + 1 && CycleIndex[i + 1] == CycleID + 1)
		{
			for (int j = i + 1; j < mRows*mCols; j++)
			{
				if (LonData[j] != mFillValue && LatData[j] != mFillValue && CycleIndex[j] == CycleID + 1)
				{
					*LongEnd = LonData[j];
					*LatEnd = LatData[j];
					check++;
					break;
				}
				//没找到时，提前跳出循环
				if (CycleIndex[j] != CycleID + 1)
					break;
			}
			if (check != 2)
			{
				*LongStart = mFillValue;
				*LatStart = mFillValue;
				*LongEnd = mFillValue;
				*LatEnd = mFillValue;
				return false;
			}
			break;
		}
	}
	if (std::isnan(*LongEnd) || std::isnan(*LatEnd))
	{
		*LongStart = mFillValue;
		*LatStart = mFillValue;
		*LongEnd = mFillValue;
		*LatEnd = mFillValue;
		return false;
	}
	return true;
}

bool CClsHDF4Operator::getCycleYMDandDurTime(long *CycleIndex, long CycleID, double *TimeData, long mStartID, long mEndID, double mFillValue, long mRows, long mCols, long *Year, long *Month, long *Day, double *DurTime)
{
	//往外推得的时间点
	double start = 0, end = 0;
	//double startTimePosi = 0, endTimePosi = 0;//位于mStartID，mEndID上的时间点
	//if (TimeData[mStartID] != mFillValue)
	//	startTimePosi = TimeData[mStartID];
	//if (TimeData[mEndID] != mFillValue)
	//	endTimePosi = TimeData[mEndID];

	//获取起始时间

	//if (mStartID == 0)
	//	mStartID++;
	//for (int i = mStartID - 1; i > -1; i--)
	//{
	//	if (TimeData[i] != mFillValue && CycleIndex[i] == CycleID)
	//	{
	//		start = TimeData[i];
	//		break;
	//	}
	//}
	//if (start == 0)
	//{
	//	if (CycleID == 0)
	//	{
	//		*DurTime = mFillValue;
	//		*Year = mFillValue;
	//		*Month = mFillValue;
	//		*Day = mFillValue;
	//		return false;
	//	}
	//	int serchID = CycleID;
	//	while (1)
	//	{
	//		serchID--;
	//		if (serchID < 0)
	//		{
	//			*DurTime = mFillValue;
	//			*Year = mFillValue;
	//			*Month = mFillValue;
	//			*Day = mFillValue;
	//			return false;
	//		}
	//		for (int i = 0; i < mRows*mCols - 1; i++)
	//		{
	//			if (CycleIndex[i] == serchID && CycleIndex[i + 1] != serchID)
	//			{
	//				for (int j = i; j > -1; j--)
	//				{
	//					if (TimeData[j] != mFillValue && CycleIndex[j] == serchID)
	//					{
	//						start = TimeData[j];
	//						break;
	//					}
	//					//没找到时，提前跳出循环
	//					if (CycleIndex[j] != serchID)
	//						break;
	//				}
	//				break;
	//			}
	//		}
	//		if (start != 0)
	//			break;
	//	}
	//}

	if (CycleID == 0)
	{
		*DurTime = mFillValue;
		*Year = mFillValue;
		*Month = mFillValue;
		*Day = mFillValue;
		return false;
	}
	int serchID = CycleID;
	serchID--;
	for (int i = 0; i < mRows*mCols - 1; i++)
	{
		if (CycleIndex[i] == serchID && CycleIndex[i + 1] != serchID)
		{
			for (int j = i; j > -1; j--)
			{
				if (TimeData[j] != mFillValue && CycleIndex[j] == serchID)
				{
					start = TimeData[j];
					break;
				}
				//没找到时，提前跳出循环
				if (CycleIndex[j] != serchID)
					break;
			}
			break;
		}
	}
	if (std::isnan(start) || start == 0)
	{
		*DurTime = mFillValue;
		*Year = mFillValue;
		*Month = mFillValue;
		*Day = mFillValue;
		return false;
	}

	//时间约束条件startTimePosi - start < 1
	//if (startTimePosi != 0)
	//{
	//	if (startTimePosi - start > 1)
	//	{
	//		*DurTime = mFillValue;
	//		*Year = mFillValue;
	//		*Month = mFillValue;
	//		*Day = mFillValue;
	//		return false;
	//	}
	//}

	//获取终止时间

	//if (mEndID == mRows*mCols - 1)
	//	mEndID--;
	//for (int i = mEndID + 1; i < mRows*mCols; i++)
	//{
	//	if (TimeData[i] != mFillValue && CycleIndex[i] == CycleID)
	//	{
	//		end = TimeData[i];
	//		break;
	//	}
	//}
	//if (end == 0)
	//{
	//	long maxID = -1;
	//	for (int i = 0; i < mRows*mCols; i++)
	//	{
	//		if (CycleIndex[i] > maxID)
	//			maxID = CycleIndex[i];
	//	}
	//	if (CycleID == maxID)
	//	{
	//		*DurTime = mFillValue;
	//		*Year = mFillValue;
	//		*Month = mFillValue;
	//		*Day = mFillValue;
	//		return false;
	//	}
	//	int serchID = CycleID;
	//	serchID++;
	//	for (int i = 0; i < mRows*mCols - 1; i++)
	//	{
	//		if (CycleIndex[i] != serchID && CycleIndex[i + 1] == serchID)
	//		{
	//			for (int j = i + 1; j < mRows*mCols; j++)
	//			{
	//				if (TimeData[j] != mFillValue && CycleIndex[j] == serchID)
	//				{
	//					end = TimeData[j];
	//					break;
	//				}
	//				//没找到时，提前跳出循环
	//				if (CycleIndex[j] != serchID)
	//					break;
	//			}
	//			break;
	//		}
	//	}
	//}
	long maxID = -1;
	for (int i = 0; i < mRows*mCols; i++)
	{
		if (CycleIndex[i] > maxID)
			maxID = CycleIndex[i];
	}
	if (CycleID == maxID)
	{
		*DurTime = mFillValue;
		*Year = mFillValue;
		*Month = mFillValue;
		*Day = mFillValue;
		return false;
	}
	serchID = CycleID;
	serchID++;
	for (int i = 0; i < mRows*mCols - 1; i++)
	{
		if (CycleIndex[i] != serchID && CycleIndex[i + 1] == serchID)
		{
			for (int j = i + 1; j < mRows*mCols; j++)
			{
				if (TimeData[j] != mFillValue && CycleIndex[j] == serchID)
				{
					end = TimeData[j];
					break;
				}
				//没找到时，提前跳出循环
				if (CycleIndex[j] != serchID)
					break;
			}
			break;
		}
	}
	if (std::isnan(end) || end == 0)
	{
		*DurTime = mFillValue;
		*Year = mFillValue;
		*Month = mFillValue;
		*Day = mFillValue;
		return false;
	}

	//时间约束条件startTimePosi - start < 1
	//if (endTimePosi != 0)
	//{
	//	if (end - endTimePosi > 1)
	//	{
	//		*DurTime = mFillValue;
	//		*Year = mFillValue;
	//		*Month = mFillValue;
	//		*Day = mFillValue;
	//		return false;
	//	}
	//}
	*DurTime = end - start;

	if (*DurTime <= 0 || start == 0/* || *DurTime >= 15*/)
	{
		*DurTime = mFillValue;
		*Year = mFillValue;
		*Month = mFillValue;
		*Day = mFillValue;
		return false;
	}

	//计算起始时间YMD
	int year[2] = { 365,366 };
	int month[2][12] = { 31,28,31,30,31,30,31,31,30,31,30,31,\
		31,29,31,30,31,30,31,31,30,31,30,31 };
	//int i;
	double Days = start;
	
	for (int i = 1950; Days >= year[CClsGeneralOperator::LeapYear(i)]; i++)
	{
		Days -= year[CClsGeneralOperator::LeapYear(i)];
		*Year = i + 1;
	}
	//double dd = Days;
	int j = 0;
	for (j = 0; Days >= month[CClsGeneralOperator::LeapYear(*Year)][j]; j++)
	{
		Days -= month[CClsGeneralOperator::LeapYear(*Year)][j];
	}
	*Month = j + 1;
	*Day = ceil(Days);
	delete year;
	for (int i = 0; i < 2; i++) {
		delete month[i];
	}
	delete month;
	return true;
}

bool CClsHDF4Operator::getCycleTempandPasl(double *temp, double *psal, double *ProfDepth, long Depth, double mProfRows, double mProfCols, double *Temp, double *Salinity, double mProfFillValue)
{
	long mColID = 0;
	long min = 100;
	bool checkin = false;
	for (int i = 0; i < mProfRows*mProfCols; i++)
	{
		long mProDepth = ProfDepth[i];
		long absvalue = abs(mProDepth - Depth);
		if (absvalue < min)
		{
			min = absvalue;
			mColID = i;
			checkin = true;
			continue;
		}
	}
	if (!checkin)
	{
		*Temp = mProfFillValue;
		*Salinity = mProfFillValue;
		return false;
	}

	if (temp[mColID] != mProfFillValue)
	{
		*Temp = temp[mColID];
	}
	else
	{
		bool check = false;
		for (int i = mColID - 2; i <= mColID + 2; i++)
		{
			if (i<0 || i>mProfRows*mProfCols - 1)
				continue;

			if (temp[i] != mProfFillValue)
			{
				*Temp = temp[i];
				check = true;
				break;
			}
		}
		if (!check || _isnan(*Temp))
		{
			*Temp = mProfFillValue;
			//*Salinity = mProfFillValue;
			//return false;
		}

	}
	//////////////
	if (psal[mColID] != mProfFillValue)
	{
		*Salinity = psal[mColID];
	}
	else
	{
		bool check = false;
		for (int i = mColID - 2; i <= mColID + 2; i++)
		{
			if (i<0 || i>mProfRows*mProfCols - 1)
				continue;

			if (psal[i] != mProfFillValue)
			{
				*Salinity = psal[i];
				check = true;
				break;
			}
		}
		if (!check || _isnan(*Salinity))
		{
			//*Temp = mProfFillValue;
			*Salinity = mProfFillValue;
			//return false;
		}
	}

	return true;
}

bool CClsHDF4Operator::WriteHDFFileFromCSV(CString FileName, CString mDsName, long *pBuffer, long mRows, long mCols, QString mDsDate, double mSpatialRes)
{
	double *Max_Min = new double[2];
	Max_Min = CClsGeneralOperator::GetMin_Max(Max_Min, pBuffer, mRows, mCols, -9999);
	double mMaxValue = Max_Min[1];
	double mMinValue = Max_Min[0];
	double mMeanValue = CClsGeneralOperator::GetMeanValue(pBuffer, mRows, mCols, -9999);
	double mStdValue = CClsGeneralOperator::GetStdValue(pBuffer, mRows, mCols, -9999);
	if (!CClsHDF4Operator::WriteCustomHDF2DFile(FileName, CClsGeneralOperator::QStrToCStr(mDsDate), "Product", "0", mDsName, pBuffer, 0.001, 0, 0, 360, -90, 90, mRows, mCols, mMaxValue, mMinValue, mMeanValue, mStdValue, -9999, mSpatialRes, "2维"))
	{
		//释放内存
		free(pBuffer);
		//QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("输出文件失败!"));
		return false;
	}
	return true;
}

long* CClsHDF4Operator::getMaxandMinFromPRES(long *DepthData, long DataNum)
{
	long *MaxMin = new long[2];
	if (DataNum == 1)
	{
		MaxMin[0] = DepthData[0];
		MaxMin[1] = DepthData[0];
		return MaxMin;
	}
	//////
	long max = -9999, min = 100000;
	double mean = 0;
	for (int i = 0; i < DataNum; i++)
	{
		mean += DepthData[i];
		if (DepthData[i] > max)
			max = DepthData[i];
		if (DepthData[i] < min)
			min = DepthData[i];
	}
	if (max - min <= 50)
	{
		MaxMin[0] = max;
		MaxMin[1] = min;
		return MaxMin;
	}
	mean = mean / DataNum;
	long deletedata = 0;
	if (abs(max - mean) > abs(min - mean))
		deletedata = max;
	else
		deletedata = min;
	long *Data = new long[DataNum - 1];
	bool check = true;
	long count = 0;
	for (int i = 0; i < DataNum; i++)
	{
		if (DepthData[i] == deletedata && check)
		{
			check = false;
		}
		else
		{
			Data[count] = DepthData[i];
			count++;
		}
	}
	for (int i = 0; i < DataNum - 1; i++) {
		DepthData[i] = Data[i];
	}
	delete[] Data;
	getMaxandMinFromPRES(DepthData, DataNum - 1);
}

//快速排序法，从小到大
void CClsHDF4Operator::quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData, double *LatData, double mJuldFillValue)
{
	//如果区间不只一个数
	if (begin < end)
	{
		double temp = TimeData[begin]; //将区间的第一个数作为基准数
		for (int index = begin; index < end; index++)
		{
			if (TimeData[index] != mJuldFillValue && !_isnan(TimeData[index]))
			{
				temp = TimeData[index];
				begin = index;
				break;
			}
			if (index == end - 1)
				return;
		}
		
		int i = begin; //从左到右进行查找时的“指针”，指示当前左位置
		int j = end; //从右到左进行查找时的“指针”，指示当前右位置
		//不重复遍历
		while (i < j)
		{
			//当右边的数大于基准数时，略过，继续向左查找
			//不满足条件时跳出循环，此时的j对应的元素是小于基准元素的
		RDebarTimeFillValue:
			while (i<j && TimeData[j] > temp)
				j--;
			if (_isnan(TimeData[j]) && i < j)
			{
				j--;
				goto RDebarTimeFillValue;
			}
			//将右边小于等于基准元素的数填入右边相应位置
			TimeData[i] = TimeData[j];
			CycleIndex[i] = CycleIndex[j];
			DepthData[i] = DepthData[j];
			LonData[i] = LonData[j];
			LatData[i] = LatData[j];
			//当左边的数小于等于基准数时，略过，继续向右查找
			//(重复的基准元素集合到左区间)
			//不满足条件时跳出循环，此时的i对应的元素是大于等于基准元素的
		LDebarTimeFillValue:
			while (i<j && TimeData[i] <= temp)
				i++;
			//将左边大于基准元素的数填入左边相应位置
			if ((TimeData[i] == mJuldFillValue || _isnan(TimeData[i])) && i < j)
			{
				i++;
				goto LDebarTimeFillValue;
			}
			TimeData[j] = TimeData[i];
			CycleIndex[j] = CycleIndex[i];
			DepthData[j] = DepthData[i];
			LonData[j] = LonData[i];
			LatData[j] = LatData[i];
		}
		//将基准元素填入相应位置
		TimeData[i] = temp;
		//此时的i即为基准元素的位置
		//对基准元素的左边子区间进行相似的快速排序
		quickSortForArgo(TimeData, begin, i - 1, CycleIndex, DepthData, LonData, LatData, mJuldFillValue);
		//对基准元素的右边子区间进行相似的快速排序
		quickSortForArgo(TimeData, i + 1, end, CycleIndex, DepthData, LonData, LatData, mJuldFillValue);
	}
	//如果区间只有一个数，则返回
	else
		return;
}

void CClsHDF4Operator::quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue)
{
	//如果区间不只一个数
	if (begin < end)
	{
		long temp = CycleIndex[begin]; //将区间的第一个数作为基准数
		for (int index = begin; index < end; index++)
		{
			if (CycleIndex[index] != mFillValue && !_isnan(CycleIndex[index]))
			{
				temp = CycleIndex[index];
				begin = index;
				break;
			}
			if (index == end - 1)
				return;
		}

		int i = begin; //从左到右进行查找时的“指针”，指示当前左位置
		int j = end; //从右到左进行查找时的“指针”，指示当前右位置
		//不重复遍历
		while (i < j)
		{
		//当右边的数大于基准数时，略过，继续向左查找
		//不满足条件时跳出循环，此时的j对应的元素是小于基准元素的
		RDebarTimeFillValue:
			while (i<j && CycleIndex[j] > temp)
				j--;
			if (_isnan(CycleIndex[j]) && i < j)
			{
				j--;
				goto RDebarTimeFillValue;
			}
			//将右边小于等于基准元素的数填入右边相应位置
			//TimeData[i] = CycleIndex[j];
			CycleIndex[i] = CycleIndex[j];
			DepthData[i] = DepthData[j];

			//当左边的数小于等于基准数时，略过，继续向右查找
			//(重复的基准元素集合到左区间)
			//不满足条件时跳出循环，此时的i对应的元素是大于等于基准元素的
		LDebarTimeFillValue:
			while (i<j && CycleIndex[i] <= temp)
				i++;
			//将左边大于基准元素的数填入左边相应位置
			if ((CycleIndex[i] == mFillValue || _isnan(CycleIndex[i])) && i < j)
			{
				i++;
				goto LDebarTimeFillValue;
			}
			CycleIndex[j] = CycleIndex[i];
			DepthData[j] = DepthData[i];

		}
		//将基准元素填入相应位置
		CycleIndex[i] = temp;
		//此时的i即为基准元素的位置
		//对基准元素的左边子区间进行相似的快速排序
		quickSortForArgo(begin, i - 1, CycleIndex, DepthData,mFillValue);
		//对基准元素的右边子区间进行相似的快速排序
		quickSortForArgo(i + 1, end, CycleIndex, DepthData,mFillValue);
	}
	//如果区间只有一个数，则返回
	else
		return;
}

void CClsHDF4Operator::quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData, double mJuldFillValue)
{
	//如果区间不只一个数
	if (begin < end)
	{
		double temp = TimeData[begin]; //将区间的第一个数作为基准数
		for (int index = begin; index < end; index++)
		{
			if (TimeData[index] != mJuldFillValue && !_isnan(TimeData[index]))
			{
				temp = TimeData[index];
				begin = index;
				break;
			}
			if (index == end - 1)
				return;
		}

		int i = begin; //从左到右进行查找时的“指针”，指示当前左位置
		int j = end; //从右到左进行查找时的“指针”，指示当前右位置
					 //不重复遍历
		while (i < j)
		{
			//当右边的数大于基准数时，略过，继续向左查找
			//不满足条件时跳出循环，此时的j对应的元素是小于基准元素的
		RDebarTimeFillValue:
			while (i<j && TimeData[j] > temp)
				j--;
			if (_isnan(TimeData[j]) && i < j)
			{
				j--;
				goto RDebarTimeFillValue;
			}
			//将右边小于等于基准元素的数填入右边相应位置
			TimeData[i] = TimeData[j];
			LonData[i] = LonData[j];
			LatData[i] = LatData[j];
			//当左边的数小于等于基准数时，略过，继续向右查找
			//(重复的基准元素集合到左区间)
			//不满足条件时跳出循环，此时的i对应的元素是大于等于基准元素的
		LDebarTimeFillValue:
			while (i<j && TimeData[i] <= temp)
				i++;
			//将左边大于基准元素的数填入左边相应位置
			if ((TimeData[i] == mJuldFillValue || _isnan(TimeData[i])) && i < j)
			{
				i++;
				goto LDebarTimeFillValue;
			}
			TimeData[j] = TimeData[i];
			LonData[j] = LonData[i];
			LatData[j] = LatData[i];
		}
		//将基准元素填入相应位置
		TimeData[i] = temp;
		//此时的i即为基准元素的位置
		//对基准元素的左边子区间进行相似的快速排序
		quickSortForArgo(TimeData, begin, i - 1, LonData, LatData, mJuldFillValue);
		//对基准元素的右边子区间进行相似的快速排序
		quickSortForArgo(TimeData, i + 1, end, LonData, LatData, mJuldFillValue);
	}
	//如果区间只有一个数，则返回
	else
		return;
}

