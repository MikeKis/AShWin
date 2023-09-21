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

template<class T> unsigned short ilog2(T x)
{
	unsigned short usret = 0;
	do {
		x >>= 1;
		if (!x)
			return usret;
		usret++;
	} while (1);
}

class BitMaskAccess
{
	size_t ind;
	size_t fl;
public:
	BitMaskAccess()
	{
		ind = 0;
		fl = 1;
	}
	BitMaskAccess(size_t a)
	{
		fl = 1ULL << (a & 0x3f);
		ind = a >> 6;
	}
	BitMaskAccess operator++()
	{
		if (fl != 0x8000000000000000ULL)
			fl <<= 1;
		else {
			fl = 1;
			++ind;
		}
		return *this;
	}
	BitMaskAccess operator++(int)
	{
		BitMaskAccess bmaold = *this;
		if (fl != 0x8000000000000000ULL)
			fl <<= 1;
		else {
			fl = 1;
			ind++;
		}
		return bmaold;
	}
	BitMaskAccess& operator+=(size_t advance)
	{
		size_t a = (ind << 6) + ilog2(fl);
		*this = BitMaskAccess(a + advance);
		return *this;
	}
	size_t nFullQW() const { return ind; }
	template<class T> friend bool operator&(const T* p, BitMaskAccess bma) { return (*((const size_t*)p + bma.ind) & bma.fl) != 0; }
	template<class T> friend void operator|=(T* p, BitMaskAccess bma) { *((size_t*)p + bma.ind) |= bma.fl; }
	template<class T> friend void operator!=(T* p, BitMaskAccess bma) { *((size_t*)p + bma.ind) &= ~bma.fl; }
};
