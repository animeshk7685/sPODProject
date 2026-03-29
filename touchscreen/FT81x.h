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
* @file                           FT81x.h
* @brief                          Contains FT81x graphics controller interface for arduino platform.
								  Tested platform version: Arduino 1.0.4 and later
* @version                        0.1.0
* @date                           2024/11/15
*
*/

#ifndef _FT81x_H_
#define _FT81x_H_

#define RTE_MODE 0	// 1 for resistive touch, 0 for capacitive touch

/* Chip identifier macros */
#define FT81x_CHIPID			0x00001308UL // FT_ROM_CHIPID (or 0x1308)

/* Macros for general purpose */
#define FT_DISABLE				0
#define FT_ENABLE				1
#define FT_FALSE				0
#define FT_TRUE					1

/* Macros used for graphics commands */
#define FT_NEVER                0
#define FT_LESS                 1
#define FT_LEQUAL               2
#define FT_GREATER              3
#define FT_GEQUAL               4
#define FT_EQUAL                5
#define FT_NOTEQUAL             6
#define FT_ALWAYS               7

/* Bitmap format macros */
#define FT_ARGB1555             0
#define FT_L1                   1
#define FT_L4                   2
#define FT_L8                   3
#define FT_RGB332               4
#define FT_ARGB2                5
#define FT_ARGB4                6
#define FT_RGB565               7
#define FT_PALETTED             8
#define FT_TEXT8X8              9
#define FT_TEXTVGA              10
#define FT_BARGRAPH             11
#define FT_L2                   17

/* Bitmap filter type macros */
#define FT_NEAREST              0
#define FT_BILINEAR             1

/* Bitmap wrap type macros */
#define FT_BORDER               0
#define FT_REPEAT               1

/* Stencil macros */
#define FT_KEEP                 1
#define FT_REPLACE              2
#define FT_INCR                 3
#define FT_DECR                 4
#define FT_INVERT               5

/* Graphics display list swap macros */
#define FT_DLSWAP_DONE          0
#define FT_DLSWAP_LINE          1
#define FT_DLSWAP_FRAME         2

/* Interrupt bits */
#define FT_INT_SWAP             0x01
#define FT_INT_TOUCH            0x02
#define FT_INT_TAG              0x04
#define FT_INT_SOUND            0x08
#define FT_INT_PLAYBACK         0x10
#define FT_INT_CMDEMPTY         0x20
#define FT_INT_CMDFLAG          0x40
#define FT_INT_CONVCOMPLETE     0x80

/* Touch mode macros */
#define FT_TMODE_OFF        	0
#define FT_TMODE_ONESHOT    	1
#define FT_TMODE_FRAME      	2
#define FT_TMODE_CONTINUOUS 	3

/* Alpha blending macros */
#define FT_ZERO                 0
#define FT_ONE                  1
#define FT_SRC_ALPHA            2
#define FT_DST_ALPHA            3
#define FT_ONE_MINUS_SRC_ALPHA  4
#define FT_ONE_MINUS_DST_ALPHA  5

/* Graphics primitives macros */
#define FT_BITMAPS              1
#define FT_POINTS               2
#define FT_LINES                3
#define FT_LINE_STRIP           4
#define FT_EDGE_STRIP_R         5
#define FT_EDGE_STRIP_L         6
#define FT_EDGE_STRIP_A         7
#define FT_EDGE_STRIP_B         8
#define FT_RECTS                9

/* Widget command macros */
#define FT_OPT_3D        0x0000  ///< 3D effect (CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR)
#define FT_OPT_FLAT      0x0100  ///< No 3D effect (CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR)
#define FT_OPT_SIGNED    0x0100  ///< The number is treated as a 32 bit signed integer (CMD_NUMBER)
#define FT_OPT_CENTERX   0x0200  ///< Horizontally-centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT_OPT_CENTERY   0x0400  ///< Vertically centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT_OPT_CENTER    0x0600  ///< Horizontally and vertically centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT_OPT_RIGHTX    0x0800  ///< Right justified style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT_OPT_NOBACK    0x1000  ///< No background drawn (CMD_CLOCK, CMD_GAUGE)
#define FT_OPT_NOTICKS   0x2000  ///< No Ticks (CMD_CLOCK, CMD_GAUGE)
#define FT_OPT_NOHM      0x4000  ///< No hour and minute hands (CMD_CLOCK)
#define FT_OPT_NOPOINTER 0x4000  ///< No pointer (CMD_GAUGE)
#define FT_OPT_NOSECS    0x8000  ///< No second hands (CMD_CLOCK)
#define FT_OPT_NOHANDS   0xC000  ///< Nohands (CMD_CLOCK)

/* Macros related to inbuilt font */
#define FT_NUMCHAR_PERFONT 		(128L)		//number of font characters per bitmap handle
#define FT_FONT_TABLE_SIZE 		(148L)		//size of the font table - utilized for loopup by the graphics engine
#define FT_FONT_TABLE_POINTER	(0xFFFFCUL)	//pointer to the inbuilt font tables starting from bitmap handle 16

