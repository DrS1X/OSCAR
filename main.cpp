#include <iostream>
#include <string>
#include <vector>
#include "algo.h"
#include "RTree.h"
#include "DataProcess.h"
#include "FileOperator.h"
#include "DataModel.h"
#include "_const.h"

using namespace std;


Meta Meta::DEF(
        1.0,
        1.0,
        FILL_VAL,
        1,
        120,
        360,
        60,
        -180,
        -60,
        180,
        PROJECTION,
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
void Test_RTree(){
    GeoRegion r(20, 30, 20, 30);
    Poly poly;
    poly.range = r;
    RNode node(&poly);

    DUR_THRESHOLD = 5;
    RTree t; // delete
    t.insert(&node);

    GeoRegion r2(10, 40, 10, 40);
    list<RNode*> result = t.query(r2);
    cout << result.size() << endl;
}
void Test_HDF5(){
    string fileFolder = "D:\\prData\\mon\\ori";
    string fileType = ".HDF5";
    std::vector<std::string> fileList;
    util::getFileList(fileFolder, fileList, fileType);

    string groupName = "Grid";
    string datasetName = "precipitation";
    Reader::ReadFile(fileList);
}
int main(int argc,char *argv[])
{
    DEBUG = true;

    if(argc <= 1){
        Test_HDF5();
        getchar();
        return 0;
    }

    string functionName = argv[1];
    string outputPath = argv[2];
    string inputPath = argv[3];
    string fileType;
    vector<string> fileList;

    if(functionName == "--HDF5Preprocess" || functionName == "-hp"){

    }else if(functionName == "--GeoTiff2HDF" || functionName == "-g"){
        fileType = ".tif";
        util::getFileList(inputPath, fileList, fileType);

        for (int i = 0; i < fileList.size(); ++i) {
            vector<string> filePath;
            string dir = fileList[i];
            string year = dir.substr(dir.size() - 4, 4);
            if (stoi(year) >= 2017 && stoi(year) < 2021) {
                string savePath = outputPath + "\\" + year;
                util::getFileList(dir, filePath, fileType);
                Convertor::GeoTiff2HDF(filePath, savePath, 60, -60);
            }
        }
    }else if(functionName == "--StandAnomaly" || functionName == "-sa"){
        fileType = ".hdf";
        util::getFileList(inputPath, fileList, fileType);
        bool result = AnomalyAnalysis::StandardAnomaly(fileList, outputPath, AnomalyAnalysis::Test);
        cout << result;
    }else if(functionName == "--ResampleBatch" || functionName == "-d"){
        fileType = ".hdf";
        util::getFileList(inputPath, fileList, fileType);
        Convertor::ResampleBatch(fileList, outputPath, 1.0);
    }else if(functionName == "--SpaceTransform" || functionName == "-st"){
        fileType = ".hdf";
        util::getFileList(inputPath, fileList, fileType);
        Convertor::SpaceTransform(fileList, outputPath);
    }else if(functionName == "--Cluster" || functionName == "-c"){
        fileType = ".tif";
        util::getFileList(inputPath, fileList, fileType);
        DcSTMC a;
        a.Run(fileList, outputPath, 10, 1);
    }else if(functionName == "--Postprocess" || functionName == "-p"){
        fileType = ".hdf";
        vector<string> fileList2;
        util::getFileList(inputPath, fileList, fileType);
        util::getFileList(argv[4], fileList2, fileType);
        Postprocessor::Resort(fileList, fileList2, outputPath);
    }else if(functionName == "--Vectorization" || functionName == "-v"){
        vector<string> fileList2;
        util::getFileList(inputPath, fileList, string(".tif"));
        util::getFileList(argv[4], fileList2, string(".hdf"));
        Vectorization(fileList, fileList2, outputPath);
    }else if( functionName == "RTreeCluster" || functionName == "-rc"){
        struct RTreeParam p{
                3,
                8,
                TimeUnit::Day,
                5,
                5.0
        };
        fileType = ".tif";
        util::getFileList(inputPath, fileList, fileType);
        RTree::Run(p, fileList, outputPath);
    }
    else{
        cout << "Following is all function:\n"
        << "1) -g / --GeoTiff2HDF\n"
        << "2) -sa / --StandAnomaly\n"
        << "3) -d / --ResampleBatch\n"
        << "4) -st / --SpaceTransform\n"
        << "5) -sta / --SpatioTemporalAnomaly\n"
        << "6) -c / --Cluster\n"
        << "7) -p / --Postprocess\n"
        << "8) -v / --Vectorization\n"
        << "9) -rc / --RTreeCluster\n"
        << endl;
    }

	return 0;
}

