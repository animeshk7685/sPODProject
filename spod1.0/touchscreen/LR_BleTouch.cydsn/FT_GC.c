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
#include "FT_GC.h"
#include "bridge.h"

FT_Status FT_GC_InitA(  uint8_t ResType)
{
	/* assign the pdn */
	PDNPin_Write(1);    	
	
	/* Initialize SPI channel */
	FT_Transport_SPI_Init();
	
	/* Bootup of graphics controller */
	FT_GC_Reset();
	/* Set the display configurations followed by external clock set, spi clock change wrt FT80x */
	FT_GC_DisplayConfigExternalClock(ResType);
	
	return FT_OK;
}

FT_Status FT_GC_Init_Soft(  uint8_t ResType)
{
	/* assign the pdn */
	PDNPin_Write(1);    	
	
	/* Initialize SPI channel */
	FT_Transport_SPI_Init();
	
	/* Bootup of graphics controller */
	FT_GC_Reset_Soft();
	/* Set the display configurations followed by external clock set, spi clock change wrt FT80x */
	FT_GC_DisplayConfigExternalClock(ResType);
	
	return FT_OK;
}

/* Api to program the output display wrt display parameters */
FT_Status FT_GC_InitB(uint16_t hperiod,uint16_t vperiod,uint16_t hfrontporch,uint16_t hbackporch,uint16_t hpulsewidth,
			   uint16_t vfrontporch,uint16_t vbackporch,uint16_t vpulsewidth,
			   uint8_t polarity,uint8_t swizzle,uint8_t fps)
{
	/* based on the input arguments compute the scan out register values */
	return FT_OK;
}

/* Exit SPI channel */			   
FT_Status FT_GC_Exit(void)
{
	/* Cross check whether SPI channel is used by others modules */
	
	FT_Transport_SPI_Exit();
	return FT_OK;
}
/* Api to get version of the library */
/* Good to have compile date as well */

void FT_GC_GetVersion(uint8_t *Major, uint8_t *Minor, uint8_t *Build)
{
	/* Update the version numbers */
	*Major = FT_GC_MAJOR;
	*Minor = FT_GC_MINOR;
	*Build = FT_GC_BUILD;
}
//FT_GC gpio bit for display enable/disable

void FT_GC_SetDisplayEnablePin(uint8_t GpioBit)
{
	/* update the display enable pin gpio bit number */
	DispGpioPin = GpioBit;
	
	/* set the direction of this bit to output. 1 is output and 0 is input */
	FT_Transport_SPI_Write(REG_GPIO_DIR,(1 << DispGpioPin) | FT_Transport_SPI_Read(REG_GPIO_DIR));
}

void FT_GC_SetAudioEnablePin(uint8_t GpioBit)
{
	//FT_GC gpio bit for audio enable/disable
	/* update the audio enable pin gpio bit number */
	AudioGpioPin = GpioBit;
	/* set the direction of this bit to output. 1 is output and 0 is input */
	FT_Transport_SPI_Write(REG_GPIO_DIR,(1 << AudioGpioPin) | FT_Transport_SPI_Read(REG_GPIO_DIR));
}


/* Apis to enable/disable backlight */
void FT_GC_DisplayOn(void)
{
	/* switch on the display, 1 means enable and 0 means disable */
	FT_Transport_SPI_Write(REG_GPIO,(1 << DispGpioPin) | FT_Transport_SPI_Read(REG_GPIO));
	
}

void FT_GC_DisplayOff(void)
{
	/* switch off the display */
	FT_Transport_SPI_Write(REG_GPIO,(~(1 << DispGpioPin)) & FT_Transport_SPI_Read(REG_GPIO));
}

void FT_GC_SetGpioDrive(uint8_t port, uint8_t level)// see datasheet 4.5.4 (0 for int and miso, 2 for gpios, 1 for everything else)
{
    uint8_t clearBitm;
    
    if(level > 3)
        level = 3;
    
    if(port == 1 && level > 1)
        level = 1;
    
    
    switch(port)
    {
        case 0:
            port = 2;
            clearBitm = 0x03;
        break;
        case 1:
            port = 4;
            clearBitm = 0x01;
        break;
        case 2:
            port = 5;
            clearBitm = 0x03;
        break;
    }
    
	/* switch off the display */
    FT_Transport_SPI_Write(REG_GPIO,(~(clearBitm << port)) & FT_Transport_SPI_Read(REG_GPIO));
	FT_Transport_SPI_Write(REG_GPIO,(level << port) | FT_Transport_SPI_Read(REG_GPIO));
}

//apis to set interrupt pin
void FT_GC_SetInterruptPin(uint16_t Intpin)
{
	/* update the interrupt pin */
	IntPin = Intpin;
}

void FT_GC_AudioOn(void)
{
	/* switch on the audio , 1 means enable and 0 means disable*/
	FT_Transport_SPI_Write(REG_GPIO,(1 << AudioGpioPin) | FT_Transport_SPI_Read(REG_GPIO));
}

void FT_GC_AudioOff(void)
{
	/* switch off the audio , 1 means enable and 0 means disable*/
	FT_Transport_SPI_Write(REG_GPIO,(~(1 << AudioGpioPin)) & FT_Transport_SPI_Read(REG_GPIO));
}


//api to reset coprocessor only - do this only when coprocessor returns error. for graphic processor error, utilize reset() api
void FT_GC_ResetCopro(void)
{
	/* first set the reset bit high */
	FT_Transport_SPI_Write(REG_CPURESET,FT_RESET_HOLD_COPROCESSOR);//first hold the coprocessor in reset
	//make the cmd read write pointers to 0
	CmdFifoWp = 0;
	FreeSpace = FT_CMDFIFO_SIZE - 4;
	FT_Transport_SPI_Write16(REG_CMD_READ,0);
	FT_Transport_SPI_Write16(REG_CMD_WRITE,0);
	CyDelay(10);//just to make sure reset is fine
	/* release the coprocessors from reset */
	FT_Transport_SPI_Write(REG_CPURESET,FT_RESET_RELEASE_COPROCESSOR);
	/* ideally CyDelay of 25ms is required for audio engine to playback mute sound to avoid pop sound */
}

//api to reset whole FT_GC via pdn - if pdn is not assigned then reset is not successful
void FT_GC_Reset(void)
{

	/* Reset of whole graphics controller */
	FT_GC_PDN_Cycle();
	/* send active command to enable SPI, followed by download of default DL into graphics engine, followed by SPI settings wrt internal clock requirements during bootup stage */
	FT_GC_ActiveInternalClock();
}

void FT_GC_Reset_Soft(void)
{

	/* Reset of whole graphics controller */
//	FT_GC_PDN_Cycle();
	/* send active command to enable SPI, followed by download of default DL into graphics engine, followed by SPI settings wrt internal clock requirements during bootup stage */
	FT_GC_ActiveInternalClock();
}