/* Audio sample type macros */
#define FT_LINEAR_SAMPLES       0	//8bit signed samples
#define FT_ULAW_SAMPLES         1	//8bit ulaw samples
#define FT_ADPCM_SAMPLES        2	//4bit ima adpcm samples

/* Synthesized sound macros */
#define FT_SILENCE              0x00

#define FT_SQUAREWAVE           0x01
#define FT_SINEWAVE             0x02
#define FT_SAWTOOTH             0x03
#define FT_TRIANGLE             0x04

#define FT_BEEPING              0x05
#define FT_ALARM                0x06
#define FT_WARBLE               0x07
#define FT_CAROUSEL             0x08

#define FT_PIPS(n)              (0x0F + (n))

#define FT_HARP                 0x40
#define FT_XYLOPHONE            0x41
#define FT_TUBA                 0x42
#define FT_GLOCKENSPIEL         0x43
#define FT_ORGAN                0x44
#define FT_TRUMPET              0x45
#define FT_PIANO                0x46
#define FT_CHIMES               0x47
#define FT_MUSICBOX             0x48
#define FT_BELL                 0x49

#define FT_CLICK                0x50
#define FT_SWITCH               0x51
#define FT_COWBELL              0x52
#define FT_NOTCH                0x53
#define FT_HIHAT                0x54
#define FT_KICKDRUM             0x55
#define FT_POP                  0x56
#define FT_CLACK                0x57
#define FT_CHACK                0x58

#define FT_MUTE                 0x60
#define FT_UNMUTE               0x61

/* Synthesized sound frequencies, midi note macros */
#define FT_MIDI_A0            	21
#define FT_MIDI_A_0           	22
#define FT_MIDI_B0            	23
#define FT_MIDI_C1            	24
#define FT_MIDI_C_1           	25
#define FT_MIDI_D1            	26
#define FT_MIDI_D_1           	27
#define FT_MIDI_E1            	28
#define FT_MIDI_F1            	29
#define FT_MIDI_F_1           	30
#define FT_MIDI_G1            	31
#define FT_MIDI_G_1           	32
#define FT_MIDI_A1            	33
#define FT_MIDI_A_1           	34
#define FT_MIDI_B1            	35
#define FT_MIDI_C2            	36
#define FT_MIDI_C_2           	37
#define FT_MIDI_D2            	38
#define FT_MIDI_D_2           	39
#define FT_MIDI_E2            	40
#define FT_MIDI_F2            	41
#define FT_MIDI_F_2           	42
#define FT_MIDI_G2            	43
#define FT_MIDI_G_2           	44
#define FT_MIDI_A2            	45
#define FT_MIDI_A_2           	46
#define FT_MIDI_B2            	47
#define FT_MIDI_C3            	48
#define FT_MIDI_C_3           	49
#define FT_MIDI_D3            	50
#define FT_MIDI_D_3           	51
#define FT_MIDI_E3            	52
#define FT_MIDI_F3            	53
#define FT_MIDI_F_3           	54
#define FT_MIDI_G3            	55
#define FT_MIDI_G_3           	56
#define FT_MIDI_A3            	57
#define FT_MIDI_A_3           	58
#define FT_MIDI_B3            	59
#define FT_MIDI_C4            	60
#define FT_MIDI_C_4           	61
#define FT_MIDI_D4            	62
#define FT_MIDI_D_4           	63
#define FT_MIDI_E4            	64
#define FT_MIDI_F4            	65
#define FT_MIDI_F_4           	66
#define FT_MIDI_G4            	67
#define FT_MIDI_G_4           	68
#define FT_MIDI_A4            	69
#define FT_MIDI_A_4           	70
#define FT_MIDI_B4            	71
#define FT_MIDI_C5            	72
#define FT_MIDI_C_5           	73
#define FT_MIDI_D5            	74
#define FT_MIDI_D_5           	75
#define FT_MIDI_E5            	76
#define FT_MIDI_F5            	77
#define FT_MIDI_F_5           	78
#define FT_MIDI_G5            	79
#define FT_MIDI_G_5           	80
#define FT_MIDI_A5            	81
#define FT_MIDI_A_5           	82
#define FT_MIDI_B5            	83
#define FT_MIDI_C6            	84
#define FT_MIDI_C_6           	85
#define FT_MIDI_D6            	86
#define FT_MIDI_D_6           	87
#define FT_MIDI_E6            	88
#define FT_MIDI_F6            	89
#define FT_MIDI_F_6           	90
#define FT_MIDI_G6            	91
#define FT_MIDI_G_6           	92
#define FT_MIDI_A6            	93
#define FT_MIDI_A_6           	94
#define FT_MIDI_B6            	95
#define FT_MIDI_C7            	96
#define FT_MIDI_C_7           	97
#define FT_MIDI_D7            	98
#define FT_MIDI_D_7           	99
#define FT_MIDI_E7            	100
#define FT_MIDI_F7            	101
#define FT_MIDI_F_7           	102
#define FT_MIDI_G7            	103
#define FT_MIDI_G_7           	104
#define FT_MIDI_A7            	105
#define FT_MIDI_A_7           	106
#define FT_MIDI_B7            	107
#define FT_MIDI_C8            	108

