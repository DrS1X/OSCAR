#include <iostream>
#include <qobject.h>
#include <vector>
#include <string>

using namespace std;

class DBSCAN
{
public:
	
	static double calculateClusterParameter(vector<string> Files, int K = 15);
	static bool core(vector<string> Files, string outputPath);
	static bool cluster(vector<string> core, string outputPath);
};
