/*
Copyright (C) 2021 Mikhail Kiselev
Автор Михаил Витальевич Киселев.
При модификации файла сохранение указания (со)авторства Михаила Витальевича Киселева обязательно.

Emulates signal from videocamera looking at a moving light spot.

*/

#include "../AShWinCommon.h"

#include "LightSpotPassive.h"

DYNAMIC_LIBRARY_ENTRY_POINT IReceptors *SetParametersIn(int &nReceptors, const pugi::xml_node &xn) 
{ 
	nReceptors = nInputNodes;
	return new LightSpot;
}

DYNAMIC_LIBRARY_ENTRY_POINT IReceptors *LoadStatus(Serializer &ser){return new LightSpot;}