/* API to do power down cycle based on PDN gpio pin */
void FT_GC_PDN_Cycle(void)
{
	/* do a power cycle by toggling power down pin followed by active command followed by display list initialization sequence */
	PDNPin_Write(1);
	CyDelay(20);
	PDNPin_Write(0);
	CyDelay(20);
	PDNPin_Write(1);
	CyDelay(20);
	
}

/* API to set active command, set internal clock and download first DL */
void FT_GC_ActiveInternalClock(void)
{
	uint8_t FT_DLCODE_BOOTUP[12] =
	{
	  0,0,0,2,  //GPU instruction CLEAR_COLOR_RGB - black color
	  7,0,0,38, //GPU instruction CLEAR
	  0,0,0,0,  //GPU instruction DISPLAY
	};
	/* change the SPI clock to <11MHz */
	FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
	FT_GC_HostCommand(FT_ACTIVE);//wake up the processor from sleep state
	CyDelay(20);
	/* download the first display list */
	FT_Transport_SPI_Write_Src(FT_RAM_DL,FT_DLCODE_BOOTUP,12);
	/* perform first swap command */
	FT_Transport_SPI_Write(REG_DLSWAP,FT_DLSWAP_FRAME);
	//make the cmd read write pointers to 0
	CmdFifoWp = 0;
	FreeSpace = FT_CMDFIFO_SIZE - 4;	
}

/* API to configure the display and set to external clock */
void FT_GC_DisplayConfigExternalClock(uint8_t ResType)
{
	/* Presently supporting two types of resolutions, qvga and wqvga */
	if(ResType == FT_DISPLAY_QVGA_320x240)
	{
		FT_Transport_SPI_Write16(REG_VSYNC0, 		FT_DISPLAY_VSYNC0_QVGA );
		FT_Transport_SPI_Write16(REG_VSYNC1, 		FT_DISPLAY_VSYNC1_QVGA );
		FT_Transport_SPI_Write16(REG_VOFFSET, 		FT_DISPLAY_VOFFSET_QVGA);
		FT_Transport_SPI_Write16(REG_VCYCLE, 		FT_DISPLAY_VCYCLE_QVGA );
		FT_Transport_SPI_Write16(REG_HSYNC0, 		FT_DISPLAY_HSYNC0_QVGA );
		FT_Transport_SPI_Write16(REG_HSYNC1, 		FT_DISPLAY_HSYNC1_QVGA );
		FT_Transport_SPI_Write16(REG_HOFFSET, 		FT_DISPLAY_HOFFSET_QVGA);
		FT_Transport_SPI_Write16(REG_HCYCLE, 		FT_DISPLAY_HCYCLE_QVGA );
		FT_Transport_SPI_Write16(REG_HSIZE,			FT_DISPLAY_HSIZE_QVGA  );
		FT_Transport_SPI_Write16(REG_VSIZE, 		FT_DISPLAY_VSIZE_QVGA  );		 		
		FT_Transport_SPI_Write16(REG_PCLK_POL, 		FT_DISPLAY_PCLKPOL_QVGA);
        FT_Transport_SPI_Write16(REG_SWIZZLE, 		FT_DISPLAY_SWIZZLE_QVGA);
		/* after configuring display parameters, configure pclk */
		FT_Transport_SPI_Write16(REG_PCLK,			FT_DISPLAY_PCLK_QVGA   );
	}
	else if(ResType == FT_DISPLAY_WQVGA_480x272)
	{
		FT_Transport_SPI_Write16(REG_VSYNC0, 		FT_DISPLAY_VSYNC0_WQVGA );
		FT_Transport_SPI_Write16(REG_VSYNC1, 		FT_DISPLAY_VSYNC1_WQVGA );
		FT_Transport_SPI_Write16(REG_VOFFSET, 		FT_DISPLAY_VOFFSET_WQVGA);
		FT_Transport_SPI_Write16(REG_VCYCLE, 		FT_DISPLAY_VCYCLE_WQVGA );
		FT_Transport_SPI_Write16(REG_HSYNC0, 		FT_DISPLAY_HSYNC0_WQVGA );
		FT_Transport_SPI_Write16(REG_HSYNC1, 		FT_DISPLAY_HSYNC1_WQVGA );
		FT_Transport_SPI_Write16(REG_HOFFSET, 		FT_DISPLAY_HOFFSET_WQVGA);
		FT_Transport_SPI_Write16(REG_HCYCLE, 		FT_DISPLAY_HCYCLE_WQVGA );
		FT_Transport_SPI_Write16(REG_HSIZE,			FT_DISPLAY_HSIZE_WQVGA 	);
		FT_Transport_SPI_Write16(REG_VSIZE, 		FT_DISPLAY_VSIZE_WQVGA 	);		 		
		FT_Transport_SPI_Write16(REG_PCLK_POL, 		FT_DISPLAY_PCLKPOL_WQVGA);
        FT_Transport_SPI_Write16(REG_SWIZZLE, 		FT_DISPLAY_SWIZZLE_WQVGA);
		/* after configuring display parameters, configure pclk */
		FT_Transport_SPI_Write16(REG_PCLK,			FT_DISPLAY_PCLK_WQVGA	);	
	}
	else
	{
		FT_Transport_SPI_Write16(REG_VSYNC0, 		FT_DISPLAY_VSYNC0 );
		FT_Transport_SPI_Write16(REG_VSYNC1, 		FT_DISPLAY_VSYNC1 );
		FT_Transport_SPI_Write16(REG_VOFFSET, 		FT_DISPLAY_VOFFSET);
		FT_Transport_SPI_Write16(REG_VCYCLE, 		FT_DISPLAY_VCYCLE );
		FT_Transport_SPI_Write16(REG_HSYNC0, 		FT_DISPLAY_HSYNC0 );
		FT_Transport_SPI_Write16(REG_HSYNC1, 		FT_DISPLAY_HSYNC1 );
		FT_Transport_SPI_Write16(REG_HOFFSET, 		FT_DISPLAY_HOFFSET);
		FT_Transport_SPI_Write16(REG_HCYCLE, 		FT_DISPLAY_HCYCLE );
		FT_Transport_SPI_Write16(REG_HSIZE,			FT_DISPLAY_HSIZE  );
		FT_Transport_SPI_Write16(REG_VSIZE,         FT_DISPLAY_VSIZE  );		 		
		FT_Transport_SPI_Write16(REG_PCLK_POL, 		FT_DISPLAY_PCLKPOL);
        FT_Transport_SPI_Write16(REG_SWIZZLE, 		FT_DISPLAY_SWIZZLE);
		/* after configuring display parameters, configure pclk */
		FT_Transport_SPI_Write16(REG_PCLK,			FT_DISPLAY_PCLK   );
	}
	
	/* send host command to change the clock source from internal to external */
	FT_GC_HostCommand(FT_CLKEXT);
	/* change the clock to maximum spi operating frequency */
//	FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);//change the clock to normal operating frequency - harcoded wrt due
}

