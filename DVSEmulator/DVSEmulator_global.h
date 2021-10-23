#ifndef DVSEMULATOR_GLOBAL_H
#define DVSEMULATOR_GLOBAL_H

//#include <QtCore/qglobal.h>

#ifdef FOR_LINUX
#include <inttypes.h>
#if defined(DVSEMULATOR_LIBRARY)
#define DVSEMULATOR_EXPORT
#else
#define DVSEMULATOR_EXPORT
#endif
typedef __uint64_t UNS64;
#endif

#define FOR_(i,a) for(i=0;i<(a);i++)
#define FORI(N) for(UNS64 _i = 0; _i < (UNS64)(N); _i++)

#endif // DVSEMULATOR_GLOBAL_H
