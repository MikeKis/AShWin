/*
Copyright (C) 2007 - 2017 Mikhail Kiselev
Автор Михаил Витальевич Киселев.
При модификации файла сохранение указания (со)авторства Михаила Витальевича Киселева обязательно.

Emulates signal from videocamera looking at a moving light spot.

*/

#include "stdafx.h"
#include "..\..\ArNI\ArNI.h"

#define EXACT_RASTER_SIZE 100
#define PIXEL_SIZE (1. / EXACT_RASTER_SIZE)
#define SPOT_SIZE 0.1
#define CAMERA_PASSAGE_TIME_MIN_MS 300
#define CAMERA_PASSAGE_TIME_MAX_MS 1000
#define CAMERA_SIZE 10
#define CAMERA_PIXEL_SIZE_PIXELS (EXACT_RASTER_SIZE / CAMERA_SIZE)
#define LOG_INTENSITY_SENSITIVITY_THRESOLD -4.5
#define MAX_RECEPTOR_INTENSITY_BASE 0.15
#define MAX_RECEPTOR_INTENSITY_SCALED (MAX_RECEPTOR_INTENSITY_BASE * rRelativeFrequencyMultiplier)
#define RELATIVE_GRADIENT_INTENSITY 1.5
#define INTENSITY_CHANGE_PER_SPIKE (2 / ((1000 * MAX_RECEPTOR_INTENSITY_SCALED / (1000. / CAMERA_PASSAGE_TIME_MIN_MS))))

using namespace std;

Rand rng;
float SpotRaster[2 * EXACT_RASTER_SIZE][2 * EXACT_RASTER_SIZE];
pair<float,float> prr_CameraCenter(-1.F, -1.F);
pair<float,float> prr_CameraSpeed;
pair<int,int> p_CameraLowerLeftCorner;

float Intensity[CAMERA_SIZE][CAMERA_SIZE];
int LastIntesityLevel[CAMERA_SIZE][CAMERA_SIZE];
int DynamicsSpikeDelay[CAMERA_SIZE][CAMERA_SIZE][2];
float Gradient[CAMERA_SIZE][CAMERA_SIZE][8];
float rRelativeFrequencyMultiplier = 1.F;

__declspec(dllexport) void SetParameters(int nInhibitoryReceptors, int nExcitatoryReceptors, const vector<string> &vstr_args)
{
	if (nInhibitoryReceptors + nExcitatoryReceptors != (sizeof(Intensity) + sizeof(Gradient)) / sizeof(float) + CAMERA_SIZE * CAMERA_SIZE * 2) {
		printf("Wrong request number of receptors!\n");
		exit(-31);
	}
	if (vstr_args.size())
		rRelativeFrequencyMultiplier = atof_s(vstr_args[0].c_str());
	int x, y;
	FOR_(x, 2 * EXACT_RASTER_SIZE)
		FOR_(y, 2 * EXACT_RASTER_SIZE) {
			float r2 = ((x + 0.5 - EXACT_RASTER_SIZE) * (x + 0.5 - EXACT_RASTER_SIZE) + (y + 0.5 - EXACT_RASTER_SIZE) * (y + 0.5 - EXACT_RASTER_SIZE)) / (EXACT_RASTER_SIZE * EXACT_RASTER_SIZE);
			SpotRaster[x][y] = exp(-r2 / (2 * SPOT_SIZE * SPOT_SIZE));
		}
}

__declspec(dllexport) void Randomize(void){rng.Randomize(0, false);}

__declspec(dllexport) void SaveStatus(FileSerializer &fs)
{
}

__declspec(dllexport) void LoadStatus(FileSerializer &fs)
{
}

float rMakeCameraVelocity(void)
{
	int i1, i2;
	do {
		i1 = CAMERA_PASSAGE_TIME_MIN_MS + rng(CAMERA_PASSAGE_TIME_MAX_MS - CAMERA_PASSAGE_TIME_MIN_MS);
		i2 = rng(CAMERA_PASSAGE_TIME_MAX_MS);
	} while (i2 > i1);
	return(1. / i1);
}

FILE *filState;
int indDynamicsReceptor;

