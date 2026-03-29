#include "touchscreen.h"
#include "GC.h"
#include "SPI.h"
#include "bridge.h"
#include "portx.h"
#include <GD23Z.h>


static const char* TAG = "GC";


#define INT_PIN 0
#define SPI_CS_PIN 10
#define SPI_CLK_PIN 6
#define SPI_MISO_PIN 2
#define SPI_MOSI_PIN 7


void PDNPin_Write(int what)
{
	//portx_write(PORTX_FT81X_PD, what);
}


static void init_PDN_pin()
{
	LOGD(TAG, "%s()\r\n", __FUNCTION__); Serial.flush();
	pinMode(INT_PIN, INPUT_PULLUP);
	PDNPin_Write(1);
}


void GC_SPI_Init()
{
	extern void FTCS_Write(uint8_t d);

	LOGD(TAG, "%s()\r\n", __FUNCTION__); Serial.flush();

    pinMode(SPI_CS_PIN, OUTPUT);
    FTCS_Write(1);
    SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_CS_PIN);
}


void GC_SPI_Exit()
{
	/* close the spi channel */
}


FT_Status GC_InitA(  uint8_t ResType)
{
	LOGD(TAG, "%s()\r\n", __FUNCTION__); Serial.flush();

	/* assign the pdn */
	init_PDN_pin();
	
	/* Initialize SPI channel */
	GC_SPI_Init();
	
	/* Bootup of graphics controller */
	GC_Reset();
	/* Set the display configurations followed by external clock set, spi clock change wrt FT80x */
	GC_DisplayConfigExternalClock(ResType);

	return FT_OK;
}

FT_Status GC_Init_Soft(  uint8_t ResType)
{
	LOGD(TAG, "%s()\r\n", __FUNCTION__); Serial.flush();

	/* assign the pdn */
	init_PDN_pin();    	
	
	/* Initialize SPI channel */
	GC_SPI_Init();
	
	/* Bootup of graphics controller */
	GC_Reset_Soft();
	/* Set the display configurations followed by external clock set, spi clock change wrt FT80x */
	GC_DisplayConfigExternalClock(ResType);

	return FT_OK;
}

/* Api to program the output display wrt display parameters */
FT_Status GC_InitB(uint16_t hperiod,uint16_t vperiod,uint16_t hfrontporch,uint16_t hbackporch,uint16_t hpulsewidth,
			   uint16_t vfrontporch,uint16_t vbackporch,uint16_t vpulsewidth,
			   uint8_t polarity,uint8_t swizzle,uint8_t fps)
{
	/* based on the input arguments compute the scan out register values */
	LOGD(TAG, "%s()\r\n", __FUNCTION__); Serial.flush();

	return FT_OK;
}


void GC_ChangeClock(uint32_t ClockValue)
{
    static uint32_t lastDivider = -1; 
    uint32_t clkDivider = (8 * 1000000 / ClockValue) - 1;
    
    if((clkDivider != lastDivider) && (clkDivider < 0xFFFF))
    {
        // Clock_1_SetDividerRegister(clkDivider, 1);  // TODO: default SPI library settings are adequate for now
        lastDivider = clkDivider;
    } 
}


/* Exit SPI channel */			   
FT_Status GC_Exit(void)
{
	/* Cross check whether SPI channel is used by others modules */
	GC_SPI_Exit();
	return FT_OK;
}


/* Api to get version of the library */
/* Good to have compile date as well */

void GC_GetVersion(uint8_t *Major, uint8_t *Minor, uint8_t *Build)
{
	/* Update the version numbers */
	*Major = GC_MAJOR;
	*Minor = GC_MINOR;
	*Build = GC_BUILD;
}
//GC gpio bit for display enable/disable

void GC_SetDisplayEnablePin(uint8_t GpioBit)
{
	LOGD(TAG, "%s(GpioBit=%d)...\r\n", __FUNCTION__, GpioBit); Serial.flush();

	/* update the display enable pin gpio bit number */
	DispGpioPin = GpioBit;
	
	/* set the direction of this bit to output. 1 is output and 0 is input */
	GD.wr(REG_GPIO_DIR,(1 << DispGpioPin) | GD.rd(REG_GPIO_DIR));
}

void GC_SetAudioEnablePin(uint8_t GpioBit)
{
	//GC gpio bit for audio enable/disable
	/* update the audio enable pin gpio bit number */
	AudioGpioPin = GpioBit;
	/* set the direction of this bit to output. 1 is output and 0 is input */
	GD.wr(REG_GPIO_DIR,(1 << AudioGpioPin) | GD.rd(REG_GPIO_DIR));
}


/* Apis to enable/disable backlight */
void GC_DisplayOn(void)
{
	//LOGD(TAG, "%s(): GpioBit=%d...\r\n", __FUNCTION__, DispGpioPin); Serial.flush();
	
	/* switch on the display, 1 means enable and 0 means disable */
	GD.wr(REG_GPIO,(1 << DispGpioPin) | GD.rd(REG_GPIO));
}//

void GC_DisplayOff(void)
{
	//LOGD(TAG, "%s(): DispGpioPin=%d...\r\n", __FUNCTION__, DispGpioPin); Serial.flush();

	/* switch off the display */
	GD.wr(REG_GPIO,(~(1 << DispGpioPin)) & GD.rd(REG_GPIO));
}