/* Apis related to graphics processor */	
//api to enable or disable interrupts
void FT_GC_EnableInterrupts(uint8_t GEnable,uint8_t Mask)
{
	FT_Transport_SPI_Write(REG_INT_EN,GEnable);//1 means enable global interrupts, 0 means disable global interrupts
	FT_Transport_SPI_Write(REG_INT_MASK,Mask);//0 means interrupts are masked, 1 means interrupts are not masked
}

//read the interrupt flag register - note that on ft800/ft801 the interrupts are clear by read
uint8_t FT_GC_ReadIntReg(void)
{
	return (FT_Transport_SPI_Read(REG_INT_FLAGS));
}

//void AssignCBFunptr(void *CBFunPtr){}//api to register callback function from application using interrupt mechanism	

/* APIs related to graphics engine */
FT_GEStatus FT_GC_AlphaFunc(uint8_t Func, uint8_t Ref) 
{
  return ( FT_GC_WriteCmd((9UL << 24) | ((Func & 7L) << 8) | ((Ref & 0xFFL) << 0)) );
}

FT_GEStatus FT_GC_Begin(uint8_t Prim) 
{
  return ( FT_GC_WriteCmd((31UL << 24) | Prim) );
}

FT_GEStatus FT_GC_BitmapHandle(uint8_t Handle)
{
  return ( FT_GC_WriteCmd((5UL << 24) | Handle) );
}

FT_GEStatus FT_GC_BitmapLayout(uint8_t Format, uint16_t Linestride, uint16_t Height) 
{
  // FT_GC_WriteCmd((7UL << 24) | ((format & 0x1FL) << 19) | ((linestride & 0x3FFL) << 9) | ((height & 0x1FFL) << 0));
  union Data32 data;
  data.A[0] = Height;
  data.A[1] = (1 & (Height >> 8)) | (Linestride << 1);
  data.A[2] = (7 & (Linestride >> 7)) | (Format << 3);
  data.A[3] = 7;
  return ( FT_GC_WriteCmd(data.UUint32) );
}

FT_GEStatus FT_GC_BitmapSize(uint8_t filter, uint8_t wrapx, uint8_t wrapy, uint16_t width, uint16_t height) 
{
  uint8_t fxy = (filter << 2) | (wrapx << 1) | (wrapy);
  // FT_GC_WriteCmd((8UL << 24) | ((uint32_t)fxy << 18) | ((width & 0x1FFL) << 9) | ((height & 0x1FFL) << 0));
  union Data32 data;
  data.A[0] = height;
  data.A[1] = (1 & (height >> 8)) | (width << 1);
  data.A[2] = (3 & (width >> 7)) | (fxy << 2);
  data.A[3] = 8;
  return ( FT_GC_WriteCmd(data.UUint32) );
}

