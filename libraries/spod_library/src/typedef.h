#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __PACKED__ __attribute__((__packed__))

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0])) 

#define my_offsetof(struct_name, field) (((u8*)&(struct_name).field) - (u8*)&(struct_name))

//typedef int bool;

#define TRUE 1
#define FALSE 0

typedef struct __attribute__((__packed__)) {
    uint16_t hex;
    float_t  dec;
} adcVals_t;

#ifdef __cplusplus
}
#endif


#endif
