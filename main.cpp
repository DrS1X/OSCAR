#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>
#include <gdal_alg.h>
#include "Algo.h"
#include "RTree.h"
#include "DataModel.h"
#include "Cst.h"
#include "Reader.h"

using namespace std;

Meta Meta::DEF(
        1.0,
        1.0f,
        120,
        360,
        -60,
        0,
        60,
        360
);

void SimulateInit() {
    Meta::DEF.isSimulated = true;
    Meta::DEF.nRow = 60;
    Meta::DEF.nCol = 20;
    Meta::DEF.startLat = 0;
    Meta::DEF.endLat = 60;
    Meta::DEF.startLon = 0;
    Meta::DEF.endLon = 20;
    Meta::DEF.nPixel = 60 * 20;
    GeoRegion::GLOBAL.rowMax = Meta::DEF.nRow - 1;
    GeoRegion::GLOBAL.colMax = Meta::DEF.nCol - 1;
}


void Test_ReadTiff() {
    string fn = "E:\\pr\\pos2015_2016\\mon\\pos\\posAnomaly-201501.tiff";

    int *pInt = new int[Meta::DEF.nPixel];
    Tif::readInt(fn, pInt);

    Tif *f = new Tif(Meta::DEF, fn);
    f->read();
}

void Test_Intersect() {
    OGRSpatialReference *poSRS = new OGRSpatialReference();

    poSRS->SetGeogCS("My geographic CRS",
                     "World Geodetic System 1984",
                     "My WGS84 Spheroid",
                     SRS_WGS84_SEMIMAJOR, SRS_WGS84_INVFLATTENING,
                     "Greenwich", 0.0);

//    string wkt1 = "POLYGON ((-1 -1,-1 3,3 3,3 -1,-1 -1))";
    string wkt1 = "POLYGON ((2 2,2 5,5 5,5 2,2 2))";
    string wkt2 = "POLYGON ((2 2,2 4,4 4,4 2,2 2))";


    const char *pszWkt1 = (char *) wkt1.c_str();
    const char *pszWkt2 = (char *) wkt2.c_str();

    OGRGeometry *poGeom1, *poGeom2, *poGeom3;
    OGRGeometryFactory::createFromWkt(&pszWkt1, poSRS, &poGeom1);
    OGRGeometryFactory::createFromWkt(&pszWkt2, poSRS, &poGeom2);

    OGRBoolean c = poGeom1->Contains(poGeom2);
    cout << c << endl;

    OGRBoolean c2 = poGeom2->Contains(poGeom1);
    cout << c2 << endl;

    OGRBoolean overlap = poGeom1->Overlaps(poGeom2);
    cout << overlap << endl;

    poGeom3 = poGeom1->Intersection(poGeom2);

    OGRPolygon *poPoly = poGeom3->toPolygon();
    double area = poPoly->get_Area();
    cout << area << endl;
}

void Test_Polygonize() {
    OGRErr err;

    char *name = "E:\\pr\\pos2015_2016\\mon\\old\\sorted\\posAnomaly-201601_Tcluster.tiff";
    GDALDataset *pDataset = GDALDataset::FromHandle(GDALOpen(name, GA_ReadOnly));
    GDALRasterBand *band = pDataset->GetRasterBand(1);
    band->SetNoDataValueAsInt64(-9999);

    char *outName = "E:\\pr\\pos2015_2016\\mon\\old\\test.shp";

    GDALAllRegister();
    GDALDriverManager *driverManger = GetGDALDriverManager();
    GDALDriver *poDriver = driverManger->GetDriverByName("ESRI Shapefile");
    GDALDataset *poDS = poDriver->Create(outName, 0, 0, 0, GDT_Unknown, NULL);
    OGRLayer *poLayer = poDS->CreateLayer("cluster", Meta::DEF.spatialReference, wkbPolygon, NULL);
    OGRFieldDefn fieldClusterID("CLUSTER", OFTInteger);
    err = poLayer->CreateField(&fieldClusterID, 1);
    OGRFieldDefn fieldOther("Other", OFTInteger);
    err = poLayer->CreateField(&fieldOther, 1);
    if (err != OGRERR_NONE) {
        cerr << err << endl;
        return;
    }

    err = GDALPolygonize(GDALRasterBand::ToHandle(band),
                         GDALRasterBand::ToHandle(band),
                         OGRLayer::ToHandle(poLayer),
                         0, NULL, NULL, NULL
    );
    if (err != OGRERR_NONE) {
        cerr << err << endl;
        return;
    }

    GDALOpenEx(outName, GDAL_OF_VECTOR, NULL, NULL, NULL);
    OGRFeature *feature;
    while (feature = poLayer->GetNextFeature()) {
        if (feature->GetFieldAsInteger(0) == 18) {
            GIntBig fid = feature->GetFID();
            OGRFeature::DestroyFeature(feature);
            poLayer->DeleteFeature(fid);
        } else {
            feature->SetField("Other", 123);
            poLayer->SetFeature(feature);
            OGRFeature::DestroyFeature(feature);
        }
    }

    GDALClose(poDS);
}