void GC_SetGpioDrive(uint8_t port, uint8_t level)// see datasheet 4.5.4 (0 for int and miso, 2 for gpios, 1 for everything else)
{
    uint8_t clearBitm;
    
	//LOGV(TAG, "%s(port=%d, level=%d)...\r\n", __FUNCTION__, port,level); Serial.flush();

    if (level > 3)
        level = 3;
    
    if (port == 1 && level > 1)
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
    GD.wr(REG_GPIO,(~(clearBitm << port)) & GD.rd(REG_GPIO));
	GD.wr(REG_GPIO,(level << port) | GD.rd(REG_GPIO));
}

//apis to set interrupt pin
void GC_SetInterruptPin(uint16_t Intpin)
{
	/* update the interrupt pin */
	IntPin = Intpin;
}

void GC_AudioOn(void)
{
	/* switch on the audio , 1 means enable and 0 means disable*/
	GD.wr(REG_GPIO,(1 << AudioGpioPin) | GD.rd(REG_GPIO));
}

void GC_AudioOff(void)
{
	/* switch off the audio , 1 means enable and 0 means disable*/
	GD.wr(REG_GPIO,(~(1 << AudioGpioPin)) & GD.rd(REG_GPIO));
}


//api to reset coprocessor only - do this only when coprocessor returns error. for graphic processor error, utilize reset() api
void GC_ResetCopro(void)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();

	/* first set the reset bit high */
	GD.wr(REG_CPURESET,FT_RESET_HOLD_COPROCESSOR);//first hold the coprocessor in reset
	GD.wr16(REG_CMD_READ,0);
	GD.wr16(REG_CMD_WRITE,0);
	CyDelay(10);//just to make sure reset is fine
	/* release the coprocessors from reset */
	GD.wr(REG_CPURESET,FT_RESET_RELEASE_COPROCESSOR);
	/* ideally CyDelay of 25ms is required for audio engine to playback mute sound to avoid pop sound */
}

//api to reset whole GC via pdn - if pdn is not assigned then reset is not successful
void GC_Reset(void)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();

	/* Reset of whole graphics controller */
	GC_PDN_Cycle();
	/* send active command to enable SPI, followed by download of default DL into graphics engine, followed by SPI settings wrt internal clock requirements during bootup stage */
	GC_ActiveInternalClock();
}

void GC_Reset_Soft(void)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();

	/* Reset of whole graphics controller */
	// GC_PDN_Cycle();
	/* send active command to enable SPI, followed by download of default DL into graphics engine, followed by SPI settings wrt internal clock requirements during bootup stage */
	GC_ActiveInternalClock();
}

/* API to do power down cycle based on PDN gpio pin */
void GC_PDN_Cycle(void)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();

	/* do a power cycle by toggling power down pin followed by active command followed by display list initialization sequence */
	PDNPin_Write(1); CyDelay(20);
	PDNPin_Write(0); CyDelay(20);
	PDNPin_Write(1); CyDelay(20);
}

/* API to set active command, set internal clock and download first DL */
void GC_ActiveInternalClock(void)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();
	uint8_t FT_DLCODE_BOOTUP[12] =
	{
	  0,0,0,2,  //GPU instruction CLEAR_COLOR_RGB - black color
	  7,0,0,38, //GPU instruction CLEAR
	  0,0,0,0,  //GPU instruction DISPLAY
	};
	/* change the SPI clock to <11MHz */
	GC_ChangeClock(FT_SPI_CLK_FREQ);
	GC_HostCommand(FT_ACTIVE);//wake up the processor from sleep state
	CyDelay(20);
	/* download the first display list */
	GD.wr_n(FT_RAM_DL,FT_DLCODE_BOOTUP,12);
	/* perform first swap command */
	GD.wr(REG_DLSWAP,FT_DLSWAP_FRAME);
}

/* API to configure the display and set to external clock */
void GC_DisplayConfigExternalClock(uint8_t ResType)
{
	LOGD(TAG, "%s()...\r\n", __FUNCTION__); Serial.flush();

	GD.wr16(REG_HSIZE,			480);
	GD.wr16(REG_HCYCLE, 		548);
	GD.wr16(REG_HOFFSET, 		43);
	GD.wr16(REG_HSYNC0, 		0);
	GD.wr16(REG_HSYNC1, 		41);

	GD.wr16(REG_VSIZE, 		272);
	GD.wr16(REG_VCYCLE, 		292);
	GD.wr16(REG_VOFFSET, 		12);
	GD.wr16(REG_VSYNC0, 		0);
	GD.wr16(REG_VSYNC1, 		10);
		 		
	GD.wr16(REG_PCLK,			5); // LCE-GRH: changed from 3 to 5
	GD.wr16(REG_SWIZZLE, 		0);
	GD.wr16(REG_PCLK_POL, 		1);
	GD.wr16(REG_CSPREAD, 		1); // LCE-GRH: added
	GD.wr16(REG_DITHER, 		1); // LCE_GRH: added
	/* after configuring display parameters, configure pclk */

	/* send host command to change the clock source from internal to external */
	GC_HostCommand(FT_CLKEXT);
	/* change the clock to maximum spi operating frequency */
//	GC_ChangeClock(FT_SPI_CLK_FREQ);//change the clock to normal operating frequency - harcoded wrt due
}

