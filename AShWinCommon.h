#ifdef FOR_LINUX
#include <inttypes.h>
typedef __uint64_t UNS64;
#endif

#define FOR_(i,a) for(i=0;i<(a);i++)
#define FORI(N) for(UNS64 _i = 0; _i < (UNS64)(N); _i++)
