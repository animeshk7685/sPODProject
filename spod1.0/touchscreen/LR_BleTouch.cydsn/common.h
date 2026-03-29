/*
 * common.h
 *
 *  Created on: Sep 26, 2014
 *      Author: Jason
 */

#ifndef COMMON_H_
#define COMMON_H_

//#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bridge.h"
#include "FT800.h"
#include "FT_GC.h"
#include "iprintf.h"
#include <project.h>
#include "FT_Transport_SPI.h"
#include "pubSub.h"
#include "mcpCan.h"
//#include "touchBLE.h"
//#include "sPodImageData.h"

#include "ota_mandatory.h"

#define IS_DEEP_SLEEP_DISABLED     (0)     
    
#define SW0 0x08
#define SW1 0x10
#define SW2 0x20 
#define SW3 0x40
#define SW4 0x80
#define SW5 0x01
#define SW6 0x02
#define SW7 0x04

#define BATT_X 150
#define BATT_Y 205
#define BATT_W 2
#define BATT_H 15
#define BATT_SPACE 5

#define THERM_X 80
#define THERM_Y 212
#define THERM_W 2
#define THERM_H 4
#define THERM_SPACE 5

#define SWITCH_X 5
#define SWITCH_Y 52
#define SWITCH_W 59
#define SWITCH_H 59
#define SWITCH_SPACE 8

#define DEBUG_PACKET 0x90
#define CONNECTED 0x7E
//#define TOUCHSCREEN_ADDR 0x54   // touch SE
#define TOUCHSCREEN_ADDR 0x57
    
#define LINK_CAN_INDEX_MASK (0xC0)
#define LINK_CAN_INDEX_VAL  (0x40)
#define LINK_CAN_MASK       (0x20)
#define INPUT_EN_CAN_MASK   (0x04)
#define INPUT_LCK_CAN_MASK  (0x02)
#define INPUT_INV_CAN_MASK  (0x01)    

#define EEPROM_BYTE_SIZE        (6*256)    
#define TOTAL_EEPROM_SIZE       (EEPROM_BYTE_SIZE + CY_FLASH_SIZEOF_ROW)
    
#define SW_OPTIONS_OFFSET          0x00    // 32*5*bool (saved as byte) = 0x20 > 0x20 
    
#define IS_DIMABLE_MASK         0x01
#define IS_MOMENTARY_MASK       0x02
#define IS_FLASH_MASK           0x04   
#define IS_STROBE_MASK          0x08
//#define IS_ICON_MASK            0x10   
#define IS_INPUT_ENABLED_MASK   0x20
#define IS_INPUT_LOCKOUT_MASK   0x40
#define IS_INPUT_INVERT_MASK    0x80
    
#define IS_STROBE_SET_MASK      (0x01)
#define IS_FLASH_SET_MASK       (0x02)
#define IS_OFF_ROAD_MASK        (0x80)

    
//#define IS_DIMABLE_OFFSET       0x00    // 32 * bool (saved as byte) = 0x20 > 0x20
//#define IS_MOMENTARY_OFFSET     0x20    // 32 * bool (saved as byte) = 0x20 > 0x40
//#define IS_FLASH_OFFSET         0x40    // 32 * bool (saved as byte) = 0x20 > 0x60
//#define IS_STROBE_OFFSET        0x60    // 32 * bool (saved as byte) = 0x20 > 0x80

#define SW_STATE_OFFSET         0x20    // 32 * byte                 = 0x20 > 0x40
    
#define STROBE_ON_OFFSET        0x40    // 32 * byte                 = 0x20 > 0x60
#define STROBE_OFF_OFFSET       0x60    // 32 * byte                 = 0x20 > 0x80
    