/* Apis related to graphics processor */	
//api to enable or disable interrupts
void GC_EnableInterrupts(uint8_t GEnable,uint8_t Mask)
{
	LOGD(TAG, "%s(GEnable=%x, Mask=%x)...\r\n", __FUNCTION__, GEnable, Mask); Serial.flush();

	GD.wr(REG_INT_EN,GEnable);//1 means enable global interrupts, 0 means disable global interrupts
	GD.wr(REG_INT_MASK,Mask);//0 means interrupts are masked, 1 means interrupts are not masked
}

//read the interrupt flag register - note that on ft800/ft801 the interrupts are clear by read
uint8_t GC_ReadIntReg(void)
{
	return (GD.rd(REG_INT_FLAGS));
}

//void AssignCBFunptr(void *CBFunPtr){}//api to register callback function from application using interrupt mechanism	

/* APIs related to graphics engine */
FT_GEStatus GC_AlphaFunc(uint8_t Func, uint8_t Ref) 
{
  return ( GC_WriteCmd((9UL << 24) | ((Func & 7L) << 8) | ((Ref & 0xFFL) << 0)) );
}

FT_GEStatus GC_Begin(uint8_t Prim) 
{
  return ( GC_WriteCmd((31UL << 24) | Prim) );
}

FT_GEStatus GC_BitmapHandle(uint8_t Handle)
{
  return ( GC_WriteCmd((5UL << 24) | Handle) );
}

FT_GEStatus GC_BitmapLayout(uint8_t Format, uint16_t Linestride, uint16_t Height) 
{
  // GC_WriteCmd((7UL << 24) | ((format & 0x1FL) << 19) | ((linestride & 0x3FFL) << 9) | ((height & 0x1FFL) << 0));
  union Data32 data;
  data.A[0] = Height;
  data.A[1] = (1 & (Height >> 8)) | (Linestride << 1);
  data.A[2] = (7 & (Linestride >> 7)) | (Format << 3);
  data.A[3] = 7;
  return ( GC_WriteCmd(data.UUint32) );
}

FT_GEStatus GC_BitmapSize(uint8_t filter, uint8_t wrapx, uint8_t wrapy, uint16_t width, uint16_t height) 
{
  uint8_t fxy = (filter << 2) | (wrapx << 1) | (wrapy);
  // GC_WriteCmd((8UL << 24) | ((uint32_t)fxy << 18) | ((width & 0x1FFL) << 9) | ((height & 0x1FFL) << 0));
  union Data32 data;
  data.A[0] = height;
  data.A[1] = (1 & (height >> 8)) | (width << 1);
  data.A[2] = (3 & (width >> 7)) | (fxy << 2);
  data.A[3] = 8;
  return ( GC_WriteCmd(data.UUint32) );
}

