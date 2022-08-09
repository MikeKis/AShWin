/*
Copyright (C) 2021 Mikhail Kiselev
Автор Михаил Витальевич Киселев.
При модификации файла сохранение указания (со)авторства Михаила Витальевича Киселева обязательно.

Emulates signal from videocamera looking at a moving light spot.

*/

#include <utility>
#include <math.h>
#include <random>
#include <fstream>

#include <sg/sg.h>

#include "../AShWinCommon.h"

#include "LightSpotPassive.h"

const double dPixelSize = 0.01F;

const int ntactCalibration = 100000;
const int nSpatialZones = 10;
const int nVelocityZones = 5;
const int MinInterspikeInterval = 3;
const int MaxInterspikeInterval = 5;
const int nInputNodes = nSpatialZones * nSpatialZones * nVelocityZones * nVelocityZones;

const unsigned minSpotPassageTime_ms = 100;
const unsigned maxSpotPassageTime_ms = 300;

using namespace std;

class RandomNumberGenerator
{
    uniform_real_distribution<> urd;
    mt19937_64                  mt;
public:
    RandomNumberGenerator(): urd(0., 1.) {}
    double operator()() {return urd(mt);}
    template<class T> T operator()(T max){return (T)((*this)() * max);}
};

class DYNAMIC_LIBRARY_EXPORTED_CLASS LightSpot: public IReceptors
{
	RandomNumberGenerator  rng;
	pair<float, float>     prr_SpotCenter;
	pair<float, float>     prr_SpotSpeed;
	vector<vector<float> > vvr_ZoneBoundaries;
	vector<float>          vr_PhaseSpacePoint;

	float rMakeSpotVelocity(void)
	{
		int i1, i2;
		do {
			i1 = minSpotPassageTime_ms + rng(maxSpotPassageTime_ms - minSpotPassageTime_ms);
			i2 = rng(maxSpotPassageTime_ms);
		} while (i2 > i1);
		return(1.F / i1);
	}

	void GenerateSignals(vector<float> &vr_PhaseSpacePoint)
	{
		if (prr_SpotCenter.first == -1) {
			prr_SpotCenter.first = (float)(-0.5 + rng());
			prr_SpotCenter.second = (float)(-0.5 + rng());
			float rSpotVelocity = rMakeSpotVelocity();
			float rSpotMovementDirection = (float)rng(2 * M_PI);
			prr_SpotSpeed.first = rSpotVelocity * sin(rSpotMovementDirection);
			prr_SpotSpeed.second = rSpotVelocity * cos(rSpotMovementDirection);
		}

		vr_PhaseSpacePoint[0] = prr_SpotCenter.first;
		vr_PhaseSpacePoint[1] = prr_SpotCenter.second;
		vr_PhaseSpacePoint[2] = prr_SpotSpeed.first;
		vr_PhaseSpacePoint[3] = prr_SpotSpeed.second;

		prr_SpotCenter.first += prr_SpotSpeed.first;
		if (prr_SpotCenter.first < -1.) {
			prr_SpotCenter.first = (float)(-1. + dPixelSize / 2);
			float rVelocity = rMakeSpotVelocity();
			auto rMovementDirection = rng(M_PI);
			prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
			prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
		}
		if (prr_SpotCenter.first >= 1.) {
			prr_SpotCenter.first = (float)(1. - dPixelSize / 2);
			float rVelocity = rMakeSpotVelocity();
			auto rMovementDirection = M_PI + rng(M_PI);
			prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
			prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
		}
		prr_SpotCenter.second += prr_SpotSpeed.second;
		if (prr_SpotCenter.second < -1.) {
			prr_SpotCenter.second = (float)(-1. + dPixelSize / 2);
			float rVelocity = rMakeSpotVelocity();
			auto rMovementDirection = rng(M_PI) - M_PI / 2;
			prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
			prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
		}
		if (prr_SpotCenter.second >= 1.) {
			prr_SpotCenter.second = (float)(1. - dPixelSize / 2);
			float rVelocity = rMakeSpotVelocity();
			auto rMovementDirection = M_PI / 2 + rng(M_PI);
			prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
			prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
		}
	}

public:
	LightSpot(): prr_SpotCenter(-1.F, -1.F), vvr_ZoneBoundaries(4), vr_PhaseSpacePoint(4)
	{
		vector<vector<float> > vvr_(4);
		for (int i = 0; i < ntactCalibration; ++i) {
			GenerateSignals(vr_PhaseSpacePoint);
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
	}
	virtual bool bGenerateReceptorSignals(char *prec, size_t neuronstrsize) override
	{
		vector<size_t> vind_(4);
		GenerateSignals(vr_PhaseSpacePoint);
		static int counter = 1;
		if (!--counter) {
			for (int k = 0; k < nInputNodes; ++k)
				*(prec + k * neuronstrsize) = false;
			counter = MinInterspikeInterval + rng(MaxInterspikeInterval - MinInterspikeInterval + 1);
		} else {
			for (size_t j = 0; j < vind_.size(); ++j)
				vind_[j] = lower_bound(vvr_ZoneBoundaries[j].begin(), vvr_ZoneBoundaries[j].end(), vr_PhaseSpacePoint[j]) - vvr_ZoneBoundaries[j].begin();
			size_t indInput = vind_[0] + vind_[1] * nSpatialZones + vind_[2] * nSpatialZones * nSpatialZones + vind_[3] * nSpatialZones * nSpatialZones * nVelocityZones;
			for (int k = 0; k < nInputNodes; ++k)
				*(prec + k * neuronstrsize) = k == indInput;
		}
		return true;
	}
	virtual void Randomize(void) override {}
	virtual void SaveStatus(Serializer &ser) const override {}
	virtual ~LightSpot() = default;
};

DYNAMIC_LIBRARY_ENTRY_POINT IReceptors *SetParametersIn(int &nReceptors, const pugi::xml_node &xn) 
{ 
	nReceptors = nInputNodes;
	return new LightSpot;
}

DYNAMIC_LIBRARY_ENTRY_POINT IReceptors *LoadStatus(Serializer &ser){return new LightSpot;}


