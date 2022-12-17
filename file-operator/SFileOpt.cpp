//
// Created by 15291 on 2022/12/15.
//

#include "SFileOpt.h"

void SFileOpt::write(string outPath, string startTime, string abnormalType, vector<Poly>& polygons) {
    const float startLog = Meta::DEF.startLon;
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

    OGRSpatialReference * ref;
    ref = new OGRSpatialReference(Meta::DEF.projection.c_str());

    string fileName = abnormalType + startTime;

    poLayer = poDS->CreateLayer(fileName.c_str(), ref, wkbPolygon, NULL);
    if (poLayer == NULL)
    {
        printf("Layer creation failed.\n");
        return;
    }

    //Fields
    {
        OGRFieldDefn oFieldName("Name", OFTString);
        oFieldName.SetWidth(20);
        poLayer->CreateField(&oFieldName, 1);

        // 先创建一个叫FieldID的整型属性
        OGRFieldDefn oFieldStormID("PRID", OFTInteger);
        poLayer->CreateField(&oFieldStormID, 1);

        // 先创建一个叫FieldID的整型属性
        OGRFieldDefn oFieldStateID("STID", OFTString);
        oFieldStateID.SetWidth(20);
        poLayer->CreateField(&oFieldStateID, 1);

        // 再创建一个叫FeatureName的字符型属性，字符长度为50
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

        //创建area字段
        OGRFieldDefn oFieldArea("Area", OFTReal);
        oFieldArea.SetWidth(20);
        oFieldArea.SetPrecision(8);
        poLayer->CreateField(&oFieldArea, 1);

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
        poLayer->CreateField(&oFieldAbnormal, 1);
    }

    for (auto polygon : polygons) {
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());


        string polygonStr = "POLYGON (";
        for (auto line: polygon.lines) {
            polygonStr += "(";
            for (auto node: line.nodes) {
                int _row = node.row;//点行号
                int _col = node.col;//点列号

                double log = startLog + (_col + 1) * resolution;//经度
                double lat = startLat - (_row + 1) * resolution;//纬度

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

        char *pszWkt = (char *) polygonStr.c_str();

        OGRGeometry *poGeom;
        OGRGeometryFactory::createFromWkt(&pszWkt, ref, &poGeom);

        {
            // calculate field of polygon
            poFeature->SetField(0, "pr");
            poFeature->SetField(1, polygon.clusterId);
            poFeature->SetField(4, startTime.c_str());
            double minLog = startLog + (polygon.minCol + 1) * resolution;//最小经度
            double minLat = startLat - (polygon.maxRow + 1) * resolution;//最小纬度
            double maxLog = startLog + (polygon.maxCol + 1) * resolution;//最大经度
            double maxLat = startLat - (polygon.minRow + 1) * resolution;//最大纬度

            double centroidLog = startLog + (polygon.centroidCol + 0.5) * resolution;//质心经度
            double centroidLat = startLat - (polygon.centroidRow + 0.5) * resolution;//质心纬度

            poFeature->SetField(5, minLog);
            poFeature->SetField(6, minLat);
            poFeature->SetField(7, maxLog);
            poFeature->SetField(8, maxLat);

            double area = polygon.area;
            /*if(area == POLYGON_FIELD_EMPTY){
                area = getArea(poGeom);
            }*/
            poFeature->SetField(9, area);

            poFeature->SetField(10, polygon.avgValue);
            //poFeature->SetField(11, polygon.volume);
            poFeature->SetField(11, polygon.maxValue);
            poFeature->SetField(12, polygon.minValue);

            //poFeature->SetField(15, polygon.length);
            poFeature->SetField(13, centroidLog);
            poFeature->SetField(14, centroidLat);
            poFeature->SetField(15, polygon.power);
            poFeature->SetField(16, abnormalType.c_str());
        }

        poFeature->SetGeometryDirectly(poGeom);

        OGRErr err = poLayer->CreateFeature(poFeature);
        if (err != OGRERR_NONE)
        {
            cout << "error: [save]Failed to create feature in shapefile. err:" << to_string(err) << endl;
            return;
        }

        OGRFeature::DestroyFeature(poFeature);
    }
    GDALClose(poDS);
}