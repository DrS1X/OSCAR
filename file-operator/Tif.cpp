#include "Tif.h"

using namespace std;

GDALDriver * initDriver(){
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    
    GDALRegister_GTiff();
    GDALDriverManager * driverManger = GetGDALDriverManager();
    GDALDriver * geoTiffDriver = driverManger->GetDriverByName("GTiff");
    return geoTiffDriver;
}
GDALDriver * Tif::driver = initDriver();


Tif::Tif(Meta _meta):Rst(_meta){}

Tif::Tif(string fName):Rst(fName){}

Tif::Tif(Meta _meta, string _name): Rst(_meta, _name){}

Tif::Tif(Meta _meta, string _name, float *_data): Rst(_meta, _name, _data){}

Tif::~Tif(){}

bool Tif::write(string fName) {
    if (fName != "") {
        name = fName + "_" + meta.date + TIF_SUFFIX;
    }

    if (!driver)
        return false;

    GDALDataset *dataset = driver->Create(
            name.c_str(),
            meta.nCol,
            meta.nRow,
            meta.nBand,
            GDT_Float32,
            NULL);
    double arg[6];
    arg[0] = meta.startLon;
    arg[1] = meta.resolution;
    arg[2] = 0;
    arg[3] = meta.endLat;
    arg[4] = 0;
    arg[5] = meta.resolution * (-1);
    dataset->SetGeoTransform(arg);

    dataset->SetMetadataItem("Date", meta.date.c_str());

    char tmp[100];
    {

    sprintf_s(tmp, "%f", meta.scale);
    dataset->SetMetadataItem("Scale", tmp);

    sprintf_s(tmp, "%f", meta.startLon);
    dataset->SetMetadataItem("StartLon", tmp);

    sprintf_s(tmp, "%f", meta.endLon);
    dataset->SetMetadataItem("EndLon", tmp);

    sprintf_s(tmp, "%f", meta.startLat);
    dataset->SetMetadataItem("StartLat", tmp);

    sprintf_s(tmp, "%f", meta.endLat);
    dataset->SetMetadataItem("EndLat", tmp);

    sprintf_s(tmp, "%d", meta.nRow);
    dataset->SetMetadataItem("NRow", tmp);

    sprintf_s(tmp, "%d", meta.nCol);
    dataset->SetMetadataItem("NCol", tmp);

    sprintf_s(tmp, "%f", meta.fillValue);
    dataset->SetMetadataItem("FillValue", tmp);
    dataset->GetRasterBand(1)->SetNoDataValue(meta.fillValue);

    sprintf_s(tmp, "%f", meta.resolution);
    dataset->SetMetadataItem("Resolution", tmp);

    if (meta.projection != "") {
        dataset->SetProjection(meta.projection.c_str());
    }
    }

    if(!IsEqual(meta.scale, 1.0f)){
        for(int i = 0 ;i < meta.nPixel; ++i)
            data[i] *= meta.scale;
    }

    CPLErr err = dataset->RasterIO(GF_Write, 0, 0, meta.nCol, meta.nRow, data,
                                   meta.nCol, meta.nRow, GDT_Float32, 1, NULL, 0, 0, 0);
    if(err == CPLErr::CE_Failure || err == CPLErr::CE_Fatal){
        return false;
    }

    double dMin, dMax, dMean, dStdDev;
    GDALRasterBand *band = dataset->GetRasterBand(1);
    band->ComputeStatistics(FALSE,&dMin,&dMax,&dMean,&dStdDev,NULL,NULL);

    sprintf_s(tmp, "%f", dMax);
    dataset->SetMetadataItem("Max", tmp);

    sprintf_s(tmp, "%f", dMin);
    dataset->SetMetadataItem("Min", tmp);

    sprintf_s(tmp, "%f", dMean);
    dataset->SetMetadataItem("Mean", tmp);

    sprintf_s(tmp, "%f", dStdDev);
    dataset->SetMetadataItem("StdDev", tmp);

    GDALClose(dataset);
    return true;
}

