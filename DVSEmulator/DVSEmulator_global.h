#ifndef DVSEMULATOR_GLOBAL_H
#define DVSEMULATOR_GLOBAL_H

#include "../AShWinCommon.h"

#ifdef FOR_LINUX
#if defined(DVSEMULATOR_LIBRARY)
#define DVSEMULATOR_EXPORT
#else
#define DVSEMULATOR_EXPORT
#endif
#else
#if defined(DVSEMULATOR_LIBRARY)
#define DVSEMULATOR_EXPORT __declspec(dllexport)
#else
#define DVSEMULATOR_EXPORT __declspec(dllimport)
#endif
#endif

#endif // DVSEMULATOR_GLOBAL_H