/* GPIO bit macros */
#define FT_GPIO0				0
#define FT_GPIO1				1	//default gpio pin for audio shutdown, 1 - eanble, 0 - disable
#define FT_GPIO7				7	//default gpio pin for display enable, 1 - enable, 0 - disable

/* Display rotation */
#define FT_DISPLAY_0			0	//0 degrees rotation
#define FT_DISPLAY_180			1	//180 degrees rotation

/* Maximum display display resolution supported by graphics engine */
//#define FT_MAX_DISPLAYWIDTH		(512L)
//#define FT_MAX_DISPLAYHEIGHT	(512L)

/* Host command macros */
#define FT_ACTIVE				0x00			// Place FT81x in active state
#define FT_STANDBY				0x41			// Place FT81x in Standby (clk running)
#define FT_SLEEP				0x42			// Place FT81x in Sleep (clk off)
#define FT_PWRDOWN				0x50			// Place FT81x in Power Down (core off)
#define FT_CLKEXT				0x44			// Select external clock source
#define FT_CLKINT				0x48			// Select internal clock source
#define FT_CLK48M				0x62			// Select 48MHz PLL output
#define FT_CLK36M				0x61			// Select 36MHz PLL output
#define FT_CORERST				0x68			// Reset core - all registers default and processors reset

/* Coprocessor reset related macros */
#define FT_RESET_HOLD_COPROCESSOR		1
#define FT_RESET_RELEASE_COPROCESSOR	0

/* Macros for sound play and stop */
#define FT_SOUND_PLAY					1
#define FT_AUDIO_PLAY					1

/* Macros for audio playback parameters*/
#define FT_AUDIO_SAMPLINGFREQ_MIN		8*1000L
#define FT_AUDIO_SAMPLINGFREQ_MAX		48*1000L

//coprocessor error macros
#define FT_COPRO_ERROR					0xfffUL

/* Macros for display panels */
#define FT_DISPLAY_VSYNC0_WQVGA 		(0L)
#define FT_DISPLAY_VSYNC1_WQVGA 		(10L)
#define FT_DISPLAY_VOFFSET_WQVGA		(12L)
#define FT_DISPLAY_VCYCLE_WQVGA 		(292L)
#define FT_DISPLAY_HSYNC0_WQVGA 		(0L)
#define FT_DISPLAY_HSYNC1_WQVGA 		(41L)
#define FT_DISPLAY_HOFFSET_WQVGA 		(43L)
#define FT_DISPLAY_HCYCLE_WQVGA 		(548L)
#define FT_DISPLAY_HSIZE_WQVGA 			(480L)//display width
#define FT_DISPLAY_VSIZE_WQVGA 			(272L)//display height
#define FT_DISPLAY_PCLKPOL_WQVGA 		(1L)
#define FT_DISPLAY_SWIZZLE_WQVGA 		(0L)
#define FT_DISPLAY_PCLK_WQVGA 			(5L)

#define FT_DISPLAY_VSYNC0_QVGA 			(0L)
#define FT_DISPLAY_VSYNC1_QVGA 			(2L)
#define FT_DISPLAY_VOFFSET_QVGA 		(13L)
#define FT_DISPLAY_VCYCLE_QVGA 			(263L)
#define FT_DISPLAY_HSYNC0_QVGA 			(0L)
#define FT_DISPLAY_HSYNC1_QVGA 			(10L)
#define FT_DISPLAY_HOFFSET_QVGA 		(70L)
#define FT_DISPLAY_HCYCLE_QVGA 			(408L)
#define FT_DISPLAY_HSIZE_QVGA 			(320L)//display width
#define FT_DISPLAY_VSIZE_QVGA 			(240L)//display height
#define FT_DISPLAY_PCLKPOL_QVGA 		(0L)
#define FT_DISPLAY_SWIZZLE_QVGA 		(2L)
#define FT_DISPLAY_PCLK_QVGA 			(8L)

/* Macros for default settings */
#define FT_DISPLAY_VSYNC0 				FT_DISPLAY_VSYNC0_WQVGA 
#define FT_DISPLAY_VSYNC1 				FT_DISPLAY_VSYNC1_WQVGA 
#define FT_DISPLAY_VOFFSET				FT_DISPLAY_VOFFSET_WQVGA
#define FT_DISPLAY_VCYCLE 				FT_DISPLAY_VCYCLE_WQVGA 
#define FT_DISPLAY_HSYNC0 				FT_DISPLAY_HSYNC0_WQVGA 
#define FT_DISPLAY_HSYNC1 				FT_DISPLAY_HSYNC1_WQVGA 
#define FT_DISPLAY_HOFFSET 				FT_DISPLAY_HOFFSET_WQVGA
#define FT_DISPLAY_HCYCLE 				FT_DISPLAY_HCYCLE_WQVGA 
#define FT_DISPLAY_HSIZE				FT_DISPLAY_HSIZE_WQVGA 	
#define FT_DISPLAY_VSIZE				FT_DISPLAY_VSIZE_WQVGA 	
#define FT_DISPLAY_PCLKPOL 				FT_DISPLAY_PCLKPOL_WQVGA
#define FT_DISPLAY_SWIZZLE 				FT_DISPLAY_SWIZZLE_WQVGA
#define FT_DISPLAY_PCLK					FT_DISPLAY_PCLK_WQVGA 	
		
