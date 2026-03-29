/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */

#ifndef BRIDGE_H__
#define BRIDGE_H__
    
#include <project.h>
#include "simpleHsm.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
    
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
    
#define FT_DISPLAYWIDTH		FT_DISPLAY_HSIZE_QVGA
#define FT_DISPLAYHEIGHT	FT_DISPLAY_VSIZE_QVGA
#define FT_DISPLAY_RESOLUTION	FT_DISPLAY_QVGA_320x240
    
//#define FT_SPI_CLK_FREQ_MIN		8*1000000
//#define FT_SPI_CLK_FREQ_MAX		21*1000000
#define FT_SPI_CLK_FREQ		        8*1000000
#define CAN_SPI_CLK_FREQ            1*1000000
    
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