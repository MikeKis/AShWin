#ifndef LIGHTSPOTPASSIVE_H
#define LIGHTSPOTPASSIVE_H

#ifdef FOR_LINUX
#if defined(LIGHTSPOTPASSIVE_LIBRARY)
#define LIGHTSPOTPASSIVE_EXPORT
#else
#define LIGHTSPOTPASSIVE_EXPORT
#endif
#else
#if defined(LIGHTSPOTPASSIVE_LIBRARY)
#define LIGHTSPOTPASSIVE_EXPORT __declspec(dllexport)
#else
#define LIGHTSPOTPASSIVE_EXPORT __declspec(dllimport)
#endif
#endif

#include <vector>

LIGHTSPOTPASSIVE_EXPORT void SetParameters();
LIGHTSPOTPASSIVE_EXPORT void GenerateSignals(std::vector<std::vector<unsigned char> > &vvuc_, std::vector<float> &vr_PhaseSpacePoint);

#define CAMERA_SIZE 20

#endif // LIGHTSPOTPASSIVE_H