#define DIM_VALUE_OFFSET        0x80    // 32 * byte                 = 0x20 > 0xA0
//#define IS_LINKED_OFFSET        0x80    // 32 * byte                 = 0x20 > 0xA0
#define IS_ICON_OFFSET          0xA0    // 32 * bool (saved as byte) = 0x20 > 0xC0
#define ICON_ID_OFFSET          0xC0    // 32 * byte                 = 0x20 > 0xE0
#define ADR_MASK_OFFSET         0xE0    // 5 * byte                  = 0x05 > 0xE5
#define BRIGHTNESS_OFFSET       0xE5    // 1 * byte                  = 0x01 > 0xE6
#define TIMEOUT_OFFSET          0xE6    // 1 * byte                  = 0x01 > 0xE7
#define DID_WAKE_OFFSET         0xE7    // 1 * bool (saved as byte)  = 0x01 > 0xE8
#define PASSKEY_OFFSET          0xE8    // 1 * uint32                = 0x04 > 0xEC
    
#define IS_FAHREN_OFFSET        0xEC    // 1 * bool (saved as byte)  = 0x01 > 0xED
#define IS_PRO_MODE_OFFSET      0xED    // 1 * bool (saved as byte)  = 0x01 > 0xEE
#define IS_PRO_WRITABLE_OFFSET  0xEE    // 1 * bool (saved as byte)  = 0x01 > 0xEF
#define IS_NO_DEEP_SLEEP_OFFSET 0xEF    // 1 * bool (saved as byte)  = 0x01 > 0xF0
#define IS_SWIPE_ENABLED_OFFSET 0xF0    // 1 * bool (saved as byte)  = 0x01 > 0xF1
#define IS_LOCKOUT_OFFSET       0xF1    // 1 * bool (saved as byte)  = 0x01 > 0xF2
    
#define IS_LINKED_OFFSET        0x100    // 32 * uint32_t                 = 0x80 > 0x180    
    
#define BUTTON_LABELS_OFFSET    0x200    // 32 * 30 bytes
    
#define SLIDER_X 283
#define SLIDER_Y 20
#define SLIDER_W 20
#define SLIDER_H 199

#define SMODE_OFF       0
#define SMODE_OPEN      1
#define SMODE_NOM       2
#define SMODE_SHORT     3
#define SMODE_FAULT     4
#define SMODE_FLASH     5
#define SMODE_STROBE    6
#define SMODE_BLINK     7    
    
#define CURRENT_FACTOR  ((5.0/2047.0)*(17.0/2.0))
    
#define RAW_TOUCHED     0    
    
stfunc currentPage;
    
enum machine_signals_t {
	TICK_SIG = SIG_USER,
	FRAME_TICK_SIG,
	TOUCH_SIG,
	TRACK_SIG,
	PRE_FRAME_TICK_SIG,
	POST_FRAME_TICK_SIG,
	SWITCH_STATUS_FROM_TS_SIG,
	SWITCH_STATUS_FROM_CAN_SIG,
	SEND_ALIVE_CAN_PACKET,
	GOTO_PAGE_SIG,
    CAN_TICK_SIG,
    SWITCH_STATUS2_FROM_CAN_SIG,
    SWITCH_STATUS_FROM_BLE_SIG,
//    BLE_INIT,
//    BLE_TICK,
    SEND_PRO_CAN_PACKET,
    BLE_SEND_SIG,
    RAW_TOUCH_SIG,
    BLE_PAIR_SIG,
    BLE_SEND_OTA_SIG,
    BLE_SEND_PRO_SIG,
    
    MAX_PUB_SIG,

};
    
enum tMainPageButtonIds {
	MAIN_PAGE_NONE,
	SWITCH_0,
	SWITCH_1,
	SWITCH_2,
	SWITCH_3,
	SWITCH_4,
	SWITCH_5,
	SWITCH_6,
	SWITCH_7,
	SLIDER,
	SETUP,
	SPOD_LOGO_TOUCH,
	SLEEP,
	MSTROBE,
	MFLASH,
	TEMP_SEL,
	ON_OFF_ROAD,
    TEST_LOGO_TOUCH,
};