/* Memory definitions */		
#define FT_RAM_G         				0x000000UL  ///< General purpose graphics RAM
#define FT_ROM_CHIPID					0x0C0000UL	///< read FROM for FT81x_CHIPID
#define FT_ROM_FONT      				0x1E0000UL  ///< Font table and bitmap
#define FT_ROM_FONT_ADDR 				0x2FFFFCUL ///< Font table pointer address
#define FT_RAM_DL        				0x300000UL  ///< Display List RAM
#define FT_RAM_REG		       			0x302000UL  ///< Registers
#define FT_RAM_CMD       				0x308000UL  ///< Command buffer
#define xFT_RAM_SCREENSHOT				0x1C2000UL		

/* Memory buffer sizes */
#define FT_RAM_G_SIZE					1024*1024L
#define FT_CMDFIFO_SIZE					4*1024L
#define FT_RAM_DL_SIZE					8*1024L
#define FT_RAM_PAL_SIZE					1*1024L

/* Coprocessor related commands */
#define CMD_APPEND           			0xFFFFFF1EUL
#define CMD_BGCOLOR          			0xFFFFFF09UL
#define CMD_BITMAP_TRANSFORM 			0xFFFFFF21UL
#define CMD_BUTTON           			0xFFFFFF0DUL
#define CMD_CALIBRATE        			0xFFFFFF15UL
#define CMD_CLOCK            			0xFFFFFF14UL
#define CMD_COLDSTART        			0xFFFFFF32UL
#define CMD_CRC              			0xFFFFFF03UL
#define CMD_DIAL             			0xFFFFFF2DUL
#define CMD_DLSTART          			0xFFFFFF00UL
#define CMD_EXECUTE          			0xFFFFFF07UL
#define CMD_FGCOLOR          			0xFFFFFF0AUL
#define CMD_GAUGE            			0xFFFFFF13UL
#define CMD_GETMATRIX        			0xFFFFFF33UL
#define CMD_GETPOINT         			0xFFFFFF08UL
#define CMD_GETPROPS         			0xFFFFFF25UL
#define CMD_GETPTR           			0xFFFFFF23UL
#define CMD_GRADCOLOR        			0xFFFFFF34UL
#define CMD_GRADIENT         			0xFFFFFF0BUL
#define CMD_HAMMERAUX        			0xFFFFFF04UL
#define CMD_IDCT             			0xFFFFFF06UL
#define CMD_INFLATE          			0xFFFFFF22UL
#define CMD_INTERRUPT        			0xFFFFFF02UL
#define CMD_KEYS             			0xFFFFFF0EUL
#define CMD_LOADIDENTITY     			0xFFFFFF26UL
#define CMD_LOADIMAGE        			0xFFFFFF24UL
#define CMD_LOGO             			0xFFFFFF31UL
#define CMD_MARCH            			0xFFFFFF05UL
#define CMD_MEMCPY           			0xFFFFFF1DUL
#define CMD_MEMCRC           			0xFFFFFF18UL
#define CMD_MEMSET           			0xFFFFFF1BUL
#define CMD_MEMWRITE         			0xFFFFFF1AUL
#define CMD_MEMZERO          			0xFFFFFF1CUL
#define CMD_NUMBER           			0xFFFFFF2EUL
#define CMD_PROGRESS         			0xFFFFFF0FUL
#define CMD_REGREAD          			0xFFFFFF19UL
#define CMD_ROTATE           			0xFFFFFF29UL
#define CMD_SCALE            			0xFFFFFF28UL
#define CMD_SCREENSAVER      			0xFFFFFF2FUL
#define CMD_SCROLLBAR        			0xFFFFFF11UL
#define CMD_SETFONT          			0xFFFFFF2BUL
#define CMD_SETMATRIX        			0xFFFFFF2AUL
#define CMD_SKETCH           			0xFFFFFF30UL
#define CMD_SLIDER           			0xFFFFFF10UL
#define CMD_SNAPSHOT         			0xFFFFFF1FUL
#define CMD_SPINNER          			0xFFFFFF16UL
#define CMD_STOP             			0xFFFFFF17UL
#define CMD_SWAP             			0xFFFFFF01UL
#define CMD_TEXT             			0xFFFFFF0CUL
#define CMD_TOGGLE           			0xFFFFFF12UL
#define CMD_TOUCH_TRANSFORM  			0xFFFFFF20UL
#define CMD_TRACK            			0xFFFFFF2CUL
#define CMD_TRANSLATE        			0xFFFFFF27UL
		