FT_GEStatus GC_BitmapSource(uint32_t Addr) 
{
  return ( GC_WriteCmd((1UL << 24) | ((Addr & 0xFFFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformA(int32_t A) 
{
  return ( GC_WriteCmd((21UL << 24) | ((A & 0x1FFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformB(int32_t B) 
{
  return ( GC_WriteCmd((22UL << 24) | ((B & 0x1FFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformC(int32_t C) 
{
  return ( GC_WriteCmd((23UL << 24) | ((C & 0xFFFFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformD(int32_t D) 
{
  return ( GC_WriteCmd((24UL << 24) | ((D & 0x1FFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformE(int32_t E) 
{
  return ( GC_WriteCmd((25UL << 24) | ((E & 0x1FFFFL) << 0)) );
}

FT_GEStatus GC_BitmapTransformF(int32_t F) 
{
  return ( GC_WriteCmd((26UL << 24) | ((F & 0xFFFFFFL) << 0)) );
}

FT_GEStatus GC_BlendFunc(uint8_t Src, uint8_t Dst) 
{
  return ( GC_WriteCmd((11UL << 24) | ((Src & 7L) << 3) | ((Dst & 7L) << 0)) );
}

FT_GEStatus GC_Call(uint16_t Dest) 
{
  return ( GC_WriteCmd((29UL << 24) | ((Dest & 0xFFFFL) << 0)) );
}

FT_GEStatus GC_Cell(uint8_t Cell) 
{
  return ( GC_WriteCmd((6UL << 24) | ((Cell & 0x7FL) << 0)) );
}

FT_GEStatus GC_ClearColorA(uint8_t Alpha) 
{
  return ( GC_WriteCmd((15UL << 24) | ((Alpha & 0xFFL) << 0)) );
}

FT_GEStatus GC_ClearColorRGB(uint8_t red, uint8_t green, uint8_t blue) 
{
  return ( GC_WriteCmd((2UL << 24) | ((red & 0xFFL) << 16) | ((green & 0xFFL) << 8) | ((blue & 0xFFL) << 0)) );
}

FT_GEStatus GC_ClearColorRGB_int(uint32_t rgb) 
{
  return ( GC_WriteCmd((2UL << 24) | (rgb & 0xFFFFFFL)) );
}    

FT_GEStatus GC_ClearCST(uint8_t c, uint8_t s, uint8_t t) 
{
  uint8_t m = (c << 2) | (s << 1) | t;
  return ( GC_WriteCmd((38UL << 24) | m) );
}    

FT_GEStatus GC_Clear(void) 
{
  return ( GC_WriteCmd((38UL << 24) | 7) );
}    

FT_GEStatus GC_ClearStencil(uint8_t s) 
{
  return ( GC_WriteCmd((17UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus GC_ClearTag(uint8_t s) 
{
  return ( GC_WriteCmd((18UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus GC_ColorA(uint8_t Alpha) 
{
  return ( GC_WriteCmd((16UL << 24) | ((Alpha & 0xFFL) << 0)) );
}    

FT_GEStatus GC_ColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a) 
{
  return ( GC_WriteCmd((32UL << 24) | ((r & 1L) << 3) | ((g & 1L) << 2) | ((b & 1L) << 1) | ((a & 1L) << 0)) );
}    

FT_GEStatus GC_ColorRGB(uint8_t red, uint8_t green, uint8_t blue) {
  // GC_WriteCmd((4UL << 24) | ((red & 0xFFL) << 16) | ((green & 0xFFL) << 8) | ((blue & 0xFFL) << 0)) );
  union Data32 data;
  data.A[0] = blue;
  data.A[1] = green;
  data.A[2] = red;
  data.A[3] = 4;
  return ( GC_WriteCmd(data.UUint32) );
}    

FT_GEStatus GC_Display(void) 
{
  return ( GC_WriteCmd((0UL << 24)) );
}    

FT_GEStatus GC_End(void) {
  return ( GC_WriteCmd((33UL << 24)) );
}    

FT_GEStatus GC_Jump(uint16_t Dest) 
{
  return ( GC_WriteCmd((30UL << 24) | ((Dest & 0x7FFL) << 0)) );
}    

FT_GEStatus GC_LineWidth(uint16_t Width) 
{
  return ( GC_WriteCmd((14UL << 24) | ((Width & 0xFFFL) << 0)) );
}    

FT_GEStatus GC_Macro(uint8_t m) 
{
  return ( GC_WriteCmd((37UL << 24) | ((m & 1L) << 0)) );
}   

FT_GEStatus GC_PointSize(uint16_t Size) 
{
  return ( GC_WriteCmd((13UL << 24) | ((Size & 0x1FFFL) << 0)) );
}    

FT_GEStatus GC_RestoreContext(void) 
{
  return ( GC_WriteCmd((35UL << 24)) );
}    

FT_GEStatus GC_Return(void) 
{
  return ( GC_WriteCmd((36UL << 24)) );
}    

FT_GEStatus GC_SaveContext(void) 
{
  return ( GC_WriteCmd((34UL << 24)) );
}    

FT_GEStatus GC_ScissorSize(uint16_t Width, uint16_t Height) 
{
  return ( GC_WriteCmd((28UL << 24) | ((Width & 0x3FFL) << 10) | ((Height & 0x3FFL) << 0)) );
}    

FT_GEStatus GC_ScissorXY(uint16_t x, uint16_t y)
{
  return ( GC_WriteCmd((27UL << 24) | ((x & 0x1FFL) << 9) | ((y & 0x1FFL) << 0)) );
}    

FT_GEStatus GC_StencilFunc(uint8_t Func, uint8_t Ref, uint8_t Mask) 
{
  return ( GC_WriteCmd((10UL << 24) | ((Func & 7L) << 16) | ((Ref & 0xFFL) << 8) | ((Mask & 0xFFL) << 0)) );
}    

FT_GEStatus GC_StencilMask(uint8_t Mask) 
{
  return ( GC_WriteCmd((19UL << 24) | ((Mask & 0xFFL) << 0)) );
}    

FT_GEStatus GC_StencilOp(uint8_t Sfail, uint8_t Spass) 
{
  return ( GC_WriteCmd((12UL << 24) | ((Sfail & 7L) << 3) | ((Spass & 7L) << 0)) );
}   

FT_GEStatus GC_TagMask(uint8_t Mask) 
{
  return ( GC_WriteCmd((20UL << 24) | ((Mask & 1L) << 0)) );
}    

FT_GEStatus GC_Tag(uint8_t s) 
{
  return ( GC_WriteCmd((3UL << 24) | ((s & 0xFFL) << 0)) );
}    

FT_GEStatus GC_Vertex2f(int16_t x, int16_t y) 
{
  // x = int(16 * x);
  // y = int(16 * y);
  return ( GC_WriteCmd((1UL << 30) | ((x & 0x7FFFL) << 15) | ((y & 0x7FFFL) << 0)) );
}    

FT_GEStatus GC_Vertex2ii(uint16_t x, uint16_t y, uint8_t Handle, uint8_t Cell) 
{
  //return (GC_WriteCmd((2UL << 30) | ((x & 0x1FFL) << 21) | ((y & 0x1FFL) << 12) | ((handle & 0x1FL) << 7) | ((cell & 0x7FL) << 0)) ));
   /* Generates invalid graphics instruction due to shift operation of -ve coefficient */
  x &= 0x1FFL;//error handling of -ve coefficients
  y &= 0x1FFL;
  union Data32 data;
  data.A[0] = Cell | ((Handle & 1) << 7);
  data.A[1] = (Handle >> 1) | (y << 4);
  data.A[2] = (y >> 4) | (x << 5);
  data.A[3] = (2 << 6) | (x >> 3);
  return ( GC_WriteCmd(data.UUint32) );
}    


/* graphics helper apis */
FT_GEStatus GC_ColorRGB_int(uint32_t rgb) 
{
  return ( GC_WriteCmd((4UL << 24) | (rgb & 0xFFFFFFL)) );
}    

/* Form two commands, one for rgb and the other for a */
FT_GEStatus GC_ColorARGB(uint32_t argb) 
{
	FT_GEStatus Status;
	GC_WriteCmd((4UL << 24) | (argb & 0xFFFFFFL));
	Status = GC_WriteCmd((16UL << 24) | ((argb>>24) & 0xFFL) );
  
  return Status;
}   



FT_GEStatus GC_Cmd_Logo(void)
{
	FT_GEStatus Status;	
	Status = GC_WriteCmd(CMD_LOGO);	
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();

	return Status;
}    


FT_GEStatus GC_Cmd_Append(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	GC_WriteCmd(CMD_APPEND);
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Num);//checking only for the last command
	
	return Status;
}    

FT_GEStatus GC_Cmd_BGColor(uint32_t c)
{
	FT_GEStatus Status;
	GC_WriteCmd(CMD_BGCOLOR);
	Status = GC_WriteCmd(c);
	
	return Status;
}    


/* API for touch transform calculation and set the transform registers */
/*FT_GEStatus GC_Cmd_Touch_Transform(int32_t x0,int32_t y0,int32_t x1,int32_t y1,int32_t x2,int32_t y2,int32_t tx0,int32_t ty0,int32_t tx1,int32_t ty1,int32_t tx2,int32_t ty2,uint32_t Result)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_BITMAP_TRANSFORM);
	GC_WriteCmd(x0);
	GC_WriteCmd(y0);
	GC_WriteCmd(x1);
	GC_WriteCmd(y1);
	GC_WriteCmd(x2);
	GC_WriteCmd(y2);
	GC_WriteCmd(tx0);
	GC_WriteCmd(ty0);
	GC_WriteCmd(tx1);
	GC_WriteCmd(ty1);
	GC_WriteCmd(tx2);
	GC_WriteCmd(ty2);
	Status = GC_WriteCmd(Result);

	return Status;
}
*/
/* make sure the string is in ram instead of program memory. it is assumed that *s is valid string and if null then it should contain \0 */
FT_GEStatus GC_Cmd_Button(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_BUTTON);
	GC_WriteCmd(((y & 0xFFFFL) <<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);//make sure last byte is added into the
	
	if (Status) LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	return Status;
}    


/* Check the result of command calibrate by cmd_GetResult */
FT_GEStatus GC_Cmd_Calibrate(uint32_t Result)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_CALIBRATE);
	Status = GC_WriteCmd(Result);//write extra word for result
	
	LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;
}    

FT_GEStatus GC_Cmd_Clock(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_CLOCK);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	GC_WriteCmd(((m & 0xFFFFL)<<16) | (h & 0xFFFFL));
	Status = GC_WriteCmd(((ms & 0xFFFFL)<<16) | (s & 0xFFFFL));
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;
}    


FT_GEStatus GC_Cmd_ColdStart(void)
{
	//should we even change command read and write pointers
	return ( GC_WriteCmd(CMD_COLDSTART) );
}   

FT_GEStatus GC_Cmd_Dial(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Val)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_DIAL);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	Status = GC_WriteCmd(Val);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;
}    

FT_GEStatus GC_Cmd_DLStart(void)
{
	return ( GC_WriteCmd(CMD_DLSTART) );
}    

FT_GEStatus GC_Cmd_FGColor(uint32_t c)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_FGCOLOR);
	Status = GC_WriteCmd(c);
	LOGV(TAG, "%s(c=%x): status=%x\r\n", __FUNCTION__, c, Status); Serial.flush();
	
	return Status;	
}    


FT_GEStatus GC_Cmd_Gauge(int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Major, uint16_t Minor, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GAUGE);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (r & 0xFFFFL));
	GC_WriteCmd(((Minor & 0xFFFFL)<<16) | (Major & 0xFFFFL));
	Status = GC_WriteCmd(((Range & 0xFFFFL)<<16) | (Val & 0xFFFFL));
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

/* Results are available from getresults api */
FT_GEStatus GC_Cmd_GetMatrix(void)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GETMATRIX);
	GC_WriteCmd(0);
	GC_WriteCmd(0);
	GC_WriteCmd(0);
	GC_WriteCmd(0);
	GC_WriteCmd(0);
	Status = GC_WriteCmd(0);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

/* perform this api and wait for the completion by finish and use getresults api  for the result*/
FT_GEStatus GC_Cmd_GetProps(uint32_t *Ptr, uint32_t *w, uint32_t *h)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GETPROPS);
	GC_WriteCmd(0);
	GC_WriteCmd(0);
	Status = GC_WriteCmd(0);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

//perform this api and wait for the completion by finish and use cmd_getresult for the result
FT_GEStatus GC_Cmd_GetPtr(uint32_t Result)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GETPTR);
	Status = GC_WriteCmd(Result);
	
	return Status;	
}    

FT_GEStatus GC_Cmd_GradColor(uint32_t c)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GRADCOLOR);
	Status = GC_WriteCmd(c);
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_GRADIENT);
	GC_WriteCmd(((y0 & 0xFFFFL)<<16)|(x0 & 0xFFFFL));
	GC_WriteCmd(rgb0);
	GC_WriteCmd(((y1 & 0xFFFFL)<<16)|(x1 & 0xFFFFL));
	Status = GC_WriteCmd(rgb1);
	
	return Status;	
}    

//after calling this api copy the raw content which is output from deflate
FT_GEStatus GC_Cmd_Inflate(uint32_t Ptr)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_INFLATE);
	Status = GC_WriteCmd(Ptr);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Interrupt(uint32_t ms)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_INTERRUPT);
	Status = GC_WriteCmd(ms);
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Keys(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_KEYS);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);	
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_LoadIdentity(void)
{
	return ( GC_WriteCmd(CMD_LOADIDENTITY) );
}    

//after this api, copy the jpeg data into fifo
FT_GEStatus GC_Cmd_LoadImage(uint32_t Ptr, int32_t Options)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_LOADIMAGE);
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Options);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Memcpy(uint32_t Dest, uint32_t Src, uint32_t Num)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_MEMCPY);
	GC_WriteCmd(Dest);
	GC_WriteCmd(Src);
	Status = GC_WriteCmd(Num);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Memset(uint32_t Ptr, uint8_t Value, uint32_t Num)
{
	FT_GEStatus Status;
	
	LOGV(TAG, "%s(Ptr=0x%x, Value=0x%x, Num=0x%x)\r\n", __FUNCTION__, Ptr, Value, Num);
	GC_WriteCmd(CMD_MEMSET);
	GC_WriteCmd(Ptr);
	GC_WriteCmd(Value);	
	Status = GC_WriteCmd(Num);
	
	return Status;	
}    

//perform this api, wait for the completion and use cmd_getresult for the result
FT_GEStatus GC_Cmd_Memcrc(uint32_t Ptr, uint32_t Num,uint32_t *Result)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_MEMCRC);
	GC_WriteCmd(Ptr);
	GC_WriteCmd(Num);
	Status = GC_WriteCmd(*Result);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Memwrite(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_MEMWRITE);
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Num);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    


FT_GEStatus GC_Cmd_Memzero(uint32_t Ptr, uint32_t Num)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_MEMZERO);
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Num);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();

	return Status;	
}    


FT_GEStatus GC_Cmd_Number(int16_t x, int16_t y, uint8_t Font, uint16_t Options, uint32_t n)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_NUMBER);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = GC_WriteCmd(n);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}   

FT_GEStatus GC_Cmd_Progress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_PROGRESS);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = GC_WriteCmd(Range);
	LOGV(TAG, "%s(): status=%x\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

//perform this api, wait for the completion and use cmd_getresult for the result
FT_GEStatus GC_Cmd_RegRead(uint32_t Ptr,uint32_t Result)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_REGREAD);
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Result);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Rotate(int32_t a)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_ROTATE);
	Status = GC_WriteCmd(a);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Scale(int32_t sx, int32_t sy)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SCALE);
	GC_WriteCmd(sx);
	Status = GC_WriteCmd(sy);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_ScreenSaver(void)
{
	return ( GC_WriteCmd(CMD_SCREENSAVER) );
}    

