#ifndef _TYPES_H_
#define _TYPES_H_

#include "typedef.h"

#define MAX_PATHNAME 256

//#define false   0
//#define true    !false

#ifndef __KERNEL__
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned long      u32;
typedef unsigned long long u64;

//typedef int bool;

#endif

#endif
