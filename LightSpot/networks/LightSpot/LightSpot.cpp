// LightSpot.cpp : Определяет экспортированные функции для приложения DLL.
//

#include <string>
#include <sstream>
#include <iomanip>

#include <NetworkConfigurator.h>
#include "../../../AShWinCommon.h"

using namespace std;

const int nWTASections = 3;

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
			ss << "I" << av_[2].size() << "(" << davgx << "," << davgy << ")/(" << sqrt(ddisx) << "," << sqrt(ddisy) << ")";
		}
		if (av_[4].size()) {
			avgdis(&av_[4].front(), av_[4].size(), davgx, ddisx);
			avgdis(&av_[5].front(), av_[5].size(), davgy, ddisy);
			ss << "D" << av_[4].size() << "(" << davgx << "," << davgy << ")/(" << sqrt(ddisx) << "," << sqrt(ddisy) << ")";
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

void LPLUSMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("L+", vstr_Meanings);}
void LMINUSMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("L-", vstr_Meanings);}
void EFFMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("EFF", vstr_Meanings);}
void GATEREWMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("GATEREW", vstr_Meanings);}
void GATEPUNMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("GATEPUN", vstr_Meanings);}

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(const pugi::xml_node &xn, const INetworkConfigurator &inc)
{ 
	auto xncopy = xn;
	auto Sections = xncopy.child("Sections");
	auto INPLink = xncopy.child("LinkINP");
	auto pilpINPLink = inc.pilpCreateProjection(INPLink, IntersectionLinkProperties::connection_excitatory);
	auto GATELink = xncopy.child("LinkGATE");
	auto pilpGATELink = inc.pilpCreateProjection(GATELink, IntersectionLinkProperties::connection_excitatory);
	auto WMEMLink = xncopy.child("LinkWMEM");
	auto pilpWMEMLink = inc.pilpCreateProjection(WMEMLink, IntersectionLinkProperties::connection_excitatory);
	auto WMEMLinkInh = xncopy.child("LinkWMEMinh");
	auto pilpWMEMLinkInh = inc.pilpCreateProjection(WMEMLinkInh, IntersectionLinkProperties::connection_inhibitory);
	inc.bAddNetwork(Sections);
	inc.bConnectPopulations("DVS", "W", pilpINPLink);
	string strWTASectionName = "W0";
	string strMEMSectionName = "MEM0";
	int mem = 30;
	FORI(nWTASections - 1) {
		++strWTASectionName[1];
		++strMEMSectionName[3];
		inc.bDuplicatePopulation("W", strWTASectionName, true);
		inc.bDuplicatePopulation("MEM", strMEMSectionName, true);
		mem *= 3;
		vector<size_t> vind_;
		inc.GetNeuronIds(strMEMSectionName, vind_);
		for (auto k: vind_)
			inc.SetNeuronProperty(k, p_ThresholdExcessIncrement, (BLIFAT_THRESHOLD_EXCESS_DECREMENT + (INTERNAL_WEIGHT - THRESHOLD_BASE) / mem) * DEFAULT_BURSTING_PERIOD);
		inc.bConnectPopulations(strWTASectionName, strMEMSectionName, pilpWMEMLink);
		inc.bConnectPopulations(strWTASectionName, strMEMSectionName, pilpWMEMLinkInh);
	}

	// We connect W and MEM after the cyclye above - otherwise W? would be also connected to MEM (because of true argument in bDuplicatePopulation).

	inc.bConnectPopulations("W", "MEM", pilpWMEMLink);
	inc.bConnectPopulations("W", "MEM", pilpWMEMLinkInh);

	inc.DestroyProjection(pilpINPLink);
	inc.DestroyProjection(pilpWMEMLink);
	inc.DestroyProjection(pilpWMEMLinkInh);
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
	inc.bConnectPopulations("Reward", "GATEREW", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.bConnectPopulations("Punishment", "GATEPUN", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.DestroyProjection(pilpGATELink);
//	inc.FixSection("LMINUS");   // To test that the initial configuration of LMINUS guarantees its silence
	inc.Finalize();
}

DYNAMIC_LIBRARY_ENTRY_POINT void SetMeaningDefinitions(vector<pair<const char *, pfnsetmeanings> > &vppchfsm_)
{
	vppchfsm_.clear();
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("DVS", DVSMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("Reward", RewardMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("Punishment", PunishmentMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("W", WTAMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("W1", WTAMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("W2", WTAMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("LPLUS", LPLUSMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("LMINUS", LMINUSMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("EFF", EFFMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("GATEREW", GATEREWMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("GATEPUN", GATEPUNMeanings));
}

DYNAMIC_LIBRARY_ENTRY_POINT void ProcessTact(unsigned CurrentTact, const INetworkConfigurator &inc)
{
	/*
	if (CurrentTact == 1000000) {
		inc.FixSection("L");
		inc.FixSection("W");
		inc.FixSection("W1");
		inc.FixSection("W2"); 
	} */
}