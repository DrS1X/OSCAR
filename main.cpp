#include <iostream>
#include <string>
#include <vector>
#include "Algo.h"
#include "RTree.h"
#include "DataProcess.h"
#include "DataModel.h"

using namespace std;

Meta Meta::DEF(
        1.0,
        120,
        360,
        -60,
        -180,
        60,
        180
);

void Test_RTreeCluster(){
    struct RTreeParam p{
            3,
            8,
            TimeUnit::Day,
            5,
            5.0,
            0.7
    };
    string fileType = ".tif";
    string inputPath = "E:\\IMERG\\repetition\\anomaly2\\std_05\\pos";
    string outputPath = "E:\\IMERG\\repetition\\RTreeCluster";
    vector<string> fileList;
    util::getFileList(inputPath, fileList, fileType);
    RTree::Run(p, fileList, outputPath);
}

void Test_ReadTiff(){
    string fn = "E:\\pr\\pos2015_2016\\mon\\pos\\posAnomaly-201501.tiff";

    int* pInt = new int[Meta::DEF.nPixel];
    Tif::readInt(fn, pInt);

    Tif* f = new Tif( Meta::DEF,fn);
    f->read();
}

void Test_RTree(){
    GeoRegion r(20, 30, 20, 30);
    Poly poly;
    poly.range = r;
    RNode node(&poly);

    RTree t; // delete
    t.insert(&node);

    GeoRegion r2(10, 40, 10, 40);
    list<RNode*> result = t.query(r2);
    cout << result.size() << endl;
}

void Test_Vec(){
    string fileType = ".tiff";
    string inputPath = "E:\\pr\\pos2015_2016\\mon\\pos";
    string inputPath2 = "E:\\pr\\pos2015_2016\\mon\\old\\sorted";
    string outputPath = "E:\\pr\\pos2015_2016\\mon\\old";
    vector<string> fileList,fileList2;
    util::getFileList(inputPath, fileList, fileType);
    util::getFileList(inputPath2, fileList2, fileType);
    Vectorization(fileList, fileList2, outputPath);
}


int main(int argc,char *argv[])
{
    if(argc <= 1){
        Test_Vec();
        return 0;
    }

    string functionName = argv[1];
    string outputPath = argv[2];
    string inputPath = argv[3];
    string fileType;
    vector<string> fileList;

    if(functionName == "--Preprocess" || functionName == "-p"){
        Meta::DEF.scale = 1.0f;

        std::vector<std::string> files;
        util::getFileList("I:\\IMERGE\\day\\negAnomaly", files, ".tiff");

        Reader rd("I:\\IMERGE\\day");
        Meta srcMeta = Meta::DEF;
        srcMeta.timeUnit = TimeUnit::Day;
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

        vector<string> fls = rd.readBatch(files, srcMeta);

        pair<vector<string>, vector<string>> lsPr = rd.filter(files,0.5);

        rd.smooth(files, rd.POSITIVE_S_PREFIX);
        rd.smooth(files, rd.NEGATIVE_S_PREFIX);
    }
    else if(functionName == "--Cluster" || functionName == "-c"){
        Meta::DEF.scale = 0.001f;

        fileType = ".tiff";
        util::getFileList(inputPath, fileList, fileType);

        DcSTMC a;
        a.V_THRESHOLD = 5.0;

        pair<vector<string>, vector<string>> pr = a.Run(fileList, outputPath, 10, 2);

        vector<string> sorted = Resort(pr.first, pr.second, outputPath);

        Vectorization(fileList, sorted, outputPath);

    }
    else if( functionName == "RTreeCluster" || functionName == "-rc"){
        Meta::DEF.scale = 1.0f;

        struct RTreeParam p{
                3,
                8,
                TimeUnit::Mon,
                5,
                5.0,
                0.7,
        };
        fileType = ".tiff";
        util::getFileList(inputPath, fileList, fileType);
        RTree::Run(p, fileList, outputPath);
    }
    else{
        cout << "Following is all function:\n"
        << "6) -c / --Cluster\n"
        << "9) -rc / --RTreeCluster\n"
        << endl;
    }

	return 0;
}

