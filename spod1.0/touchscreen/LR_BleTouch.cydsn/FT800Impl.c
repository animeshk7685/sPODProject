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

#include "FT800Impl.h"

FT_Status FT800Impl_InitA(uint8_t ResType)
{
	return ( FT_GC_InitA(ResType) );
}

/* Api to program the output display wrt display parameters */

FT_Status FT800Impl_InitB(uint16_t hperiod,uint16_t vperiod,uint16_t hfrontporch,uint16_t hbackporch,uint16_t hpulsewidth,
			   uint16_t vfrontporch,uint16_t vbackporch,uint16_t vpulsewidth,
			   uint8_t polarity,uint8_t swizzle,uint8_t fps)
{
	FT_Status Status;
	Status = FT_GC_InitB( hperiod,vperiod,hfrontporch,hbackporch,hpulsewidth,vfrontporch,vbackporch,vpulsewidth,polarity,swizzle,fps);
	
	return Status;	
}


FT_Status FT800Impl_Exit()
{
	return ( FT_GC_Exit() );
	
}
/* Api to get version of the library */

void FT800Impl_GetVersion(uint8_t *Major, uint8_t *Minor, uint8_t *Build)
{
	/* Update the arguments with version numbers */
	*Major = FT800IMPL_MAJOR;
	*Minor = FT800IMPL_MINOR;
	*Build = FT800IMPL_BUILD;
}
/* Apis related to graphics processor */	

uint8_t FT800Impl_SetIntMask(uint8_t IntMask)
{
	/* Write the interrupt mask value to register */
	FT_Transport_SPI_Write(REG_INT_MASK,IntMask);	
	
	return 0;
}
/* Api to detect pen down or pen up from the hardware */

uint8_t FT800Impl_IsPendown()
{
	//return 0 if pen up and return 1 if pen down
	if(FT_Transport_SPI_Read32(REG_TOUCH_DIRECT_XY)&0x80000000)//bit31 indicates whether pen down or pen up. 0 - touch is sensed, 1 - no touch
	{
		return 0;//pen up
	}
	
	return 1;//pen down
}
//set touch parameters

void FT800Impl_SetTouch(uint8_t AdcMode,uint16_t Charge,uint8_t Settle,uint16_t Oversample, uint16_t RZTreshold)
{
	FT_Transport_SPI_Write(REG_TOUCH_ADC_MODE,AdcMode);
	FT_Transport_SPI_Write16(REG_TOUCH_CHARGE,Charge);
	FT_Transport_SPI_Write(REG_TOUCH_SETTLE,Settle);
	FT_Transport_SPI_Write16(REG_TOUCH_OVERSAMPLE,Oversample);
	FT_Transport_SPI_Write(REG_TOUCH_RZTHRESH,RZTreshold);	
}

/* [] END OF FILE */
