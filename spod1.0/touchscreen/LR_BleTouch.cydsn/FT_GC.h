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
* @file                           FT_GC.h
* @brief                          Contains FT graphics controller interface for arduino platform.
								  Tested platform version: Arduino 1.0.4 and later
* @version                        1.1.0
* @date                           2014/02/05
*
*/

#ifndef _FT_GC_H_
#define _FT_GC_H_
    
#include <project.h>
#include "FT_Transport_SPI.h"
#include "FT800.h"
#include "bridge.h"

/* Definitions used for debug. Uncomment the below to enable debug from graphics controller library */
#define FT_GCDEBUG0 255 //switchoff debug
#define FT_GCDEBUG1 1 //most critical debug information
#define FT_GCDEBUG2 2 //mid critical debug information
#define FT_GCDEBUG3 3 //least critical debug information

/* Change the below statement wrt debug criticality */
#define FT_GCDEBUG FT_GCDEBUG0

/* Version number of FT_GC */
#define FT_GC_MAJOR				1
#define FT_GC_MINOR				2
#define FT_GC_BUILD				0
    
    

/* FT_GC status enum - used for api return type, error type etc */
typedef enum FT_Status
{	
	/* Common enums */
	FT_OK = 0,
	FT_ERROR = 1,
	FT_WARNING = 2,
	FT_ERROR_INIT = 3,
	FT_ERROR_CHIPID = 4,
	
	/* Library related enums */
	FT_ERROR_NOPINASSIGNED = 50,
}FT_Status;

/* Audio coprocessor related enums */
typedef enum FT_AEStatus
{		
	FT_AE_OK = 0,	
	FT_AE_ERROR_FORMAT = 1,
	FT_AE_ERROR_SAMPLINGFREQ_OUTOFRANGE = 2,	//assert for boundary
	FT_AE_PLAYBACK_STOPPED = 3,
	FT_AE_PLAYBACK_CONTINUE = 4,
}FT_AEStatus;

/* Status enums for graphics engine */
typedef enum FT_GEStatus
{	
	FT_GE_OK = 0,	
	FT_GE_BUSY = 1,
	FT_GE_FINISHED = 2,
	
	/* Graphics related error enums */
	FT_GE_ERROR_INVALID_PRIMITIVE = 20,
	FT_GE_ERROR_INVALID_BITMAP_FORMAT = 21,
	FT_GE_ERROR_INVALID_BITMAP_HANDLENUM = 22,
	FT_GE_ERROR_VERTEX_OUTOFRANGE = 23,

	/* Coprocessor related enums */
	FT_GE_ERROR = 50,						//undefined error
	FT_GE_ERROR_JPEG = 51,					//erranious jpeg data
	FT_GE_ERROR_DEFLATE = 52,				//erranious deflated data
	FT_GE_ERROR_DISPLAYLIST_OVERFLOW = 53,	//DL buffer overflow
	FT_GE_ERROR_INVALID_WIDGET_PARAMS = 54,	//invalid input parameters - out of bound
	
	/* Display parameters error */
	FT_GE_ERROR_DISPLAYPARAMS = 100,//error in the display parameters
}FT_GEStatus;

/* Touch coprocessor related enums */
typedef enum FT_TEStatus
{	
	FT_TE_OK = 0,	
	FT_TE_ERROR_RZTHRESHOLD = 1,		//threshold out of bound
	FT_TE_ERROR_FILTERPARAM = 2,		//filter out of bound
	FT_TE_ERROR_MODE = 3,				//mode out of range
	FT_TE_ERROR_INVALIDPARAM = 4,		//generic invalid param
}FT_TEStatus;

uint8_t  GInit;
uint8_t PDNPin;
uint8_t TrnsFlag;//Global flag to indicate that initialization is done
int32_t  GError;//Global error flag
uint16_t IntPin;//interrupt pin	
uint8_t  DispGpioPin;
uint8_t AudioGpioPin;//default FT_GC pin assignments for diaplay and audio control	

uint16_t CmdFifoWp;
uint16_t FreeSpace;//command fifo write pointer 


 
/************************************************************************************************************************************************************
Display parameters used for various options are

FT_GC_DisplayResolution			Width 	Height	Swizzle	Polarity	PClk	HCycle	Hoffset		Hsync0		Hsync1		VCycle	Voffset		Vsync0	Vsync1	
FT_DISPLAY_QVGA_320x240   		320		240		3		0			8		408		70			0			10			263			13		0		2
FT_DISPLAY_WQVGA_480x272		480		272		0		1			5		548		43			0			41			292			12		0		10
*************************************************************************************************************************************************************/
/*
typedef enum FT_GC_DispRes
{
	FT_DISPLAY_QVGA_320x240 = 0,
	FT_DISPLAY_WQVGA_480x272 = 1,
}FT_GC_DispRes;
*/
#define		FT_DISPLAY_QVGA_320x240		0UL
#define		FT_DISPLAY_WQVGA_480x272 	1UL
/* Structure definitions */

