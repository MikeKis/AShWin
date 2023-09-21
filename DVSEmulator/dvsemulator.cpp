#include <stdexcept>

#include "dvsemulator.h"

using namespace std;

DVSEmulator::DVSEmulator(unsigned Width, unsigned Height, unsigned maxCalibrationSize):
    vvd_StateBrightness(Height, vector<double>(Width, 0.)),
    maxCalibrationQueueSize(maxCalibrationSize)
{
}

void DVSEmulator::AddFrame(const std::vector<std::vector<unsigned char> > &vvuc_Frame, std::vector<unsigned> *pvfl_SpikeSignal)
{
    if (!pvfl_SpikeSignal) {
        qvvuc_forCalibration.push_back(vvuc_Frame);
        if (qvvuc_forCalibration.size() > maxCalibrationQueueSize)
            qvvuc_forCalibration.pop_front();
    } else {
        BitMaskAccess bma;
        unsigned *pfl = &pvfl_SpikeSignal->front();
        size_t m;
        FOR_(m, vvuc_Frame.size())
            FORI(vvuc_Frame[m].size()) {
                vvd_StateBrightness[m][_i] += vvuc_Frame[m][_i];
                if (vvd_StateBrightness[m][_i] >= dBrightnessThreshold) {
                    pfl |= bma;
                    vvd_StateBrightness[m][_i] -= dBrightnessThreshold;
                }
                ++bma;
            }
        if (vvuc_CurrentState.empty())
            vvuc_CurrentState = vvuc_Frame;
        else {
            BitMaskAccess bmaDecrease = bma;
            bmaDecrease += vvuc_Frame.size() * vvuc_Frame.front().size();
            FOR_(m, vvuc_Frame.size())
                FORI(vvuc_Frame[m].size()) {
                    auto n = (unsigned short)vvuc_CurrentState[m][_i] + ucChangeThreshold;
                    if (vvuc_Frame[m][_i] >= n) {
                        pfl |= bma;
                        vvuc_CurrentState[m][_i] = (unsigned char)n;
                    } else {
                        auto o = (int)vvuc_CurrentState[m][_i] - (int)ucChangeThreshold;
                        if ((int)vvuc_Frame[m][_i] <= o) {
                            pfl |= bmaDecrease;
                            vvuc_CurrentState[m][_i] = (unsigned char)o;
                        }
                    }
                    ++bma;
                    ++bmaDecrease;
                }
        }
    }
}

void DVSEmulator::Calibrate(float rTargetSpikeFrequency)
{
    size_t tot = 0;
    for (const auto &i: qvvuc_forCalibration)
        for (const auto &j: i)
            for (auto k: j)
                tot += k;
    double dnTargetSpikes = rTargetSpikeFrequency * (qvvuc_forCalibration.size() * qvvuc_forCalibration.front().size() * qvvuc_forCalibration.front().front().size());
    if (!dnTargetSpikes)
        throw std::runtime_error("invalid calibration parameters");
    dBrightnessThreshold = tot / dnTargetSpikes;
    dnTargetSpikes *= 2;
    for(ucChangeThreshold = 1; ucChangeThreshold < 128; ++ucChangeThreshold) {
        auto CurrentLevel = qvvuc_forCalibration.front();
        auto l = qvvuc_forCalibration.begin();
        size_t nSpikes = 0;
        while (++l != qvvuc_forCalibration.end()) {
            size_t m;
            FOR_(m, l->size())
                FORI((*l)[m].size()) {
                    auto n = (unsigned short)CurrentLevel[m][_i] + ucChangeThreshold;
                    if ((*l)[m][_i] >= n) {
                        ++nSpikes;
                        CurrentLevel[m][_i] = (unsigned char)n;
                    } else {
                        auto o = (int)CurrentLevel[m][_i] - (int)ucChangeThreshold;
                        if ((int)(*l)[m][_i] <= o) {
                            ++nSpikes;
                            CurrentLevel[m][_i] = (unsigned char)o;
                        }
                    }
                }
        }
        if (nSpikes <= dnTargetSpikes)
            break;
    }
    if (ucChangeThreshold == 128)
        throw std::runtime_error("invalid calibration parameters");
}
