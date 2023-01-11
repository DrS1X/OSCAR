//
// Created by 15291 on 2022/12/15.
//

#include "SFileOpt.h"

void SFileOpt::write(string outPath, string startTime, string abnormalType, vector<Poly>& polygons) {
    const float startLon = Meta::DEF.startLon;
    const float startLat = Meta::DEF.startLat;
    const float resolution = Meta::DEF.resolution;

    GDALAllRegister();
    //保存shp
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    // 为了使属性表字段支持中文，请添加下面这句
    CPLSetConfigOption("SHAPE_ENCODING", "");

    const char *pszDriverName = "ESRI Shapefile";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (poDriver == NULL)
    {
        printf("%s driver not available.\n", pszDriverName);
        return;
    }

    GDALDataset *poDS;

    poDS = poDriver->Create(outPath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == NULL)
    {
        printf("Creation of output file-operator failed.\n");
        return;
    }

    OGRLayer *poLayer;

    OGRErr err;

    OGRSpatialReference * poSRS = new OGRSpatialReference();

    err = poSRS->SetGeogCS( "My geographic CRS",
                    "World Geodetic System 1984",
                    "My WGS84 Spheroid",
                    SRS_WGS84_SEMIMAJOR, SRS_WGS84_INVFLATTENING,
                    "Greenwich", 0.0);

//    err = poSRS->importFromWkt(Meta::DEF.projection.c_str());

//    err = poSRS->SetWellKnownGeogCS("WGS 84");

//    char *pChar = "GEOGCS[\"My_GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",180.0],UNIT[\"Degree\",0.0174532925199433]]";

//    err = poSRS->importFromESRI(&pChar);
//    err = OSRImportFromWkt(poSRS, &pChar);

    if(err != OGRERR_NONE){
        cerr << "[write] fail to set CRS. err code: " << err << endl;
    }

    string fileName = abnormalType + startTime;

    poLayer = poDS->CreateLayer(fileName.c_str(), poSRS, wkbPolygon, NULL);
    if (poLayer == NULL)
    {
        printf("Layer creation failed.\n");
        return;
    }

    //Fields
    {
        OGRFieldDefn oFieldStormID("PRID", OFTInteger);
        poLayer->CreateField(&oFieldStormID, 1);

        OGRFieldDefn oFieldStateID("STID", OFTString);
        oFieldStateID.SetWidth(20);
        poLayer->CreateField(&oFieldStateID, 1);

        OGRFieldDefn oFieldSeqID("SQID", OFTString);
        oFieldSeqID.SetWidth(20);
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

        //创建平均降雨量字段
        OGRFieldDefn oFieldAvgRainFall("AvgValue", OFTReal);
        oFieldAvgRainFall.SetWidth(20);
        oFieldAvgRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldAvgRainFall, 1);

        //创建最大降雨量字段
        OGRFieldDefn oFieldMaxRainFall("MaxValue", OFTReal);
        oFieldMaxRainFall.SetWidth(20);
        oFieldMaxRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldMaxRainFall, 1);

        //创建最大降雨量字段
        OGRFieldDefn oFieldMinRainFall("MinValue", OFTReal);
        oFieldMinRainFall.SetWidth(20);
        oFieldMinRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldMinRainFall, 1);

        /*//创建area字段
        OGRFieldDefn oFieldArea("Area", OFTReal);
        oFieldArea.SetWidth(20);
        oFieldArea.SetPrecision(8);
        poLayer->CreateField(&oFieldArea, 1);

        //创建质心字段
        OGRFieldDefn oFieldLogCore("CentLon", OFTReal);
        oFieldLogCore.SetWidth(20);
        oFieldLogCore.SetPrecision(8);
        poLayer->CreateField(&oFieldLogCore, 1);

        //创建质心字段
        OGRFieldDefn oFieldLatCore("CentLat", OFTReal);
        oFieldLatCore.SetWidth(20);
        oFieldLatCore.SetPrecision(8);
        poLayer->CreateField(&oFieldLatCore, 1);

        //创建Power字段
        OGRFieldDefn oFieldPower("Power", OFTInteger);
        poLayer->CreateField(&oFieldPower, 1);

        //创建Abnormal异常类型
        OGRFieldDefn oFieldAbnormal("Abnormal", OFTString);
        oFieldAbnormal.SetWidth(20);
        poLayer->CreateField(&oFieldAbnormal, 1);*/
    }

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
        OGRGeometryFactory::createFromWkt(&pszWkt, poSRS, &poGeom);

        {
            // calculate field of polygon
            poFeature->SetField(0, polygon.clusterId);
            poFeature->SetField(1, polygon.id);
            poFeature->SetField(2, 0);
            poFeature->SetField(3, startTime.c_str());

            double minLog = Meta::DEF.getLon(polygon.minCol);
            double minLat = Meta::DEF.getLat(polygon.maxRow);
            double maxLog = Meta::DEF.getLon(polygon.maxCol);
            double maxLat = Meta::DEF.getLat(polygon.minRow);


            poFeature->SetField(4, minLog);
            poFeature->SetField(5, minLat);
            poFeature->SetField(6, maxLog);
            poFeature->SetField(7, maxLat);

            poFeature->SetField(8, polygon.avgValue);
            poFeature->SetField(9, polygon.maxValue);
            poFeature->SetField(10, polygon.minValue);

            /*double area = polygon.area;
            if(area < 0){
                area = 0;
            }
            poFeature->SetField(9, area);

            double centroidLog = Meta::DEF.getLon(polygon.centroidCol);//质心经度
            double centroidLat = Meta::DEF.getLat(polygon.centroidRow);//质心纬度
            poFeature->SetField(13, centroidLog);
            poFeature->SetField(14, centroidLat);
            poFeature->SetField(15, polygon.power);
            poFeature->SetField(16, abnormalType.c_str());*/
        }

        poFeature->SetGeometryDirectly(poGeom);

        err = poLayer->CreateFeature(poFeature);
        if (err != OGRERR_NONE)
        {
            cout << "error: [save]Failed to create feature in shapefile. err:" << to_string(err) << endl;
            return;
        }

        OGRFeature::DestroyFeature(poFeature);
    }
    GDALClose(poDS);
}