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

void LMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("L", vstr_Meanings);}
void EFFMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("EFF", vstr_Meanings);}
void GATEREWMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("GATEREW", vstr_Meanings);}
void GATEPUNMeanings(const std::vector<std::vector<std::pair<int, int> > > &vvp_Synapses, std::vector<std::string> &vstr_Meanings) {ActionSpecificMeanings("GATEPUN", vstr_Meanings);}

IntersectionLinkProperties *pilpLLLink;

DYNAMIC_LIBRARY_ENTRY_POINT const IntersectionLinkProperties *GetLinkLL(const euclidean_space_point &espPresynaptic, const euclidean_space_point &espPostsynaptic) {return abs(espPresynaptic[0] - espPostsynaptic[0]) == 0.5 ? pilpLLLink : nullptr;}

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(const pugi::xml_node &xn, const INetworkConfigurator &inc)
{ 
	auto xncopy = xn;
	auto Sections = xncopy.child("Sections");
	auto INPLink = xncopy.child("LinkINP");
	auto pilpINPLink = inc.pilpCreateProjection(IntersectionLinkProperties::connection_excitatory, &INPLink);
	auto INPGATELink = xncopy.child("LinkINPGATE");
	auto pilpINPGATELink = inc.pilpCreateProjection(IntersectionLinkProperties::connection_excitatory, &INPGATELink);
	auto GATELink = xncopy.child("LinkGATE");
	auto pilpGATELink = inc.pilpCreateProjection(IntersectionLinkProperties::connection_excitatory, &GATELink);
	auto PoissonLink = xncopy.child("LinkPoisson");
	auto pilpPoissonLink = inc.pilpCreateProjection(IntersectionLinkProperties::connection_excitatory, &PoissonLink);
	auto LLLink = xncopy.child("LinkLL");
	pilpLLLink = inc.pilpCreateProjection(IntersectionLinkProperties::connection_inhibitory, &LLLink);
	inc.bAddNetwork(Sections);
	inc.bConnectPopulations("DVS", "L", pilpINPLink);
	inc.DestroyProjection(pilpINPLink);
	inc.bConnectPopulations("DVS", "SENSORYGATE", pilpINPGATELink);
	inc.DestroyProjection(pilpINPGATELink);
	inc.bConnectPopulations("L", "L", GetLinkLL);
	inc.DestroyProjection(pilpLLLink);
	inc.bConnectPopulations("Reward", "GATEREW", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.bConnectPopulations("Punishment", "GATEPUN", pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.bConnectPopulations("Poisson", "SENSORYGATE", pilpPoissonLink);
	inc.DestroyProjection(pilpGATELink);
	inc.DestroyProjection(pilpPoissonLink);
}

DYNAMIC_LIBRARY_ENTRY_POINT void SetMeaningDefinitions(vector<pair<const char *, pfnsetmeanings> > &vppchfsm_)
{
	vppchfsm_.clear();
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("DVS", DVSMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("Reward", RewardMeanings));
	vppchfsm_.push_back(pair<const char *, pfnsetmeanings>("Punishment", PunishmentMeanings));
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