typedef struct sTagXY
{
	int16_t y;		//y coordinate of touch object
	int16_t x;		//x coordinate of touch object
	uint16_t tag;	//TAG value of the object
}sTagXY;

typedef struct sTrackTag
{
	uint16_t tag;	//TAG value of the object
	uint16_t track;	//track value of the object	
}sTrackTag;

typedef struct sTouchRaw
{
    bool touch;
	uint16_t x;	//10b right-justified "x" coordinate of touch object, MSB also status bit (0 if touched)
	uint16_t y;	//10b right-justified "y" coordinate of touch object
}sTouchRaw;

/* FT80x font table structure */
/* Font table address in ROM can be found by reading 32bit value from FT_FONT_TABLE_POINTER location. */
/* 16 font tables are present at the address read from location FT_FONT_TABLE_POINTER */
typedef struct FT_Fonts
{
	/* All the values are in bytes */
	/* Width of each character font from 0 to 127 */
	uint8_t	FontWidth[FT_NUMCHAR_PERFONT];
	/* Bitmap format of font wrt bitmap formats supported by FT800 - L1, L4, L8 */
	uint32_t	FontBitmapFormat;
	/* Font line stride in FT800 ROM */
	uint32_t	FontLineStride;
	/* Font width in pixels */
	uint32_t	FontWidthInPixels;
	/* Font height in pixels */
	uint32_t	FontHeightInPixels;
	/* Pointer to font graphics raw data */
	uint32_t	PointerToFontGraphicsData;
}FT_Fonts_t;


 /*
 Following are the assumptions made for FT_GC library
 1. SPI library from arduino is included and working fine on this platform
 2. Output parameters are wrt LCD display specification
 3. 
 */
	

	FT_Status FT_GC_InitA(uint8_t ResType);//api to set the resolution of output display	
    FT_Status FT_GC_Init_Soft(uint8_t ResType);     // same as InitA, but without the hardware reset
	FT_Status FT_GC_InitB(uint16_t hperiod,uint16_t vperiod,uint16_t hfrontporch,uint16_t hbackporch,uint16_t hpulsewidth,
				   uint16_t vfrontporch,uint16_t vbackporch,uint16_t vpulsewidth,
				   uint8_t polarity,uint8_t swizzle,uint8_t fps);/* Api to program the output display wrt display parameters */
	FT_Status FT_GC_Exit(void);//exit state of graphics processor
	void FT_GC_GetVersion(  uint8_t *Major, uint8_t *Minor, uint8_t *Build);/* Api to get version of the library */
    
	void FT_GC_SetDisplayEnablePin(  uint8_t GpioBit);//FT_GC gpio bit for display enable/disable
	void FT_GC_SetAudioEnablePin(  uint8_t GpioBit);//FT_GC gpio bit for audio enable/disable
	void FT_GC_DisplayOn( );/* Apis to enable/disable backlight */
	void FT_GC_DisplayOff( );
	void FT_GC_AudioOn( );
	void FT_GC_AudioOff( );
    void FT_GC_SetGpioDrive(uint8_t port, uint8_t level);
	void FT_GC_SetInterruptPin(  uint16_t Intpin);//apis to set interrupt pin
	void FT_GC_ResetCopro( );//api to reset only coprocessor	
	void FT_GC_Reset( );//api to reset whole FT_GC via pdn - if pdn is not assigned then reset is not successful
    void FT_GC_Reset_Soft();
	/* Apis related to power up/power down funcationality */
	void FT_GC_DisplayConfigExternalClock(  uint8_t ResType);
	void FT_GC_ActiveInternalClock( );
	void FT_GC_PDN_Cycle( );
	
	/* Apis related to graphics processor */	
	//enable or disable interrupts
	void FT_GC_EnableInterrupts(  uint8_t GEnable,uint8_t Mask);
	uint8_t FT_GC_ReadIntReg( );//read the interrupt flag register - note that on FT_GC the interrupts are clear by read
	//void AssignCBFunPtr(void *CBFunPtr);//api to register callback function from application using interrupt mechanism	
	
	/* APIs related to graphics engine */
	FT_GEStatus FT_GC_AlphaFunc(  uint8_t Func, uint8_t Ref);
	FT_GEStatus FT_GC_Begin(  uint8_t Prim);
	FT_GEStatus FT_GC_BitmapHandle(  uint8_t Handle);
	FT_GEStatus FT_GC_BitmapLayout(  uint8_t Format, uint16_t Linestride, uint16_t Height);
	FT_GEStatus FT_GC_BitmapSize(  uint8_t Filter, uint8_t wrapx, uint8_t wrapy, uint16_t width, uint16_t height);
	FT_GEStatus FT_GC_BitmapSource(  uint32_t Addr);
	FT_GEStatus FT_GC_BitmapTransformA(  int32_t A);
	FT_GEStatus FT_GC_BitmapTransformB(  int32_t B);
	FT_GEStatus FT_GC_BitmapTransformC(  int32_t C);
	FT_GEStatus FT_GC_BitmapTransformD(  int32_t D);
	FT_GEStatus FT_GC_BitmapTransformE(  int32_t E);
	FT_GEStatus FT_GC_BitmapTransformF(  int32_t F);
	FT_GEStatus FT_GC_BlendFunc(  uint8_t Src, uint8_t Dst);
	FT_GEStatus FT_GC_Call(  uint16_t Dest);
	FT_GEStatus FT_GC_Cell(  uint8_t Cell);
	FT_GEStatus FT_GC_ClearColorA(  uint8_t Alpha);
	FT_GEStatus FT_GC_ClearColorRGB(  uint8_t red, uint8_t green, uint8_t blue);
	FT_GEStatus FT_GC_ClearColorRGB_int(  uint32_t rgb);
	FT_GEStatus FT_GC_ClearCST(  uint8_t c, uint8_t s, uint8_t t);
	FT_GEStatus FT_GC_Clear( );
	FT_GEStatus FT_GC_ClearStencil(  uint8_t s);
	FT_GEStatus FT_GC_ClearTag(  uint8_t s);
	FT_GEStatus FT_GC_ColorA(  uint8_t Alpha);
	FT_GEStatus FT_GC_ColorMask(  uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	FT_GEStatus FT_GC_ColorRGB(  uint8_t red, uint8_t green, uint8_t blue);
	FT_GEStatus FT_GC_Display( );
	FT_GEStatus FT_GC_End( );
	FT_GEStatus FT_GC_Jump(  uint16_t Dest);
	FT_GEStatus FT_GC_LineWidth(  uint16_t Width);
	FT_GEStatus FT_GC_Macro(  uint8_t m);
	FT_GEStatus FT_GC_PointSize(  uint16_t Size);
	FT_GEStatus FT_GC_RestoreContext( );
	FT_GEStatus FT_GC_Return( );
	FT_GEStatus FT_GC_SaveContext( );
	FT_GEStatus FT_GC_ScissorSize(  uint16_t width, uint16_t height);
	FT_GEStatus FT_GC_ScissorXY(  uint16_t x, uint16_t y);
	FT_GEStatus FT_GC_StencilFunc(  uint8_t Func, uint8_t Ref, uint8_t Mask);
	FT_GEStatus FT_GC_StencilMask(  uint8_t Mask);
	FT_GEStatus FT_GC_StencilOp(  uint8_t Sfail, uint8_t Spass);
	FT_GEStatus FT_GC_TagMask(  uint8_t Mask);
	FT_GEStatus FT_GC_Tag(  uint8_t s);
	FT_GEStatus FT_GC_Vertex2f(  int16_t x, int16_t y);
	FT_GEStatus FT_GC_Vertex2ii(  uint16_t x, uint16_t y, uint8_t Handle, uint8_t Cell);

	/* graphics helper apis */
	FT_GEStatus FT_GC_ColorRGB_int(  uint32_t rgb);
	FT_GEStatus FT_GC_ColorARGB(  uint32_t argb);

	/* APIs related to coprocessor commands, widgets etc */
	FT_GEStatus FT_GC_Cmd_Logo( );
	FT_GEStatus FT_GC_Cmd_Append(  uint32_t Ptr, uint32_t Num);
	FT_GEStatus FT_GC_Cmd_BGColor(  uint32_t c);
	FT_GEStatus FT_GC_Cmd_Button(  int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Font, uint16_t Options, const char *s);
	FT_GEStatus FT_GC_Cmd_Calibrate(  uint32_t Result);
	FT_GEStatus FT_GC_Cmd_Clock(  int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms);
	FT_GEStatus FT_GC_Cmd_ColdStart( );
	FT_GEStatus FT_GC_Cmd_Dial(  int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Val);
	FT_GEStatus FT_GC_Cmd_DLStart( );
	FT_GEStatus FT_GC_Cmd_FGColor(  uint32_t c);
	FT_GEStatus FT_GC_Cmd_Gauge(  int16_t x, int16_t y, int16_t r, uint16_t Options, uint16_t Major, uint16_t Minor, uint16_t Val, uint16_t Range);
	FT_GEStatus FT_GC_Cmd_GetMatrix( );
	FT_GEStatus FT_GC_Cmd_GetProps(  uint32_t *Ptr, uint32_t *w, uint32_t *h);
	FT_GEStatus FT_GC_Cmd_GetPtr(  uint32_t Result);
	FT_GEStatus FT_GC_Cmd_GradColor(  uint32_t c);
	FT_GEStatus FT_GC_Cmd_Gradient(  int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1);
	FT_GEStatus FT_GC_Cmd_Inflate(  uint32_t Ptr);
	FT_GEStatus FT_GC_Cmd_Interrupt(  uint32_t ms);
	FT_GEStatus FT_GC_Cmd_Keys(  int16_t x, int16_t y, int16_t w, int16_t h, uint8_t Font, uint16_t Options, const char *s);
	FT_GEStatus FT_GC_Cmd_LoadIdentity(  );
	FT_GEStatus FT_GC_Cmd_LoadImage(  uint32_t Ptr, int32_t Options);
	FT_GEStatus FT_GC_Cmd_Memcpy(  uint32_t Dest, uint32_t Src, uint32_t Num);
	FT_GEStatus FT_GC_Cmd_Memset(  uint32_t Ptr, uint8_t Value, uint32_t Num);
	FT_GEStatus FT_GC_Cmd_Memcrc(  uint32_t Ptr, uint32_t Num,uint32_t *Result);
	FT_GEStatus FT_GC_Cmd_Memwrite(  uint32_t Ptr, uint32_t Num);
	FT_GEStatus FT_GC_Cmd_Memzero(  uint32_t Ptr, uint32_t Num);
	FT_GEStatus FT_GC_Cmd_Number(  int16_t x, int16_t y, uint8_t Font, uint16_t Options, uint32_t n);
	FT_GEStatus FT_GC_Cmd_Progress(  int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Range);
	FT_GEStatus FT_GC_Cmd_RegRead(  uint32_t Ptr,uint32_t Result);
	FT_GEStatus FT_GC_Cmd_Rotate(  int32_t a);
	FT_GEStatus FT_GC_Cmd_Scale(  int32_t sx, int32_t sy);
	FT_GEStatus FT_GC_Cmd_ScreenSaver( );
	FT_GEStatus FT_GC_Cmd_Scrollbar(  int16_t x, int16_t y, int16_t w, int16_t h, uint16_t Options, uint16_t Val, uint16_t Size, uint16_t Range);
	FT_GEStatus FT_GC_Cmd_SetFont(  uint8_t Font, uint32_t Ptr);
	FT_GEStatus FT_GC_Cmd_SetMatrix( );
	FT_GEStatus FT_GC_Cmd_Sketch(  int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t Ptr, uint16_t Format);
	FT_GEStatus FT_GC_Cmd_Slider(  int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t Options, uint16_t val, uint16_t Range);
	FT_GEStatus FT_GC_Cmd_Snapshot(  uint32_t OutputAddr);
	FT_GEStatus FT_GC_Cmd_Spinner(  int16_t x, int16_t y, uint8_t Style, uint8_t Scale);
	FT_GEStatus FT_GC_Cmd_Stop( );
	FT_GEStatus FT_GC_Cmd_Swap( );
	FT_GEStatus FT_GC_Cmd_Text(  int16_t x, int16_t y, uint8_t font, uint16_t Options, const char *s);
	FT_GEStatus FT_GC_Cmd_Toggle(  int16_t x, int16_t y, int16_t w, uint8_t font, uint16_t Options, uint16_t State, const char *s);
	FT_GEStatus FT_GC_Cmd_Track(  int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t Tag);
	FT_GEStatus FT_GC_Cmd_Translate(  int32_t tx, int32_t ty);  

	/* Apis related to audio engine */
	FT_AEStatus FT_GC_PlaySound_Volume(  uint8_t Volume,uint16_t SoundNote);
	FT_AEStatus FT_GC_PlaySound(  uint16_t SoundNote);//higher byte is the note and lower byte is the sound
	void FT_GC_StopSound( );//volume will not be modified
	void FT_GC_SetSoundVolume(  uint8_t Volume);
	uint8_t FT_GC_GetSoundVolume( );

	FT_AEStatus FT_GC_PlayAudio(  uint8_t Volume,uint8_t Format,uint16_t SamplingFreq,uint32_t BufferAddr,uint32_t BufferSize,uint8_t Loop);//one shot or continuous, sampling frequency is from 8k to 48k
	void FT_GC_SetAudioVolume(  uint8_t Volume);
	FT_AEStatus FT_GC_GetAudioStats(  uint32_t *CurrPlayAddr);//returns playback stopped or continue
	uint8_t FT_GC_GetAudioVolume( );
	void FT_GC_StopAudio( );//volume will not be modified

	/* Apis related to touch engine */
	void FT_GC_SetTouchMode(  uint8_t TMode);//one of 0ff/oneshot/frame/continuous. default being continuous
    void FT_GC_SetTouchAdcMode(uint8_t TAMode);
    void FT_GC_SetTouchCharge(uint16_t TChrg);
    void FT_GC_SetTouchSettle(uint8_t TSet);
    void FT_GC_SetTouchOversample(uint8_t TSam);
    void FT_GC_SetTouchRzThresh(uint16_t TRThs);

	void FT_GC_SetHostTagXY(  uint16_t xoffset,uint16_t yoffset);//api to set coordinates for host specific tag query
	uint8_t FT_GC_GetHostTagXY( );//api to get TAG from FT_GC for coordinates set by  SetHostTagXY() api - host needs to wait for at least 1 frame to get these query values
	void FT_GC_GetTagXY(  sTagXY *sTagxy);//get the touched object tag and repective xy coordinates
	void FT_GC_GetTrackTag(  sTrackTag *sTracktag);//get the track value and the tag value
    void FT_GC_GetRawTouch(sTouchRaw *sTouchRaw);
	
	/* APIS related to power modes */
	void FT_GC_HostCommand(  uint32_t HostCommand);	
	//void CheckIntFlag(uint8_t IntFlag);	

	/* Special apis */
	//apis for ease of usage in FT_GC
	void FT_GC_DLStart( );//inserts cmd_dlstart() followed by clear(1,1,1) graphics command
	void FT_GC_DLEnd( );//inserts display() gpu instruction at the end and inserts cmd_swap() command
	FT_GEStatus FT_GC_CheckLogo( );//special api to check logo completion

	//apis to render all the commands to hardware
	FT_GEStatus FT_GC_Flush( );//api to flush out all the commands to FT_GC, does not wait for the completion of the rendering
	FT_GEStatus FT_GC_Finish( );//flushes out all the commands to FT_GC and waits for the completion of execution
	FT_GEStatus FT_GC_CheckFinish( );//checks fifo and returns the status	
	uint32_t FT_GC_GetError( );//error from graphics controller library

	/* API related to coprocessor fifo buffer management */
	//please note that all the below apis are transfer commands
	FT_GEStatus FT_GC_WriteCmd(  uint32_t Cmd);
	FT_GEStatus FT_GC_WriteCmd_Src(  uint8_t *Src,uint32_t NBytes);//api to send N bytes to command 	
	//FT_GEStatus WriteCmdfromflash(prog_uchar *Src,uint32_t NBytes);	
	//void StartTransferRCmd(uint32_t Addr);//assert CSpin and send read command
	FT_GEStatus FT_GC_TransferCmd(  uint32_t Cmd);
	FT_GEStatus FT_GC_TransferCmd_Src(  uint8_t *Src,uint32_t NBytes);	
	//FT_GEStatus TransferCmdfromflash(prog_uchar *Src,uint32_t NBytes);	
	void FT_GC_EndTransferCmd(  );//de assert CSpin
	FT_GEStatus FT_GC_Cmd_GetResult(  uint32_t *Result);//reads the result of the previous commands such as cmd_memcrc,cmd_calibration, cmd_regread which has return values. if busy returns busy status
	FT_GEStatus FT_GC_Cmd_GetResults(  int8_t *pA,uint16_t NBytes);//reads N bytes of result bytes from current write pointer
	FT_GEStatus FT_GC_UpdateFreeSpace( );
	FT_GEStatus FT_GC_ChkGetFreeSpace(  uint16_t NBytes);
	FT_GEStatus FT_GC_StartTransferCmd( );
    FT_GEStatus FT_GC_WriteCmdfromflash( const uint8_t *Src,uint32_t NBytes);

/* Api to set cs, pdn, int pin. CS pin is mandatory. If PDN and INT pins are set then gpios are initialized */		

/* API to initialize the display wrt input configuration */


#endif /* _FT_GC_H_ */


