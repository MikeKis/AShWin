// LightSpot.cpp : Определяет экспортированные функции для приложения DLL.
//

#include <string>
#include <sstream>
#include <iomanip>

#include <NetworkConfigurator.h>
#include "../../../AShWinCommon.h"

using namespace std;

void DVSMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings)
{
	int i = 0;
	int y, x;
	for (y = 19; y >= 0; --y)
		for (x = 0; x < 20; ++x) {
			stringstream ss;
			ss << "ABS(" << x << "," << y << ")";
			vstr_Meanings[i++] = ss.str();
		}
	for (y = 19; y >= 0; --y)
		for (x = 0; x < 20; ++x) {
			stringstream ss;
			ss << "INC(" << x << "," << y << ")";
			vstr_Meanings[i++] = ss.str();
		}
	for (y = 19; y >= 0; --y)
		for (x = 0; x < 20; ++x) {
			stringstream ss;
			ss << "DEC(" << x << "," << y << ")";
			vstr_Meanings[i++] = ss.str();
		}
}

void RewardMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {vstr_Meanings[0] = "REW";}
void PunishmentMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {vstr_Meanings[0] = "PUN";}

void WTAMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings)
{
	FORI(vstr_Meanings.size()) {
		stringstream ss;
		ss << 'W' << fixed << setprecision(1);
		vector<int> av_[6];
		for (auto i: vvp_Synapses[_i])
			if (i.second > 30000) {
				int rec = -1 - i.first;
				int sec = rec / 400;
				int off = rec - sec * 400;
				int y = off / 20;
				int x = off - y * 20;
				y = 19 - y;
				av_[sec * 2].push_back(x);
				av_[sec * 2 + 1].push_back(y);
			}
		double davgx, ddisx, davgy, ddisy;
		if (av_[0].size()) {
			avgdis(&av_[0].front(), av_[0].size(), davgx, ddisx);
			avgdis(&av_[1].front(), av_[1].size(), davgy, ddisy);
			ss << "A" << av_[0].size() << "(" << davgx << "," << davgy << ")/(" << sqrt(ddisx) << "," << sqrt(ddisy) << ")";
		}
		if (av_[2].size()) {
			avgdis(&av_[2].front(), av_[2].size(), davgx, ddisx);
			avgdis(&av_[3].front(), av_[3].size(), davgy, ddisy);
			ss << "I" << av_[0].size() << "(" << davgx << "," << davgy << ")/(" << sqrt(ddisx) << "," << sqrt(ddisy) << ")";
		}
		if (av_[4].size()) {
			avgdis(&av_[4].front(), av_[4].size(), davgx, ddisx);
			avgdis(&av_[5].front(), av_[5].size(), davgy, ddisy);
			ss << "D" << av_[0].size() << "(" << davgx << "," << davgy << ")/(" << sqrt(ddisx) << "," << sqrt(ddisy) << ")";
		}
		vstr_Meanings[_i] = ss.str();
	}
}

void ActionSpecificMeanings(string strPopulationType, vector<string> &vstr_Meanings)
{
	vstr_Meanings[0] = strPopulationType + "-UP1";
	vstr_Meanings[1] = strPopulationType + "-UP2";
	vstr_Meanings[2] = strPopulationType + "-UP3";
	vstr_Meanings[3] = strPopulationType + "-RIGHT1";
	vstr_Meanings[4] = strPopulationType + "-RIGHT2";
	vstr_Meanings[5] = strPopulationType + "-RIGHT3";
	vstr_Meanings[6] = strPopulationType + "-DOWN1";
	vstr_Meanings[7] = strPopulationType + "-DOWN2";
	vstr_Meanings[8] = strPopulationType + "-DOWN3";
	vstr_Meanings[9] = strPopulationType + "-LEFT1";
	vstr_Meanings[10] = strPopulationType + "-LEFT2";
	vstr_Meanings[11] = strPopulationType + "-LEFT3";
}

void LMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("L", vstr_Meanings);}
void EFFMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("EFF", vstr_Meanings);}
void ACTMEMMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("ACTMEM", vstr_Meanings);}
void ACTGATEREWMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("ACTGATEREW", vstr_Meanings);}
void ACTGATEPUNMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("ACTGATEPUN", vstr_Meanings);}
void ANDACTGATEREWMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("ANDACTGATEREW", vstr_Meanings);}
void ANDACTGATEPUNMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("ANDACTGATEPUN", vstr_Meanings);}

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(const pugi::xml_node &xn, const INetworkConfigurator &inc)
{ 
	auto xncopy = xn;
	auto Sections = xncopy.child("Sections");
	auto INPLink = xncopy.child("LinkINP");
	auto pilpINPLink = inc.pilpCreateProjection(INPLink, IntersectionLinkProperties::connection_excitatory);
	auto GATELink = xncopy.child("LinkGATE");
	auto pilpGATELink = inc.pilpCreateProjection(GATELink, IntersectionLinkProperties::connection_inhibitory);
	inc.bAddNetwork(Sections);
	inc.bConnectPopulations("DVS", "W", pilpINPLink);
	inc.bDuplicatePopulation("W", "W1", true);
	inc.bDuplicatePopulation("W", "W2", true);
	inc.DestroyProjection(pilpINPLink);
	vector<size_t> vind_EFFNeurons;
	inc.GetNeuronIds("EFF", vind_EFFNeurons);
	for (int j = 0; j < 12; j += 3) {

		// 1st neuron should spike only once on activation from LPLUSPopulation

		inc.SetNeuronProperty(vind_EFFNeurons[j], p_BurstingPeriod, 0);

		// 2nd neuron should emit 2 spikes on activation from LPLUSPopulation

		inc.SetNeuronProperty(vind_EFFNeurons[j + 1], p_BurstingPeriod, 3);
		inc.SetNeuronProperty(vind_EFFNeurons[j + 1], p_ThresholdExcessDecrement, 10000);
		inc.SetNeuronProperty(vind_EFFNeurons[j + 1], p_ThresholdExcessIncrement, INTERNAL_WEIGHT * 1.5);

		// 3nd neuron should emit 3 spikes on activation from LPLUSPopulation

		inc.SetNeuronProperty(vind_EFFNeurons[j + 2], p_BurstingPeriod, 3);
		inc.SetNeuronProperty(vind_EFFNeurons[j + 2], p_ThresholdExcessDecrement, 10000);
		inc.SetNeuronProperty(vind_EFFNeurons[j + 2], p_ThresholdExcessIncrement, INTERNAL_WEIGHT * 1.33);

	}
	inc.bConnectPopulations("Reward", "ACTGATEREW", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.bConnectPopulations("Punishment", "ACTGATEPUN", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.DestroyProjection(pilpGATELink);
	inc.SetMeaningDefinitions("DVS", DVSMeanings);
	inc.SetMeaningDefinitions("Reward", RewardMeanings);
	inc.SetMeaningDefinitions("Punishment", PunishmentMeanings);
	inc.SetMeaningDefinitions("W", WTAMeanings);
	inc.SetMeaningDefinitions("W1", WTAMeanings);
	inc.SetMeaningDefinitions("W2", WTAMeanings);
	inc.SetMeaningDefinitions("L", LMeanings);
	inc.SetMeaningDefinitions("EFF", EFFMeanings); 
	inc.SetMeaningDefinitions("ACTMEM", ACTMEMMeanings);
	inc.SetMeaningDefinitions("ACTGATEREW", ACTGATEREWMeanings);
	inc.SetMeaningDefinitions("ACTGATEPUN", ACTGATEPUNMeanings);
	inc.SetMeaningDefinitions("ANDACTMEMREW", ANDACTGATEREWMeanings);
	inc.SetMeaningDefinitions("ANDACTMEMPUN", ANDACTGATEPUNMeanings);
	inc.Finalize();
}
