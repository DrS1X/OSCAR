#include "opt.h"
#include "_const.h"
#include <math.h>
#include "hdf.h"
#include "mfhdf.h"
#include <string>
#include <vector>
#include <float.h>


hdfOpt::hdfOpt() {};

hdfOpt::hdfOpt(string m_MarineParameterName, int m_DatesetsType) {
    this->m_MarineParameterName = m_MarineParameterName;
    this->m_DatesetsType = m_DatesetsType;
}

hdfOpt::hdfOpt(Meta meta) {
    this->meta = meta;
}

Meta hdfOpt::getHDFMeta(string templateFile) {
    Meta meta;

    string strTemp = templateFile;
    string tName = strTemp.c_str();

    hdfOpt pReadHDF;

    string DsName = Def.DataSetName.c_str();

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

bool hdfOpt::meanAndStandardDeviation(vector<string> Files, double *pMeanBuffer, double *pStdBuffer) {
    int FileNum = Files.size();
    hdfOpt ReadHDF;
    string templateFile = Files[0].c_str();
    string mDsName = Def.DataSetName.c_str();
    long mRows = ReadHDF.GetDatasetsRows(templateFile, mDsName);
    long mCols = ReadHDF.GetDatasetsCols(templateFile, mDsName);
    double mMissingValue = ReadHDF.GetDataSetsMissingValue(templateFile, mDsName);
    double mScale = ReadHDF.GetDataSetsScale(templateFile, mDsName);

    for (long j = 0; j < mRows * mCols; j++) {
        pMeanBuffer[j] = 0.0;
        pStdBuffer[j] = 0.0;
    }

    int *pValueNum = new int[mRows * mCols];
    for (long i = 0; i < mRows * mCols; ++i)
        pValueNum[i] = 0;

    //?????????
    for (long i = 0; i < FileNum; i++) {
        string tStr = Files[i].c_str();
        //???建????,?洢??????
        long *pBuffer = NULL;
        pBuffer = (long *) malloc(mRows * mCols * sizeof(long));

        //???????
        if (!ReadHDF.GetDsByDsnameFROMProduct(pBuffer, tStr, mDsName, 0, mRows, 0, mCols)) {
            //??????
            free(pBuffer);
            return false;
        }

        //int serialNum = opt::getDayOfYear(Files[i]);
        //????
        for (long j = 0; j < mRows * mCols; j++) {
            if (pBuffer[j] != (long) mMissingValue) {
                pMeanBuffer[j] += ((double) pBuffer[j] * mScale);
                pStdBuffer[j] += pow(((double) pBuffer[j] * mScale), 2.0);
                pValueNum[j] += 1;
            }
        }

        //??????
        free(pBuffer);
    }

    //?????????????\?????????
    for (long j = 0; j < mRows * mCols; j++) {
        if (pValueNum[j] != 0) {
            pMeanBuffer[j] = ((double) pMeanBuffer[j] / pValueNum[j]);  //  ???????
            double tV1 = (double) pStdBuffer[j] / pValueNum[j];
            double tV2 = pow(pMeanBuffer[j], 2.0);
            if (tV1 >= tV2)
                pStdBuffer[j] = sqrt(tV1 - tV2);
            else
                pStdBuffer[j] = 0.0;
        } else {
            pMeanBuffer[j] = 0.0;
            pStdBuffer[j] = 0.0;
        }
    }
    delete[] pValueNum;
    return true;
}

bool hdfOpt::writeHDF(string Filename, Meta meta, long *pBuffer) {
    double *Max_Min = new double[2];
    Max_Min = opt::GetMin_Max(Max_Min, pBuffer, meta.Rows, meta.Cols);
    double mMaxValue = Max_Min[1];
    double mMinValue = Max_Min[0];
    double mMeanValue = opt::GetMeanValue(pBuffer, meta.Rows, meta.Cols);
    double mStdValue = opt::GetStdValue(pBuffer, meta.Rows, meta.Cols);
    delete Max_Min;

    string Date = meta.Date.c_str();

    if (!this->WriteCustomHDF2DFile(Filename.c_str(), Date, Def.ProductType.c_str(), "0",
                                    Def.DataSetName.c_str(), pBuffer, meta.Scale, meta.Offset, meta.StartLog,
                                    meta.EndLog, meta.StartLat, meta.EndLat,
                                    meta.Rows, meta.Cols, mMaxValue, mMinValue, mMeanValue, mStdValue,
                                    meta.MissingValue, meta.Resolution, "2?"))
        return false;

    return true;
}

bool hdfOpt::writeHDF(string Filename, Meta meta, int *buf) {
    double sum = 0;
    //???????????С?
    double MaxValue = DBL_MIN, MinValue = DBL_MAX;
    int count = 0;
    for (int j = 0; j < meta.Size; j++) {
        if (buf[j] > MaxValue)
            MaxValue = buf[j];
        if (buf[j] < MinValue)
            MinValue = buf[j];
        sum += buf[j];
        count++;
    }
    //???????
    double MeanValue = sum / count;
    sum = 0;
    for (int j = 0; j < meta.Size; j++) {
        sum += ((double) buf[j] - MeanValue) * ((double) buf[j] - MeanValue);
    }
    //???????
    double StdValue = sqrt(sum / count);

    string Date = meta.Date.c_str();

    if (!this->WriteCustomHDF2DFile(Filename.c_str(), Date, Def.ProductType.c_str(), "0",
                                    Def.DataSetName.c_str(), buf, meta.Scale, meta.Offset, meta.StartLog, meta.EndLog,
                                    meta.StartLat, meta.EndLat,
                                    meta.Rows, meta.Cols, MaxValue, MinValue, MeanValue, StdValue, meta.MissingValue,
                                    meta.Resolution, "2?"))
        return false;

    return true;
}

bool hdfOpt::readHDF(string fileName, long *buf) {
    return this->GetDsByDsnameFROMProduct(buf, fileName.c_str(), Def.DataSetName.c_str(), 0, Def.Rows, 0, Def.Cols);
}

bool hdfOpt::readHDF(string fileName, int *buf) {
    //????????id
    const char *lpsz = fileName.c_str();
    int fid = SDstart(lpsz, DFACC_READ);

    //???????
    const char *tagChar = Def.DataSetName.c_str();
    int32 tagIndex = SDnametoindex(fid, tagChar);

    //??id?л??????????????id??
    int sid = SDselect(fid, tagIndex);
    //?????????????? ?????????? ????????С ??????????????? ???????????????????
    status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);
    int a = datatype;
    int b = rank;
    int c = attrnum;
    long t = 0;
    //???????
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    endge[0] = Def.Rows;
    endge[1] = Def.Cols;
    endge[2] = 1;

    if (datatype != 24) {
        cout << "The datatype of file is not int32!" << endl;
        return false;
    }

    status = SDreaddata(sid, start, NULL, endge, buf);

    status = SDendaccess(sid);
    status = SDend(fid);

    return true;
}

