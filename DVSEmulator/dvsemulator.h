#ifndef DVSEMULATOR_H
#define DVSEMULATOR_H

#include <vector>
#include <utility>
#include <deque>

#include "DVSEmulator_global.h"

class DVSEMULATOR_EXPORT DVSEmulator
{
    std::vector<std::vector<double> >                     vvd_StateBrightness;
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
    size_t GetSpikeSignalDim() const {return vvd_StateBrightness.size() * vvd_StateBrightness.front().size() * 3;}

    friend inline Serializer &operator<<(Serializer &ser, const DVSEmulator &dvs)
    {
        ser << dvs.vvd_StateBrightness;
        ser << dvs.vvuc_CurrentState;
        ser << dvs.dBrightnessThreshold;
        ser << dvs.ucChangeThreshold;
        ser << dvs.maxCalibrationQueueSize;
        ser << dvs.qvvuc_forCalibration;
        return ser;
    }
    friend inline Serializer &operator>>(Serializer &ser, DVSEmulator &dvs)
    {
        ser >> dvs.vvd_StateBrightness;
        ser >> dvs.vvuc_CurrentState;
        ser >> dvs.dBrightnessThreshold;
        ser >> dvs.ucChangeThreshold;
        ser >> dvs.maxCalibrationQueueSize;
        ser >> dvs.qvvuc_forCalibration;
        return ser;
    }

};

#endif // DVSEMULATOR_H
