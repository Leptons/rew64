#ifndef __REW64_H__
#define __REW64_H__

#include <stdlib.h>

typedef int bool;
#define true 1
#define false 0

typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef __uint128_t uint128;

typedef uint64 mem_addr;

#define streq(a, b) !strcmp(a, b)

#endif // __REW64_H__
