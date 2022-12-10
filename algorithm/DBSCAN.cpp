#include<iostream>
#include <queue>
#include <array>
#include "algo.h"
#include "util/util.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;


using namespace std;

class DBSCAN
{
public:

    static double calculateClusterParameter(vector<string> Files, int K = 15);
    static bool core(vector<string> Files, string outputPath);
    static bool cluster(vector<string> core, string outputPath);
};


const double CORE = INT_MAX;
const double BEG = INT_MIN; // already cluster as beginning

const int SUR[9][2] = { {-1,0}, {-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{0,0}};


/*double DBSCAN::calculateClusterParameter(vector<string> files, int K) {
	vector<double *> data;
	for (int i = 0; i < files.size(); ++i) {
		double* buf = new double[META_DEF.Size];
		data.push_back(buf);
		TifOpt::readGeoTiff(files[i].c_str(), buf);
	}
}*/

bool DBSCAN::core(vector<string> Files, string outputPath) {
	int n = Files.size();
	const long rows = 120;
	const long cols = 360;
	const long size = rows * cols;
	vector<double *> data;
	vector<string> dateList;
	for (int i = 0; i < n; ++i) {
		double* buf = new double[size];
		data.push_back(buf);
		TifOpt::readGeoTiff(Files[i].c_str(), buf);
		dateList.push_back(Files[i].substr(Files[i].find_last_of(".") - 8, 8));
	}

	//TifOpt go(META_DEF);
	hdfOpt ho(META_DEF);
	int t_interval = 1;
	int threshold = (t_interval * 2 + 1) * 3;
	for (int t = t_interval; t < n - t_interval; ++t){
		long* buf = new long[size];
		for (int r = 1; r < rows - 1; ++r) {
			for (int c = 1; c < cols - 1; ++c) {
				//ʱ��������
				int cnt = 0;
				for (int t0 = t - t_interval; t0 <= t + t_interval; ++t0) {
					if (data[t0][r * cols + c] == 0){
						cnt = 0;
						break;
					}
					else
						cnt++;

					int r0, c0, s;
					for (s = 0; s < 8; ++s) {
						r0 = r + SUR[s][0];
						c0 = c + SUR[s][1];
						if (data[t0][r0 * cols + c0] != 0)
							++cnt;
					}
				}

				if (cnt >= threshold)
					//data[t][r * colNum + c] = CORE;
					buf[r * cols + c] = CORE;
				else
					buf[r * cols + c] = data[t][r * cols + c] / META_DEF.scale;
			}
		}	

		string outputFileName = util::generateFileName(Files[t], outputPath, "Core", "hdfOpt");
		Meta meta = META_DEF;
		meta.date = dateList[t];
		if(!ho.writeHDF(outputFileName.c_str(), meta, buf)){
		//if (!go.writeGeoTiff(outputFileName.c_str(), meta, data[t])) {
			cout << "Write Error" << endl;
		}
	}

	for (int i = 0; i < n; ++i) {
		delete[] data[i];
	}

	return true;
}

bool DBSCAN::cluster(vector<string> core, string outputPath) {
	hdfOpt ho(META_DEF);
	vector<long*> data;
	vector<long*> res;
	vector<string> dateList;
	for (string f : core) {
		long* buf = new long[META_DEF.nPixel];
		data.push_back(buf);
		if (!ho.readHDF(f, buf)) {
			cout << "Read Error" << endl;
			return false;
		}
		dateList.push_back(f.substr(f.find_last_of(".") - 8, 8));

		long* tmp = new long[META_DEF.nPixel];
		for (int i = 0; i < META_DEF.nPixel; ++i)
			tmp[i] = 0;
		res.push_back(tmp);
	}
	int cs = META_DEF.nCol;
	int t_interval = 1;
	int threshold = (t_interval * 2 + 1) * 3;
	int n = core.size();
	int eventID = 2;
	for (int t = t_interval; t < n - t_interval; ++t) {
		for (int r = 1; r < META_DEF.nRow - 1; ++r) {
			for (int c = 1; c < META_DEF.nCol - 1; ++c) {
				if (data[t][r * cs + c] != CORE || data[t][r * cs + c] == BEG)
					continue;

				queue<array<int, 3> > q;
				data[t][r * cs + c] = BEG;
				res[t][r * cs + c] = eventID;
				array<int, 3> beginningPoint = {t, r, c};
				q.push(beginningPoint);
				while (!q.empty()) {
					//ʱ��������	
					array<int, 3> coor = q.front();
					q.pop();
					int t1 = coor[0], r1 = coor[1], c1 = coor[2];

					for (int t2 = t1 - t_interval; t2 <= t1 + t_interval; ++t2) {
						int r2, c2, s;
						for (s = 0; s < 9; ++s) {
							r2 = r1 + SUR[s][0];
							c2 = c1 + SUR[s][1];
							if (res[t2][r2 * cs + c2] == 0 && data[t2][r2 * cs + c2] == CORE //?
								&& data[t2][r2 * cs + c2] != BEG && data[t2][r2 * cs + c2] != 0) {
								res[t2][r2 * cs + c2] = eventID;
								if (data[t2][r2 * cs + c2] == CORE) {
									array<int, 3> tmp = {t2, r2, c2};
									q.push(tmp);
								}
							}
						}
					}
				}

				++eventID;
			}
		}
	}

	return true;
}
