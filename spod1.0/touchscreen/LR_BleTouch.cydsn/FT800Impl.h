/*****************************************************************************
* Copyright (c) Future Technology Devices International 2014
* propriety of Future Technology devices International.
*
* Software License Agreement
*
* This code is provided as an example only and is not guaranteed by FTDI. 
* FTDI accept no responsibility for any issues resulting from its use. 
* The developer of the final application incorporating any parts of this 
* sample project is responsible for ensuring its safe and correct operation 
* and for any consequences resulting from its use.
*****************************************************************************/
/**
* @file                           FT800Impl.h
* @brief                          Contains FT800 graphics controller implementation for arduino platform.
								  Tested platform version: Arduino 1.0.4 and later
* @version                        1.1.0
* @date                           2014/02/05
*
*/


#ifndef _FT800IMPL_H_
#define _FT800IMPL_H_   

#include "FT_GC.h"

/* Macros for FT800Impl library version */
#define FT800IMPL_MAJOR 				(1)
#define FT800IMPL_MINOR 				(0)
#define FT800IMPL_BUILD 				(0)

/* FT800Impl status enum - used for api return type, error type etc */

 
/************************************************************************************************************************************************************
Display parameters used for various options are

FT800_DisplayResolution			Width 	Height	Swizzle	Polarity	PClk	HCycle	Hoffset		Hsync0		Hsync1		VCycle	Voffset		Vsync0	Vsync1	
FT_DISPLAY_QVGA_320x240   		320		240		3		0			8		408		70			0			10			263			13		0		2
FT_DISPLAY_WQVGA_480x272		480		272		0		1			5		548		43			0			41			292			12		0		10
*************************************************************************************************************************************************************/



 /*
 Following are the assumptions made for FT800Impl library
 1. SPI library from arduino is included and working fine on this platform
 2. 
 3. 
 */


/* Api to set cs, pdn, int pin. CS pin is mandatory. If PDN and INT pins are set then gpios are initialized */	
/* API utilizing default pin for cs, pdn. For interrupt pin - no pin will be assigned */	



FT_Status FT800Impl_InitA(uint8_t ResType);
/* Api to program the output display wrt display parameters */

FT_Status FT800Impl_InitB(uint16_t hperiod,uint16_t vperiod,uint16_t hfrontporch,uint16_t hbackporch,uint16_t hpulsewidth,
			   uint16_t vfrontporch,uint16_t vbackporch,uint16_t vpulsewidth,
			   uint8_t polarity,uint8_t swizzle,uint8_t fps);

FT_Status FT800Impl_Exit();
/* Api to get version of the library */

void FT800Impl_GetVersion(uint8_t *Major, uint8_t *Minor, uint8_t *Build);
/* Apis related to graphics processor */	

uint8_t FT800Impl_SetIntMask(uint8_t IntMask);
/* Api to detect pen down or pen up from the hardware */

uint8_t FT800Impl_IsPendown();

void FT800Impl_SetTouch(uint8_t AdcMode,uint16_t Charge,uint8_t Settle,uint16_t Oversample, uint16_t RZTreshold);
#endif /* _FT800IMPL_H_ */