BOOL hdfOpt::WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                                  string m_DsName, int *pBuffer, double m_Scale, double m_Offset, double m_StartLog,
                                  double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows, long m_Cols,
                                  double m_MaxValue, double m_MinValue, double m_MeanValue, double m_StdValue,
                                  long m_FillValue, double m_Resolution, string m_Dimension) {
    int fid = SDstart(Filename.c_str(), DFACC_CREATE);
    if (fid == -1)return false;

    int32 startWrite[2], dimesizeWrite[2];

    //?????????id
    dimesizeWrite[0] = m_Rows;
    dimesizeWrite[1] = m_Cols;

    const char *tagChar = m_DsName.c_str();
    int sid = SDcreate(fid, tagChar, DFNT_INT32, 2, dimesizeWrite); //????????
    if (sid == -1) {
        //????hdf???
        status = SDend(fid);
        return false;
    }

    //д???????
    startWrite[0] = 0;
    startWrite[1] = 0;

    status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP) pBuffer);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д?????????
    //?????????
    tagChar = m_ImageDate.c_str();
    status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????
    tagChar = m_DataType.c_str();
    status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????????
    tagChar = "Product";
    status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????
    tagChar = m_Dimension.c_str();
    status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д????????????????
    //???????????
    status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    double mOffsets = 0.00;
    status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????γ??
    status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????????????
    status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????
    status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????????С?
    /*double minValue, maxValue;
    double  *tempValue = new double[2];
    tempValue = opt::GetMin_Max(pBuffer, m_Rows, m_Cols);*/
    //minValue = tempValue[0]; m_MaxValue = tempValue[1];

    status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //???????????????С?
    status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //д????????????
    status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????hdf???
    status = SDendaccess(sid);
    status = SDend(fid);
    return true;
}

