﻿// LightSpot.cpp : Определяет экспортированные функции для приложения DLL.
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
	auto WTALPLUSLink = Sections.child("LinkWTALPLUS");
	auto pilpWTALPLUSLink = inc.pilpCreateProjection(WTALPLUSLink, IntersectionLinkProperties::connection_excitatory);
	auto WTALMINUSLink = Sections.child("LinkWTALMINUS");
	auto pilpWTALMINUSLink = inc.pilpCreateProjection(WTALMINUSLink, IntersectionLinkProperties::connection_excitatory);
	auto GATELink = Sections.child("LinkGATE");
	auto pilpGATELink = inc.pilpCreateProjection(GATELink, IntersectionLinkProperties::connection_excitatory);
//	pilpULLPLUSLink->SetConstantInitialWeight(1.);
	std::string LPLUSPopulation = "LPLUS";
	std::string LMINUSPopulation = "LMINUS";
	inc.bAddNetwork(Sections);
	for (auto i: vstr_WTASections) {
		inc.bConnectPopulations(i, LPLUSPopulation, pilpWTALPLUSLink);   // LPLUSPopulation should be finalized!
		inc.bConnectPopulations(i, LMINUSPopulation, pilpWTALMINUSLink);   // LPLUSPopulation should be finalized!
	}
	inc.DestroyProjection(pilpWTALPLUSLink);
	inc.DestroyProjection(pilpWTALMINUSLink);
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
	inc.FinalizePoplulation(LPLUSPopulation);   // It is necessary because this population was connected additionally with another population
	inc.FinalizePoplulation(ACTGATEREWPopulation);   // It is necessary because this population was connected additionally with another population
	inc.FinalizePoplulation(ACTGATEPUNPopulation);   // It is necessary because this population was connected additionally with another population
	inc.Finalize();
}
