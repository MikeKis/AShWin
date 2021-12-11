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
	auto Sections = xncopy.child("Sections");
	pugi::xml_node section = Sections.child("Section");
	auto props = section.child("props");
	auto nWTANeurons = inc.GetNNeurons();
	props.child("N").set_value(str(nWTANeurons).c_str());
	auto ULMEMPopulation = inc.CreatePopulation(section);
}
