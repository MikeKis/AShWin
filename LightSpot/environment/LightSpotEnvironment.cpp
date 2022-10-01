/*
Copyright (C) 2021 Mikhail Kiselev
Автор Михаил Витальевич Киселев.
При модификации файла сохранение указания (со)авторства Михаила Витальевича Киселева обязательно.

Emulates signal from videocamera looking at a moving light spot.

*/

#define _USE_MATH_DEFINES

#include <utility>
#include <math.h>
#include <random>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <sg/sg.h>

#include "../../AShWinCommon.h"
#include "../../DVSEmulator/dvsemulator.h"
#include "LightSpotEnvironment.h"

#ifdef FOR_LINUX
#define LIGHTSPOTENVIRONMENT_EXPORT extern "C"
#else
#define LIGHTSPOTENVIRONMENT_EXPORT __declspec(dllexport)
#endif

using namespace std;
using namespace boost::interprocess;

#define CAMERA_SIZE 20
const double dPixelSize = 1. / EXACT_RASTER_SIZE;
const double dSpotSize = 0.01;
const unsigned CameraPixelSize_pixels = EXACT_RASTER_SIZE / CAMERA_SIZE;
const double dSpotHalfsize = SPOT_HALFSIZE_PIXEL * dPixelSize;
const double dlogIntensitySensitivityThreshold = pow(dSpotHalfsize / dSpotSize, 2) * -0.5;

const float rTargetDVSIntensity = 0.003F;

const unsigned minSpotPassageTime_ms = 300;
const unsigned maxSpotPassageTime_ms = 1000;

const double dFriction = 0.03;
const float rSpikeEffect = 0.03F;
const double dTargetRange = 0.15;
const int TargetReachedSpikePeriod = 6;

const double dDistanceChangeThreshold = 0.1;

class RandomNumberGenerator
{
	uniform_real_distribution<> urd;
	mt19937_64                  mt;
public:
	RandomNumberGenerator() : urd(0., 1.) {}
	double operator()() { return urd(mt); }
	template<class T> T operator()(T max) {return (T)((*this)() * max);}
	void Randomize(void)
	{
		mt19937_64 mtRandomized(std::random_device{}());
		mt = mtRandomized;
	}
};

float SpotRaster[2 * SPOT_HALFSIZE_PIXEL][2 * SPOT_HALFSIZE_PIXEL];

class EnvironmentState
{
	unique_ptr<shared_memory_object> shm;
	unique_ptr<mapped_region>        region;
	string                           strSharedMemoryName;
public:
	pair<float, float> *pprr_CameraCenter;
	pair<float, float> *pprr_SpotCenter = NULL;
	EnvironmentState() 
	{
		strSharedMemoryName = ENVIRONMENT_STATE_SHARED_MEMORY_NAME;
		bool bExists = true;
		do {
			try {
				//Create a shared memory object.
				shm.reset(new shared_memory_object(open_only, strSharedMemoryName.c_str(), read_only));
				++strSharedMemoryName.front();
			}
			catch (...) {
				bExists = false;
			}
		} while (bExists);
		//Create a shared memory object.
		shm.reset(new shared_memory_object(create_only, strSharedMemoryName.c_str(), read_write));

		//Set size
		shm->truncate(sizeof(pair<pair<float, float>, pair<float, float> >));

		//Map the whole shared memory in this process
		region.reset(new mapped_region(*shm, read_write));
        pprr_CameraCenter = (pair<float, float> *)region->get_address();
	}
	~EnvironmentState()	{shared_memory_object::remove(strSharedMemoryName.c_str());}
	double dDistance() const 
	{
		return sqrt((pprr_CameraCenter->first - pprr_SpotCenter->first) * (pprr_CameraCenter->first - pprr_SpotCenter->first) + (pprr_CameraCenter->second - pprr_SpotCenter->second) * (pprr_CameraCenter->second - pprr_SpotCenter->second));
	}
} es;