BOOL hdfOpt::WriteCustomHDF2DFile(string Filename, string m_ImageDate,
                                  string m_ProductType, string m_DataType,
                                  string m_DsName, long *pBuffer, double m_Scale,
                                  double m_Offset, double m_StartLog, double m_EndLog,
                                  double m_StartLat, double m_EndLat, long m_Rows,
                                  long m_Cols, double m_MaxValue, double m_MinValue,
                                  double m_MeanValue, double m_StdValue, long m_FillValue,
                                  double m_Resolution, string m_Dimension) {
    //???????     DFNT_CHAR8
    //???????     DFNT_CHAR8
    //????????     DFNT_CHAR8
    //γ??	       DFNT_CHAR8
    //???????     DFNT_INT32
    //???????     DFNT_FLOAT64
    //???????     DFNT_FLOAT64
    //???????     DFNT_FLOAT64
    //???????     DFNT_FLOAT64
    //???γ??     DFNT_FLOAT64
    //???γ??     DFNT_FLOAT64
    //????	       DFNT_UINT16,
    //????	       DFNT_UINT16
    //????       DFNT_FLOAT64
    //??С?       DFNT_FLOAT64
    //???         DFNT_FLOAT64
    //????         DFNT_FLOAT64
    //???         DFNT_UINT16
    //???????   DFNT_ FLOAT64

    int fid = SDstart(Filename.c_str(), DFACC_CREATE);
    if (fid == -1)return false;

    int32 startWrite[2], dimesizeWrite[2];

    //?????????id
    dimesizeWrite[0] = m_Rows;
    dimesizeWrite[1] = m_Cols;

    int sid = SDcreate(fid, m_DsName.c_str(), DFNT_INT32, 2, dimesizeWrite);
    if (sid == -1) {
        //????hdf???
        status = SDend(fid);
        return false;
    }

    //д???????
    startWrite[0] = 0;
    startWrite[1] = 0;

    //??????-9999
    double dFactory = 1;// 0.001 / m_Scale;
    for (long i = 0; i < m_Rows * m_Cols; i++) {
        if (pBuffer[i] <= -9998) {
            pBuffer[i] = -9999;
        }
    }


    //????product??Scale???0.01
    //FillValue ???-9999
    //m_Scale = 0.001;
    m_FillValue = -9999;

    m_MeanValue /= dFactory;
    m_MaxValue /= dFactory;
    m_MinValue /= dFactory;
    m_StdValue /= dFactory;

    for (long i = 0; i < m_Rows * m_Cols; i++) {
        if (pBuffer[i] != m_FillValue) {
            pBuffer[i] /= dFactory;
        }
    }

    status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP) pBuffer);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д?????????
    //?????????
    const char *tagChar;
    tagChar = m_ImageDate.c_str();
    status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????
    tagChar = m_DataType.c_str();
    status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????????
    tagChar = "Product";
    status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????
    tagChar = m_Dimension.c_str();
    status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д????????????????
    //???????????
    status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????γ??
    status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????????????
    status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????
    status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????????С?
    //double minValue, maxValue;
    //double  *tempValue = new double[2];
    //tempValue = opt::GetMin_Max(pBuffer, m_Rows, m_Cols);
    //minValue = tempValue[0]; m_MaxValue = tempValue[1];

    status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //?????????????????
    status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //д????????????
    status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????hdf???
    status = SDendaccess(sid);
    status = SDend(fid);
    return true;
}

