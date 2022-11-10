#include <iostream>
#include <string>
#include <vector>
#include "Convertor.h"
#include "AnomalyAnalysis.h"
#include "opt.h"
#include "algo.h"
#include "RTree.h"
#include "Postprocessor.h"
#include "Vectorization.h"

using namespace std;

int GEO_WINDOW;
int DUR_THRESHOLD;
int CORE_THRESHOLD;
int ATTRIBUTE_THRESHOLD;
string OUTPUT_PATH;
int CLUSTER_ID = 0;
TimeUnit UNIT;

void Test_RTreeCluster(){
    struct RTreeParam p{
            3,
            8,
            TimeUnit::Day,
            5,
            5.0
    };
    string fileType = ".tif";
    string inputPath = "E:\\IMERG\\repetition\\anomaly2\\std_05\\pos";
    string outputPath = "E:\\IMERG\\repetition\\RTreeCluster";
    vector<string> fileList;
    opt::getFileList(inputPath, fileList, fileType);
    RTree::Run(p, fileList, outputPath);
}
void Test_RTree(){
    Range r(20,30, 20,30);
    Poly poly;
    poly.range = r;
    RNode node(&poly);

    DUR_THRESHOLD = 5;
    RTree t;
    t.insert(&node);

    Range r2(10,40, 10,40);
    vector<RNode*> result = t.query(r2);
    cout << result.size() << endl;
}
int main(int argc,char *argv[])
{
    if(argc <= 1){
        Test_RTreeCluster();
        return 0;
    }

    string functionName = argv[1];
    string outputPath = argv[2];
    string inputPath = argv[3];
    string fileType;
    vector<string> fileList;

    if(functionName == "--GeoTiff2HDF" || functionName == "-g"){
        fileType = ".tif";
        opt::getFileList(inputPath, fileList, fileType);

        for (int i = 0; i < fileList.size(); ++i) {
            vector<string> filePath;
            string dir = fileList[i];
            string year = dir.substr(dir.size() - 4, 4);
            if (stoi(year) >= 2017 && stoi(year) < 2021) {
                string savePath = outputPath + "\\" + year;
                opt::getFileList(dir, filePath, fileType);
                Convertor::GeoTiff2HDF(filePath, savePath, 60, -60);
            }
        }
    }else if(functionName == "--StandAnomaly" || functionName == "-sa"){
        fileType = ".hdf";
        opt::getFileList(inputPath, fileList, fileType);
        bool result = AnomalyAnalysis::StandardAnomaly(fileList, outputPath, AnomalyAnalysis::Test);
        cout << result;
    }else if(functionName == "--Resample" || functionName == "-r"){
        fileType = ".hdf";
        opt::getFileList(inputPath, fileList, fileType);
        Convertor::Resample(fileList, outputPath, 1.0);
    }else if(functionName == "--SpaceTransform" || functionName == "-st"){
        fileType = ".hdf";
        opt::getFileList(inputPath, fileList, fileType);
        Convertor::SpaceTransform(fileList, outputPath);
    }else if(functionName == "-c" || functionName == "--Cluster"){
        fileType = ".tif";
        opt::getFileList(inputPath, fileList, fileType);
        DcSTMC a;
        a.Run(fileList, outputPath, 10, 1);
    }else if(functionName == "-p" || functionName == "--Postprocess"){
        fileType = ".hdf";
        vector<string> fileList2;
        opt::getFileList(inputPath, fileList, fileType);
        opt::getFileList(argv[4], fileList2, fileType);
        Postprocessor::Resort(fileList, fileList2, outputPath);
    }else if(functionName == "-v" || functionName == "--Vectorization"){
        vector<string> fileList2;
        opt::getFileList(inputPath, fileList, string(".tif"));
        opt::getFileList(argv[4], fileList2, string(".hdf"));
        Vectorization(fileList, fileList2, outputPath);
    }else if(functionName == "-rc" || functionName == "RTreeCluster"){
        struct RTreeParam p{
                3,
                8,
                TimeUnit::Day,
                5,
                5.0
        };
        fileType = ".tif";
        opt::getFileList(inputPath, fileList, fileType);
        RTree::Run(p, fileList, outputPath);
    }
    else{
        cout << "Following is all function:\n"
        << "1) -g / --GeoTiff2HDF\n"
        << "2) -sa / --StandAnomaly\n"
        << "3) -r / --Resample\n"
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