void Test_Corner() {
    vector<int> line{0, 69, 87, 101, 113, 123, 133, 142, 150, 159, 166, 174, 181, 189, 196, 203, 210, 217, 224, 231,
                     238, 245, 252, 259, 266, 272, 279, 286, 293, 300, 307, 314, 321, 328, 335, 342, 350, 357, 364, 372,
                     379, 387, 394, 402, 409, 417, 425, 433, 441, 449, 458, 466, 475, 483, 492, 501, 510, 520, 530, 539,
                     549, 559, 570, 580, 591, 602, 613, 625, 637, 649, 662, 674, 688, 702, 716, 731, 746, 763, 779, 797,
                     815, 834, 854, 875, 898, 922, 947, 975, 1005, 1037, 1072, 1112, 1155, 1205, 1262, 1330, 1415, 1523,
                     1682, 1962, 4286};
    float step = 10138;
    vector<float> derivative1(line.size() - 1);
    vector<float> derivative2(derivative1.size() - 1);
    for (int i = 1; i < line.size(); ++i) {
        derivative1[i - 1] = (line[i] - line[i - 1]) / step;
    }
    float maxDerivative2 = 0.0f;
    int idx;
    for (int i = 1; i < derivative1.size(); ++i) {
        derivative2[i - 1] = (derivative1[i] - derivative1[i - 1]) / step;
        if (derivative2[i - 1] > maxDerivative2) {
            maxDerivative2 = derivative2[i - 1];
            idx = i - 1;
        }
    }

    cout << idx + 1 << endl;

}

void Test_BackGround(){
    SimulateInit();
    filesystem::path inPath("E:\\pr\\simulate\\tif2");
    filesystem::path maskOutPath("E:\\pr\\simulate");
    double mean;
    float time = 1;
    Background( inPath, &mean, &time, maskOutPath);
}

