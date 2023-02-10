//
// Created by 15291 on 2022/12/15.
//

#include "Shp.h"


GDALDriver * initShpDriver(){
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    CPLSetConfigOption("SHAPE_ENCODING", "");

    GDALAllRegister();
    GDALDriverManager * driverManger = GetGDALDriverManager();
    GDALDriver * driver = driverManger->GetDriverByName("ESRI Shapefile");
    return driver;
}
GDALDriver* Shp::driver = initShpDriver();

void Shp::createFields(OGRLayer *poLayer) {
    OGRFieldDefn oFieldStormID("ClusterID", OFTInteger);
    poLayer->CreateField(&oFieldStormID, 1);

    OGRFieldDefn oFieldStateID("PolyID", OFTInteger);
    poLayer->CreateField(&oFieldStateID, 1);

    OGRFieldDefn oFieldSeqID("SeqID", OFTInteger);
    poLayer->CreateField(&oFieldSeqID, 1);

    // 再创建一个叫FeatureName的字符型属性，字符长度为50
    OGRFieldDefn oFieldTime("Time", OFTString);
    oFieldTime.SetWidth(20);
    poLayer->CreateField(&oFieldTime, 1);

    OGRFieldDefn oFieldMinLog("MinLon", OFTReal);
    oFieldMinLog.SetWidth(20);
    oFieldMinLog.SetPrecision(8);
    poLayer->CreateField(&oFieldMinLog, 1);
    OGRFieldDefn oFieldMinLat("MinLat", OFTReal);
    oFieldMinLat.SetWidth(20);
    oFieldMinLat.SetPrecision(8);
    poLayer->CreateField(&oFieldMinLat, 1);
    OGRFieldDefn oFieldMaxLog("MaxLon", OFTReal);
    oFieldMaxLog.SetWidth(20);
    oFieldMaxLog.SetPrecision(8);
    poLayer->CreateField(&oFieldMaxLog, 1);
    OGRFieldDefn oFieldMaxLat("MaxLat", OFTReal);
    oFieldMaxLat.SetWidth(20);
    oFieldMaxLat.SetPrecision(8);
    poLayer->CreateField(&oFieldMaxLat, 1);

    OGRFieldDefn oFieldAvgRainFall("AvgValue", OFTReal);
    oFieldAvgRainFall.SetWidth(20);
    oFieldAvgRainFall.SetPrecision(8);
    poLayer->CreateField(&oFieldAvgRainFall, 1);

    OGRFieldDefn oFieldMaxRainFall("MaxValue", OFTReal);
    oFieldMaxRainFall.SetWidth(20);
    oFieldMaxRainFall.SetPrecision(8);
    poLayer->CreateField(&oFieldMaxRainFall, 1);

    OGRFieldDefn oFieldMinRainFall("MinValue", OFTReal);
    oFieldMinRainFall.SetWidth(20);
    oFieldMinRainFall.SetPrecision(8);
    poLayer->CreateField(&oFieldMinRainFall, 1);

    OGRFieldDefn oFieldPixCnt("PixCnt", OFTInteger);
    poLayer->CreateField(&oFieldPixCnt, 1);
}

void Shp::setFields(Poly* polygon, string time, OGRFeature *poFeature){
    // calculate field of polygon
    poFeature->SetField(0, polygon->cid);
    poFeature->SetField(1, polygon->id);
    poFeature->SetField(2, 0);
    poFeature->SetField(3, time.c_str());

    double minLog = Meta::DEF.getLon(polygon->range.colMin);
    double minLat = Meta::DEF.getLat(polygon->range.rowMax);
    double maxLog = Meta::DEF.getLon(polygon->range.colMax);
    double maxLat = Meta::DEF.getLat(polygon->range.rowMin);


    poFeature->SetField(4, minLog);
    poFeature->SetField(5, minLat);
    poFeature->SetField(6, maxLog);
    poFeature->SetField(7, maxLat);

    poFeature->SetField(8, polygon->avg);
    poFeature->SetField(9, polygon->maxValue);
    poFeature->SetField(10, polygon->minValue);

    int pixCnt = poFeature->GetGeometryRef()->toPolygon()->get_Area();
    poFeature->SetField(11, pixCnt);
}

void Shp::write(string outPath, string startTime, string abnormalType, vector<Poly>& polygons) {
    OGRErr err;

    if (driver == NULL)
    {
        cerr<< "[write] driver not available." << endl;
        return;
    }

    GDALDataset *poDS;

    poDS = driver->Create(outPath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == NULL)
    {
        printf("Creation of output file-operator failed.\n");
        return;
    }

    OGRLayer *poLayer;
    string fileName = abnormalType + startTime;
    poLayer = poDS->CreateLayer(fileName.c_str(), Meta::DEF.spatialReference, wkbPolygon, NULL);
    if (poLayer == NULL)
    {
        printf("Layer creation failed.\n");
        return;
    }

    createFields(poLayer);

    for (auto polygon : polygons) {
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        string polygonStr = "POLYGON (";
        for (auto line: polygon.lines) {
            polygonStr += "(";
            for (auto node: line.nodes) {
                int _row = node.row;
                int _col = node.col;

                double log = Meta::DEF.getLon(_col);
                double lat = Meta::DEF.getLat(_row);

                polygonStr += to_string(log) + " " + to_string(lat);
                polygonStr += ",";
            }
            //移除最后一个逗号
            if (polygonStr[polygonStr.size() - 1] == ',') {
                polygonStr = polygonStr.substr(0, polygonStr.size() - 1);
            }
            polygonStr += "),";
        }
        //移除最后一个逗号
        if (polygonStr[polygonStr.size() - 1] == ',') {
            polygonStr = polygonStr.substr(0, polygonStr.size() - 1);
        }
        polygonStr += ")";

        const char *pszWkt = (char *) polygonStr.c_str();
        OGRGeometry *poGeom;
        OGRGeometryFactory::createFromWkt(&pszWkt, Meta::DEF.spatialReference, &poGeom);

        setFields(&polygon, startTime, poFeature);

        poFeature->SetGeometryDirectly(poGeom);

        err = poLayer->CreateFeature(poFeature);
        if (err != OGRERR_NONE)
        {
            cerr << "[write]Failed to create feature in shapefile. err:" << to_string(err) << endl;
            return;
        }

        OGRFeature::DestroyFeature(poFeature);
    }
    GDALClose(poDS);
}