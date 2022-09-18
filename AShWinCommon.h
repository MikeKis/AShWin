#pragma once

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <list>
#include <deque>

#ifdef FOR_LINUX
#include <inttypes.h>
typedef __uint64_t UNS64;
#define sscanf_s sscanf
#define DYNAMIC_LIBRARY_ENTRY_POINT extern "C"
#define DYNAMIC_LIBRARY_EXPORTED_CLASS
#else 
typedef unsigned __int64 UNS64;
#define DYNAMIC_LIBRARY_ENTRY_POINT __declspec(dllexport)
#define DYNAMIC_LIBRARY_EXPORTED_CLASS __declspec(dllexport)
#endif

#include "pugixml-1.10/src/pugixml.hpp"

#ifndef FUNS64

inline int atoi_s(const char *pch)
{
    if (!*pch)
        return 0;
    int ret;
    if (sscanf_s(pch, "%d", &ret) != 1)
        throw std::runtime_error("int format conversion error");
    return ret;
}

inline std::string tostr(int i) {
	char buf[30];
#ifndef FOR_LINUX
	sprintf_s(buf, sizeof(buf), "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	return(std::string(buf));
}

template<class T> void avgdis(const T *p, size_t n, double &davg, double &ddis)
{
	if (!n)
		throw std::runtime_error("avgdis");
	if (n == 1) {
		davg = *p;
		ddis = 0.;
		return;
	}
	size_t l;
	davg = 0;
	for (l = 0; l < n; l++)
		davg += p[l];
	davg /= n;
	ddis = 0;
	for (l = 0; l < n; l++) {
		double d = p[l] - davg;
		ddis += d * d;
	}
	ddis /= n - 1;
}

#endif

#define FOR_(i,a) for(i=0;i<(a);i++)
#define FOR_ALL(i,a) for(i=(a).begin();i!=(a).end();i++)
#define FORI(N) for(UNS64 _i = 0; _i < (UNS64)(N); _i++)
#define FIND_FIRST(i,a,cond) for(i=(a).begin();i!=(a).end() && !(cond);i++)