FT_GEStatus FT_GC_BitmapSource(uint32_t Addr) 
{
  return ( FT_GC_WriteCmd((1UL << 24) | ((Addr & 0xFFFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformA(int32_t A) 
{
  return ( FT_GC_WriteCmd((21UL << 24) | ((A & 0x1FFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformB(int32_t B) 
{
  return ( FT_GC_WriteCmd((22UL << 24) | ((B & 0x1FFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformC(int32_t C) 
{
  return ( FT_GC_WriteCmd((23UL << 24) | ((C & 0xFFFFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformD(int32_t D) 
{
  return ( FT_GC_WriteCmd((24UL << 24) | ((D & 0x1FFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformE(int32_t E) 
{
  return ( FT_GC_WriteCmd((25UL << 24) | ((E & 0x1FFFFL) << 0)) );
}

FT_GEStatus FT_GC_BitmapTransformF(int32_t F) 
{
  return ( FT_GC_WriteCmd((26UL << 24) | ((F & 0xFFFFFFL) << 0)) );
}

FT_GEStatus FT_GC_BlendFunc(uint8_t Src, uint8_t Dst) 
{
  return ( FT_GC_WriteCmd((11UL << 24) | ((Src & 7L) << 3) | ((Dst & 7L) << 0)) );
}

FT_GEStatus FT_GC_Call(uint16_t Dest) 
{
  return ( FT_GC_WriteCmd((29UL << 24) | ((Dest & 0xFFFFL) << 0)) );
}

FT_GEStatus FT_GC_Cell(uint8_t Cell) 
{
  return ( FT_GC_WriteCmd((6UL << 24) | ((Cell & 0x7FL) << 0)) );
}

FT_GEStatus FT_GC_ClearColorA(uint8_t Alpha) 
{
  return ( FT_GC_WriteCmd((15UL << 24) | ((Alpha & 0xFFL) << 0)) );
}

FT_GEStatus FT_GC_ClearColorRGB(uint8_t red, uint8_t green, uint8_t blue) 
{
  return ( FT_GC_WriteCmd((2UL << 24) | ((red & 0xFFL) << 16) | ((green & 0xFFL) << 8) | ((blue & 0xFFL) << 0)) );
}

FT_GEStatus FT_GC_ClearColorRGB_int(uint32_t rgb) 
{
  return ( FT_GC_WriteCmd((2UL << 24) | (rgb & 0xFFFFFFL)) );
}    

FT_GEStatus FT_GC_ClearCST(uint8_t c, uint8_t s, uint8_t t) 
{
  uint8_t m = (c << 2) | (s << 1) | t;
  return ( FT_GC_WriteCmd((38UL << 24) | m) );
}    

FT_GEStatus FT_GC_Clear(void) 
{
  return ( FT_GC_WriteCmd((38UL << 24) | 7) );
}    

FT_GEStatus FT_GC_ClearStencil(uint8_t s) 
{
  return ( FT_GC_WriteCmd((17UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_ClearTag(uint8_t s) 
{
  return ( FT_GC_WriteCmd((18UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_ColorA(uint8_t Alpha) 
{
  return ( FT_GC_WriteCmd((16UL << 24) | ((Alpha & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_ColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a) 
{
  return ( FT_GC_WriteCmd((32UL << 24) | ((r & 1L) << 3) | ((g & 1L) << 2) | ((b & 1L) << 1) | ((a & 1L) << 0)) );
}    

FT_GEStatus FT_GC_ColorRGB(uint8_t red, uint8_t green, uint8_t blue) {
  // FT_GC_WriteCmd((4UL << 24) | ((red & 0xFFL) << 16) | ((green & 0xFFL) << 8) | ((blue & 0xFFL) << 0)) );
  union Data32 data;
  data.A[0] = blue;
  data.A[1] = green;
  data.A[2] = red;
  data.A[3] = 4;
  return ( FT_GC_WriteCmd(data.UUint32) );
}    

FT_GEStatus FT_GC_Display(void) 
{
  return ( FT_GC_WriteCmd((0UL << 24)) );
}    

FT_GEStatus FT_GC_End(void) {
  return ( FT_GC_WriteCmd((33UL << 24)) );
}    

FT_GEStatus FT_GC_Jump(uint16_t Dest) 
{
  return ( FT_GC_WriteCmd((30UL << 24) | ((Dest & 0x7FFL) << 0)) );
}    

FT_GEStatus FT_GC_LineWidth(uint16_t Width) 
{
  return ( FT_GC_WriteCmd((14UL << 24) | ((Width & 0xFFFL) << 0)) );
}    

FT_GEStatus FT_GC_Macro(uint8_t m) 
{
  return ( FT_GC_WriteCmd((37UL << 24) | ((m & 1L) << 0)) );
}   

FT_GEStatus FT_GC_PointSize(uint16_t Size) 
{
  return ( FT_GC_WriteCmd((13UL << 24) | ((Size & 0x1FFFL) << 0)) );
}    

FT_GEStatus FT_GC_RestoreContext(void) 
{
  return ( FT_GC_WriteCmd((35UL << 24)) );
}    

FT_GEStatus FT_GC_Return(void) 
{
  return ( FT_GC_WriteCmd((36UL << 24)) );
}    

FT_GEStatus FT_GC_SaveContext(void) 
{
  return ( FT_GC_WriteCmd((34UL << 24)) );
}    

FT_GEStatus FT_GC_ScissorSize(uint16_t Width, uint16_t Height) 
{
  return ( FT_GC_WriteCmd((28UL << 24) | ((Width & 0x3FFL) << 10) | ((Height & 0x3FFL) << 0)) );
}    

FT_GEStatus FT_GC_ScissorXY(uint16_t x, uint16_t y)
{
  return ( FT_GC_WriteCmd((27UL << 24) | ((x & 0x1FFL) << 9) | ((y & 0x1FFL) << 0)) );
}    

FT_GEStatus FT_GC_StencilFunc(uint8_t Func, uint8_t Ref, uint8_t Mask) 
{
  return ( FT_GC_WriteCmd((10UL << 24) | ((Func & 7L) << 16) | ((Ref & 0xFFL) << 8) | ((Mask & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_StencilMask(uint8_t Mask) 
{
  return ( FT_GC_WriteCmd((19UL << 24) | ((Mask & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_StencilOp(uint8_t Sfail, uint8_t Spass) 
{
  return ( FT_GC_WriteCmd((12UL << 24) | ((Sfail & 7L) << 3) | ((Spass & 7L) << 0)) );
}   

FT_GEStatus FT_GC_TagMask(uint8_t Mask) 
{
  return ( FT_GC_WriteCmd((20UL << 24) | ((Mask & 1L) << 0)) );
}    

FT_GEStatus FT_GC_Tag(uint8_t s) 
{
  return ( FT_GC_WriteCmd((3UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus FT_GC_Vertex2f(int16_t x, int16_t y) 
{
  // x = int(16 * x);
  // y = int(16 * y);
  return ( FT_GC_WriteCmd((1UL << 30) | ((x & 0x7FFFL) << 15) | ((y & 0x7FFFL) << 0)) );
}    

FT_GEStatus FT_GC_Vertex2ii(uint16_t x, uint16_t y, uint8_t Handle, uint8_t Cell) 
{
  //return (FT_GC_WriteCmd((2UL << 30) | ((x & 0x1FFL) << 21) | ((y & 0x1FFL) << 12) | ((handle & 0x1FL) << 7) | ((cell & 0x7FL) << 0)) ));
   /* Generates invalid graphics instruction due to shift operation of -ve coefficient */
  x &= 0x1FFL;//error handling of -ve coefficients
  y &= 0x1FFL;
  union Data32 data;
  data.A[0] = Cell | ((Handle & 1) << 7);
  data.A[1] = (Handle >> 1) | (y << 4);
  data.A[2] = (y >> 4) | (x << 5);
  data.A[3] = (2 << 6) | (x >> 3);
  return ( FT_GC_WriteCmd(data.UUint32) );
}    


/* graphics helper apis */
FT_GEStatus FT_GC_ColorRGB_int(uint32_t rgb) 
{
  return ( FT_GC_WriteCmd((4UL << 24) | (rgb & 0xFFFFFFL)) );
}    

/* Form two commands, one for rgb and the other for a */
FT_GEStatus FT_GC_ColorARGB(uint32_t argb) 
{
	FT_GEStatus Status;
	FT_GC_WriteCmd((4UL << 24) | (argb & 0xFFFFFFL));
	Status = FT_GC_WriteCmd((16UL << 24) | ((argb>>24) & 0xFFL) );
  
  return Status;
}   



FT_GEStatus FT_GC_Cmd_Logo(void)
{
	FT_GEStatus Status;	
	Status = FT_GC_WriteCmd(CMD_LOGO);		
	return Status;
}    


FT_GEStatus FT_GC_Cmd_Append(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	FT_GC_WriteCmd(CMD_APPEND);
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Num);//checking only for the last command
	
	return Status;
}    

FT_GEStatus FT_GC_Cmd_BGColor(uint32_t c)
{
	FT_GEStatus Status;
	FT_GC_WriteCmd(CMD_BGCOLOR);
	Status = FT_GC_WriteCmd(c);
	
	return Status;
}    


/* API for touch transform calculation and set the transform registers */
/*FT_GEStatus FT_GC_Cmd_Touch_Transform(int32_t x0,int32_t y0,int32_t x1,int32_t y1,int32_t x2,int32_t y2,int32_t tx0,int32_t ty0,int32_t tx1,int32_t ty1,int32_t tx2,int32_t ty2,uint32_t Result)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_BITMAP_TRANSFORM);
	FT_GC_WriteCmd(x0);
	FT_GC_WriteCmd(y0);
	FT_GC_WriteCmd(x1);
	FT_GC_WriteCmd(y1);
	FT_GC_WriteCmd(x2);
	FT_GC_WriteCmd(y2);
	FT_GC_WriteCmd(tx0);
	FT_GC_WriteCmd(ty0);
	FT_GC_WriteCmd(tx1);
	FT_GC_WriteCmd(ty1);
	FT_GC_WriteCmd(tx2);
	FT_GC_WriteCmd(ty2);
	Status = FT_GC_WriteCmd(Result);

	return Status;
}
*/
/* make sure the string is in ram instead of program memory. it is assumed that *s is valid string and if null then it should contain \0 */
FT_GEStatus FT_GC_Cmd_Button(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_BUTTON);
	FT_GC_WriteCmd(((y & 0xFFFFL) <<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = FT_GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);//make sure last byte is added into the
	
	return Status;
}    


/* Check the result of command calibrate by cmd_GetResult */
FT_GEStatus FT_GC_Cmd_Calibrate(uint32_t Result)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_CALIBRATE);
	Status = FT_GC_WriteCmd(Result);//write extra word for result
	
	return Status;
}    

FT_GEStatus FT_GC_Cmd_Clock(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_CLOCK);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	FT_GC_WriteCmd(((m & 0xFFFFL)<<16) | (h & 0xFFFFL));
	Status = FT_GC_WriteCmd(((ms & 0xFFFFL)<<16) | (s & 0xFFFFL));
	
	return Status;
}    


FT_GEStatus FT_GC_Cmd_ColdStart(void)
{
	//should we even change command read and write pointers
	return ( FT_GC_WriteCmd(CMD_COLDSTART) );
}   

FT_GEStatus FT_GC_Cmd_Dial(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Val)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_DIAL);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	Status = FT_GC_WriteCmd(Val);
	
	return Status;
}    

FT_GEStatus FT_GC_Cmd_DLStart(void)
{
	return ( FT_GC_WriteCmd(CMD_DLSTART) );
}    

FT_GEStatus FT_GC_Cmd_FGColor(uint32_t c)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_FGCOLOR);
	Status = FT_GC_WriteCmd(c);
	
	return Status;	
}    


FT_GEStatus FT_GC_Cmd_Gauge(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Major, uint16_t Minor, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GAUGE);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	FT_GC_WriteCmd(((Minor & 0xFFFFL)<<16) | (Major & 0xFFFFL));
	Status = FT_GC_WriteCmd(((Range & 0xFFFFL)<<16) | (Val & 0xFFFFL));
	
	return Status;	
}    

/* Results are available from getresults api */
FT_GEStatus FT_GC_Cmd_GetMatrix(void)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GETMATRIX);
	FT_GC_WriteCmd(0);
	FT_GC_WriteCmd(0);
	FT_GC_WriteCmd(0);
	FT_GC_WriteCmd(0);
	FT_GC_WriteCmd(0);
	Status = FT_GC_WriteCmd(0);
	
	return Status;	
}    

/* perform this api and wait for the completion by finish and use getresults api  for the result*/
FT_GEStatus FT_GC_Cmd_GetProps(uint32_t *Ptr, uint32_t *w, uint32_t *h)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GETPROPS);
	FT_GC_WriteCmd(0);
	FT_GC_WriteCmd(0);
	Status = FT_GC_WriteCmd(0);
	
	return Status;	
}    

//perform this api and wait for the completion by finish and use cmd_getresult for the result
FT_GEStatus FT_GC_Cmd_GetPtr(uint32_t Result)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GETPTR);
	Status = FT_GC_WriteCmd(Result);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_GradColor(uint32_t c)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GRADCOLOR);
	Status = FT_GC_WriteCmd(c);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_GRADIENT);
	FT_GC_WriteCmd(((y0 & 0xFFFFL)<<16)|(x0 & 0xFFFFL));
	FT_GC_WriteCmd(rgb0);
	FT_GC_WriteCmd(((y1 & 0xFFFFL)<<16)|(x1 & 0xFFFFL));
	Status = FT_GC_WriteCmd(rgb1);
	
	return Status;	
}    

//after calling this api copy the raw content which is output from deflate
FT_GEStatus FT_GC_Cmd_Inflate(uint32_t Ptr)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_INFLATE);
	Status = FT_GC_WriteCmd(Ptr);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Interrupt(uint32_t ms)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_INTERRUPT);
	Status = FT_GC_WriteCmd(ms);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Keys(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_KEYS);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = FT_GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);	
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_LoadIdentity(void)
{
	return ( FT_GC_WriteCmd(CMD_LOADIDENTITY) );
}    

//after this api, copy the jpeg data into fifo
FT_GEStatus FT_GC_Cmd_LoadImage(uint32_t Ptr, int32_t Options)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_LOADIMAGE);
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Options);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Memcpy(uint32_t Dest, uint32_t Src, uint32_t Num)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_MEMCPY);
	FT_GC_WriteCmd(Dest);
	FT_GC_WriteCmd(Src);
	Status = FT_GC_WriteCmd(Num);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Memset(uint32_t Ptr, uint8_t Value, uint32_t Num)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_MEMSET);
	FT_GC_WriteCmd(Ptr);
	FT_GC_WriteCmd(Value);	
	Status = FT_GC_WriteCmd(Num);
	
	return Status;	
}    

//perform this api, wait for the completion and use cmd_getresult for the result
FT_GEStatus FT_GC_Cmd_Memcrc(uint32_t Ptr, uint32_t Num,uint32_t *Result)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_MEMCRC);
	FT_GC_WriteCmd(Ptr);
	FT_GC_WriteCmd(Num);
	Status = FT_GC_WriteCmd(*Result);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Memwrite(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_MEMWRITE);
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Num);
	
	return Status;	
}    


FT_GEStatus FT_GC_Cmd_Memzero(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_MEMZERO);
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Num);
	return Status;	
}    


FT_GEStatus FT_GC_Cmd_Number(int16_t x, int16_t y, uint8_t Font, uint16_t Options, uint32_t n)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_NUMBER);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = FT_GC_WriteCmd(n);
	
	return Status;	
}   

FT_GEStatus FT_GC_Cmd_Progress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_PROGRESS);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = FT_GC_WriteCmd(Range);
	
	return Status;	
}    

//perform this api, wait for the completion and use cmd_getresult for the result
FT_GEStatus FT_GC_Cmd_RegRead(uint32_t Ptr,uint32_t Result)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_REGREAD);
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Result);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Rotate(int32_t a)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_ROTATE);
	Status = FT_GC_WriteCmd(a);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Scale(int32_t sx, int32_t sy)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SCALE);
	FT_GC_WriteCmd(sx);
	Status = FT_GC_WriteCmd(sy);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_ScreenSaver(void)
{
	return ( FT_GC_WriteCmd(CMD_SCREENSAVER) );
}    

FT_GEStatus FT_GC_Cmd_Scrollbar(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Size, uint16_t Range)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SCROLLBAR);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = FT_GC_WriteCmd(((Range & 0xFFFFL)<<16) | (Size & 0xFFFFL));
	
	return Status;	
}    

//make sure ptr is pointing to table and in turn table has pointer to the actual bitmap data
FT_GEStatus FT_GC_Cmd_SetFont(uint8_t Font, uint32_t Ptr)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SETFONT);
	FT_GC_WriteCmd(Font);
	Status = FT_GC_WriteCmd(Ptr);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_SetMatrix(void)
{
	return ( FT_GC_WriteCmd(CMD_SETMATRIX) );	
}    


FT_GEStatus FT_GC_Cmd_Sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t Ptr, uint16_t Format)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SKETCH);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(Ptr);
	Status = FT_GC_WriteCmd(Format);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Slider(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t Options, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SLIDER);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = FT_GC_WriteCmd(Range);
	
	return Status;	
}    

//perform this api and wait for the completion
FT_GEStatus FT_GC_Cmd_Snapshot(uint32_t OutputAddr)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SNAPSHOT);
	Status = FT_GC_WriteCmd(OutputAddr);
	
	return Status;	
}    

//note that macro 0 and macro 1 are modified by the coprocessor when performing this function
FT_GEStatus FT_GC_Cmd_Spinner(int16_t x, int16_t y, uint8_t Style, uint8_t Scale)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_SPINNER);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	Status = FT_GC_WriteCmd(((Scale & 0xFFFFL)<<16) | (Style & 0xFFFFL));
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Stop(void)
{
	return ( FT_GC_WriteCmd(CMD_STOP) );
}    

FT_GEStatus FT_GC_Cmd_Swap(void)
{
	return ( FT_GC_WriteCmd(CMD_SWAP) );
}    

FT_GEStatus FT_GC_Cmd_Text(int16_t x, int16_t y, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	FT_GC_WriteCmd(CMD_TEXT);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = FT_GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Toggle(int16_t x, int16_t y, int16_t w, uint8_t Font, uint16_t Options, uint16_t State, const char *s)
{
	FT_GEStatus Status;
	FT_GC_WriteCmd(CMD_TOGGLE);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((Font & 0xFFFFL)<<16) | (w & 0xFFFFL));
	FT_GC_WriteCmd(((State & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = FT_GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Track(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Tag)
{
	FT_GEStatus Status;
	FT_GC_WriteCmd(CMD_TRACK);
	FT_GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	FT_GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	Status = FT_GC_WriteCmd(Tag);
	
	return Status;	
}    

FT_GEStatus FT_GC_Cmd_Translate(int32_t tx, int32_t ty)
{
	FT_GEStatus Status;
	
	FT_GC_WriteCmd(CMD_TRANSLATE);
	FT_GC_WriteCmd(tx);
	Status = FT_GC_WriteCmd(ty);
	
	return Status;	
}    


/* Apis related to audio engine */
FT_AEStatus FT_GC_PlaySound_Volume(uint8_t Volume,uint16_t SoundNote)
{
	FT_Transport_SPI_Write(REG_VOL_SOUND,Volume);//change the volume of synthesized sound, 0 means off, 255 means max on
	FT_Transport_SPI_Write16(REG_SOUND,SoundNote);
	FT_Transport_SPI_Write(REG_PLAY,FT_SOUND_PLAY);
	
	return FT_AE_OK;
}    
//higher byte is the note and lower byte is the sound
FT_AEStatus FT_GC_PlaySound(uint16_t SoundNote)
{
	FT_Transport_SPI_Write16(REG_SOUND,SoundNote);
	FT_Transport_SPI_Write(REG_PLAY,FT_SOUND_PLAY);
	return FT_AE_OK;
}    

//volume will not be modified
void FT_GC_StopSound(void)
{
	FT_Transport_SPI_Write16(REG_SOUND,FT_SILENCE);//configure silence
	FT_Transport_SPI_Write(REG_PLAY,FT_SOUND_PLAY);//play the silence
}    

void FT_GC_SetSoundVolume(uint8_t Volume)
{
	FT_Transport_SPI_Write(REG_VOL_SOUND,Volume);
}    

uint8_t FT_GC_GetSoundVolume(void)
{
	return FT_Transport_SPI_Read(REG_VOL_SOUND);
}    

//one shot or continuous, sampling frequency is from 8k to 48k
FT_AEStatus FT_GC_PlayAudio(uint8_t Volume,uint8_t Format,uint16_t SamplingFreq,uint32_t BufferAddr,uint32_t BufferSize,uint8_t Loop)
{
	if((SamplingFreq*1L < FT_AUDIO_SAMPLINGFREQ_MIN*1L) | (SamplingFreq*1L > FT_AUDIO_SAMPLINGFREQ_MAX*1L))
	{
		return FT_AE_ERROR_SAMPLINGFREQ_OUTOFRANGE;
	}
	if(Format > FT_ADPCM_SAMPLES)
	{
		return FT_AE_ERROR_FORMAT;
	}
	FT_Transport_SPI_Write(REG_VOL_PB,Volume);
	FT_Transport_SPI_Write32(REG_PLAYBACK_START,BufferAddr);
	FT_Transport_SPI_Write32(REG_PLAYBACK_LENGTH,BufferSize);
	FT_Transport_SPI_Write32(REG_PLAYBACK_FREQ,SamplingFreq);
	FT_Transport_SPI_Write(REG_PLAYBACK_FORMAT,Format);
	FT_Transport_SPI_Write(REG_PLAYBACK_LOOP,Loop);//0 means one shot and 1 means loop
	
	FT_Transport_SPI_Write(REG_PLAYBACK_PLAY,FT_AUDIO_PLAY);

	return FT_AE_OK;
}    

void FT_GC_SetAudioVolume(uint8_t Volume)
{
	FT_Transport_SPI_Write(REG_VOL_PB,Volume);
}    

//returns playback stopped or continue
FT_AEStatus FT_GC_GetAudioStats(uint32_t *CurrPlayAddr)
{
	*CurrPlayAddr = FT_Transport_SPI_Read32(REG_PLAYBACK_READPTR);
	
	//in case of loop, check for the playback status
	if(0 == FT_Transport_SPI_Read(REG_PLAYBACK_PLAY))
	{
		return FT_AE_PLAYBACK_STOPPED;
	}

	return FT_AE_PLAYBACK_CONTINUE;
}    

uint8_t FT_GC_GetAudioVolume(void)
{
	return  (FT_Transport_SPI_Read(REG_VOL_PB));
}    

//volume will not be modified
void FT_GC_StopAudio(void)
{
	//configure audio with length o and play
	FT_Transport_SPI_Write32(REG_PLAYBACK_LENGTH,0);
	FT_Transport_SPI_Write(REG_PLAYBACK_LOOP,0);//0 means one shot and 1 means loop
	
	FT_Transport_SPI_Write(REG_PLAYBACK_PLAY,FT_AUDIO_PLAY);	
}    


/* Apis related to touch engine */
//one of 0ff/oneshot/frame/continuous. default being continuous
void FT_GC_SetTouchMode(uint8_t TMode)
{
	FT_Transport_SPI_Write(REG_TOUCH_MODE,TMode);
}    

void FT_GC_SetTouchAdcMode(uint8_t TAMode)
{
	FT_Transport_SPI_Write(REG_TOUCH_ADC_MODE,TAMode);
} 

void FT_GC_SetTouchCharge(uint16_t TChrg)
{
	FT_Transport_SPI_Write16(REG_TOUCH_CHARGE,TChrg);
} 

void FT_GC_SetTouchSettle(uint8_t TSet)
{
	FT_Transport_SPI_Write(REG_TOUCH_SETTLE,TSet);
} 

void FT_GC_SetTouchOversample(uint8_t TSam)
{
	FT_Transport_SPI_Write(REG_TOUCH_OVERSAMPLE,TSam);
} 

void FT_GC_SetTouchRzThresh(uint16_t TRThs)
{
	FT_Transport_SPI_Write16(REG_TOUCH_RZTHRESH,TRThs);
} 



//api to set coordinates for host specific tag query
void FT_GC_SetHostTagXY(uint16_t xoffset,uint16_t yoffset)
{
	uint8_t A[6];
	//little endian specific
	A[0] = xoffset & 0xFF;
	A[1] = xoffset >> 8;
	A[4] = yoffset & 0xFF;
	A[5] = yoffset >> 8;
	FT_Transport_SPI_Write_Src(REG_TAG_X,A,6);
}    

//api to get TAG from FT_GC for coordinates set by  SetHostTagXY() api - host needs to wait for at least 1 frame to get these query values
uint8_t FT_GC_GetHostTagXY(void)
{
	return ( FT_Transport_SPI_Read(REG_TAG) );
}   

//get the touched object tag and repective xy coordinates
void FT_GC_GetTagXY(sTagXY *sTagxy)
{
	FT_Transport_SPI_Read_Src(REG_TOUCH_TAG_XY,(uint8_t *)sTagxy,6);
}    

//get the track value and the tag value
void FT_GC_GetTrackTag(sTrackTag *sTracktag)
{
	uint32_t *ptr = (uint32_t *)sTracktag;
	*ptr = FT_Transport_SPI_Read32(REG_TRACKER);
}   

void FT_GC_GetRawTouch(sTouchRaw *sTouchRaw)
{
    uint32_t regRead;
//	uint32_t *ptr = (uint32_t *)sTouchRaw;
	regRead = FT_Transport_SPI_Read32(REG_TOUCH_DIRECT_XY);
    
    sTouchRaw->touch = (regRead >> 31) & 0x00000001;
    sTouchRaw->x = (regRead >> 16) & 0x000003FF;
    sTouchRaw->y = regRead & 0x000003FF;
}



/* APIS related to power modes */
void FT_GC_HostCommand(uint32_t HostCommand)
{
    
    
//	uint32_t Addr;
	/* construct host command and send to graphics controller */
//	Addr = HostCommand<<16;
//	FT_Transport_SPI_Read(Addr);//ideally sending 3 bytes is sufficient
    FT_Transport_SPI_HostCmd(HostCommand);
	CyDelay(20);//worst scenario
}    
	
//void FT_GC_CheckIntFlag(uint8_t IntFlag){}
FT_GEStatus FT_GC_UpdateFreeSpace()
{	
	if(TrnsFlag)
	{
		FT_GC_EndTransferCmd();
		//update the write pointer
		FT_Transport_SPI_Write16(REG_CMD_WRITE,CmdFifoWp);
		FT_GC_StartTransferCmd();
	}
	else
	{
		//update the write pointer
		FT_Transport_SPI_Write16(REG_CMD_WRITE,CmdFifoWp);
	}
	return FT_GE_OK;
}    

/* API related to coprocessor fifo write command */
FT_GEStatus FT_GC_ChkGetFreeSpace(uint16_t NBytes)
{	
	//return busy if no space
	if(FreeSpace < NBytes)
	{
		if(TrnsFlag)
		{
			FT_GC_EndTransferCmd();
			TrnsFlag = 1;//coz EndTransferCmd will make it 0
		}
		//update the write pointer
		FT_Transport_SPI_Write16(REG_CMD_WRITE,CmdFifoWp);
		while(FreeSpace < NBytes)
		{
			uint16_t rdptr = FT_Transport_SPI_Read16(REG_CMD_READ);
			if(rdptr == FT_COPRO_ERROR)
			{
				return FT_GE_ERROR;
			}
			//update the freespace by reading the register
			FreeSpace = ((CmdFifoWp - rdptr)&0xffc);
			FreeSpace = (FT_CMDFIFO_SIZE - 4) - FreeSpace;
		}		
		if(TrnsFlag)
		{
			FT_GC_StartTransferCmd();
		}
	}
	return FT_GE_OK;
}    


//assert CSpin and send write command
FT_GEStatus FT_GC_StartTransferCmd()
{
	//start write transaction
	FT_Transport_SPI_StartWrite(FT_RAM_CMD + CmdFifoWp);
	TrnsFlag = 1;
	return FT_GE_OK;
}    


FT_GEStatus FT_GC_TransferCmd(uint32_t Cmd)
{
	if(FreeSpace <4)
	{
		//blocking call till freespace is available
		if(FT_GE_ERROR == FT_GC_ChkGetFreeSpace(4))
		{
			return FT_GE_ERROR;
		}
	}
	FT_Transport_SPI_Transfer32(Cmd);
	CmdFifoWp = (CmdFifoWp + 4)&0xfff;
	FreeSpace -= 4;
	
	return FT_GE_OK;
}    

FT_GEStatus FT_GC_TransferCmd_Src(uint8_t *Src,uint32_t NBytes)
{
	uint32_t i,Count;
	//align the NBytes to multiple of 4
	NBytes = (NBytes + 3)&(~3);
	//transfer the whole buffer into command buffer
	while(NBytes)
	{
		Count = NBytes;
		if(Count > FreeSpace)
		{
			//first update the free space
			FT_GC_UpdateFreeSpace();
			//then transfer the data
			Count = min(FreeSpace,Count);
			for(i = 0;i<Count;i++)	
				FT_Transport_SPI_Transfer(*Src++);
			CmdFifoWp = (CmdFifoWp + Count)&0xfff;
			FreeSpace -= Count;
			//get the free space
			NBytes -= Count;
			Count = min(NBytes,FT_CMDFIFO_SIZE/2);//atleast wait for half the buffer completion
			if(FT_GE_ERROR == FT_GC_ChkGetFreeSpace(Count))
			{
				return FT_GE_ERROR;
			}
		}
		else
		{
			//transfer of data to command buffer
			for(i = 0;i<Count;i++)	
				FT_Transport_SPI_Transfer(*Src++);
			CmdFifoWp = (CmdFifoWp + Count)&0xfff;
			FreeSpace -= Count;
			NBytes -= Count;
		}		
	}
	return FT_GE_OK;
}    

FT_GEStatus FT_GC_TransferCmdfromflash(const uint8_t *Src, uint32_t NBytes)
{
	uint32_t i,Count;
	//align the NBytes to multiple of 4
	NBytes = (NBytes + 3)&(~3);
	//transfer the whole buffer into command buffer
	while(NBytes)
	{
		Count = NBytes;
		if(Count > FreeSpace)
		{
			//first update the free space
			FT_GC_UpdateFreeSpace();
			//then transfer the data
			Count = min(FreeSpace,Count);
			for(i = 0;i<Count;i++)	
			{
				FT_Transport_SPI_Transfer(*Src);
				Src++;
			}	
			CmdFifoWp = (CmdFifoWp + Count)&0xfff;
			FreeSpace -= Count;
			//get the free space
			NBytes -= Count;
			Count = min(NBytes,(uint32_t)(FT_CMDFIFO_SIZE/2));//atleast wait for half the buffer completion
			if(FT_GE_ERROR == FT_GC_ChkGetFreeSpace(Count))
			{
				return FT_GE_ERROR;
			}
		}
		else
		{
			//transfer of data to command buffer
			for(i = 0;i<Count;i++)	
			{
				FT_Transport_SPI_Transfer(*Src);
				Src++;
			}	
			CmdFifoWp = (CmdFifoWp + Count)&0xfff;
			FreeSpace -= Count;
			NBytes -= Count;
		}		
	}
	return FT_GE_OK;
}    

//end the command transfer
void FT_GC_EndTransferCmd(void)
{	
	FT_Transport_SPI_EndTransfer();
	//update the write pointer of fifo
	FT_Transport_SPI_Write32(REG_CMD_WRITE, CmdFifoWp);
	TrnsFlag = 0;
}    


FT_GEStatus FT_GC_WriteCmd(uint32_t Cmd)
{
	FT_GEStatus Status;
    
#if 0	
	if(0 == TrnsFlag)
		StartTransferCmd();
	Status = TransferCmd(Cmd);
	if(0 == TrnsFlag)
		EndTransferCmd();
#else
	if(0 == TrnsFlag)
	{
		FT_GC_StartTransferCmd();
		Status = FT_GC_TransferCmd(Cmd);
		FT_GC_EndTransferCmd();
	}
	else
	{
		Status = FT_GC_TransferCmd(Cmd);
	}

#endif

	return Status;
}

/*
void FT_GC_TransferCmdStr(const char *Str)
{
	TransferCmd(Str,strlen(Str)+1);
}
FT_GEStatus FT_GC_WriteCmdStr(const char* Str)
{
	StartTransferCmd();
	TransferCmd(Str,strlen(Str)+1);
	EndTransferCmd();
}*/

//api to send N bytes to command buffer, this api pads with 0's at the end 
FT_GEStatus FT_GC_WriteCmd_Src(uint8_t *Src,uint32_t NBytes)
{
	FT_GEStatus Status;
	if(0 == TrnsFlag)
	{
		FT_GC_StartTransferCmd();
		Status = FT_GC_TransferCmd_Src(Src,NBytes);
		FT_GC_EndTransferCmd();//here transflag is made to 0
	}
	else
		Status = FT_GC_TransferCmd_Src(Src,NBytes);
		
	return Status;
}   


FT_GEStatus FT_GC_WriteCmdfromflash(const uint8_t *Src,uint32_t NBytes)
{
	FT_GEStatus Status;
	if(0 == TrnsFlag)
	{
		FT_GC_StartTransferCmd();
		Status =  FT_GC_TransferCmdfromflash(Src,NBytes);		
		FT_GC_EndTransferCmd();//here transflag is made to 0
	}
	else{
		Status = FT_GC_TransferCmdfromflash(Src,NBytes);
    }
		
	return Status;
}    
		

//reads the result of the previous commands such as cmd_memcrc,cmd_calibration, cmd_regread which has return values. if busy returns busy status
FT_GEStatus FT_GC_Cmd_GetResult(uint32_t *Result)
{
	*Result = FT_Transport_SPI_Read32(FT_RAM_CMD + ((CmdFifoWp - 4)&0xFFC));//make sure no other commands are issued after cmd_memcrc/cmd_calibration/cmd_regread
	
	return FT_GE_OK;
}    

//api to read n bytes from the current write pointer location
//make sure no other commands are issued after cmd_memcrc/cmd_calibration/cmd_regread
//assumed that number of bytes in array is allocated/managed by application
FT_GEStatus FT_GC_Cmd_GetResults(int8_t *pA,uint16_t NBytes)
{
	//handling of circular buffer
	if((CmdFifoWp - NBytes) < 0)
	{
		uint16_t ReadLen = NBytes - CmdFifoWp;
		FT_Transport_SPI_Read_Src((FT_RAM_CMD + (FT_CMDFIFO_SIZE - ReadLen)),(uint8_t *)pA,ReadLen);//first read the end bytes
		pA += ReadLen;
		ReadLen = NBytes - ReadLen;
		FT_Transport_SPI_Read_Src(FT_RAM_CMD,(uint8_t *)pA,ReadLen);//later read the starting bytes
	}
	else
	{
		FT_Transport_SPI_Read_Src(FT_RAM_CMD + (CmdFifoWp - NBytes),(uint8_t *)pA,NBytes);
	}
	
	return FT_GE_OK;
}    


/* Special apis */
//apis for ease of usage in FT_GC
//inserts cmd_dlstart() followed by clear(1,1,1) graphics command
void FT_GC_DLStart(void)
{
	FT_GC_Cmd_DLStart();
	FT_GC_ClearCST(1,1,1);
}    


//inserts display() gpu instruction at the end and inserts cmd_swap() command
void FT_GC_DLEnd(void)
{
	FT_GC_Display();
	FT_GC_Cmd_Swap();
}    


//special api to check logo completion
FT_GEStatus FT_GC_CheckLogo(void)
{
	uint16_t ReadCmdPtr = FT_Transport_SPI_Read16(REG_CMD_READ) ;
	if((ReadCmdPtr == FT_Transport_SPI_Read16(REG_CMD_WRITE)) && (ReadCmdPtr == 0))
	{
		CmdFifoWp = 0;
		return FT_GE_FINISHED;
	}
	else if(FT_COPRO_ERROR == ReadCmdPtr)
	{
		return FT_GE_ERROR;
	}
	else
	{
		return FT_GE_BUSY;
	}
	
}   


//apis to render all the commands to hardware
//api to flush out all the commands to FT_GC, does not wait for the completion of the rendering
FT_GEStatus FT_GC_Flush(void)
{
	if(TrnsFlag)
	{
		FT_GC_EndTransferCmd();
	}	
	FT_Transport_SPI_Write16(REG_CMD_WRITE,CmdFifoWp);
	
	return FT_GE_OK;
}    


//flushes out all the commands to FT_GC and waits for the completion of execution
FT_GEStatus FT_GC_Finish(void)
{
	uint16_t ReadPrt;
	
	if(TrnsFlag)
	{
		FT_GC_EndTransferCmd();
	}
	FT_Transport_SPI_Write32(REG_CMD_WRITE,CmdFifoWp);
//    FT_Transport_SPI_Write(REG_DLSWAP,FT_DLSWAP_FRAME);
	
	while((ReadPrt = FT_Transport_SPI_Read16(REG_CMD_READ)) != CmdFifoWp)
	{
		if(FT_COPRO_ERROR == ReadPrt)
		{
			return FT_GE_ERROR;
		}
	}	
	return FT_GE_OK;
}    


//checks fifo and returns the status	
FT_GEStatus FT_GC_CheckFinish(void)
{
	uint16_t ReadPrt = FT_Transport_SPI_Read16(REG_CMD_READ);
	

	if (FT_COPRO_ERROR == ReadPrt)
	{
		return FT_GE_ERROR;
	}
	else if(ReadPrt != CmdFifoWp)
	{
		return FT_GE_BUSY;
	}	//success case return finished
	return FT_GE_FINISHED;
}    


//error from graphics controller library
uint32_t FT_GC_GetError(void)
{
	if(FT_COPRO_ERROR == FT_Transport_SPI_Read16(REG_CMD_READ))
	{
		return FT_GE_ERROR;
	}
	return FT_GE_OK;	
}

/* [] END OF FILE */