FT_GEStatus GC_Cmd_Scrollbar(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Size, uint16_t Range)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SCROLLBAR);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = GC_WriteCmd(((Range & 0xFFFFL)<<16) | (Size & 0xFFFFL));
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

//make sure ptr is pointing to table and in turn table has pointer to the actual bitmap data
FT_GEStatus GC_Cmd_SetFont(uint8_t Font, uint32_t Ptr)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SETFONT);
	GC_WriteCmd(Font);
	Status = GC_WriteCmd(Ptr);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_SetMatrix(void)
{
	return ( GC_WriteCmd(CMD_SETMATRIX) );	
}    


FT_GEStatus GC_Cmd_Sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t Ptr, uint16_t Format)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SKETCH);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(Ptr);
	Status = GC_WriteCmd(Format);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Slider(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t Options, uint16_t Val, uint16_t Range)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SLIDER);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((Val & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = GC_WriteCmd(Range);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

//perform this api and wait for the completion
FT_GEStatus GC_Cmd_Snapshot(uint32_t OutputAddr)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SNAPSHOT);
	Status = GC_WriteCmd(OutputAddr);
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

//note that macro 0 and macro 1 are modified by the coprocessor when performing this function
FT_GEStatus GC_Cmd_Spinner(int16_t x, int16_t y, uint8_t Style, uint8_t Scale)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_SPINNER);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	Status = GC_WriteCmd(((Scale & 0xFFFFL)<<16) | (Style & 0xFFFFL));
	LOGV(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	
	return Status;	
}    