pair<float, float> prr_SpotSpeed;
pair<float, float> prr_CameraSpeed(0.F, 0.F);
pair<int, int> p_SpotUpperLeftCornerRelativetoCamera_pixels(0x7fffffff, 0x7fffffff);

RandomNumberGenerator rng;

float rMakeSpotVelocity(void)
{
	int i1, i2;
	do {
		i1 = minSpotPassageTime_ms + rng(maxSpotPassageTime_ms - minSpotPassageTime_ms);
		i2 = rng(maxSpotPassageTime_ms);
	} while (i2 > i1);
	return(1.F / i1);
}

vector<vector<unsigned char> > vvuc_Last(CAMERA_SIZE, vector<unsigned char>(CAMERA_SIZE));

void GenerateSignals(vector<vector<unsigned char> > &vvuc_, vector<float> &vr_PhaseSpacePoint)
{
	int x, y;
	if (!es.pprr_SpotCenter) {
		es.pprr_SpotCenter = &((pair<pair<float, float>, pair<float, float> > *)es.pprr_CameraCenter)->second;
		es.pprr_CameraCenter->first = (float)(-0.5 + rng());
		es.pprr_CameraCenter->second = (float)(-0.5 + rng());
		es.pprr_SpotCenter->first = (float)(-1. + 2 * rng());
		es.pprr_SpotCenter->second = (float)(-1 + 2 * rng());
		float rSpotVelocity = rMakeSpotVelocity();
		float rSpotMovementDirection = (float)rng(2 * M_PI);
		prr_SpotSpeed.first = rSpotVelocity * sin(rSpotMovementDirection);
		prr_SpotSpeed.second = rSpotVelocity * cos(rSpotMovementDirection);
	}

	// мы сохраняем координаты и скорость светового пятна в координатах камеры

	vr_PhaseSpacePoint[0] = es.pprr_SpotCenter->first - es.pprr_CameraCenter->first;
	vr_PhaseSpacePoint[1] = es.pprr_SpotCenter->second - es.pprr_CameraCenter->second;
	vr_PhaseSpacePoint[2] = prr_SpotSpeed.first;
	vr_PhaseSpacePoint[3] = prr_SpotSpeed.second;

	// Это координаты верхнего левого угла картинки пятна относительно верхнего правого угла камеры. Учитываем, что пихельная Y-координата идет против метрической координаты (сверху вниз)!

	pair<int, int> p_NewSpotUpperLeftCornerRelativetoCamera_pixels(
		(int)((es.pprr_SpotCenter->first - dSpotHalfsize - (es.pprr_CameraCenter->first - 0.5)) / dPixelSize),
		(int)((es.pprr_CameraCenter->second + 0.5 - (es.pprr_SpotCenter->second + dSpotHalfsize)) / dPixelSize)
	);

	if (p_NewSpotUpperLeftCornerRelativetoCamera_pixels != p_SpotUpperLeftCornerRelativetoCamera_pixels) {
		p_SpotUpperLeftCornerRelativetoCamera_pixels = p_NewSpotUpperLeftCornerRelativetoCamera_pixels;
		FOR_(y, CAMERA_SIZE)
			FOR_(x, CAMERA_SIZE) {
                double d = 0.;
                int x1, y1;
                pair<int, int> p_CameraPixelUpperLeftCorner(x * CameraPixelSize_pixels, y * CameraPixelSize_pixels);
                FOR_(y1, CameraPixelSize_pixels) {
                    int iy = p_CameraPixelUpperLeftCorner.second + y1;
                    int yinSpot = iy - p_NewSpotUpperLeftCornerRelativetoCamera_pixels.second;
                    if (yinSpot >= 0 && yinSpot < 2 * SPOT_HALFSIZE_PIXEL)
                        FOR_(x1, CameraPixelSize_pixels) {
                        int ix = p_CameraPixelUpperLeftCorner.first + x1;
                        int xinSpot = ix - p_NewSpotUpperLeftCornerRelativetoCamera_pixels.first;
                        if (xinSpot >= 0 && xinSpot < 2 * SPOT_HALFSIZE_PIXEL && yinSpot >= 0 && yinSpot < 2 * SPOT_HALFSIZE_PIXEL) {
                            auto d1 = SpotRaster[yinSpot][xinSpot];
                            d += d1;
                        }
                    }
                }
                if (d) {
                    d /= CameraPixelSize_pixels * CameraPixelSize_pixels;
                    auto d2 = log(d);
                    vvuc_Last[y][x] = d2 < dlogIntensitySensitivityThreshold ? 0 : (unsigned char)((dlogIntensitySensitivityThreshold - d2) / dlogIntensitySensitivityThreshold * 255);
                } else vvuc_Last[y][x] = 0;
            }
	}
	vvuc_ = vvuc_Last;
	es.pprr_SpotCenter->first += prr_SpotSpeed.first;
	if (es.pprr_SpotCenter->first < -1.) {
		es.pprr_SpotCenter->first = (float)(-1. + dPixelSize / 2);
		float rVelocity = rMakeSpotVelocity();
		auto rMovementDirection = rng(M_PI);
		prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
		prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
	}
	if (es.pprr_SpotCenter->first >= 1.) {
		es.pprr_SpotCenter->first = (float)(1. - dPixelSize / 2);
		float rVelocity = rMakeSpotVelocity();
		auto rMovementDirection = M_PI + rng(M_PI);
		prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
		prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
	}
	es.pprr_SpotCenter->second += prr_SpotSpeed.second;
	if (es.pprr_SpotCenter->second < -1.) {
		es.pprr_SpotCenter->second = (float)(-1. + dPixelSize / 2);
		float rVelocity = rMakeSpotVelocity();
		auto rMovementDirection = rng(M_PI) - M_PI / 2;
		prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
		prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
	}
	if (es.pprr_SpotCenter->second >= 1.) {
		es.pprr_SpotCenter->second = (float)(1. - dPixelSize / 2);
		float rVelocity = rMakeSpotVelocity();
		auto rMovementDirection = M_PI / 2 + rng(M_PI);
		prr_SpotSpeed.first = (float)(rVelocity * sin(rMovementDirection));
		prr_SpotSpeed.second = (float)(rVelocity * cos(rMovementDirection));
	}
	es.pprr_CameraCenter->first += prr_CameraSpeed.first;
	if (es.pprr_CameraCenter->first < -0.5) {
		es.pprr_CameraCenter->first = -0.5F;
		prr_CameraSpeed.first = 0.F;
	}
	if (es.pprr_CameraCenter->first > 0.5) {
		es.pprr_CameraCenter->first = 0.5F;
		prr_CameraSpeed.first = 0.F;
	}
	es.pprr_CameraCenter->second += prr_CameraSpeed.second;
	if (es.pprr_CameraCenter->second < -0.5) {
		es.pprr_CameraCenter->second = -0.5F;
		prr_CameraSpeed.second = 0.F;
	}
	if (es.pprr_CameraCenter->second > 0.5) {
		es.pprr_CameraCenter->second = 0.5F;
		prr_CameraSpeed.second = 0.F;
	}
	if (prr_CameraSpeed.first || prr_CameraSpeed.second) {
		double dCameraSpeed = sqrt(prr_CameraSpeed.first * prr_CameraSpeed.first + prr_CameraSpeed.second * prr_CameraSpeed.second);
		double dCameraSpeedNew = dCameraSpeed - dFriction;
		if (dCameraSpeedNew > 0.) {
			double d = dCameraSpeedNew / dCameraSpeed;
			prr_CameraSpeed.first *= (float)d;
			prr_CameraSpeed.second *= (float)d;
		}
		else prr_CameraSpeed.first = prr_CameraSpeed.second = 0.F;
	}
}

