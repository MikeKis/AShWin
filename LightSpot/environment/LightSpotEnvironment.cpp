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

#include "../../AShWinCommon.h"

#ifdef FOR_LINUX
#define LIGHTSPOTENVIRONMENT_EXPORT
#else
#define LIGHTSPOTENVIRONMENT_EXPORT __declspec(dllexport)
#endif

using namespace std;

#define CAMERA_SIZE 20
#define SPOT_HALFSIZE_PIXEL 30
const unsigned ExactRasterSize = 100;
const double dPixelSize = 1. / ExactRasterSize;
const double dSpotSize = 0.01;
const unsigned CameraPixelSize_pixels = ExactRasterSize / CAMERA_SIZE;
const double dSpotHalfsize = SPOT_HALFSIZE_PIXEL * dPixelSize;
const double dlogIntensitySensitivityThreshold = pow(dSpotHalfsize / dSpotSize, 2) * -0.5;

const unsigned minSpotPassageTime_ms = 100;
const unsigned maxSpotPassageTime_ms = 300;

const double dFriction = 0.03;

class RandomNumberGenerator
{
	uniform_real_distribution<> urd;
	mt19937_64                  mt;
public:
	RandomNumberGenerator() : urd(0., 1.) {}
	double operator()() { return urd(mt); }
	template<class T> T operator()(T max) { return (T)((*this)() * max); }
};

float SpotRaster[2 * SPOT_HALFSIZE_PIXEL][2 * SPOT_HALFSIZE_PIXEL];
pair<float, float> prr_CameraCenter(-1.F, -1.F);
pair<float, float> prr_SpotCenter(-1.F, -1.F);
pair<float, float> prr_SpotSpeed;
pair<float, float> prr_CameraSpeed(0.F, 0.F);
pair<int, int> p_SpotUpperLeftCornerRelativetoCamera_pixels(0x7fffffff, 0x7fffffff);

RandomNumberGenerator rng;

LIGHTSPOTENVIRONMENT_EXPORT void SetParametersIn()
{
	int x, y;
	FOR_(x, 2 * SPOT_HALFSIZE_PIXEL)
		FOR_(y, 2 * SPOT_HALFSIZE_PIXEL) {
			double d2 = (x + 0.5 - SPOT_HALFSIZE_PIXEL) * (x + 0.5 - SPOT_HALFSIZE_PIXEL) + (y + 0.5 - SPOT_HALFSIZE_PIXEL) * (y + 0.5 - SPOT_HALFSIZE_PIXEL);
			SpotRaster[x][y] = (float)exp(-d2 * dPixelSize * dPixelSize / (2 * dSpotSize * dSpotSize));
		}
}

float rMakeSpotVelocity(void)
{
	int i1, i2;
	do {
		i1 = minSpotPassageTime_ms + rng(maxSpotPassageTime_ms - minSpotPassageTime_ms);
		i2 = rng(maxSpotPassageTime_ms);
	} while (i2 > i1);
	return(1.F / i1);
}

LIGHTSPOTENVIRONMENT_EXPORT void GenerateSignals(vector<vector<unsigned char> > &vvuc_, vector<float> &vr_PhaseSpacePoint)
{
	static vector<vector<unsigned char> > vvuc_Last(CAMERA_SIZE, vector<unsigned char>(CAMERA_SIZE));
	int x, y;
	if (prr_CameraCenter.first == -1) {
		prr_CameraCenter.first = (float)(-0.5 + rng());
		prr_CameraCenter.second = (float)(-0.5 + rng());
		prr_SpotCenter.first = (float)(-0.5 + rng());
		prr_SpotCenter.second = (float)(-0.5 + rng());
		float rSpotVelocity = rMakeSpotVelocity();
		float rSpotMovementDirection = (float)rng(2 * M_PI);
		prr_SpotSpeed.first = rSpotVelocity * sin(rSpotMovementDirection);
		prr_SpotSpeed.second = rSpotVelocity * cos(rSpotMovementDirection);
	}

	// мы сохраняем координаты и скорость светового пятна в координатах камеры

	vr_PhaseSpacePoint[0] = prr_SpotCenter.first - prr_CameraCenter.first;
	vr_PhaseSpacePoint[1] = prr_SpotCenter.second - prr_CameraCenter.second;
	vr_PhaseSpacePoint[2] = prr_SpotSpeed.first;
	vr_PhaseSpacePoint[3] = prr_SpotSpeed.second;

	// Это координаты верхнего левого угла картинки пятна относительно верхнего правого угла камеры. Учитываем, что пихельная Y-координата идет против метрической координаты (сверху вниз)!

	pair<int, int> p_NewSpotUpperLeftCornerRelativetoCamera_pixels(
		(int)((prr_SpotCenter.first - dSpotHalfsize - (prr_CameraCenter.first - 0.5)) / dPixelSize),
		(int)((prr_CameraCenter.second + 0.5 - (prr_SpotCenter.second + dSpotHalfsize)) / dPixelSize)
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
			}
			else vvuc_Last[y][x] = 0;
		}
	}
	vvuc_ = vvuc_Last;
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
	prr_CameraCenter.first += prr_CameraSpeed.first;
	if (prr_CameraCenter.first < -0.5) {
		prr_CameraCenter.first = -0.5F;
		prr_CameraSpeed.first = 0.F;
	}
	if (prr_CameraCenter.first > 0.5) {
		prr_CameraCenter.first = 0.5F;
		prr_CameraSpeed.first = 0.F;
	}
	prr_CameraCenter.second += prr_CameraSpeed.second;
	if (prr_CameraCenter.second < -0.5) {
		prr_CameraCenter.second = -0.5F;
		prr_CameraSpeed.second = 0.F;
	}
	if (prr_CameraCenter.second > 0.5) {
		prr_CameraCenter.second = 0.5F;
		prr_CameraSpeed.second = 0.F;
	}
	if (prr_CameraSpeed.first || prr_CameraSpeed.second) {
		double dCameraSpeed = sqrt(prr_CameraSpeed.first * prr_CameraSpeed.first + prr_CameraSpeed.second * prr_CameraSpeed.second);
		double dCameraSpeedNew = dCameraSpeed - dFriction;
		if (dCameraSpeedNew > 0.) {
			double d = dCameraSpeedNew / dCameraSpeed;
			prr_CameraSpeed.first *= d;
			prr_CameraSpeed.second *= d;
		} else prr_CameraSpeed.first = prr_CameraSpeed.second = 0.F;
	}
}


