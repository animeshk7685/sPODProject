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
#ifndef __MAIN_H__
#define __MAIN_H__

#define I2C_CLK_PIN 3
#define I2C_DAT_PIN 4

#define DEVICE_TYPE SWITCH_HD_V1

#define NUMBER_OF_TICKS		48000  // 1ms

#define WDT_INTERVAL_1S     250u                       /* millisecond */ // TODO: deprecated (currently unused)

#define DEEP_SLEEP_ENABLED  (1)     // leave on, disable in pro-app settings
#define IS_DEFAULT_PRO      (0)     // firmware defaults to pro
#define SW0 0x08
#define SW1 0x10
#define SW2 0x20
#define SW3 0x40
#define SW4 0x80
#define SW5 0x01
#define SW6 0x02
#define SW7 0x04

#define RED         0
#define GREEN       1
#define BLUE        2
#define INDICATOR   3
#define LED_ALL     4
#define LED_ONOFF   5
#define AUTO        6

#define RELEASED (0)
#define PRESSED (1)
#define HELD (2)

#define MOMENTARY (1)
#define TOGGLE (0)

#define SLEEP_MIN   120
//#define SLEEP_MIN   (1)

#define     DEEP_SLEEP_MS  21600000 // 6 hrs
//#define     DEEP_SLEEP_MS  120000  // 2 min
//#define     DEEP_SLEEP_MS  15000  // 15 sec

#define VOLT_WAKE_THRESH     (30)   // ~1.75v

#define VOLT_AVG_SAMPLE_S   (60)    // total samples in seconds
#define VOLT_AVG_SAMPLE_S1  (30)    // number of seconds of oldest samples to avg for compare
#define VOLT_AVG_SAMPLE_S2   (10)    // num of most recent seconds to avg for reported average

#define PASS_NEW    0
#define PASS_CLEAR  1
#define UNSECURED   0
#define SECURED     1


typedef struct switches
{
/*000*/    uint32_t links;

/*004*/    uint8_t type;
/*005*/    uint8_t avail1;
/*006*/    uint8_t avail2;
/*007*/    uint8_t avail3;

/*008*/    uint8_t isDimmable;
/*009*/    uint8_t isStrobeOrFlash;
/*00A*/    uint8_t strobeOn;
/*00B*/    uint8_t strobeOff;
}tSwitches;

typedef struct settings
{
/*000*/    uint8_t appSourceAddress;
/*001*/    uint8_t sourceAddress;
/*002*/    uint8_t isProMode;
/*003*/    uint8_t noDeepSleep;

/*004*/    int32_t passkey;
/*008*/    uint32_t backlightIntensity[3];
/*014*/    uint32_t indicatorIntensity;
    
/*018*/    tSwitches switches[64]; // 12*64=0x300
    
/*318*/    uint8_t sleepTimer;
/*319*/    uint8_t isSecChng;
/*31A*/    uint8_t isWakeFromIgn;
/*31B*/    uint8_t isWritable;
    
/*31C*/    uint8_t isWakeFromDS;
/*31D*/    uint8_t avail[0x3FC-0x31D];
/*3FC*/    uint32_t crc;
}tSettings;


extern uint8_t cfgindex;

extern tSettings* settings;

extern bool needsWakeupVavgReset;

extern void saveSettings();
extern void readSettings();
extern void factorySettings();

extern bool send_heartbeat(void* unused);
extern bool dim(void* unused);
extern void rs485_receive();

#endif
/* [] END OF FILE */