class DYNAMIC_LIBRARY_EXPORTED_CLASS DVSCamera: public IReceptors, public DVSEmulator
{
protected:
	virtual bool bGenerateReceptorSignals(char *prec, size_t neuronstrsize) override
	{
		vector<vector<unsigned char> > vvuc_;
		vector<float> vr_PhaseSpacePoint(4);
		GenerateSignals(vvuc_, vr_PhaseSpacePoint);
		vector<bool> vb_Spikes(GetSpikeSignalDim());
		AddFrame(vvuc_, &vb_Spikes);
		for (auto i: vb_Spikes) {
			*prec = i;
			prec += neuronstrsize;
		}
		return true;
	}
public:
	DVSCamera(): IReceptors(CAMERA_SIZE * CAMERA_SIZE * 3, 3), DVSEmulator(CAMERA_SIZE, CAMERA_SIZE) {}
	virtual void Randomize(void) override {rng.Randomize();}
    virtual void SaveStatus(Serializer &ser) const override
    {
        IReceptors::SaveStatus(ser);
        ser << *((const DVSEmulator *)this);
        ser << *es.pprr_CameraCenter;
        ser << *es.pprr_SpotCenter;
        ser << prr_SpotSpeed;
        ser << prr_CameraSpeed;
        ser << p_SpotUpperLeftCornerRelativetoCamera_pixels;
        ser << rng;
        ser << vvuc_Last;
    }
	virtual ~DVSCamera() = default;
    void LoadStatus(Serializer &ser)
    {
        IReceptors::LoadStatus(ser);
        ser >> *((DVSEmulator *)this);
        ser >> *es.pprr_CameraCenter;
        es.pprr_SpotCenter = &((pair<pair<float, float>, pair<float, float> > *)es.pprr_CameraCenter)->second;
        ser >> *es.pprr_SpotCenter;
        ser >> prr_SpotSpeed;
        ser >> prr_CameraSpeed;
        ser >> p_SpotUpperLeftCornerRelativetoCamera_pixels;
        ser >> rng;
        ser >> vvuc_Last;
    }
};

