#include <iostream>
#include <string>
#include <vector>
#include "Convertor.h"
#include "AnomalyAnalysis.h"
#include "opt.h"
#include "DcSTMC.h"
#include "Postprocessor.h"
#include "Vectorization.h"

using namespace std;

void func(std::vector<int>& vec){
    vec.push_back(1);
}

int main()
{
	cout << "Chose the function by number" << endl;
	cout << "1) GeoTiff2HDF" << endl;
	cout << "2) StandAnomaly" << endl;
	cout << "3) Resample" << endl;
	cout << "4) SpaceTransform" << endl;
	cout << "5) SpatiotemporalAnomaly" << endl;
	cout << "6) Cluster" << endl;
	cout << "7) Preprocess" << endl;
	cout << "8) Vectorization" << endl;

	int chosedFunction;
	//for (;;) {
	cin >> chosedFunction;

	switch (chosedFunction)
	{
	case 1: {
		string fileDirectory = "E:\\IMERG\\ori\\tif";
		string fileType = ".tif";

		string saveDir("E:\\IMERG\\ori\\hdf");
		vector<string> subFileDirectory;
		opt::getFileList(fileDirectory, subFileDirectory);
		for (int i = 0; i < subFileDirectory.size(); ++i) {
			vector<string> filePath;
			string dir = subFileDirectory[i];
			string year = dir.substr(dir.size() - 4, 4);
			if (stoi(year) >= 2017 && stoi(year) < 2021) {
				string savePath = saveDir + "\\" + year;
				opt::getFileList(dir, filePath, fileType);
				Convertor::GeoTiff2HDF(filePath, savePath, 60, -60);
			}
		}

		break;
	}
	case 2: {
		string fileDirectory = "E:\\IMERG\\test2\\ori"; // "E:\\IMERG\\ori\\hdf";
		string fileType = ".hdf";

		vector<string> list;
		opt::getFileList(fileDirectory, list, fileType);

		string savePath = "E:\\IMERG\\test2\\anomaly"; //"E:\\IMERG\\anomaly";

		bool result = AnomalyAnalysis::StandardAnomaly(list, savePath, AnomalyAnalysis::Test);

		cout << result;
		break;
	}
	case 3: {
		vector<string> fileSet;
        string fileDirectory = "E:\\IMERG\\test\\anomaly"; // "E:\\IMERG\\ori\\hdf";
        string fileType = ".hdf";
		opt::getFileList(fileDirectory, fileSet, fileType);
		string output = "E:\\IMERG\\test\\resample";
		Convertor::Resample(fileSet, output, 1.0);
		break;
	}
	case 4: {
		vector<string> fileSet;
        string fileDirectory = "E:\\IMERG\\test\\resample"; // "E:\\IMERG\\ori\\hdf";
        string fileType = ".hdf";
		opt::getFileList(fileDirectory, fileSet, fileType);
		string output = "E:\\IMERG\\test\\spaceTransform";
		Convertor::SpaceTransform(fileSet, output);
		break;
	}
	case 5: {
		vector<string> fileSet;
        string fileDirectory = "E:\\IMERG\\test\\spaceTransform"; // "E:\\IMERG\\ori\\hdf";
        string fileType = ".hdf";
		opt::getFileList(fileDirectory, fileSet, fileType);
		string output = "E:\\IMERG\\test\\std_05";
		AnomalyAnalysis::SpatiotemporalAnomaly(fileSet, output, 0.5);
		break;
	}
	case 6: {
		vector<string> fileSet;
        string fileDirectory = "E:\\IMERG\\test\\std_05\\pos"; // "E:\\IMERG\\ori\\hdf";
        string fileType = ".tiff";
		opt::getFileList(fileDirectory, fileSet, fileType);
		string output = "E:\\IMERG\\test\\cluster_std05_T1";
		DcSTMC a;
		a.Run(fileSet, output, 10, 1);
		break;
	}
	case 7: {
		vector<string> fileSet0, fileSet1;
		opt::getFileList(string("E:\\IMERG\\test\\cluster_std05_T1\\RepeatFile"), fileSet0, string(".hdf"));
		opt::getFileList(string("E:\\IMERG\\test\\cluster_std05_T1\\OtherFile"), fileSet1, string(".hdf"));
		string output = "E:\\IMERG\\test\\postprocess";
		Postprocessor::Resort(fileSet0, fileSet1, output);
		break;
	}
	case 8: {
		vector<string> fileSet0, fileSet1;
		opt::getFileList(string("E:\\IMERG\\test\\std_05\\pos"), fileSet0, string(".tif"));
		opt::getFileList(string("E:\\IMERG\\test\\postprocess"), fileSet1, string(".hdf"));
		string output = "E:\\IMERG\\test\\vectorization";
		Vectorization(fileSet0, fileSet1, output);
		break;
	}

	case 0: {
		vector<string> fileSet0, fileSet1;
		opt::getFileList(string("E:\\IMERG\\test\\tmp\\vec\\ori"), fileSet0, string(".tif"));
		opt::getFileList(string("E:\\IMERG\\test\\tmp\\vec\\spa"), fileSet1, string(".hdf"));
		string output = "E:\\IMERG\\test\\tmp\\vec";
		Vectorization(fileSet0, fileSet1, output);
		break;
	}

    case 11:{
        std::vector<int> vec;
        func(vec);
        cout<<vec[0]<< endl;
    }
	default:
		break;
	}
	//}
	//return a.exec();
	return 0;
}

