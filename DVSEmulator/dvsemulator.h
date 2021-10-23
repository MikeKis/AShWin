#ifndef DVSEMULATOR_H
#define DVSEMULATOR_H

#include <vector>
#include <utility>
#include <deque>

#include "DVSEmulator_global.h"

class DVSEMULATOR_EXPORT DVSEmulator
{
    std::vector<std::vector<double> >                     vdd_StateBrightness;
    std::vector<std::vector<unsigned char> >              vvuc_CurrentState;
    double                                                dBrightnessThreshold = 1e100;
    unsigned char                                         ucChangeThreshold = 0xff;
    unsigned                                              maxCalibrationQueueSize;
    std::deque<std::vector<std::vector<unsigned char> > > qvvuc_forCalibration;
public:
    DVSEmulator(unsigned Width, unsigned Height, unsigned maxCalibrationSize = 30000);
    void AddFrame(const std::vector<std::vector<unsigned char> > &vvuc_Frame, std::vector<bool> *pvb_SpikeSignal = nullptr);
    void Calibrate(float rTargetSpikeFrequency);
    void ResetCalibration(){qvvuc_forCalibration.clear();}
};

#endif // DVSEMULATOR_H