int ntact = 0;
vector<int> vn_TactsInside;

class DYNAMIC_LIBRARY_EXPORTED_CLASS Evaluator: public IReceptors
{
	double      dCurrentDistance;
	int         TargetReachedSpikeCnt;
public:
	Evaluator(bool bRew): IReceptors(1, 3), TargetReachedSpikeCnt(bRew ? 0 : -1), dCurrentDistance(es.dDistance()) {}
	virtual bool bGenerateReceptorSignals(char *prec, size_t neuronstrsize) override
	{
		double dNewDistance = es.dDistance();
		double d = dNewDistance - dCurrentDistance;
		*prec = 0;
		if (d > dDistanceChangeThreshold) {
			dCurrentDistance = dNewDistance;
			if (!bReward() /* && ntact >= 1000000 */)
				*prec = 1;
		} else if (d < -dDistanceChangeThreshold) {
			dCurrentDistance = dNewDistance;
			if (bReward() /* && ntact >= 1000000 */)
				*prec = 1;
		}
		if (bReward()) {
			if (!(ntact % 200000)) {
				vn_TactsInside.push_back(0);
			}
			if (dNewDistance < dTargetRange) {
				if (++TargetReachedSpikeCnt == TargetReachedSpikePeriod) {
					*prec = 1;
					TargetReachedSpikeCnt = 0;
				}
				++vn_TactsInside.back();
			}
			++ntact;
		}
		return true;
	}
	virtual void Randomize(void) override {};
    virtual void SaveStatus(Serializer &ser) const override
    {
        IReceptors::SaveStatus(ser);
        ser << dCurrentDistance;
        ser << TargetReachedSpikeCnt;
        ser << ntact;
        ser << vn_TactsInside;
    }
    virtual ~Evaluator() = default;
    void LoadStatus(Serializer &ser)
    {
        IReceptors::LoadStatus(ser);
        ser >> dCurrentDistance;
        ser >> TargetReachedSpikeCnt;
        ser >> ntact;
        ser >> vn_TactsInside;
    }
    bool bReward() const {return TargetReachedSpikeCnt != -1;}
};

