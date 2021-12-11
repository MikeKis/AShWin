// LightSpot.cpp : Определяет экспортированные функции для приложения DLL.
//

#include <string>

#include <NetworkConfigurator.h>
#include "../../../AShWinCommon.h"

using namespace std;

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(const pugi::xml_node &xn, const INetworkConfigurator &inc)
{ 
	string strSavedNetwork = xn.child_value("Saved");
	inc.LoadNetwork(strSavedNetwork);
	auto Sections = xn.child("Sections");
	for (pugi::xml_node section = Sections.child("Section"); section; section = section.next_sibling("Section")) 
		inc.CreatePopulation(section);
}
