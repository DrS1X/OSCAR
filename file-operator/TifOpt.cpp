#include "TifOpt.h"

using namespace std;

TifOpt::TifOpt()
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	geoTiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
}

bool TifOpt::write(RFile file){
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    if (!geoTiffDriver)
    return false;

    string fileName = file.name + ".tiff";
    GDALDataset* geoTiffDataset = geoTiffDriver->Create(
            fileName.c_str(),
            file.meta.nCol,
            file.meta.nRow,
            file.meta.nBand,
            GDT_Float32,
            NULL);

    double argout[6];
    argout[0] = file.meta.startLon;
    argout[1] = file.meta.resolution;
    argout[2] = 0;
    argout[3] = file.meta.startLat;
    argout[4] = 0;
    argout[5] = file.meta.resolution*(-1);
    geoTiffDataset->SetGeoTransform(argout);

    geoTiffDataset->SetMetadataItem("Date", file.meta.date.c_str());

    char tmp[100];

    sprintf_s(tmp, "%f", file.meta.scale);
    geoTiffDataset->SetMetadataItem("Scale", tmp);

    sprintf_s(tmp, "%f", file.meta.startLon);
    geoTiffDataset->SetMetadataItem("StartLon", tmp);

    sprintf_s(tmp, "%f", file.meta.endLon);
    geoTiffDataset->SetMetadataItem("EndLon", tmp);

    sprintf_s(tmp, "%f", file.meta.startLat);
    geoTiffDataset->SetMetadataItem("StartLat", tmp);

    sprintf_s(tmp, "%f", file.meta.endLat);
    geoTiffDataset->SetMetadataItem("EndLat", tmp);

    sprintf_s(tmp, "%d", file.meta.nRow);
    geoTiffDataset->SetMetadataItem("NRow", tmp);

    sprintf_s(tmp, "%d", file.meta.nCol);
    geoTiffDataset->SetMetadataItem("NCol", tmp);

    sprintf_s(tmp, "%f", file.meta.fillValue);
    geoTiffDataset->SetMetadataItem("FillValue", tmp);
    geoTiffDataset->GetRasterBand(1)->SetNoDataValue(file.meta.fillValue);

    sprintf_s(tmp, "%f", file.meta.maxV);
    geoTiffDataset->SetMetadataItem("MaxValue", tmp);

    sprintf_s(tmp, "%f", file.meta.minV);
    geoTiffDataset->SetMetadataItem("MinValue", tmp);

    sprintf_s(tmp, "%f", file.meta.mean);
    geoTiffDataset->SetMetadataItem("MeanValue", tmp);

    sprintf_s(tmp, "%f", file.meta.standard);
    geoTiffDataset->SetMetadataItem("StdValue", tmp);

    sprintf_s(tmp, "%f", file.meta.resolution);
    geoTiffDataset->SetMetadataItem("Resolution", tmp);

    const char * Projection_info = file.meta.projection.c_str();
    if (Projection_info != "\0")
    {
        geoTiffDataset->SetProjection(Projection_info);
    }
    geoTiffDataset->RasterIO(GF_Write, 0, 0, file.meta.nCol, file.meta.nRow, file.data,
                             file.meta.nCol, file.meta.nRow, GDT_Float32, 1, NULL, 0, 0, 0);
    GDALClose(GDALDatasetH(geoTiffDataset));

    return true;
}

bool TifOpt::read(RFile file) {
	//GDALDriver *geoTiffDriver;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset *pDataSet = (GDALDataset*)GDALOpen(file.name.c_str(), GA_ReadOnly);
	if (pDataSet != NULL)
	{
		long iRow = pDataSet->GetRasterYSize();
		long iCol = pDataSet->GetRasterXSize();
		GDALRasterBand *demband = pDataSet->GetRasterBand(1);
		demband->RasterIO(GF_Read, 0, 0, iCol, iRow, file.data, iCol, iRow, GDT_Float32, 0, 0);
		demband->SetNoDataValue(-9999);
		GDALClose(GDALDatasetH(pDataSet));
		return true;
	}
	return false;
}

void TifOpt::getMeta(std::string fileName, Meta& meta) {

}


bool TifOpt::readFlatten(string file, int* pData) {
    const char* in_fileName = file.c_str();
    //GDALDriver *geoTiffDriver;
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    GDALDataset *pDataSet = (GDALDataset*)GDALOpen(in_fileName, GA_ReadOnly);
    if (pDataSet != NULL)
    {
        long iRow = pDataSet->GetRasterYSize();
        long iCol = pDataSet->GetRasterXSize();
        GDALRasterBand *demband = pDataSet->GetRasterBand(1);
        demband->RasterIO(GF_Read, 0, 0, iCol, iRow, pData, iCol, iRow, GDT_Int32, 0, 0);
        demband->SetNoDataValue(-9999);
        GDALClose(GDALDatasetH(pDataSet));
        return true;
    }
    return false;
}

bool TifOpt::readFlatten(string file, double* pData){
    return true;
}

bool TifOpt::writeFlatten(string file, Meta meta, int * pData){
    return true;
}