enum tSettingsPageButtonIds {
	SETTINGS_PAGE_NONE, 
    SETTINGS_SWITCH_0, 
    SETTINGS_SWITCH_1, 
    SETTINGS_SWITCH_2, 
    SETTINGS_SWITCH_3, 
    SETTINGS_SWITCH_4, 
    SETTINGS_SWITCH_5, 
    SETTINGS_SWITCH_6, 
    SETTINGS_SWITCH_7, 
    SET_SLIDER, 
    SET_SETUP, 
    SET_SPOD, 
    PLACE, 
    BACK,
	SOURCE1, 
    SOURCE2, 
    SOURCE3, 
    SOURCE4, 
    BRIGHT_SLIDER, 
    ON_SLIDER, 
    OFF_SLIDER, 
    LINK, 
    BLE_PAIR, 
    BLE_RESET,
    BACKLIGHT_TOGGLE,
    ICON_TOGGLE,
    SWIPE_TOGGLE,
    LOCKOUT_TOGGLE,
    INPUTS,
    BUTTON_ID_LAST  // don't put anything after...
};


typedef struct{
	uint32_t regTouchTransformA;
	uint32_t regTouchTransformB;
	uint32_t regTouchTransformC;
	uint32_t regTouchTransformD;
	uint32_t regTouchTransformE;
	uint32_t regTouchTransformF;
    uint32_t initialized;
	uint32_t crc;
    
}tCalibrate;

typedef struct Bitmap_header
{
       uint8_t Format;
       int16_t Width;
       int16_t Height;
       int16_t Stride;
       int32_t Arrayoffset;
} tBitmap_header;

typedef struct Button_label
{
	char line1[10];
	char line2[10];
	char line3[10];

}tButtonLabel;

typedef struct Config
{
    uint8_t cps;
	uint8_t fps;
	unsigned long sleepTimer;
	unsigned long aliveTimer;
	uint8_t dayBrightness;
	uint8_t nightBrightness;
	uint8_t dayNightThreshold;
	uint32_t fgColor;
	uint32_t bgColor;
	uint32_t color;
    uint32_t greyColor;
	uint32_t gradColor;
	uint32_t highlightColor;

	uint32_t sliderBgColor;
	uint32_t sliderFgColor;
	uint32_t sliderColor;

	uint32_t pageBgColor;


}tConfig;

tConfig settings;

extern void InitializeBootloaderSRAM(void);

bool isDaytime();
void DisplaySleep(bool isSleep);
//float celsiusToFahrenheit(uint8_t cels);

char ScratchArray[100];
char ScratchArray1[20];
char ScratchArray2[20];
uint8_t configureIndex;

extern bool fahOn;

extern uint8_t sourceAdrMask;

extern double voltage;
extern uint8_t temperature;

extern bool isConnected;
extern bool authGood;



extern const char buttonDefaultLabels[960];

extern volatile uint8_t currentSwitchIndex;

extern tButtonLabel buttonLabels[32];

extern uint8_t eepromLocalArray[EEPROM_BYTE_SIZE];
extern uint32_t *eepromLocalArray32;

extern uint16_t switchVal[32];		// pwm value
extern uint8_t onTime_buffer[32];
extern uint8_t offTime_buffer[32];

extern bool switchStatus[];
extern uint8_t switchMode[];
extern float switchCurrent[];
extern bool switchIsDimmable[];
extern bool switchIsMomentary[];
extern bool switchIsStrobe[];
extern uint8_t switchStrobeOn[];
extern uint8_t switchStrobeOff[];
extern bool switchIsFlash[];
extern bool switchStrobeSet[];
extern bool switchFlashSet[];
extern uint32_t switchIsLinked[];
extern bool sourceAddr[4];
extern bool isOnRoad;
extern bool inputIsEnabled[];
extern bool inputIsLockout[];
extern bool inputIsInvert[];

