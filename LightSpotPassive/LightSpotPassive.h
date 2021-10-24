#ifndef LIGHTSPOTPASSIVE_H
#define LIGHTSPOTPASSIVE_H

#include <vector>

void SetParameters();
void GenerateSignals(std::vector<std::vector<unsigned char> > &vvuc_, std::vector<float> &vr_PhaseSpacePoint);

#define CAMERA_SIZE 20

#endif // LIGHTSPOTPASSIVE_H
