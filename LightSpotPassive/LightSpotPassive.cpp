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

#include "AShWinCommon.h"

#define EXACT_RASTER_SIZE 100
#define PIXEL_SIZE (1. / EXACT_RASTER_SIZE)
#define SPOT_SIZE 0.1
#define CAMERA_PASSAGE_TIME_MIN_MS 300
#define CAMERA_PASSAGE_TIME_MAX_MS 1000
#define CAMERA_SIZE 20
#define CAMERA_PIXEL_SIZE_PIXELS (EXACT_RASTER_SIZE / CAMERA_SIZE)
#define LOG_INTENSITY_SENSITIVITY_THRESOLD -4.5
#define MAX_RECEPTOR_INTENSITY_BASE 0.15
#define MAX_RECEPTOR_INTENSITY_SCALED (-255 / LOG_INTENSITY_SENSITIVITY_THRESOLD)

using namespace std;

class RandomNumberGenerator
{
    uniform_real_distribution<> urd;
    mt19937_64                  mt;
public:
    RandomNumberGenerator(): urd(0., 1.) {}
    double operator()() {return urd(mt);}
    template<class T> T operator()(T max){return (*this)() * max;}
};

float SpotRaster[2 * EXACT_RASTER_SIZE][2 * EXACT_RASTER_SIZE];
pair<float,float> prr_CameraCenter(-1.F, -1.F);
pair<float,float> prr_CameraSpeed;
pair<int,int> p_CameraLowerLeftCorner;

RandomNumberGenerator rng;

void SetParameters()
{
	int x, y;
	FOR_(x, 2 * EXACT_RASTER_SIZE)
		FOR_(y, 2 * EXACT_RASTER_SIZE) {
			float r2 = ((x + 0.5 - EXACT_RASTER_SIZE) * (x + 0.5 - EXACT_RASTER_SIZE) + (y + 0.5 - EXACT_RASTER_SIZE) * (y + 0.5 - EXACT_RASTER_SIZE)) / (EXACT_RASTER_SIZE * EXACT_RASTER_SIZE);
			SpotRaster[x][y] = exp(-r2 / (2 * SPOT_SIZE * SPOT_SIZE));
		}
}

float rMakeCameraVelocity(void)
{
	int i1, i2;
	do {
		i1 = CAMERA_PASSAGE_TIME_MIN_MS + rng(CAMERA_PASSAGE_TIME_MAX_MS - CAMERA_PASSAGE_TIME_MIN_MS);
		i2 = rng(CAMERA_PASSAGE_TIME_MAX_MS);
	} while (i2 > i1);
    return(1.F / i1);
}

ofstream ofsState("LightSpotPassiveState.csv");

void GenerateSignals(vector<vector<unsigned char> > &vvuc_)
{
    static vector<vector<unsigned char> > vvuc_Last;
	int x, y, i;
	if (prr_CameraCenter.first == -1) {
		prr_CameraCenter.first = -0.5 + rng();
		prr_CameraCenter.second = -0.5 + rng();
		float rCameraVelocity = rMakeCameraVelocity();
        float rCameraMovementDirection = rng(2 * M_PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
        ofsState << "x,y,vx,vy\n";
	}
    ofsState << 0.5 - prr_CameraCenter.first << ',' << 0.5 - prr_CameraCenter.second << ',' << -prr_CameraSpeed.first << ',' -prr_CameraSpeed.second << endl;   // мы сохраняем координаты и скорость светового пятна в координатах неподвижной камеры (0,0) - (1,1)
	pair<int,int> p_NewCameraLowerLeftCorner((int)((prr_CameraCenter.first + 0.5) / PIXEL_SIZE), (int)((prr_CameraCenter.second + 0.5) / PIXEL_SIZE));
	if (p_NewCameraLowerLeftCorner != p_CameraLowerLeftCorner) {
		p_CameraLowerLeftCorner = p_NewCameraLowerLeftCorner;
		FOR_(x, CAMERA_SIZE)
			FOR_(y, CAMERA_SIZE) {
				double d = 0.;
				int x1, y1;
				pair<int,int> p_CameraPixelLowerLeftCorner(p_CameraLowerLeftCorner.first + x * CAMERA_PIXEL_SIZE_PIXELS, p_CameraLowerLeftCorner.second + y * CAMERA_PIXEL_SIZE_PIXELS);
				FOR_(x1, CAMERA_PIXEL_SIZE_PIXELS)
					FOR_(y1, CAMERA_PIXEL_SIZE_PIXELS) {
						auto d1 = SpotRaster[p_CameraPixelLowerLeftCorner.second + y1][p_CameraPixelLowerLeftCorner.first + x1];
						d += d1;
					}
				d /= CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS;
				auto d2 = log(d);
                vvuc_Last[y][x] = d2 < LOG_INTENSITY_SENSITIVITY_THRESOLD ? 0. : (LOG_INTENSITY_SENSITIVITY_THRESOLD - d2) / LOG_INTENSITY_SENSITIVITY_THRESOLD * MAX_RECEPTOR_INTENSITY_SCALED;
			}
	}
    vvuc_ = vvuc_Last;
	prr_CameraCenter.first += prr_CameraSpeed.first;
	if (prr_CameraCenter.first < -0.5) {
		prr_CameraCenter.first = (float)(-0.5 + PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
        float rCameraMovementDirection = rng(M_PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	if (prr_CameraCenter.first >= 0.5) {
		prr_CameraCenter.first = (float)(0.5 - PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
        float rCameraMovementDirection = M_PI + rng(M_PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	prr_CameraCenter.second += prr_CameraSpeed.second;
	if (prr_CameraCenter.second < -0.5) {
		prr_CameraCenter.second = (float)(-0.5 + PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
        float rCameraMovementDirection = rng(M_PI) - M_PI / 2;
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	if (prr_CameraCenter.second >= 0.5) {
		prr_CameraCenter.second = (float)(0.5 - PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
        float rCameraMovementDirection = M_PI / 2 + rng(M_PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
}