/* Register definitions */
#define FT81x_REG_ID               0x302000UL  ///< Identification register, always reads as 7Ch
#define FT81x_REG_FRAMES           0x302004UL  ///< Frame counter, since reset
#define FT81x_REG_CLOCK            0x302008UL  ///< Clock cycles, since reset
#define FT81x_REG_FREQUENCY        0x30200CUL  ///< Main clock frequency (Hz)
#define FT81x_REG_RENDERMODE       0x302010UL  ///< Rendering mode: 0 = normal, 1 = single-line
#define FT81x_REG_SNAPY            0x302014UL  ///< Scanline select for RENDERMODE 1
#define FT81x_REG_SNAPSHOT         0x302018UL  ///< Trigger for RENDERMODE 1
#define FT81x_REG_SNAPFORMAT       0x30201CUL  ///< Pixel format for scanline readout
#define FT81x_REG_CPURESET         0x302020UL  ///< Graphics, audio and touch engines reset control. Bit2: audio, bit1: touch, bit0: graphics
#define FT81x_REG_TAP_CRC          0x302024UL  ///< Live video tap crc. Frame CRC is computed every DL SWAP.
#define FT81x_REG_TAP_MASK         0x302028UL  ///< Live video tap mask
#define FT81x_REG_HCYCLE           0x30202CUL  ///< Horizontal total cycle count
#define FT81x_REG_HOFFSET          0x302030UL  ///< Horizontal display start offset
#define FT81x_REG_HSIZE            0x302034UL  ///< Horizontal display pixel count
#define FT81x_REG_HSYNC0           0x302038UL  ///< Horizontal sync fall offset
#define FT81x_REG_HSYNC1           0x30203CUL  ///< Horizontal sync rise offset
#define FT81x_REG_VCYCLE           0x302040UL  ///< Vertical total cycle count
#define FT81x_REG_VOFFSET          0x302044UL  ///< Vertical display start offset
#define FT81x_REG_VSIZE            0x302048UL  ///< Vertical display line count
#define FT81x_REG_VSYNC0           0x30204CUL  ///< Vertical sync fall offset
#define FT81x_REG_VSYNC1           0x302050UL  ///< Vertical sync rise offset
#define FT81x_REG_DLSWAP           0x302054UL  ///< Display list swap control
#define FT81x_REG_ROTATE           0x302058UL  ///< Screen rotation control. Allow normal/mirrored/inverted for landscape or portrait orientation.
#define FT81x_REG_OUTBITS          0x30205CUL  ///< Output bit resolution, 3 bits each for R/G/B. Default is 6/6/6 bits for FT810/FT811, and 8/8/8 bits for FT812/FT813 (0b’000 means 8 bits)
#define FT81x_REG_DITHER           0x302060UL  ///< Output dither enable
#define FT81x_REG_SWIZZLE          0x302064UL  ///< Output RGB signal swizzle
#define FT81x_REG_CSPREAD          0x302068UL  ///< Output clock spreading enable
#define FT81x_REG_PCLK_POL         0x30206CUL  ///< PCLK polarity: 0 = output on PCLK rising edge, 1 = output on PCLK falling edge
#define FT81x_REG_PCLK             0x302070UL  ///< PCLK frequency divider, 0 = disable
#define FT81x_REG_TAG_X            0x302074UL  ///< Tag query X coordinate
#define FT81x_REG_TAG_Y            0x302078UL  ///< Tag query Y coordinate
#define FT81x_REG_TAG              0x30207CUL  ///< Tag query result
#define FT81x_REG_VOL_PB           0x302080UL  ///< Volume for playback
#define FT81x_REG_VOL_SOUND        0x302084UL  ///< Volume for synthesizer sound
#define FT81x_REG_SOUND            0x302088UL  ///< Sound effect select
#define FT81x_REG_PLAY             0x30208CUL  ///< Start effect playback
#define FT81x_REG_GPIO_DIR         0x302090UL  ///< Legacy GPIO pin direction, 0 = input, 1 = output
#define FT81x_REG_GPIO             0x302094UL  ///< Legacy GPIO read/write
#define FT81x_REG_GPIOX_DIR        0x302098UL  ///< Extended GPIO pin direction, 0 = input ,1 = output
#define FT81x_REG_GPIOX            0x30209CUL  ///< Extended GPIO read/write
#define FT81x_REG_INT_FLAGS        0x3020A8UL  ///< Interrupt flags, clear by read
#define FT81x_REG_INT_EN           0x3020ACUL  ///< Global interrupt enable, 1=enable
#define FT81x_REG_INT_MASK         0x3020B0UL  ///< Individual interrupt enable, 1=enable
#define FT81x_REG_PLAYBACK_START   0x3020B4UL  ///< Audio playback RAM start address
#define FT81x_REG_PLAYBACK_LENGTH  0x3020B8UL  ///< Audio playback sample length (bytes)
#define FT81x_REG_PLAYBACK_READPTR 0x3020BCUL  ///< Audio playback current read pointer
#define FT81x_REG_PLAYBACK_FREQ    0x3020C0UL  ///< Audio playback sampling frequency (Hz)
#define FT81x_REG_PLAYBACK_FORMAT  0x3020C4UL  ///< Audio playback format
#define FT81x_REG_PLAYBACK_LOOP    0x3020C8UL  ///< Audio playback loop enable
#define FT81x_REG_PLAYBACK_PLAY    0x3020CCUL  ///< Start audio playback
#define FT81x_REG_PWM_HZ           0x3020D0UL  ///< BACKLIGHT PWM output frequency (Hz)
#define FT81x_REG_PWM_DUTY         0x3020D4UL  ///< BACKLIGHT PWM output duty cycle 0=0%, 128=100%
#define FT81x_REG_MACRO_0          0x3020D8UL  ///< Display list macro command 0
#define FT81x_REG_MACRO_1          0x3020DCUL  ///< Display list macro command 1
#define FT81x_REG_CMD_READ         0x3020F8UL  ///< Command buffer read pointer
#define FT81x_REG_CMD_WRITE        0x3020FCUL  ///< Command buffer write pointer
#define FT81x_REG_CMD_DL           0x302100UL  ///< Command display list offset
#define FT81x_REG_BIST_EN          0x302174UL  ///< BIST memory mapping enable
#define FT81x_REG_TRIM             0x302180UL  ///< Internal relaxation clock trimming
#define FT81x_REG_ANA_COMP         0x302184UL  ///< Analogue control register
#define FT81x_REG_SPI_WIDTH        0x302188UL  ///< QSPI bus width setting, Bit [2]: extra dummy cycle on read, Bit [1:0]: bus width (0=1-bit, 1=2-bit, 2=4-bit)
#define FT81x_REG_DATESTAMP        0x302564UL  ///< Stamp date code
#define FT81x_REG_CMDB_SPACE       0x302574UL  ///< Command DL (bulk) space available
#define FT81x_REG_CMDB_WRITE       0x302578UL  ///< Command DL (bulk) write
#define FT81x_REG_MEDIAFIFO_READ   0x309014UL  ///< Read pointer for media FIFO in general purpose graphics RAM
#define FT81x_REG_MEDIAFIFO_WRITE  0x309018UL  ///< Write pointer for media FIFO in general purpose graphics RAM