BOOL hdfOpt::WriteCustomHDF2DFile(string Filename, string m_ImageDate, string m_ProductType, string m_DataType,
                                  string m_DsName, unsigned char *pBuffer, double m_Scale, double m_Offset,
                                  double m_StartLog, double m_EndLog, double m_StartLat, double m_EndLat, long m_Rows,
                                  long m_Cols, double m_MaxValue, double m_MinValue, double m_MeanValue,
                                  double m_StdValue, long m_FillValue, double m_Resolution, string m_Dimension) {
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

    int fid = SDstart(Filename.c_str(), DFACC_CREATE);
    if (fid == -1)return false;

    int32 startWrite[2], dimesizeWrite[2];

    //?????????id
    dimesizeWrite[0] = m_Rows;
    dimesizeWrite[1] = m_Cols;

    const char *tagChar = m_DsName.c_str();
    int sid = SDcreate(fid, tagChar, DFNT_INT32, 2, dimesizeWrite);
    if (sid == -1) {
        //????hdf???
        status = SDend(fid);
        return false;
    }

    //д???????
    startWrite[0] = 0;
    startWrite[1] = 0;

    status = SDwritedata(sid, startWrite, NULL, dimesizeWrite, (VOIDP) pBuffer);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д?????????
    //?????????
    tagChar = m_ImageDate.c_str();
    status = SDsetattr(fid, "ImageDate", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????
    tagChar = m_DataType.c_str();
    status = SDsetattr(fid, "DataType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????????
    tagChar = "Product";
    status = SDsetattr(fid, "ProductType", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //??????
    tagChar = m_Dimension.c_str();
    status = SDsetattr(fid, "Dimension", DFNT_CHAR8, 50, (VOIDP) tagChar);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //д????????????????
    //???????????
    status = SDsetattr(sid, "Scale", DFNT_FLOAT64, 1, &m_Scale);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    double mOffsets = 0.00;
    status = SDsetattr(sid, "Offsets", DFNT_FLOAT64, 1, &m_Offset);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????γ??
    status = SDsetattr(sid, "StartLog", DFNT_FLOAT64, 1, &m_StartLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLog", DFNT_FLOAT64, 1, &m_EndLog);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StartLat", DFNT_FLOAT64, 1, &m_StartLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "EndLat", DFNT_FLOAT64, 1, &m_EndLat);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //?????????????
    status = SDsetattr(sid, "Rows", DFNT_UINT16, 1, &m_Rows);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "Cols", DFNT_UINT16, 1, &m_Cols);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????
    status = SDsetattr(sid, "FillValue", DFNT_INT32, 1, &m_FillValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //???????????????С?
    /*double minValue, maxValue;
    double  *tempValue = new double[2];
    tempValue = opt::GetMin_Max(pBuffer, m_Rows, m_Cols);*/
    //minValue = tempValue[0]; m_MaxValue = tempValue[1];

    status = SDsetattr(sid, "MaxValue", DFNT_FLOAT64, 1, &m_MaxValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "MinValue", DFNT_FLOAT64, 1, &m_MinValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //???????????????С?
    status = SDsetattr(sid, "MeanVaue", DFNT_FLOAT64, 1, &m_MeanValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    status = SDsetattr(sid, "StdValue", DFNT_FLOAT64, 1, &m_StdValue);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }
    //д????????????
    status = SDsetattr(sid, "DSResolution", DFNT_FLOAT64, 1, &m_Resolution);
    if (status == -1) {
        //????hdf???
        status = SDendaccess(sid);
        status = SDend(fid);
        return false;
    }

    //????hdf???
    status = SDendaccess(sid);
    status = SDend(fid);
    return true;
}

string hdfOpt::GetFileProductType(string Filename) {
    return "Product";
}

long *hdfOpt::getMaxandMinFromPRES(long *DepthData, long DataNum) {
    long *MaxMin = new long[2];
    if (DataNum == 1) {
        MaxMin[0] = DepthData[0];
        MaxMin[1] = DepthData[0];
        return MaxMin;
    }
    //////
    long max = -9999, min = 100000;
    double mean = 0;
    for (int i = 0; i < DataNum; i++) {
        mean += DepthData[i];
        if (DepthData[i] > max)
            max = DepthData[i];
        if (DepthData[i] < min)
            min = DepthData[i];
    }
    if (max - min <= 50) {
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
    for (int i = 0; i < DataNum; i++) {
        if (DepthData[i] == deletedata && check) {
            check = false;
        } else {
            Data[count] = DepthData[i];
            count++;
        }
    }
    for (int i = 0; i < DataNum - 1; i++) {
        DepthData[i] = Data[i];
    }
    delete[] Data;
    getMaxandMinFromPRES(DepthData, DataNum - 1);
    return 0;
}

//???????????С????
void
hdfOpt::quickSortForArgo(double *TimeData, long begin, long end, long *CycleIndex, long *DepthData, double *LonData,
                         double *LatData, double mJuldFillValue) {
    //?????????????
    if (begin < end) {
        double temp = TimeData[begin]; //?????????????????????
        for (int index = begin; index < end; index++) {
            if (TimeData[index] != mJuldFillValue && !_isnan(TimeData[index])) {
                temp = TimeData[index];
                begin = index;
                break;
            }
            if (index == end - 1)
                return;
        }

        int i = begin; //????????в???????????????????λ??
        int j = end; //?????????в???????????????????λ??
        //?????????
        while (i < j) {
            //???????????????????????????????????
            //?????????????????????????j??????????С????????
            RDebarTimeFillValue:
            while (i < j && TimeData[j] > temp)
                j--;
            if (_isnan(TimeData[j]) && i < j) {
                j--;
                goto RDebarTimeFillValue;
            }
            //?????С???????????????????????λ??
            TimeData[i] = TimeData[j];
            CycleIndex[i] = CycleIndex[j];
            DepthData[i] = DepthData[j];
            LonData[i] = LonData[j];
            LatData[i] = LatData[j];
            //????????С????????????????????????????
            //(????????????????????)
            //?????????????????????????i??????????????????????
            LDebarTimeFillValue:
            while (i < j && TimeData[i] <= temp)
                i++;
            //??????????????????????????λ??
            if ((TimeData[i] == mJuldFillValue || _isnan(TimeData[i])) && i < j) {
                i++;
                goto LDebarTimeFillValue;
            }
            TimeData[j] = TimeData[i];
            CycleIndex[j] = CycleIndex[i];
            DepthData[j] = DepthData[i];
            LonData[j] = LonData[i];
            LatData[j] = LatData[i];
        }
        //???????????????λ??
        TimeData[i] = temp;
        //?????i??????????λ??
        //????????????????????????????????
        quickSortForArgo(TimeData, begin, i - 1, CycleIndex, DepthData, LonData, LatData, mJuldFillValue);
        //????????????????????????????????
        quickSortForArgo(TimeData, i + 1, end, CycleIndex, DepthData, LonData, LatData, mJuldFillValue);
    }
        //????????????????????
    else
        return;
}

void hdfOpt::quickSortForArgo(long begin, long end, long *CycleIndex, long *DepthData, double mFillValue) {
    //?????????????
    if (begin < end) {
        long temp = CycleIndex[begin]; //?????????????????????
        for (int index = begin; index < end; index++) {
            if (CycleIndex[index] != mFillValue && !_isnan(CycleIndex[index])) {
                temp = CycleIndex[index];
                begin = index;
                break;
            }
            if (index == end - 1)
                return;
        }

        int i = begin; //????????в???????????????????λ??
        int j = end; //?????????в???????????????????λ??
        //?????????
        while (i < j) {
            //???????????????????????????????????
            //?????????????????????????j??????????С????????
            RDebarTimeFillValue:
            while (i < j && CycleIndex[j] > temp)
                j--;
            if (_isnan(CycleIndex[j]) && i < j) {
                j--;
                goto RDebarTimeFillValue;
            }
            //?????С???????????????????????λ??
            //TimeData[i] = CycleIndex[j];
            CycleIndex[i] = CycleIndex[j];
            DepthData[i] = DepthData[j];

            //????????С????????????????????????????
            //(????????????????????)
            //?????????????????????????i??????????????????????
            LDebarTimeFillValue:
            while (i < j && CycleIndex[i] <= temp)
                i++;
            //??????????????????????????λ??
            if ((CycleIndex[i] == mFillValue || _isnan(CycleIndex[i])) && i < j) {
                i++;
                goto LDebarTimeFillValue;
            }
            CycleIndex[j] = CycleIndex[i];
            DepthData[j] = DepthData[i];

        }
        //???????????????λ??
        CycleIndex[i] = temp;
        //?????i??????????λ??
        //????????????????????????????????
        quickSortForArgo(begin, i - 1, CycleIndex, DepthData, mFillValue);
        //????????????????????????????????
        quickSortForArgo(i + 1, end, CycleIndex, DepthData, mFillValue);
    }
        //????????????????????
    else
        return;
}

void hdfOpt::quickSortForArgo(double *TimeData, long begin, long end, double *LonData, double *LatData,
                              double mJuldFillValue) {
    //?????????????
    if (begin < end) {
        double temp = TimeData[begin]; //?????????????????????
        for (int index = begin; index < end; index++) {
            if (TimeData[index] != mJuldFillValue && !_isnan(TimeData[index])) {
                temp = TimeData[index];
                begin = index;
                break;
            }
            if (index == end - 1)
                return;
        }

        int i = begin; //????????в???????????????????λ??
        int j = end; //?????????в???????????????????λ??
        //?????????
        while (i < j) {
            //???????????????????????????????????
            //?????????????????????????j??????????С????????
            RDebarTimeFillValue:
            while (i < j && TimeData[j] > temp)
                j--;
            if (_isnan(TimeData[j]) && i < j) {
                j--;
                goto RDebarTimeFillValue;
            }
            //?????С???????????????????????λ??
            TimeData[i] = TimeData[j];
            LonData[i] = LonData[j];
            LatData[i] = LatData[j];
            //????????С????????????????????????????
            //(????????????????????)
            //?????????????????????????i??????????????????????
            LDebarTimeFillValue:
            while (i < j && TimeData[i] <= temp)
                i++;
            //??????????????????????????λ??
            if ((TimeData[i] == mJuldFillValue || _isnan(TimeData[i])) && i < j) {
                i++;
                goto LDebarTimeFillValue;
            }
            TimeData[j] = TimeData[i];
            LonData[j] = LonData[i];
            LatData[j] = LatData[i];
        }
        //???????????????λ??
        TimeData[i] = temp;
        //?????i??????????λ??
        //????????????????????????????????
        quickSortForArgo(TimeData, begin, i - 1, LonData, LatData, mJuldFillValue);
        //????????????????????????????????
        quickSortForArgo(TimeData, i + 1, end, LonData, LatData, mJuldFillValue);
    }
        //????????????????????
    else
        return;
}

void *hdfOpt::allow_memory(long datasize, int datatype) {
    switch (datatype) {
        case 3:
            return ((uchar8 *) calloc(datasize, sizeof(uchar8)));
            break;
        case 4:
            return ((char8 *) calloc(datasize, sizeof(char8)));
            break;
        case 5:
            return ((float32 *) calloc(datasize, sizeof(float32)));
            break;
        case 6:
            return ((float64 *) calloc(datasize, sizeof(float64)));
            break;
        case 20:
            return ((int8 *) calloc(datasize, sizeof(int8)));
            break;
        case 21:
            return ((uint8 *) calloc(datasize, sizeof(uint8)));
            break;
        case 22:
            return ((int16 *) calloc(datasize, sizeof(int16)));
            break;
        case 23:
            return ((uint16 *) calloc(datasize, sizeof(uint16)));
            break;
        case 24:
            return ((int32 *) calloc(datasize, sizeof(int32)));
            break;
        case 25:
            return ((uint32 *) calloc(datasize, sizeof(uint32)));
            break;
        default:
            cout << "不存在的数据类型!" << endl;
            return NULL;
            break;
    }
}

long hdfOpt::GetDatesetsNum(string Filename) {
    m_DataType = GetFileProductType(Filename);

    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //获取文件中数据集的个数 
    attrnum = 0;
    status = SDfileinfo(fid, &datasetsnum, &attrnum);

    if (datasetsnum < 0)    //文件中没有数据集 
        return false;

    status = SDend(fid);
    return datasetsnum;
}

double hdfOpt::GetDataSetsScale(string Filename, string Dsname) {
    double tValue;
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());
    int sid = SDselect(fid, tagIndex);

    m_DataType = GetFileProductType(Filename);

    const char *tagChar = "Scale";
    tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    if (datatype == 4) {
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        //tValue = _ttof(mResStr);

        tValue = atof(mResStr.c_str());
    } else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, float64_databuffer);
        tValue = float64_databuffer[0];
        free(float64_databuffer);
    }

    SDendaccess(sid);
    status = SDend(fid);

    return tValue;
}

double hdfOpt::GetDataSetsOffsets(string Filename, string Dsname) {
    double tValue;
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());
    int sid = SDselect(fid, tagIndex);

    m_DataType = GetFileProductType(Filename);


    const char *tagChar = "Offsets";
    tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    if (datatype == 4) {
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        //double tValue = _ttof(mResStr);
        tValue = atof(mResStr.c_str());
    } else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, float64_databuffer);
        tValue = float64_databuffer[0];
        free(float64_databuffer);
    }
    //float64_databuffer = NULL;
    //float64_databuffer = (float64*)allow_memory(globleattr, datatype);
    //status = SDreadattr(sid, tagIndex, float64_databuffer);
    //tValue = float64_databuffer[0];
    //free(float64_databuffer);

    SDendaccess(sid);
    status = SDend(fid);
    return tValue;
}