extern int optind, opterr, optopt;
extern char *optarg;

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        Test_BackGround();
        return 0;
    }

    opterr = 0;
    map<char, float> para;
    int func = getopt(argc, argv, "pkf:drhSe");
    if (func == 'S' || func == 'e') {
        SimulateInit();
    } else if (func == 'f') {
        para['p'] = ::atof(optarg);
    } else if (func == 'k' || func == 'd' || func == 'r') {
        int paraKey;
        while ((paraKey = getopt(argc, argv, "bx:n:s:t:c:v:")) && paraKey != '?') {
            if (paraKey == 'b')
                para[paraKey] = 1.0f;
            else
                para[paraKey] = ::atof(optarg);
        }
        --optind;
    } else if (func == 'h') {
        cout << "Following is all function:\n"
             << "-p: preprocess\n"
             << "-k: Kth"
             << "-d: DcSTCA\n"
             << "-r: cluster based RTree\n"
             << "-f: filter\n"
             << "-S: simulate data\n"
             << "-e: cluster result evaluation\n"
             << "Example:\n"
             << "-S -o E:\\pr\\simulate -i E:\\pr\\simulate\\csv\n"
             << "-k -t 1 -o E:\\pr\\simulate -i E:\\pr\\simulate\\src -S\n"
             << "-d -b -t 1 -x 27 -n 3 -s 1 -m -o E:\\pr\\simulate -i E:\\pr\\simulate\\src -S\n"
             << "-r -b -m -o E:\\pr\\simulate -i E:\\pr\\simulate\\src -S\n"
             << "-e -o E:\\pr\\simulate\\R_Batch_0.500000oTh\\R_0.500000_7_3.090000 -i E:\\pr\\simulate\\truth -i E:\\pr\\simulate\\R_Batch_0.500000oTh\\R_0.500000_7_3.090000\\cid -S\n"
             << "-d -t 1 -c 15 -v 3 -m -o E:\\pr\\simulate -i E:\\pr\\simulate\\tif2 -S\n"
             << "-r -t 1 -c 5 -v 3 -m -o E:\\pr\\simulate -i E:\\pr\\simulate\\tif2 -S\n"
             << endl;
        return 0;
    }

    int timeUnit = getopt(argc, argv, "md");
    if (timeUnit == 'm')
        Meta::DEF.timeUnit = TimeUnit::Mon;
    else if (timeUnit == 'd')
        Meta::DEF.timeUnit = TimeUnit::Day;
    else {
        --optind;
    }

    string outPath, inPath;
    int io = getopt(argc, argv, "i:o:");
    if (io == 'i')
        inPath = optarg;
    else if (io == 'o')
        outPath = optarg;

    io = getopt(argc, argv, "i:o:");
    if (io == 'i')
        inPath = optarg;
    else if (io == 'o')
        outPath = optarg;


    int isSimulated = getopt(argc, argv, "S");
    if (isSimulated == 'S')
        SimulateInit();
    else
        --optind;


    switch (func) {
        case 'S': {
            SimulateData(inPath, outPath);

            break;
        }
        case 'p': {
            Meta srcMeta = Meta::DEF;
            srcMeta.fillValue = 29999;
            srcMeta.scale = 0.001f;
            srcMeta.nRow = 1800;
            srcMeta.nCol = 3600;
            srcMeta.nPixel = srcMeta.nRow * srcMeta.nCol;
            srcMeta.resolution = 0.1;
            srcMeta.startLat = -90;
            srcMeta.endLat = 90;
            srcMeta.startLon = -180;
            srcMeta.endLon = 180;

            ReadBatch(inPath, outPath, srcMeta);

            break;
        }
        case 'k': {
            Kth(inPath, outPath, para['t']);

            break;
        }
        case 'f': {
            Filter(inPath, outPath, para['p']);

            break;
        }
        case 'd': {
            if (para.find('b') != para.end()) {
                int maxK = para.find('x') == para.end() ? 0 : para['x'];
                int minK = para.find('n') == para.end() ? 0 : para['n'];
                int stepK = para.find('s') == para.end() ? 1 : para['s'];
                DcSTCABatch(inPath, outPath, para['t'], maxK, minK, stepK);
            } else {
                DcSTCA a;
                a.Run(inPath, outPath, para['t'], para['c'], para['v']);
            }

            break;
        }
        case 'r': {
            if (para.find('b') != para.end()) {
                int maxK = para.find('x') == para.end() ? 9 : para['x'];
                int minK = para.find('n') == para.end() ? 4 : para['n'];
                int stepK = para.find('s') == para.end() ? 1 : para['s'];
                vector<float> oThs{0.25, 0.5,0.75};
                for (const auto oTh: oThs)
                    RBatch(inPath, outPath, oTh);
            } else
                RTree::Run(para['t'], para['c'], para['v'], inPath, outPath); // para['t'] -> oTh

            break;
        }
        case 'e': {
            string res;
            getopt(argc, argv, "i:");
            res = optarg;
            Evaluation(inPath, res, outPath);

            break;
        }
    }

    return 0;
}