#define REG_CTOUCH_MODE				0x302104UL
#define REG_CTOUCH_EXTENDED			0x302108UL
#define REG_CTOUCH_TOUCH1_XY			0x30211CUL
#define REG_CTOUCH_TOUCH4_Y			0x302120UL
#define REG_CTOUCH_TOUCH_XY 			0x302124UL
#define REG_CTOUCH_TAG_XY			0x302128UL
#define REG_CTOUCH_TAG				0x30212CUL
#define REG_CTOUCH_TAG1_XY			0x302130UL
#define REG_CTOUCH_TAG1				0x302134UL
#define REG_CTOUCH_TAG2_XY			0x302138UL
#define REG_CTOUCH_TAG2				0x30213CUL
#define REG_CTOUCH_TAG3_XY			0x302140UL
#define REG_CTOUCH_TAG3				0x302144UL
#define REG_CTOUCH_TAG4_XY			0x302148UL
#define REG_CTOUCH_TAG4				0x30214CUL
#define REG_CTOUCH_TOUCH4_X			0x30216CUL
#define REG_CTOUCH_TOUCH2_XY			0x30218CUL
#define REG_CTOUCH_TOUCH3_XY			0x302190UL

#if 0
#define REG_ID							FT81x_REG_ID
#define REG_FRAMES						FT81x_REG_FRAMES
#define REG_CLOCK						FT81x_REG_CLOCK
#define REG_FREQUENCY					FT81x_REG_FREQUENCY	
#define REG_SCREENSHOT_EN				FT81x_REG_RENDERMODE		
#define REG_SCREENSHOT_Y				FT81x_REG_SNAPY		
#define REG_SCREENSHOT_START 			FT81x_REG_SNAPSHOT			
#define REG_CPURESET 					FT81x_REG_CPURESET	
#define REG_TAP_CRC 					FT81x_REG_TAP_CRC	
#define REG_TAP_MASK 					FT81x_REG_TAP_MASK	
#define REG_HCYCLE 						FT81x_REG_HCYCLE
#define REG_HOFFSET 					FT81x_REG_HOFFSET	
#define REG_HSIZE 						FT81x_REG_HSIZE
#define REG_HSYNC0 						FT81x_REG_HSYNC0
#define REG_HSYNC1 						FT81x_REG_HSYNC1
#define REG_VCYCLE 						FT81x_REG_VCYCLE
#define REG_VOFFSET 					FT81x_REG_VOFFSET	
#define REG_VSIZE 						FT81x_REG_VSIZE
#define REG_VSYNC0 						FT81x_REG_VSYNC0
#define REG_VSYNC1 						FT81x_REG_VSYNC1
#define REG_DLSWAP 						FT81x_REG_DLSWAP
#define REG_ROTATE 						FT81x_REG_ROTATE
#define REG_OUTBITS 					FT81x_REG_OUTBITS	
#define REG_DITHER 						FT81x_REG_DITHER
#define REG_SWIZZLE 					FT81x_REG_SWIZZLE	
#define REG_CSPREAD 					FT81x_REG_CSPREAD	
#define REG_PCLK_POL 					FT81x_REG_PCLK_POL	
#define REG_PCLK 						FT81x_REG_PCLK
#define REG_TAG_X 						FT81x_REG_TAG_X
#define REG_TAG_Y 						FT81x_REG_TAG_Y
#define REG_TAG 						FT81x_REG_TAG
#define REG_VOL_PB 						FT81x_REG_VOL_PB
#define REG_VOL_SOUND 					FT81x_REG_VOL_SOUND	
#define REG_SOUND 						FT81x_REG_SOUND
#define REG_PLAY 						FT81x_REG_PLAY
#define REG_GPIO_DIR 					FT81x_REG_GPIO_DIR	
#define REG_GPIO 						FT81x_REG_GPIO
#define REG_INT_FLAGS           		FT81x_REG_INT_FLAGS
#define REG_INT_EN              		FT81x_REG_INT_EN
#define REG_INT_MASK            		FT81x_REG_INT_MASK
#define REG_PLAYBACK_START      		FT81x_REG_PLAYBACK_START
#define REG_PLAYBACK_LENGTH     		FT81x_REG_PLAYBACK_LENGTH
#define REG_PLAYBACK_READPTR    		FT81x_REG_PLAYBACK_READPTR
#define REG_PLAYBACK_FREQ       		FT81x_REG_PLAYBACK_FREQ
#define REG_PLAYBACK_FORMAT     		FT81x_REG_PLAYBACK_FORMAT
#define REG_PLAYBACK_LOOP       		FT81x_REG_PLAYBACK_LOOP
#define REG_PLAYBACK_PLAY       		FT81x_REG_PLAYBACK_PLAY
#define REG_PWM_HZ              		FT81x_REG_PWM_HZ
#define REG_PWM_DUTY            		FT81x_REG_PWM_DUTY
#define REG_MACRO_0             		FT81x_REG_MACRO_0
#define REG_MACRO_1             		FT81x_REG_MACRO_1
#define xREG_SCREENSHOT_BUSY				0x1024D8UL
#define REG_CMD_READ            		FT81x_REG_CMD_READ
#define REG_CMD_WRITE           		FT81x_REG_CMD_WRITE
#define REG_CMD_DL              		FT81x_REG_CMD_DL