extern bool globalNeedsSleepReset;
extern bool globalNeedsWake;

extern uint8_t iconIsOn[];
extern uint8_t iconId[];

//extern bool iconIsInit[];
//extern uint32_t iconFlashBaseAddr;
extern uint32_t iconGpuBaseAddr;

extern const uint8_t eepromBlock[TOTAL_EEPROM_SIZE] CY_ALIGN(CY_FLASH_SIZEOF_ROW);

extern bool devInit;
extern bool wakeFromHib;

extern bool pairOn;

extern bool updateSent;

extern bool is24v_not12;

extern bool isPro;
extern bool isProWritable;
extern bool isProTempWritable;
extern bool isDeepSleepDisabled;

extern bool isSwipeOn;
extern bool isLockoutOn;

//extern uint8_t CY_NOINIT hibernate_mode;

//static unsigned char crc8_table[256]; /* 8-bit table */

extern const unsigned char crc8_table[];

extern int made_table;

machineEvent newEvt;

int iSqrt(int x);

void init_crc8(void);

void crc8(unsigned char *crc, unsigned char m);


void drawInvisibleButton( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn);

unsigned int rand_interval(unsigned int min, unsigned int max);

void drewBatteryMeter(float range);

void drawTemperatureMeter(float range, bool fah);

void drawImage(const tBitmap_header header[], int16_t x, int16_t y, float sX, float sY, sTagXY touch, uint8_t tagVal, uint32_t imageId);

void drawTriText(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, bool isOn, uint32_t options, uint32_t color, bool offset);

void drawSingleTriText( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, bool isOn, uint32_t options, uint32_t color);

void drawButtonSmall( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn, float current);

void drawButton( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn);

void drawButtonColor( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn, uint32_t onColor, uint32_t offColor);

void drawSwitchCurrent( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, float current);

uint16_t normalizeSlider(uint16_t val);

void drawSlider( int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t val, sTrackTag touch, uint8_t tagVal);

void drawPercentGauge( int16_t x, int16_t y, const char * label, uint8_t font, uint32_t val);

void drawBatteryVoltage( int16_t x, int16_t y, const char * label, uint8_t font, double val);

void drawSourceNum( int16_t x, int16_t y, const char * label, uint8_t font, double val);

float celsiusToFahrenheit(uint8_t cels);

void drawTempVal( int16_t x, int16_t y, const char * label, uint8_t font, uint8_t val, bool fah);

void drawVersionNum( int16_t x, int16_t y, const char * label, uint8_t font, uint16_t ver, char rev);

void drawBacklightVal( int16_t x, int16_t y, const char * label, uint8_t font, uint8_t val, bool timer);

uint8_t movingAvg(uint8_t tvalue);


/* Helper API to convert decimal to ascii - pSrc shall contain NULL terminated string */
int32_t Dec2Ascii(char *pSrc, int32_t value);

uint32_t getTextIndex(char* text);

char *dtostrf (double val, signed char width, unsigned char prec, char *sout);

uint8_t oneHot(uint8_t sw);

uint8_t indexPos(uint8_t pos);

void setWakeFromHib(bool set);
void updateSourceAddress(void);
extern void saveSettingRow(uint8 row);
extern void loadIcon(uint8_t cindex, uint8_t* data);//[512], bool clr);

stnext canComm(int signal, void* params);
stnext mainIdle(int signal, void* params);
stnext onMainPage(int signal, void* params);
stnext onSettingsPage(int signal, void* params);
stnext onConfigurePage(int signal, void* params);
stnext onSwitchOptionsPage(int signal, void* params);
stnext bleComm(int signal, void* params);
//stnext onConfigureTextPage(int signal, void* params);
stnext onInputsPage(int signal, void* params);

extern uint32 millis();

void printByteArray(char* header, uint8_t* addr, uint8_t num);

#endif /* MAIN_H_ */
