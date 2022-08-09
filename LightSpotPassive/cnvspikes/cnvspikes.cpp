// cnvspikes.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <fstream>
#include <algorithm>
#include <random>
#include "../LightSpotPassive.h"

using namespace std;

class RandomNumberGenerator
{
	uniform_real_distribution<> urd;
	mt19937_64                  mt;
public:
	RandomNumberGenerator() : urd(0., 1.) {}
	double operator()() { return urd(mt); }
	template<class T> T operator()(T max) { return (T)((*this)() * max); }
};

const int ntactCalibration = 100000;
const int ntactRecord = 1000000;
const int nSpatialZones = 10;
const int nVelocityZones = 5;
const float rInputIntensity = 0.3;
const int nInputNodes = nSpatialZones * nSpatialZones * nVelocityZones * nVelocityZones;

RandomNumberGenerator rng;

int main(int ARGC, char *ARGV[])
{
	vector<vector<unsigned char> > vvuc_;
	vector<float> vr_PhaseSpacePoint(4);
	vector<vector<float> > vvr_(4);
	vector<vector<float> > vvr_ZoneBoundaries(4);
	SetParameters();
	for (int i = 0; i < ntactCalibration; ++i) {
		GenerateSignals(vvuc_, vr_PhaseSpacePoint);
		for (size_t j = 0; j < vvr_.size(); ++j) 
			vvr_[j].push_back(vr_PhaseSpacePoint[j]);
	}
	sort(vvr_[0].begin(), vvr_[0].end());
	vvr_ZoneBoundaries[0].resize(nSpatialZones - 1);
	for (int i = 1; i < nSpatialZones; ++i)
		vvr_ZoneBoundaries[0][i - 1] = vvr_[0][i * ntactCalibration / nSpatialZones];
	sort(vvr_[1].begin(), vvr_[1].end());
	vvr_ZoneBoundaries[1].resize(nSpatialZones - 1);
	for (int i = 1; i < nSpatialZones; ++i)
		vvr_ZoneBoundaries[1][i - 1] = vvr_[1][i * ntactCalibration / nSpatialZones];
	sort(vvr_[2].begin(), vvr_[2].end());
	vvr_ZoneBoundaries[2].resize(nVelocityZones - 1);
	for (int i = 1; i < nVelocityZones; ++i)
		vvr_ZoneBoundaries[2][i - 1] = vvr_[2][i * ntactCalibration / nVelocityZones];
	sort(vvr_[3].begin(), vvr_[3].end());
	vvr_ZoneBoundaries[3].resize(nVelocityZones - 1);
	for (int i = 1; i < nVelocityZones; ++i)
		vvr_ZoneBoundaries[3][i - 1] = vvr_[3][i * ntactCalibration / nVelocityZones];
	vector<int> vind_(4);
	ofstream ofstxt(ARGV[1]);
	for (int i = 0; i < ntactRecord; ++i) {
		GenerateSignals(vvuc_, vr_PhaseSpacePoint);
		if (rng() > rInputIntensity) 
			for (int k = 0; k < nInputNodes; ++k)
				ofstxt << '.';
		else {
			for (size_t j = 0; j < vvr_.size(); ++j)
				vind_[j] = lower_bound(vvr_ZoneBoundaries[j].begin(), vvr_ZoneBoundaries[j].end(), vr_PhaseSpacePoint[j]) - vvr_ZoneBoundaries[j].begin();
			int indInput = vind_[0] + vind_[1] * nSpatialZones + vind_[2] * nSpatialZones * nSpatialZones + vind_[3] * nSpatialZones * nSpatialZones * nVelocityZones;
		}
	}
}