double hdfOpt::GetDataSetsMissingValue(string Filename, string Dsname) {
    double tValue;
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());
    int sid = SDselect(fid, tagIndex);

    m_DataType = GetFileProductType(Filename);

    const char *tagChar = "FillValue";
    tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    if (datatype == 4) {
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        //double tValue = _ttof(mResStr);
        tValue = atof(mResStr.c_str());
    } else {
        int16_databuffer = NULL;
        int16_databuffer;
        int64_t *fiilValue = (int64_t *) allow_memory(globleattr, datatype);

        long *fill = new long(globleattr);
        status = SDreadattr(sid, tagIndex, fill);
        tValue = (double) fill[0];
        free(fiilValue);
    }
    SDendaccess(sid);
    status = SDend(fid);
    return tValue;
}

long hdfOpt::GetDatasetsRows(string Filename, string Dsname) {
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //获取文件中数据集的个数 
    attrnum = 0;
    status = SDfileinfo(fid, &datasetsnum, &attrnum);

    if (datasetsnum < 0)    //文件中没有数据集 
    {
        status = SDend(fid);
        return false;
    }

    m_DataType = this->GetFileProductType(Filename);


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

long hdfOpt::GetDatasetsCols(string Filename, string Dsname) {
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //获取文件中数据集的个数 
    attrnum = 0;
    status = SDfileinfo(fid, &datasetsnum, &attrnum);

    if (datasetsnum < 0)    //文件中没有数据集 
    {
        status = SDend(fid);
        return false;
    }

    m_DataType = this->GetFileProductType(Filename);


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

double hdfOpt::GetDatasetsStartLog(string Filename, string Dsname) {
    double mStartLog;
    //获取文件类型
    m_DataType = GetFileProductType(Filename);

    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //假定所有数据集的空间分辨率相等，从第一个数据集中获取
    int sid = SDselect(fid, 0);
    //获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
    const char *tagChar = "StartLog";
    int32 tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    if (status == -1) {
        status = SDendaccess(sid);
        status = SDend(fid);
        return NULL;
    }

    //经过C#程序输出的hdf
    if (datatype == 4) {
        double tempNum;
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        tempNum = atof(mResStr.c_str());
        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }
        //经过本QT程序输出的hdf
    else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, float64_databuffer);
        mStartLog = float64_databuffer[0];

        free(float64_databuffer);

        status = SDendaccess(sid);
        status = SDend(fid);

        return mStartLog;
    }

}

