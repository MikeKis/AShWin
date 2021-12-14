// LightSpotPassiveClustering.cpp : Определяет экспортированные функции для приложения DLL.
//


#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>
#include <array>
#include <math.h>

#include "../../AShWinCommon.h"

const double dVelocityCoordinateMetricFactor = 200.;
const double dPhaseSpaceDeviation = 1.2;

using namespace std;

size_t StartTime;
size_t OperationTime = 0xffffffff;
size_t MeasurementTime = 0xffffffff;
vector<array<double, 4> > PhaseSpacePoint;
int ExpId;
vector<array<double, 4> > NeuronCenter;
vector<size_t> vn_NeuronSpikes;
int TimeQuant = 30;
double derr2 = 0.;
int nGoodQuants = 0;

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(int ExperimentId, size_t tactTermination, const pugi::xml_node &xn)
{
	auto starttime = xn.child("start_time");
	StartTime = 0;
	if (starttime)
		StartTime = atoi_s(xn.child_value("start_time"));
	OperationTime = atoi_s(xn.child_value("operation_time"));
	if (StartTime + OperationTime > tactTermination)
		OperationTime = tactTermination - StartTime;
	MeasurementTime = atoi_s(xn.child_value("measurement_time"));
	if (MeasurementTime >= OperationTime)
		throw std::runtime_error("invalid settings");
	TimeQuant = atoi_s(xn.child_value("time_quant"));
	size_t tact = 0;
	ifstream ifscsv("passive.csv");
	string str;
	while (getline(ifscsv, str).good()) {
		if (tact >= StartTime) {
			stringstream ss(str);
			char ch;
			array<double, 4> a;
			FORI(4) {
				ss >> a[_i];
				ss >> ch;
			}
			PhaseSpacePoint.push_back(a);
		}
		if (++tact == StartTime + OperationTime)
			break;
	}
	if (tact < StartTime + OperationTime)
		throw std::runtime_error("passive.csv - unexpected eof");
	ExpId = ExperimentId;
}

size_t tact = 0;

DYNAMIC_LIBRARY_ENTRY_POINT bool ObtainOutputSpikes(const vector<int> &v_Firing, int nEquilibriumPeriods)
{
	int j;
	if (tact >= StartTime) {
		if (tact < StartTime + OperationTime - MeasurementTime)
			for (auto i: v_Firing) {
				if (i >= vn_NeuronSpikes.size()) {
					vn_NeuronSpikes.resize(i + 1, 0);
					NeuronCenter.resize(i + 1);
				}
				FORI(4) {
					NeuronCenter[i][_i] = (NeuronCenter[i][_i] * vn_NeuronSpikes[i] + PhaseSpacePoint[tact - StartTime][_i]) / (vn_NeuronSpikes[i] + 1);
					++vn_NeuronSpikes[i];
				}
			}
		else {
			static vector<int> vn_inQuant(vn_NeuronSpikes.size(), 0);
			static int ninQuantTotal = 0;
			for (auto i: v_Firing) {
				if (i < vn_inQuant.size())
					++vn_inQuant[i];
				++ninQuantTotal;
			}
			if ((tact - (StartTime + OperationTime - MeasurementTime)) % TimeQuant == TimeQuant - 1) {
				bool bGoodQuant = false;
				double adReal[4] = { 0., 0., 0., 0. };
				for (size_t k = tact - (TimeQuant - 1); k <= tact; ++k) {
					if (abs(PhaseSpacePoint[k - StartTime][0]) < 0.5 && abs(PhaseSpacePoint[k - StartTime][1]) < 0.5)
						bGoodQuant = true;
					FORI(4)
						adReal[_i] += PhaseSpacePoint[k - StartTime][_i];
				}
				if (bGoodQuant) {
					FORI(4)
						adReal[_i] /= TimeQuant;
					double adPredicted[4] = { 0., 0., 0., 0. };
					if (ninQuantTotal) {
						FOR_(j, 4) {
							adPredicted[j] = 0.;
							FORI(vn_inQuant.size())
								adPredicted[j] += NeuronCenter[_i][j] * vn_inQuant[_i];
						}
						FORI(4)
							adPredicted[_i] /= ninQuantTotal;
					}
					derr2 += (adPredicted[0] - adReal[0]) * (adPredicted[0] - adReal[0]) +
						     (adPredicted[1] - adReal[1]) * (adPredicted[1] - adReal[1]) +
						     dVelocityCoordinateMetricFactor * dVelocityCoordinateMetricFactor * ((adPredicted[2] - adReal[2]) * (adPredicted[2] - adReal[2]) + (adPredicted[3] - adReal[3]) * (adPredicted[3] - adReal[3]));
					++nGoodQuants;
				}
				fill(vn_inQuant.begin(), vn_inQuant.end(), 0);
				ninQuantTotal = 0;
			}
		}
	}
	++tact;
	return OperationTime > 0;
}
DYNAMIC_LIBRARY_ENTRY_POINT int Finalize(int OriginalTerminationCode)
{
	if (OriginalTerminationCode < 0 || !OriginalTerminationCode && tact < StartTime + OperationTime)
		return OriginalTerminationCode;
	if (tact < StartTime || tact < StartTime + OperationTime)
		return 0;
	int ret = (int)(10000 - 5000 * sqrt(derr2 / nGoodQuants) / dPhaseSpaceDeviation);  
	return max(1, ret);
}


