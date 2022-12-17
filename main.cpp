#include <iostream>
#include <string>
#include <vector>
#include "algo.h"
#include "RTree.h"
#include "DataProcess.h"
#include "RFileOpt.h"
#include "Hdf5Opt.h"
#include "TifOpt.h"
#include "DataModel.h"
#include "_const.h"

using namespace std;


Meta Meta::DEF(
        1.0,
        1.0,
        120,
        360,
        60,
        -180,
        -60,
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
    string fileFolder = "E:\\pr\\v2\\src";
    string fileType = ".HDF5";
    std::vector<std::string> files;
    util::getFileList(fileFolder, files, fileType);

    RFileOpt* fi = new Hdf5Opt("Grid", "precipitation");
    RFileOpt* fo = new TifOpt();
    Reader reader("E:\\pr\\v2\\", 1, fi, fo);
    reader.readBatch(files, TimeUnit::Mon);
}
int main(int argc,char *argv[])
{
    if(argc <= 1){
        Test_HDF5();
        return 0;
    }

    string functionName = argv[1];
    string outputPath = argv[2];
    string inputPath = argv[3];
    string fileType;
    vector<string> fileList;

    if(functionName == "--HDF5Preprocess" || functionName == "-hp"){

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

