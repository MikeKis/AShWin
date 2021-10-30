#include <fstream>
#include <string>
#include <iostream>

#include "../../LightSpotPassive/LightSpotPassive.h"
#include "../../DVSEmulator/dvsemulator.h"

using namespace std;

int main(int ARGC, char *ARGV[])
{
    SetParameters();
    DVSEmulator dve(CAMERA_SIZE, CAMERA_SIZE);
    vector<vector<unsigned char> > vvuc_;
    vector<float> vr_PhaseSpacePoint(4);
    FORI(10000) {
        GenerateSignals(vvuc_, vr_PhaseSpacePoint);
        dve.AddFrame(vvuc_);
    }
    cout << "calilbration signal generated\n";
    dve.Calibrate(0.003F);
    cout << "calilbration done\n";
    vector<bool> vb_Spikes(dve.GetSpikeSignalDim());
    auto History = atoi_s(ARGV[1]);
    ofstream ofsrec(string(ARGV[2]) + ".rec");
    ofstream ofscsv(string(ARGV[2]) + ".csv");
    FORI(History) {
        GenerateSignals(vvuc_, vr_PhaseSpacePoint);
        dve.AddFrame(vvuc_, &vb_Spikes);
        for (auto i: vb_Spikes)
            ofsrec << (i ? '@' : '.');
        ofsrec << endl;
        bool b = false;
        for (auto r: vr_PhaseSpacePoint) {
            if (b)
                ofscsv << ',';
            ofscsv << r;
            b = true;
        }
        ofscsv << endl;
        if (!(_i % 10000))
            cout << _i << " tacts generated\n";
    }
    return 0;
}