__declspec(dllexport) bool bGenerateSignals(char *prec, size_t neuronstrsize)
{
	int x, y, i;
	bool bInitialization = false;
	if (prr_CameraCenter.first == -1) {
		prr_CameraCenter.first = -0.5 + rng();
		prr_CameraCenter.second = -0.5 + rng();
		float rCameraVelocity = rMakeCameraVelocity();
		float rCameraMovementDirection = rng(2 * PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
		filState = fopen("LightSpotPassiveState.csv", "wt");
		fputs("x,y,vx,vy\n", filState);
		indDynamicsReceptor = (sizeof(Intensity) + sizeof(Gradient)) / sizeof(float);
		bInitialization = true;
		memset(DynamicsSpikeDelay, 0xff, sizeof(DynamicsSpikeDelay));
	}
	fprintf(filState, "%f,%f,%f,%f\n", 0.5 - prr_CameraCenter.first, 0.5 - prr_CameraCenter.second, -prr_CameraSpeed.first, -prr_CameraSpeed.second);   // мы сохраняем координаты и скорость светового пятна в координатах неподвижной камеры (0,0) - (1,1)
	pair<int,int> p_NewCameraLowerLeftCorner((int)((prr_CameraCenter.first + 0.5) / PIXEL_SIZE), (int)((prr_CameraCenter.second + 0.5) / PIXEL_SIZE));
	memset(prec + indDynamicsReceptor, 0, CAMERA_SIZE * CAMERA_SIZE * 2);
	FOR_(y, CAMERA_SIZE)
		FOR_(x, CAMERA_SIZE) {
			auto indrec = indDynamicsReceptor + (y * CAMERA_SIZE + x) * 2;
			if (DynamicsSpikeDelay[y][x][0] >= 0)
				if (!DynamicsSpikeDelay[y][x][0]--)
					*(prec + indrec * neuronstrsize) = 1;
			if (DynamicsSpikeDelay[y][x][1] >= 0)
				if (!DynamicsSpikeDelay[y][x][1]--)
					*(prec + (indrec + 1) * neuronstrsize) = 1;
		}
	if (p_NewCameraLowerLeftCorner != p_CameraLowerLeftCorner) {
		p_CameraLowerLeftCorner = p_NewCameraLowerLeftCorner;
		FOR_(x, CAMERA_SIZE)
			FOR_(y, CAMERA_SIZE) {
				auto indrec = indDynamicsReceptor + (y * CAMERA_SIZE + x) * 2;
				double d = 0.;
				int x1, y1;
				pair<int,int> p_CameraPixelLowerLeftCorner(p_CameraLowerLeftCorner.first + x * CAMERA_PIXEL_SIZE_PIXELS, p_CameraLowerLeftCorner.second + y * CAMERA_PIXEL_SIZE_PIXELS);
				double adGradient[4];
				for (auto &d1: adGradient)
					d1 = 0.;
				FOR_(x1, CAMERA_PIXEL_SIZE_PIXELS)
					FOR_(y1, CAMERA_PIXEL_SIZE_PIXELS) {
						auto d1 = SpotRaster[p_CameraPixelLowerLeftCorner.second + y1][p_CameraPixelLowerLeftCorner.first + x1];
						d += d1;
						if (y1 >= CAMERA_PIXEL_SIZE_PIXELS / 2)
							adGradient[0] += d1;
						else adGradient[0] -= d1;
						if (x1 >= CAMERA_PIXEL_SIZE_PIXELS / 2)
							adGradient[2] += d1;
						else adGradient[2] -= d1;
						if (x1 + y1 >= CAMERA_PIXEL_SIZE_PIXELS)
							adGradient[1] += d1;
						else if (x1 + y1 < CAMERA_PIXEL_SIZE_PIXELS - 1)
							adGradient[1] -= d1;
						if (x1 > y1)
							adGradient[3] += d1;
						else if (x1 < y1)
							adGradient[3] -= d1;
					}
				d /= CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS;
				int NewIntensityLevel = (int)(d / INTENSITY_CHANGE_PER_SPIKE);
				if (!bInitialization) {
					if (NewIntensityLevel != LastIntesityLevel[y][x]) {
						if (NewIntensityLevel > LastIntesityLevel[y][x]) {
							if (DynamicsSpikeDelay[y][x][0] >= 0) 
								*(prec + indrec * neuronstrsize) = 1;
							DynamicsSpikeDelay[y][x][0] = !*(prec + indrec * neuronstrsize) ? rng(4) : 1 + rng(3);
							if (!DynamicsSpikeDelay[y][x][0]--) 
								*(prec + indrec * neuronstrsize) = 1;
							LastIntesityLevel[y][x]++;
						} else {
							if (DynamicsSpikeDelay[y][x][1] >= 0)
								*(prec + (indrec + 1) * neuronstrsize) = 1;
							DynamicsSpikeDelay[y][x][1] = !*(prec + (indrec + 1) * neuronstrsize) ? rng(4) : 1 + rng(3);
							if (!DynamicsSpikeDelay[y][x][1]--) 
								*(prec + (indrec + 1) * neuronstrsize) = 1;
							LastIntesityLevel[y][x]--;
						}
					}
				} else LastIntesityLevel[y][x] = NewIntensityLevel;
				auto d2 = log(d);
				Intensity[y][x] = d2 < LOG_INTENSITY_SENSITIVITY_THRESOLD ? 0. : (LOG_INTENSITY_SENSITIVITY_THRESOLD - d2) / LOG_INTENSITY_SENSITIVITY_THRESOLD * MAX_RECEPTOR_INTENSITY_SCALED;
				if (adGradient[0] >= 0.) {
					Gradient[y][x][4] = 0.;
					Gradient[y][x][0] = MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[0] / (CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS / 2);
				} else {
					Gradient[y][x][0] = 0.;
					Gradient[y][x][4] = -MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[0] / (CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS / 2);
				}
				if (adGradient[2] >= 0.) {
					Gradient[y][x][6] = 0.;
					Gradient[y][x][2] = MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[2] / (CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS / 2);
				} else {
					Gradient[y][x][2] = 0.;
					Gradient[y][x][6] = -MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[2] / (CAMERA_PIXEL_SIZE_PIXELS * CAMERA_PIXEL_SIZE_PIXELS / 2);
				}
				if (adGradient[1] >= 0.) {
					Gradient[y][x][5] = 0.;
					Gradient[y][x][1] = MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[1] / (CAMERA_PIXEL_SIZE_PIXELS * (CAMERA_PIXEL_SIZE_PIXELS - 1) / 2);
				} else {
					Gradient[y][x][1] = 0.;
					Gradient[y][x][5] = -MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[1] / (CAMERA_PIXEL_SIZE_PIXELS * (CAMERA_PIXEL_SIZE_PIXELS - 1) / 2);
				}
				if (adGradient[3] >= 0.) {
					Gradient[y][x][7] = 0.;
					Gradient[y][x][3] = MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[3] / (CAMERA_PIXEL_SIZE_PIXELS * (CAMERA_PIXEL_SIZE_PIXELS - 1) / 2);
				} else {
					Gradient[y][x][3] = 0.;
					Gradient[y][x][7] = -MAX_RECEPTOR_INTENSITY_SCALED * RELATIVE_GRADIENT_INTENSITY * adGradient[3] / (CAMERA_PIXEL_SIZE_PIXELS * (CAMERA_PIXEL_SIZE_PIXELS - 1) / 2);
				}
			}
	}
	i = 0;
	FOR_(y, CAMERA_SIZE)
		FOR_(x, CAMERA_SIZE)
			*(prec + i++ * neuronstrsize) = Intensity[y][x] && rng() < Intensity[y][x] ? 1 : 0;
	FOR_(y, CAMERA_SIZE)
		FOR_(x, CAMERA_SIZE)
			for (auto r: Gradient[y][x])
				*(prec + i++ * neuronstrsize) = rng() < r ? 1 : 0;
	prr_CameraCenter.first += prr_CameraSpeed.first;
	if (prr_CameraCenter.first < -0.5) {
		prr_CameraCenter.first = (float)(-0.5 + PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
		float rCameraMovementDirection = rng(PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	if (prr_CameraCenter.first >= 0.5) {
		prr_CameraCenter.first = (float)(0.5 - PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
		float rCameraMovementDirection = PI + rng(PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	prr_CameraCenter.second += prr_CameraSpeed.second;
	if (prr_CameraCenter.second < -0.5) {
		prr_CameraCenter.second = (float)(-0.5 + PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
		float rCameraMovementDirection = rng(PI) - PI / 2;
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	if (prr_CameraCenter.second >= 0.5) {
		prr_CameraCenter.second = (float)(0.5 - PIXEL_SIZE / 2);
		float rCameraVelocity = rMakeCameraVelocity();
		float rCameraMovementDirection = PI / 2 + rng(PI);
		prr_CameraSpeed.first = rCameraVelocity * sin(rCameraMovementDirection);
		prr_CameraSpeed.second = rCameraVelocity * cos(rCameraMovementDirection);
	}
	return(true);
}


