// LightSpot.cpp : Определяет экспортированные функции для приложения DLL.
//

#include <string>

#include <NetworkConfigurator.h>
#include "../../../AShWinCommon.h"

using namespace std;

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(const pugi::xml_node &xn, const INetworkConfigurator &inc)
{ 
	auto xncopy = xn;
	string strSavedNetwork = xncopy.child_value("Saved");
	inc.LoadNetwork(strSavedNetwork);
	auto vstr_WTASections = inc.vstr_GetSectionNames();
	auto nWTANeurons = inc.GetNNeurons();
	auto Sections = xncopy.child("Sections");
	auto ULMEMLink = Sections.child("LinkWTALPLUS");
	auto pilpULLPLUSLink = inc.pilpCreateProjection(ULMEMLink, IntersectionLinkProperties::connection_excitatory);
	auto REWGATELink = Sections.child("LinkREWGATE");
	auto pilpREWGATELink = inc.pilpCreateProjection(REWGATELink, IntersectionLinkProperties::connection_excitatory);
	pilpULLPLUSLink->SetConstantInitialWeight(1.);
	std::string LPLUSPopulation = "LPLUS";
	inc.bAddNetwork(Sections);
	for (auto i: vstr_WTASections)
		inc.bConnectPopulations(i, LPLUSPopulation, pilpULLPLUSLink);   // LPLUSPopulation should be finalized!
	inc.DestroyProjection(pilpULLPLUSLink);
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
	std::string ACTGATEPopulation = "ACTGATE";
	inc.bConnectPopulations("Reward", ACTGATEPopulation, pilpREWGATELink);   // LPLUSPopulation should be finalized!
	inc.DestroyProjection(pilpREWGATELink);
	inc.FinalizePoplulation(LPLUSPopulation);   // It is necessary because this population was connected additionally with another population
	inc.FinalizePoplulation(ACTGATEPopulation);   // It is necessary because this population was connected additionally with another population
	inc.Finalize();
}