FT_GEStatus GC_Cmd_Stop(void)
{
	return ( GC_WriteCmd(CMD_STOP) );
}    

FT_GEStatus GC_Cmd_Swap(void)
{
	return ( GC_WriteCmd(CMD_SWAP) );
}    

FT_GEStatus GC_Cmd_Text(int16_t x, int16_t y, uint8_t Font, uint16_t Options, const char *s)
{
	FT_GEStatus Status;
	GC_WriteCmd(CMD_TEXT);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Options & 0xFFFFL)<<16) | (Font & 0xFFFFL));
	Status = GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);
	
	if (Status) LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	return Status;	
}    

FT_GEStatus GC_Cmd_Toggle(int16_t x, int16_t y, int16_t w, uint8_t Font, uint16_t Options, uint16_t State, const char *s)
{
	FT_GEStatus Status;
	GC_WriteCmd(CMD_TOGGLE);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((Font & 0xFFFFL)<<16) | (w & 0xFFFFL));
	GC_WriteCmd(((State & 0xFFFFL)<<16) | (Options & 0xFFFFL));
	Status = GC_WriteCmd_Src((uint8_t *)s,strlen((const char *)s) + 1);
	
	if (Status) LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	return Status;	
}    

FT_GEStatus GC_Cmd_Track(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Tag)
{
	FT_GEStatus Status;
	GC_WriteCmd(CMD_TRACK);
	GC_WriteCmd(((y & 0xFFFFL)<<16) | (x & 0xFFFFL));
	GC_WriteCmd(((h & 0xFFFFL)<<16) | (w & 0xFFFFL));
	Status = GC_WriteCmd(Tag);
	
	if (Status) LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	return Status;	
}    

