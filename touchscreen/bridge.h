#ifndef BRIDGE_H__
#define BRIDGE_H__
    
#include "simple_hsm.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
    
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
    
#define FT_DISPLAYWIDTH		480
#define FT_DISPLAYHEIGHT	272
#define FT_DISPLAY_RESOLUTION	FT_DISPLAY_WQVGA_480x272/*FT_DISPLAY_QVGA_320x240*/
    
#define FT_SPI_CLK_FREQ_MIN		8*1000000
#define FT_SPI_CLK_FREQ_MAX		30*1000000
#define FT_SPI_CLK_FREQ		    FT_SPI_CLK_FREQ_MIN
#define CAN_SPI_CLK_FREQ      1*1000000
    
union Data32
{
	uint32_t UUint32;
	uint8_t  A[4];
};

union Data16
{
	uint16_t UUint16;
	uint8_t  A[2];
};

#endif