#define REG_TOUCH_OVERSAMPLE    		0x302114UL
#if RTE_MODE
// Bridgetek FT81x Series Programmers Guide section 3.3.3 (resistive touch)
#define REG_TOUCH_MODE          		0x302104UL
#define REG_TOUCH_ADC_MODE      		0x302108UL
#define REG_TOUCH_CHARGE        		0x30210CUL
#define REG_TOUCH_SETTLE        		0x302110UL
#define REG_TOUCH_RZTHRESH      		0x302118UL
#define REG_TOUCH_RAW_XY        		0x30211CUL
#define REG_TOUCH_RZ            		0x302120UL
#define REG_TOUCH_SCREEN_XY     		0x302124UL
#define REG_TOUCH_TAG_XY        		0x302128UL
#define REG_TOUCH_TAG           		0x30212CUL
#else
// Bridgetek FT81x Series Programmers Guide section 3.3.4 (capactive touch)
#define REG_CTOUCH_MODE				0x302104UL
#define REG_CTOUCH_EXTENDED			0x302108UL
#define REG_CTOUCH_TOUCH1_XY			0x30211CUL
#define REG_CTOUCH_TOUCH4_Y			0x302120UL
#define REG_CTOUCH_TOUCH_XY 			0x302124UL
#define REG_CTOUCH_TAG_XY			0x302128UL
#define REG_CTOUCH_TAG				0x30212CUL
#define REG_CTOUCH_TAG1_XY			0x302130UL
#define REG_CTOUCH_TAG1				0x302134UL
#define REG_CTOUCH_TAG2_XY			0x302138UL
#define REG_CTOUCH_TAG2				0x30213CUL
#define REG_CTOUCH_TAG3_XY			0x302140UL
#define REG_CTOUCH_TAG3				0x302144UL
#define REG_CTOUCH_TAG4_XY			0x302148UL
#define REG_CTOUCH_TAG4				0x30214CUL
#define REG_CTOUCH_TOUCH4_X			0x30216CUL
#define REG_CTOUCH_TOUCH2_XY			0x30218CUL
#define REG_CTOUCH_TOUCH3_XY			0x302190UL
#endif

// following registers are common to both RTE and CTE (resistive and capacitive)
#define REG_TOUCH_TRANSFORM_A   		0x302150UL
#define REG_TOUCH_TRANSFORM_B   		0x302154UL
#define REG_TOUCH_TRANSFORM_C   		0x302158UL
#define REG_TOUCH_TRANSFORM_D   		0x30215CUL
#define REG_TOUCH_TRANSFORM_E   		0x302160UL
#define REG_TOUCH_TRANSFORM_F   		0x302164UL
#define REG_TOUCH_CONFIG                0x302168UL  // LCE-GRH added

#define xREG_SCREENSHOT_READ				0x102554UL
#define REG_TRIM						FT81x_REG_TRIM
#define REG_TOUCH_DIRECT_XY 			0x30218CUL
#define REG_TOUCH_DIRECT_Z1Z2 			0x302190UL
#define REG_TRACKER						0x309000UL
#define REG_MEDIAFIFO_READ 				FT81x_REG_MEDIAFIFO_READ
#define REG_MEDIAFIFO_WRITE 			FT81x_REG_MEDIAFIFO_WRITE