FT_GEStatus GC_Cmd_Translate(int32_t tx, int32_t ty)
{
	FT_GEStatus Status;
	
	GC_WriteCmd(CMD_TRANSLATE);
	GC_WriteCmd(tx);
	Status = GC_WriteCmd(ty);
	
	if (Status) LOGD(TAG, "%s(): status=%x\r\n", __FUNCTION__, Status); Serial.flush();
	return Status;	
}    


/* Apis related to audio engine */
FT_AEStatus GC_PlaySound_Volume(uint8_t Volume,uint16_t SoundNote)
{
	GD.wr(REG_VOL_SOUND,Volume);//change the volume of synthesized sound, 0 means off, 255 means max on
	GD.wr16(REG_SOUND,SoundNote);
	GD.wr(REG_PLAY,FT_SOUND_PLAY);
	
	return FT_AE_OK;
}    
//higher byte is the note and lower byte is the sound
FT_AEStatus GC_PlaySound(uint16_t SoundNote)
{
	GD.wr16(REG_SOUND,SoundNote);
	GD.wr(REG_PLAY,FT_SOUND_PLAY);
	return FT_AE_OK;
}    

//volume will not be modified
void GC_StopSound(void)
{
	GD.wr16(REG_SOUND,FT_SILENCE);//configure silence
	GD.wr(REG_PLAY,FT_SOUND_PLAY);//play the silence
}    

void GC_SetSoundVolume(uint8_t Volume)
{
	GD.wr(REG_VOL_SOUND,Volume);
}    

uint8_t GC_GetSoundVolume(void)
{
	return GD.rd(REG_VOL_SOUND);
}    

//one shot or continuous, sampling frequency is from 8k to 48k
FT_AEStatus GC_PlayAudio(uint8_t Volume,uint8_t Format,uint16_t SamplingFreq,uint32_t BufferAddr,uint32_t BufferSize,uint8_t Loop)
{
	if((SamplingFreq*1L < FT_AUDIO_SAMPLINGFREQ_MIN*1L) | (SamplingFreq*1L > FT_AUDIO_SAMPLINGFREQ_MAX*1L))
	{
		return FT_AE_ERROR_SAMPLINGFREQ_OUTOFRANGE;
	}
	if(Format > FT_ADPCM_SAMPLES)
	{
		return FT_AE_ERROR_FORMAT;
	}
	GD.wr(REG_VOL_PB,Volume);
	GD.wr32(REG_PLAYBACK_START,BufferAddr);
	GD.wr32(REG_PLAYBACK_LENGTH,BufferSize);
	GD.wr32(REG_PLAYBACK_FREQ,SamplingFreq);
	GD.wr(REG_PLAYBACK_FORMAT,Format);
	GD.wr(REG_PLAYBACK_LOOP,Loop);//0 means one shot and 1 means loop
	
	GD.wr(REG_PLAYBACK_PLAY,FT_AUDIO_PLAY);

	return FT_AE_OK;
}    

void GC_SetAudioVolume(uint8_t Volume)
{
	GD.wr(REG_VOL_PB,Volume);
}    

//returns playback stopped or continue
FT_AEStatus GC_GetAudioStats(uint32_t *CurrPlayAddr)
{
	*CurrPlayAddr = GD.rd32(REG_PLAYBACK_READPTR);
	
	//in case of loop, check for the playback status
	if(0 == GD.rd(REG_PLAYBACK_PLAY))
	{
		return FT_AE_PLAYBACK_STOPPED;
	}

	return FT_AE_PLAYBACK_CONTINUE;
}    

uint8_t GC_GetAudioVolume(void)
{
	return  (GD.rd(REG_VOL_PB));
}    

//volume will not be modified
void GC_StopAudio(void)
{
	//configure audio with length o and play
	GD.wr32(REG_PLAYBACK_LENGTH,0);
	GD.wr(REG_PLAYBACK_LOOP,0);//0 means one shot and 1 means loop
	
	GD.wr(REG_PLAYBACK_PLAY,FT_AUDIO_PLAY);	
}    


