/*
 * common.h
 *
 *  Created on: Sep 26, 2014
 *      Author: Jason
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bridge.h"
#include "FT81x.h"
#include "GC.h"
#include "pub_sub.h"
//#include "mcp_can.h"


#define CY_FLASH_SIZEOF_ROW	0x100

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

#define SWITCH_FONT 26
#define SWITCH_W 62
#define SWITCH_H 70
#define SWITCH_SPACE 8

#define DEBUG_PACKET 0x90
#define CONNECTED 0x7E
#define TOUCHSCREEN_ADDR 0x57
    
#define LINK_CAN_INDEX_MASK (0xC0)
#define LINK_CAN_INDEX_VAL  (0x40)
#define LINK_CAN_MASK       (0x20)
#define INPUT_EN_CAN_MASK   (0x04)
#define INPUT_LCK_CAN_MASK  (0x02)
#define INPUT_INV_CAN_MASK  (0x01)    

#define IS_DIMABLE_MASK         0x01
#define IS_MOMENTARY_MASK       0x02
#define IS_FLASH_MASK           0x04   
#define IS_STROBE_MASK          0x08
#define IS_INPUT_ENABLED_MASK   0x20
#define IS_INPUT_LOCKOUT_MASK   0x40
#define IS_INPUT_INVERT_MASK    0x80
    
#define IS_STROBE_SET_MASK      (0x01)
#define IS_FLASH_SET_MASK       (0x02)
#define IS_OFF_ROAD_MASK        (0x80)

    
#define CURRENT_FACTOR  ((5.0/2047.0)*(17.0/2.0))
    
#define RAW_TOUCHED     0    
    
extern stfunc currentPage;

typedef enum {
	WARNING_PAGE = 0,
	HOME_PAGE,
	CONFIG_SETUP_PAGE,
	SWITCH_SETUP_PAGE,
	ADVANCED_PAGE,
	TEST_PAGE,
	SWITCH_LABEL_PAGE,
	SETTINGS_PAGE,
	APPLY_PAGE,
	HELP_PAGE,
	MORE_INFO_PAGE,
	FACTORY_RESET_PAGE,
	TRIGGERS_SELECT_PAGE,
	TRIGGERS_OUTPUT_PAGE,
	TRIGGERS_TEST_PAGE,
	CONFIG_TRIGGERS_PAGE,
} page_number_t;

enum machine_signals_t {
	TICK_SIG = SIG_USER,
	FRAME_TICK_SIG,
	TOUCH_SIG,
	TRACK_SIG,
	PRE_FRAME_TICK_SIG,
	POST_FRAME_TICK_SIG,
	SEND_ALIVE_CAN_PACKET,
	GOTO_PAGE_SIG,
    CAN_TICK_SIG,
    SWITCH_STATUS2_FROM_CAN_SIG,
    SEND_PRO_CAN_PACKET,
    RAW_TOUCH_SIG,
    
    MAX_PUB_SIG,
};
  
enum tTags_enum {
	NO_TAG = 0,
	SWITCH_0 = 1,
	SWITCH_1 = 2,
	SWITCH_2 = 3,
	SWITCH_3 = 4,
	SWITCH_4 = 5,
	SWITCH_5 = 6,
	SWITCH_6 = 7,
	SWITCH_7 = 8,
	CONFIG_LEFT,
	CONFIG_RIGHT,
	CONFIG_SETUP,
	SETTINGS,
	SPOD_LOGO_TOUCH,
	SLEEP,
	MSTROBE,
	MFLASH,
    TEST_LOGO_TOUCH,
	TRIGGERS_TAG,
	SWITCH_LABEL_TAG,
	HOME_TAG,
	BACK_TAG,
	ADVANCED_TAG,
	CLEAR_TAG,
	TEST_TAG,
	APPLY_TAG,
	HELP_TAG,
	TOGGLE_TAG,
	MOMENTARY_TAG,
	LONG_PRESS_TAG,
	DOUBLE_PRESS_TAG,
	CIRCUIT_TAG,
	CIRCUIT_TAG2,
	CIRCUIT_TAG3,
	CIRCUIT_TAG4,
	CIRCUIT_TAG5,
	CIRCUIT_TAG6,
	CIRCUIT_TAG7,
	CIRCUIT_TAG8,
	CIRCUIT_TAG9,
	CIRCUIT_TAG10, // 40
	CIRCUIT_TAG11,
	CIRCUIT_TAG12,
	CIRCUIT_TAG13,
	CIRCUIT_TAG14,
	CIRCUIT_TAG15,
	CIRCUIT_TAG16,
	CIRCUIT_TAG17,
	CIRCUIT_TAG18,
	CIRCUIT_TAG19, // 50
	CIRCUIT_TAG20,
	CIRCUIT_RIGHT,
	TRIGGER_TAG,
	TRIGGER_TAG2,
	TRIGGER_TAG3,
	TRIGGER_TAG4,
	TRIGGER_TAG5,
	TRIGGER_TAG6,
	TRIGGER_TAG7,
	TRIGGER_TAG8,
	ONOFF_TAG,
	STROBE_TAG,
	WIG_TAG,
	WAG_TAG,
	PRIMARY_TAG,
	SECONDARY_TAG,
	MORE_INFO_TAG,
	FACTORY_RESET_TAG,
	BACKLIGHT_TAG,
	TRIGGER_SIDE_TAG,
	IGNITION_TAG,
	LOCKOUT_TAG,
	PASSWORD_TAG,
	STROBE_FREQ_TAG,
	WIGWAG_FREQ_TAG,
	BL_RED_TAG,
	BL_GRN_TAG,
	BL_BLU_TAG,
	ABORT_TAG,
	CONFIRM_TAG,
	CONFIG_TRIGGERS_TAG,
	TRIGGER_MODE_TAG,
	TRIGGER_POLARITY_TAG,
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
	FREQUENCY_SLIDER,
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


typedef struct {
	uint32_t regTouchTransformA;
	uint32_t regTouchTransformB;
	uint32_t regTouchTransformC;
	uint32_t regTouchTransformD;
	uint32_t regTouchTransformE;
	uint32_t regTouchTransformF;
    uint32_t initialized;
	uint32_t crc;
} tCalibrate;

typedef struct Bitmap_header {
	uint8_t Format;
	int16_t Width;
	int16_t Height;
	int16_t Stride;
	int32_t Arrayoffset;
} tBitmap_header;

typedef struct Button_label {
	char line1[10];
	char line2[10];
	char line3[10];
} tButtonLabel;

typedef struct Config {
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

	///////////////////////////////////////////////////// added for spod2.0
	bool wakeFromHib;
	bool is24v_not12;
	bool isDeepSleepDisabled;
	bool isLockoutOn;
	uint8_t sourceAdrMask;
	tButtonLabel buttonLabels[64];
	uint8_t iconIsOn[64];
} tConfig;


extern tCalibrate calibrate;
extern tConfig settings;

extern void DisplaySleep(bool isSleep);

extern char ScratchArray[100];
extern char ScratchArray1[20];
extern char ScratchArray2[20];
extern uint8_t configureIndex;
extern uint8_t cfgindex;

extern uint8_t sourceAdrMask;

extern double voltage;
extern uint8_t temperature;

extern bool isConnected;
extern bool authGood;


extern bool globalNeedsSleepReset;
extern bool globalNeedsWake;

extern const tButtonLabel buttonDefaultLabels[64];
extern float switchCurrent[64];

extern uint32_t iconGpuBaseAddr;

extern bool isDeepSleepDisabled;

extern machineEvent newEvt;


int iSqrt(int x);

void crc8(unsigned char *crc, unsigned char m);

void drawInvisibleButton( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn);
unsigned int rand_interval(unsigned int min, unsigned int max);
void drawBatteryMeter(float range);
void drawTemperatureMeter(float range, bool fah);
void drawImage(const tBitmap_header header[], int16_t x, int16_t y, float sX, float sY, sTagXY touch, uint8_t tagVal, uint32_t imageId);
void drawText(int16_t x, int16_t y, uint8_t font, const char *line);
void drawTextColor(int16_t x, int16_t y, uint8_t font, const char *line, uint32_t color);
void drawTriText(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, bool isOn, uint32_t options, uint32_t color, bool offset);
void drawSingleTriText( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, bool isOn, uint32_t options, uint32_t color);
void drawButtonSmall( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, uint32_t isOn, float current);
void drawButton( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn);
void drawButton2(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line, uint8_t tagVal, bool on, uint32_t on_color);
void drawButtonColor( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn, uint32_t onColor, uint32_t offColor);
void drawSwitchCurrent( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, float current);
uint16_t normalizeSlider(uint16_t val);
void drawSlider( int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t val, sTrackTag touch, uint8_t tagVal);
void drawPercentGauge( int16_t x, int16_t y, const char * label, uint8_t font, uint32_t val);
void drawBatteryVoltage( int16_t x, int16_t y, const char * label, uint8_t font, double val);
void drawNum( int16_t x, int16_t y, uint8_t font, uint32_t color, double val);
void drawInt( int16_t x, int16_t y, uint8_t font, uint32_t color, uint32_t val);
void drawSourceNum( int16_t x, int16_t y, const char * label, uint8_t font, double val);

float celsiusToFahrenheit(uint8_t cels);

void drawTempVal( int16_t x, int16_t y, const char * label, uint8_t font, uint8_t val, bool fah);
void drawVersion( int16_t x, int16_t y, const char * label, uint8_t font, char* version);
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
extern void loadIcon(uint8_t cindex, uint8_t* data);//[512], bool clr);

extern stnext canComm(int signal, void* params);
extern stnext mainIdle(int signal, void* params);
extern stnext onHomePage(int signal, void* params);
extern stnext onConfigSetupPage(int signal, void* params);
extern stnext onSwitchSetupPage(int signal, void* params);
extern stnext onOutputSetupPage(int signal, void* params);
extern stnext onApplyPage(int signal, void* params);
extern stnext onTestPage(int signal, void* params);
extern stnext onSettingsPage(int signal, void* params);
extern stnext onConfigurePage(int signal, void* params);
extern stnext onSwitchOptionsPage(int signal, void* params);
extern stnext bleComm(int signal, void* params);
extern stnext onInputsPage(int signal, void* params);
extern stnext onFactoryResetPage(int signal, void* params);
extern stnext onAutoTriggerSelectPage(int signal, void* params);
extern stnext onAutoTriggerOutputPage(int signal, void* params);
extern stnext onAutoTriggerTestPage(int signal, void* params);
extern stnext onConfigTriggersPage(int signal, void* params);

extern void printByteArray(char* header, uint8_t* addr, uint8_t num);

extern void saveCalibration();
extern void loadCalibration();

#endif /* MAIN_H_ */