double hdfOpt::GetDatasetsStartLat(string Filename, string Dsname) {
    double mStartLat;
    //获取文件类型
    m_DataType = GetFileProductType(Filename);

    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //假定所有数据集的空间分辨率相等，从第一个数据集中获取
    int sid = SDselect(fid, 0);
    //获取数据集的名称 数据集的维数 数据集的大小 数据集中数据类型 数据集中数据属性的个数
    const char *tagChar = "StartLat";
    int32 tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    if (status == -1) {
        status = SDendaccess(sid);
        status = SDend(fid);
        return NULL;
    }

    //经过C#程序输出的hdf
    if (datatype == 4) {
        double tempNum;
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        tempNum = atof(mResStr.c_str());
        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }//经过本QT程序输出的hdf
    else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
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

double hdfOpt::GetDatasetsEndLog(string Filename, string Dsname) {
    double tValue;
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());
    int sid = SDselect(fid, tagIndex);

    m_DataType = GetFileProductType(Filename);

    const char *tagChar = "EndLog";
    tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    //经过C#程序输出的hdf
    if (datatype == 4) {
        double tempNum;
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        tempNum = atof(mResStr.c_str());
        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }//经过本QT程序输出的hdf
    else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
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

double hdfOpt::GetDatasetsEndLat(string Filename, string Dsname) {
    double tValue;
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());
    int sid = SDselect(fid, tagIndex);

    m_DataType = GetFileProductType(Filename);



    const char *tagChar = "EndLat";
    tagIndex = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, tagIndex, filename, &datatype, &globleattr);

    //经过C#程序输出的hdf
    if (datatype == 4) {
        double tempNum;
        char8_databuffer = NULL;
        char8_databuffer = (char8 *) allow_memory(globleattr, datatype);
        status = SDreadattr(sid, tagIndex, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        tempNum = atof(mResStr.c_str());
        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }//经过本QT程序输出的hdf
    else {
        float64_databuffer = NULL;
        float64_databuffer = (float64 *) allow_memory(globleattr, datatype);
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

double hdfOpt::GetDatasetsSpatialResolution_New(string mFileName, string DsName)
{
    int fid;
    if ((fid = SDstart(mFileName.c_str(), DFACC_READ)) == -1)
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
    int index = SDnametoindex(fid, DsName.c_str());

    int sid = SDselect(fid, index);
    status = SDgetinfo(sid, filename, &rank, dimesize, &datatype, &attrnum);

    //循环获取数据集的基本信息
    string tempStr;
    const char* tagChar = "DSResolution";
    index = SDfindattr(sid, tagChar);
    status = SDattrinfo(sid, index, filename, &datatype, &globleattr);

    double tempNum;

    //经过C#程序输出的hdf
    if (datatype == 4) {
        char8_databuffer = NULL;
        char8_databuffer = (char8*)allow_memory(globleattr, datatype);
        status = SDreadattr(sid, index, char8_databuffer);
        string mResStr = char8_databuffer;
        free(char8_databuffer);
        //double tempNum = _ttof(mResStr);
        tempNum = atof(mResStr.c_str());

        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }//经过本QT程序输出的hdf
    else {
        float64_databuffer = NULL;
        float64_databuffer = (float64*)allow_memory(globleattr, datatype);
        status = SDreadattr(sid, index, float64_databuffer);
        tempNum = float64_databuffer[0];
        free(float64_databuffer);

        status = SDendaccess(sid);
        status = SDend(fid);

        return tempNum;
    }
}

bool hdfOpt::GetDsByDsnameFROMProduct(long * pBuffer, string Filename, string Dsname, long mStartRow, long m_Rows, long mStartCol, long m_Cols)
{
    //获取文件的id
    int fid = SDstart(Filename.c_str(), DFACC_READ);

    //找数据集
    int32 tagIndex = SDnametoindex(fid, Dsname.c_str());

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
            cout << "数据类型有误!" << endl;
            break;
    }

    //关闭打开hdf文件
    status = SDendaccess(sid);
    status = SDend(fid);

    return true;
}

string hdfOpt::GetFileDateTime(string Filename)
{
    string YYYYMMDDD;
    m_DataType = this->GetFileProductType(Filename);

        //获取文件的id
        int fid = SDstart(Filename.c_str(), DFACC_READ);

        const char* tagChar = "ImageDate";
        int32 tagIndex = SDfindattr(fid, tagChar);
        status = SDattrinfo(fid, tagIndex, filename, &datatype, &globleattr);

        char8_databuffer = NULL;
        char8_databuffer = (char8*)allow_memory(globleattr, datatype);
        status = SDreadattr(fid, tagIndex, char8_databuffer);
        string ss = char8_databuffer;
        ss = ss.substr(0,24);
        YYYYMMDDD = ss;
        //以上三行的原始代码是：YYYYMMDDD = char8_databuffer;
        free(char8_databuffer);

        status = SDend(fid);


    return YYYYMMDDD;
}