bool Tif::read(string fName) {
    if(fName != ""){
        name = fName;
    }

    GDALDataset *pDataset = GDALDataset::FromHandle(GDALOpen(name.c_str(), GA_ReadOnly));
    if (pDataset == NULL) {
        cerr << "[read] fail to read tiff file." << endl;
        return false;
    }

    int nRow = pDataset->GetRasterYSize();
    int nCol = pDataset->GetRasterXSize();
    int nPixel = nRow * nCol;
    meta.nRow = nRow;
    meta.nCol = nCol;
    meta.nPixel = nPixel;
    const char* pDate = pDataset->GetMetadataItem("Date");
    if(pDate == nullptr) {
        meta.date = GetDate(name);
        if(meta.timeUnit == TimeUnit::Mon)
            meta.date = meta.date.substr(0,6);
    }else {
        meta.date = pDate;
    }

    GDALRasterBand *band = pDataset->GetRasterBand(1);
    GDALDataType dataType = band->GetRasterDataType();

    switch(dataType){
        case GDT_UInt16: {
            std::uint16_t* pBuf = new std::uint16_t[nPixel];

            CPLErr err = band->RasterIO(GF_Read, 0, 0, nCol, nRow, pBuf,
                                        nCol, nRow, dataType, 0, 0);
            if(err == CPLErr::CE_Failure || err == CPLErr::CE_Fatal){
                return false;
            }

            for(int i = 0; i < nPixel; ++i){
                data[i] = pBuf[i] * meta.scale;
            }
            delete[] pBuf;
            break;
        }
        case GDT_Float32: {
            float *pBuf = new float[nPixel];

            CPLErr err = band->RasterIO(GF_Read, 0, 0, nCol, nRow, pBuf,
                                        nCol, nRow, dataType, 0, 0);
            if (err == CPLErr::CE_Failure || err == CPLErr::CE_Fatal) {
                return false;
            }

            for (int i = 0; i < nPixel; ++i) {
                data[i] = pBuf[i] * meta.scale;
            }
            delete[] pBuf;
            break;
        }

    }

    GDALClose(pDataset);
    return true;
}

bool Tif::readInt(string fn, int* pData, float scale){
    Meta meta = Meta::DEF;
    meta.scale = 1.0f;
    Tif tif(meta, fn);
    if(!tif.read())
        return false;

    for(int i = 0; i < Meta::DEF.nPixel; ++i){
        pData[i] = tif.data[i] / scale;
    }

    return true;
}

bool Tif::writeInt(string fn, Meta& meta, int* pData){
    if(meta.nPixel == 0){
        meta.nPixel = meta.nRow * meta.nCol;
    }
    float* pBuf = new float[meta.nPixel];

    for(int i = 0; i < meta.nPixel; ++i){
        pBuf[i] = pData[i];
    }

    Tif tif(meta, fn, pBuf);
    if(!tif.write())
        return false;

    return true;
}

void Tif::smooth(){
    for (int m = 1; m < meta.nRow - 1; m++)
    {
        for (int n = 1; n < meta.nCol - 1; n++)
        {
            if (isZero(m,n) || isFillValue(m,n)) {
                int count_fill = 0;
                float fill_value = 0;
                for (int mm = m - 1; mm < m + 2; mm++)
                {
                    for (int nn = n - 1; nn < n + 2; nn++)
                    {
                        if (mm == m && nn == n)
                        {
                            continue;
                        }
                        else
                        {
                            if (!isZero(mm,nn) && !isFillValue(mm,nn))
                            {
                                count_fill++;
                                fill_value += get(mm,nn);
                            }
                        }
                    }
                }
                if (count_fill == 8)
                {
                    update(m,n, fill_value / count_fill);
                }
            }else{
                int count_delete = 0;
                for (int mm = m - 1; mm < m + 2; mm++) {
                    for (int nn = n - 1; nn < n + 2; nn++) {
                        if (mm == m && nn == n) {
                            continue;
                        } else {
                            if (isZero(mm,nn) || isFillValue(mm,nn)) {
                                count_delete++;
                            }
                        }
                    }
                }
                if (count_delete >= 6) {
                    update(m,n, 0.0f);
                }
            }
        }
    }
}