/* FT81x graphics engine specific macros useful for static display list generation */
#define VERTEX2F(x,y) ((1UL<<30)|(((x)&32767UL)<<15)|(((y)&32767UL)<<0))
#define VERTEX2II(x,y,handle,cell) ((2UL<<30)|(((x)&511UL)<<21)|(((y)&511UL)<<12)|(((handle)&31UL)<<7)|(((cell)&127UL)<<0))
#define BITMAP_SOURCE(addr) ((1UL<<24)|(((addr)&1048575UL)<<0))
#define CLEAR_COLOR_RGB(red,green,blue) ((2UL<<24)|(((red)&255UL)<<16)|(((green)&255UL)<<8)|(((blue)&255UL)<<0))
#define TAG(s) ((3UL<<24)|(((s)&255UL)<<0))
#define COLOR_RGB(red,green,blue) ((4UL<<24)|(((red)&255UL)<<16)|(((green)&255UL)<<8)|(((blue)&255UL)<<0))
#define BITMAP_HANDLE(handle) ((5UL<<24)|(((handle)&31UL)<<0))
#define CELL(cell) ((6UL<<24)|(((cell)&127UL)<<0))
#define BITMAP_LAYOUT(format,linestride,height) ((7UL<<24)|(((format)&31UL)<<19)|(((linestride)&1023UL)<<9)|(((height)&511UL)<<0))
#define BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((8UL<<24)|(((filter)&1UL)<<20)|(((wrapx)&1UL)<<19)|(((wrapy)&1UL)<<18)|(((width)&511UL)<<9)|(((height)&511UL)<<0))
#define ALPHA_FUNC(func,ref) ((9UL<<24)|(((func)&7UL)<<8)|(((ref)&255UL)<<0))
#define STENCIL_FUNC(func,ref,mask) ((10UL<<24)|(((func)&7UL)<<16)|(((ref)&255UL)<<8)|(((mask)&255UL)<<0))
#define BLEND_FUNC(src,dst) ((11UL<<24)|(((src)&7UL)<<3)|(((dst)&7UL)<<0))
#define STENCIL_OP(sfail,spass) ((12UL<<24)|(((sfail)&7UL)<<3)|(((spass)&7UL)<<0))
#define POINT_SIZE(size) ((13UL<<24)|(((size)&8191UL)<<0))
#define LINE_WIDTH(width) ((14UL<<24)|(((width)&4095UL)<<0))
#define CLEAR_COLOR_A(alpha) ((15UL<<24)|(((alpha)&255UL)<<0))
#define COLOR_A(alpha) ((16UL<<24)|(((alpha)&255UL)<<0))
#define CLEAR_STENCIL(s) ((17UL<<24)|(((s)&255UL)<<0))
#define CLEAR_TAG(s) ((18UL<<24)|(((s)&255UL)<<0))
#define STENCIL_MASK(mask) ((19UL<<24)|(((mask)&255UL)<<0))
#define TAG_MASK(mask) ((20UL<<24)|(((mask)&1UL)<<0))
#define BITMAP_TRANSFORM_A(a) ((21UL<<24)|(((a)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(b) ((22UL<<24)|(((b)&131071UL)<<0))
#define BITMAP_TRANSFORM_C(c) ((23UL<<24)|(((c)&16777215UL)<<0))
#define BITMAP_TRANSFORM_D(d) ((24UL<<24)|(((d)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(e) ((25UL<<24)|(((e)&131071UL)<<0))
#define BITMAP_TRANSFORM_F(f) ((26UL<<24)|(((f)&16777215UL)<<0))
#define SCISSOR_XY(x,y) ((27UL<<24)|(((x)&511UL)<<9)|(((y)&511UL)<<0))
#define SCISSOR_SIZE(width,height) ((28UL<<24)|(((width)&1023UL)<<10)|(((height)&1023UL)<<0))
#define CALL(dest) ((29UL<<24)|(((dest)&65535UL)<<0))
#define JUMP(dest) ((30UL<<24)|(((dest)&65535UL)<<0))
#define BEGIN(prim) ((31UL<<24)|(((prim)&15UL)<<0))
#define COLOR_MASK(r,g,b,a) ((32UL<<24)|(((r)&1UL)<<3)|(((g)&1UL)<<2)|(((b)&1UL)<<1)|(((a)&1UL)<<0))
#define CLEAR(c,s,t) ((38UL<<24)|(((c)&1UL)<<2)|(((s)&1UL)<<1)|(((t)&1UL)<<0))
#define END() ((33UL<<24))
#define SAVE_CONTEXT() ((34UL<<24))
#define RESTORE_CONTEXT() ((35UL<<24))
#define RETURN() ((36UL<<24))
#define MACRO(m) ((37UL<<24)|(((m)&1UL)<<0))
//#define DISPLAY() ((0UL<<24))
#endif

#endif /* _FT81x_H_ */