LIGHTSPOTENVIRONMENT_EXPORT IReceptors *SetParametersIn(int &nReceptors, const pugi::xml_node &xn)
{
	static int CallNo = 0;
	int x, y;
	DVSCamera *pdvs;
	vector<vector<unsigned char> > vvuc_;
	vector<float> vr_PhaseSpacePoint(4);
	switch (CallNo++) {
		case 0: FOR_(x, 2 * SPOT_HALFSIZE_PIXEL)
					FOR_(y, 2 * SPOT_HALFSIZE_PIXEL) {
						double d2 = (x + 0.5 - SPOT_HALFSIZE_PIXEL) * (x + 0.5 - SPOT_HALFSIZE_PIXEL) + (y + 0.5 - SPOT_HALFSIZE_PIXEL) * (y + 0.5 - SPOT_HALFSIZE_PIXEL);
						SpotRaster[x][y] = (float)exp(-d2 * dPixelSize * dPixelSize / (2 * dSpotSize * dSpotSize));
					}
				nReceptors = CAMERA_SIZE * CAMERA_SIZE * 3;
				pdvs = new DVSCamera;
				FORI(10000) {
					GenerateSignals(vvuc_, vr_PhaseSpacePoint);
					pdvs->AddFrame(vvuc_);
				}
				cout << "calilbration signal generated\n";
				pdvs->Calibrate(rTargetDVSIntensity);
				cout << "calilbration done\n";
				return pdvs;
		case 1: return new Evaluator(false);
		case 2: return new Evaluator(true);
		default: cout << "Too many calls of SetParametersIn\n";
			    exit(-1);
	}
}

LIGHTSPOTENVIRONMENT_EXPORT IReceptors *LoadStatus(Serializer &ser)
{
    static int CallNo = 0;
    int x, y;
    DVSCamera *pdvs;
    Evaluator *peva;
    vector<vector<unsigned char> > vvuc_;
    vector<float> vr_PhaseSpacePoint(4);
    switch (CallNo++) {
        case 0: FOR_(x, 2 * SPOT_HALFSIZE_PIXEL)
                    FOR_(y, 2 * SPOT_HALFSIZE_PIXEL) {
                        double d2 = (x + 0.5 - SPOT_HALFSIZE_PIXEL) * (x + 0.5 - SPOT_HALFSIZE_PIXEL) + (y + 0.5 - SPOT_HALFSIZE_PIXEL) * (y + 0.5 - SPOT_HALFSIZE_PIXEL);
                        SpotRaster[x][y] = (float)exp(-d2 * dPixelSize * dPixelSize / (2 * dSpotSize * dSpotSize));
                    }
                pdvs = new DVSCamera;
                pdvs->LoadStatus(ser);
                return pdvs;
        case 1: peva = new Evaluator(false);
                peva->LoadStatus(ser);
                return peva;
        case 2: peva = new Evaluator(true);
                peva->LoadStatus(ser);
                return peva;
        default: cout << "Too many calls of SetParametersIn\n";
                exit(-1);
    }
}

LIGHTSPOTENVIRONMENT_EXPORT void SetParametersOut(int ExperimentId, size_t tactTermination, const pugi::xml_node &xn) {}

LIGHTSPOTENVIRONMENT_EXPORT bool ObtainOutputSpikes(const vector<int> &v_Firing, int nEquilibriumPeriods)
{
	for (auto i: v_Firing) {
		int Direction = i / 3;
		switch (Direction) {
			case 0: prr_CameraSpeed.second += rSpikeEffect;
				    break;
			case 1: prr_CameraSpeed.first += rSpikeEffect;
				    break;
			case 2: prr_CameraSpeed.second -= rSpikeEffect;
				    break;
			case 3: prr_CameraSpeed.first -= rSpikeEffect;
				    break;
		}
	}
	return true;
}

LIGHTSPOTENVIRONMENT_EXPORT int Finalize(int OriginalTerminationCode) 
{
	cout << "Tacts inside:\n";
	for (auto z: vn_TactsInside)
		cout << z << endl;
	return vn_TactsInside.back() / 20;
}

LIGHTSPOTENVIRONMENT_EXPORT void Serialize(Serializer &ser, bool bSave){}