/* Apis related to touch engine */
//one of 0ff/oneshot/frame/continuous. default being continuous
void GC_SetTouchMode(uint8_t TMode)
{
	Serial.printf("%s(mode=0x%x)\r\n", __FUNCTION__, TMode);
	GD.wr(REG_CTOUCH_MODE,TMode);
}    

void GC_SetTouchAdcMode(uint8_t TAMode)
{
	Serial.printf("%s(mode=0x%x) (REG_CTOUCH_EXTENDED)\r\n", __FUNCTION__, TAMode);
	GD.wr(REG_CTOUCH_EXTENDED,TAMode);
} 

void GC_SetTouchCharge(uint16_t TChrg)
{
	Serial.printf("%s(charge=0x%x) (COMMENTED OUT!)\r\n", __FUNCTION__, TChrg);
	//GD.wr16(REG_TOUCH_CHARGE,TChrg);
} 

void GC_SetTouchSettle(uint8_t TSet)
{
	Serial.printf("%s(set=0x%x) (COMMENTED OUT!)\r\n", __FUNCTION__, TSet);
	//GD.wr(REG_TOUCH_SETTLE,TSet);
} 

void GC_SetTouchOversample(uint8_t TSam)
{
	Serial.printf("%s(sam=0x%x) (COMMENTED OUT!)\r\n", __FUNCTION__, TSam);
	//GD.wr(REG_TOUCH_OVERSAMPLE,TSam);
} 

void GC_SetTouchRzThresh(uint16_t TRThs)
{
	Serial.printf("%s(thresh=0x%x) (COMMENTED OUT!)\r\n", __FUNCTION__, TRThs);
	//GD.wr16(REG_TOUCH_RZTHRESH,TRThs);
} 



//api to set coordinates for host specific tag query
void GC_SetHostTagXY(uint16_t xoffset,uint16_t yoffset)
{
	uint8_t A[6];
	//little endian specific
	A[0] = xoffset & 0xFF;
	A[1] = xoffset >> 8;
	A[4] = yoffset & 0xFF;
	A[5] = yoffset >> 8;
	GD.wr_n(REG_TAG_X,A,6);
}    

//api to get TAG from GC for coordinates set by  SetHostTagXY() api - host needs to wait for at least 1 frame to get these query values
uint8_t GC_GetHostTagXY(void)
{
	return GD.rd(REG_TAG);
}   

//get the touched object tag and repective xy coordinates
void GC_GetTagXY(sTagXY *sTagxy)
{
	GD.rd_n((uint8_t *)sTagxy, REG_CTOUCH_TAG_XY,6);
}    

//get the track value and the tag value
void GC_GetTrackTag(sTrackTag *sTracktag)
{
	uint32_t *ptr = (uint32_t *)sTracktag;
	*ptr = GD.rd32(REG_TRACKER);
}   

void GC_GetRawTouch(sTouchRaw *sTouchRaw)
{
    uint32_t regRead;
	regRead = GD.rd32(REG_TOUCH_DIRECT_XY);
    
    sTouchRaw->touch = (regRead >> 31) & 0x00000001;
    sTouchRaw->x = (regRead >> 16) & 0x000003FF;
    sTouchRaw->y = regRead & 0x000003FF;
}

uint32_t GC_GetTouchConfig()
{
	uint32_t config = GD.rd32(REG_TOUCH_CONFIG);
	Serial.printf("%s(): 0x%x\r\n", __FUNCTION__, config);
	return config;
}

/* APIS related to power modes */
void GC_HostCommand(uint32_t HostCommand)
{
    GD.hostcmd(HostCommand);
	CyDelay(20);//worst scenario  -- TODO: is this necessary?
}    
	

FT_GEStatus GC_WriteCmd(uint32_t Cmd)
{
	GD.cmd32(Cmd);
	return FT_GE_OK;
}

//api to send N bytes to command buffer, this api pads with 0's at the end 
FT_GEStatus GC_WriteCmd_Src(uint8_t *Src,uint32_t NBytes)
{
	GD.cmd_n(Src, NBytes);
	return FT_GE_OK;
}   


/* Special apis */
//apis for ease of usage in GC
//inserts cmd_dlstart() followed by clear(1,1,1) graphics command
void GC_DLStart(void)
{
	GC_Cmd_DLStart();
	GC_ClearCST(1,1,1);
}    


//inserts display() gpu instruction at the end and inserts cmd_swap() command
void GC_DLEnd(void)
{
	GC_Display();
	GC_Cmd_Swap();
}    


//special api to check logo completion
FT_GEStatus GC_CheckLogo(void)
{
	uint16_t ReadCmdPtr = GD.rd16(REG_CMD_READ) ;
	if((ReadCmdPtr == GD.rd16(REG_CMD_WRITE)) && (ReadCmdPtr == 0))
	{
		return FT_GE_FINISHED;
	}
	else if(FT_COPRO_ERROR == ReadCmdPtr)
	{
		LOGE(TAG, "%s(): FT_GE_ERROR!!!\r\n", __FUNCTION__);
		for (;;);
		return FT_GE_ERROR;
	}
	else
	{
		return FT_GE_BUSY;
	}
	
}   

/* [] END OF FILE */
