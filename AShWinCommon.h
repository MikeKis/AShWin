#ifdef FOR_LINUX
#include <inttypes.h>
typedef __uint64_t UNS64;
#define sscanf_s sscanf
#else 
typedef unsigned __int64 UNS64;
#define DYNAMIC_LIBRARY_ENTRY_POINT __declspec(dllexport)
#endif

#include "pugixml-1.10/src/pugixml.hpp"

#define FOR_(i,a) for(i=0;i<(a);i++)
#define FORI(N) for(UNS64 _i = 0; _i < (UNS64)(N); _i++)

inline int atoi_s(const char *pch)
{
    if (!*pch)
        return 0;
    int ret;
    if (sscanf_s(pch, "%d", &ret) != 1)
        throw std::runtime_error("int format conversion error");
    return ret;
}
