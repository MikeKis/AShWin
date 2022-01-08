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
	for (auto i: vstr_WTASections)
		inc.FixSection(i);
	auto Sections = xncopy.child("Sections");
	auto WTALLink = xncopy.child("LinkWTAL");
	auto pilpWTALLink = inc.pilpCreateProjection(WTALLink, IntersectionLinkProperties::connection_excitatory);
	auto GATELink = xncopy.child("LinkGATE");
	auto pilpGATELink = inc.pilpCreateProjection(GATELink, IntersectionLinkProperties::connection_excitatory);
//	pilpULLPLUSLink->SetConstantInitialWeight(1.);
	std::string LPopulation = "L";
	inc.bAddNetwork(Sections);
	for (auto i: vstr_WTASections) 
		inc.bConnectPopulations(i, LPopulation, pilpWTALLink);   // LPopulation should be finalized!
	inc.DestroyProjection(pilpWTALLink);
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
	std::string ACTGATEREWPopulation = "ACTGATEREW";
	std::string ACTGATEPUNPopulation = "ACTGATEPUN";
	inc.bConnectPopulations("Reward", ACTGATEREWPopulation, pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.bConnectPopulations("Punishment", ACTGATEPUNPopulation, pilpGATELink);   // LPLUSPopulation should be finalized!
	inc.DestroyProjection(pilpGATELink);
	inc.FinalizePoplulation(LPopulation);   // It is necessary because this population was connected additionally with another population
	inc.FinalizePoplulation(ACTGATEREWPopulation);   // It is necessary because this population was connected additionally with another population
	inc.FinalizePoplulation(ACTGATEPUNPopulation);   // It is necessary because this population was connected additionally with another population
	inc.Finalize();
}
