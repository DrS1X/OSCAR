#include<iostream>
#include<memory>
#include <queue>
#include <array>
#include "DBSCAN.h"
#include "ClsGeneralOperator.h"
#include "ClsHDF4Operator.h"
#include "ClsGDALOperator.h"
#include "CONST.h"

using namespace std;

const double CORE = INT_MAX;
const double BEG = INT_MIN; // already cluster as beginning

const int SUR[9][2] = { {-1,0}, {-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{0,0}};


double DBSCAN::calculateClusterParameter(vector<string> files, int K) {
	vector<double *> data;
	for (int i = 0; i < files.size(); ++i) {
		double* buf = new double[Def.Size];
		data.push_back(buf);
		CClsGDALOperator::readGeoTiff(files[i].c_str(), buf);
	}
}

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
		CClsGDALOperator::readGeoTiff(Files[i].c_str(), buf);
		dateList.push_back(Files[i].substr(Files[i].find_last_of(".") - 8, 8));
	}

	//CClsGDALOperator go(Def);
	CClsHDF4Operator ho(Def);
	int t_interval = 1;
	int threshold = (t_interval * 2 + 1) * 3;
	for (int t = t_interval; t < n - t_interval; ++t){
		long* buf = new long[size];
		for (int r = 1; r < rows - 1; ++r) {
			for (int c = 1; c < cols - 1; ++c) {
				//时空立方体
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
					//data[t][r * cols + c] = CORE;
					buf[r * cols + c] = CORE;
				else
					buf[r * cols + c] = data[t][r * cols + c] / Def.Scale;
			}
		}	

		string outputFileName = CClsGeneralOperator::generateFileName(Files[t], outputPath, "Core","hdf");
		Meta meta = Def;
		meta.Date = dateList[t];
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
	CClsHDF4Operator ho(Def);
	vector<long*> data;
	vector<long*> res;
	vector<string> dateList;
	for (string f : core) {
		long* buf = new long[Def.Size];
		data.push_back(buf);
		if (!ho.readHDF(f, buf)) {
			cout << "Read Error" << endl;
			return false;
		}
		dateList.push_back(f.substr(f.find_last_of(".") - 8, 8));

		long* tmp = new long[Def.Size];
		for (int i = 0; i < Def.Size; ++i)
			tmp[i] = 0;
		res.push_back(tmp);
	}
	int cs = Def.Cols;
	int t_interval = 1;
	int threshold = (t_interval * 2 + 1) * 3;
	int n = core.size();
	int eventID = 2;
	for (int t = t_interval; t < n - t_interval; ++t) {
		for (int r = 1; r < Def.Rows - 1; ++r) {
			for (int c = 1; c < Def.Cols - 1; ++c) {
				if (data[t][r * cs + c] != CORE || data[t][r * cs + c] == BEG)
					continue;

				queue<array<int, 3> > q;
				data[t][r * cs + c] = BEG;
				res[t][r * cs + c] = eventID;
				array<int, 3> beginningPoint = {t, r, c};
				q.push(beginningPoint);
				while (!q.empty()) {
					//时空立方体	
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
