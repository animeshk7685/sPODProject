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
#include <project.h>
#include "pt.h"
#include "timer.h"
#include "mcpCan.h"
#include "myPrintf.h"
#include "HandleLowPower.h"
#include "ota_mandatory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define DEEP_SLEEP_ENABLED  (1)     // leave on, disable in pro-app settings
#define IS_DEFAULT_PRO      (0)     // firmware defaults to pro

void saveSettings();

static uint32 crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32 crc32(uint32 crc, const void *buf, size_t size)
{
	const uint8_t *p;

	p = buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}

void StackEventHandler( uint32 eventCode, void *eventParam);

uint32 millis(void);

float generateRandom(void);

unsigned int rand_interval(unsigned int min, unsigned int max);
void generatePasskey(uint8 clear);
void writePassKey(uint32 thisKey);
void write_millis(uint32_t ms);
void updateAuth(void);

//1ms
#define NUMBER_OF_TICKS		 48000

#define EEPROM_NUM_ROWS     (2)
#define EEPROM_BYTE_SIZE                (CY_FLASH_SIZEOF_ROW * EEPROM_NUM_ROWS)

const uint8_t eepromBlock[EEPROM_BYTE_SIZE] CY_ALIGN(CY_FLASH_SIZEOF_ROW) = {};

#define EEPROM_FLASH_ADDR_ROW           (((int)eepromBlock - CY_FLASH_BASE) / CY_FLASH_SIZEOF_ROW)
#define EEPROM_FLASH_ADDR               (EEPROM_FLASH_ADDR_ROW * CY_FLASH_SIZEOF_ROW)

struct state {
  char *name;
  struct pt pt;
  struct timer timer;
};

typedef struct switches
{
    uint32_t links;
    uint8_t type;
    uint8_t isDimmable;
    uint8_t isStrobeOrFlash;
    uint8_t strobeOn;
    uint8_t strobeOff;
    
}tSwitches;

typedef struct settings
{
    uint8_t appSourceAddress;
    uint8_t sourceAddress;
    int32_t passkey;
    uint32_t backlightIntensity[3]; //3 bytes
    uint32_t indicatorIntensity; //1 bytes
    
    tSwitches switches[32];
    
//    uint8_t switchType[32];// 1 bytes
//    uint8_t switchIsDimmable[32]; //1 bytes
//    uint32_t switchLinks[32]; //8 bytes
//    uint8_t switchStrobeOrFlash[32]; //2 bytes
//    uint8_t switchStrobeOn[32];
//    uint8_t switchStrobeOff[32];
    
    uint8_t sleepTimer;
    uint8_t isSecChng;
    uint8_t isWakeFromIgn;
    
    uint8_t isProMode;
    uint8_t noDeepSleep;
    uint8_t isWritable;
    
    uint8_t isWakeFromDS;
    
    uint32_t crc;
    
}tSettings;



enum sPOD_Device_Type {
    BANTAM_V1,
    TOUCHSCREEN_V1,
    SWITCH_HD_V1,
    SOURCE_LT_V1
};

#define DEVICE_TYPE SWITCH_HD_V1

//uint32 passkey = 1234;

mcpCan can;

uint32 ticks = 0;
uint32 isrTicks = 0;
uint32 ioAddress1 = 0x20;
uint32 ioAddress2 = 0x24;
uint8 isConnected = 0;
uint8 sendNotifications = 0;
uint8 pairingNeeded = 0;
uint8 pinGenerationNeeded = 0;
uint8 pinClearNeeded = 0;
uint16 tempPin = 0;

uint8 i2cBuffer[3];

uint8_t ledStatus[8];
uint8 switchStatus[8];
uint8 buttonStatus[8];

uint8 switchIsFlashing[8];
uint8 switchWasHeld[8];
uint8 switchIsDimming[8];
uint8 dimValue[32];
uint8 dimDirection[8];


uint8 switchShort[8];

uint8 bleRxBuffer[32];
uint8 bleTxBuffer[20];

uint8 runHardwareTest = 1;

uint8 isAwake = 1;

bool authGood = false;
bool isTempWritable = false;


uint8 saveSettingsFlag = 0;

static bool globalNeedsSleepReset = false;

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

//#define EEPROM_ADDR 130944

#define RELEASED (0)
#define PRESSED (1)
#define HELD (2)

#define MOMENTARY (1)
#define TOGGLE (0)

//tSettings* eeprom = (void *)EEPROM_ADDR;
tSettings settingsStruct;
tSettings* settings = &settingsStruct;

unsigned int rand_interval(unsigned int min, unsigned int max) 
{
	unsigned int r;
	const unsigned int range = 1 + max - min;
	const unsigned int buckets = RAND_MAX/range;
	const unsigned int limit = buckets * range;

	do
	{
		r = rand();
	}while (r >= limit);

	return min + (r/buckets);
}

#define PASSKEY_OFFSET                  0
#define PASS_INIT_OFFSET                1

CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};

#define PASS_DBG_TIMEOUT    30000       //    (30 * 1000)
#define PASSKEY_TIMEOUT     90000      //(1.5 * 60 * 1000)

static uint32_t passkeyTimer = 0;
static bool passkey_init = false;

void generatePasskey(uint8 clear)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    myPrintf("Prepare to wipe bonding\n");
    
    if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
    {
        CyBle_GappStopAdvertisement ();
    } 
        
    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
    
    while (CyBle_GetState() == CYBLE_STATE_CONNECTED)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    apiResult = CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
    
    if(apiResult != CYBLE_ERROR_OK)
        myPrintf("Err CyBle_GapRemoveDeviceFromWhiteList(): %d\n", apiResult);
        
    while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));

    if(clear)
    {
        settings->passkey = -1;
        CyBle_GapFixAuthPassKey(1, 0);
    }
    else
    {
//        if(settings->passkey == 0 || settings->passkey == 1234)
//        {
            settings->passkey = 0;
            CyBle_GapFixAuthPassKey(1, 0);
            CyBle_GapFixAuthPassKey(0, 0);
//        }
//        else
//        {
//            
//            CyBle_GapFixAuthPassKey(1, settings->passkey);
//        }
//        
    }
    passkey_init = true;
    
//    CyBle_GapFixAuthPassKey(1, settings->passkey);
    writePassKey(settings->passkey);
    passkeyTimer = millis();
    
    
    myPrintf("Generate new passkey: %d \n", settings->passkey);
        
    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
    if(apiResult != CYBLE_ERROR_OK)
        myPrintf("Err CyBle_GappStartAdvertisement(): %d\n", apiResult);
}

void write_millis(uint32_t ms)
{
    //uint32_t ms = millis();
    
    myPrintf("(");
    
    if(ms/3600000)
    myPrintf("%d:",(ms/3600000));
    
    ms = ms % 3600000;
    
    if(ms/60000)
    myPrintf("%d:",(ms/60000));
    
    ms = ms % 60000;
    
    if(ms/10000 == 0)
    myPrintf("0");
    
    myPrintf("%d.",(ms/1000));
    
    ms = ms % 1000;
    
    if(ms/100 == 0)
    myPrintf("0");
    
    if(ms/10 == 0)
    myPrintf("0");
    
    myPrintf("%d)\n",ms);
    
}


uint8 oneHot(uint8 sw) {

    switch (sw) {
        case 0:
            return SW0;
            break;
        case 1:
            return SW1;
            break;
        case 2:
            return SW2;
            break;
        case 3:
            return SW3;
            break;
        case 4:
            return SW4;
            break;
        case 5:
            return SW5;
            break;
        case 6:
            return SW6;
            break;
        case 7:
            return SW7;
            break;
        default:
            return 0;
            break;
    }
};

uint8 indexPos(uint8 pos) {

    switch(pos) {
        case SW0:
            return 0;
            break;
        case SW1:
            return 1;
            break;
        case SW2:
            return 2;
            break;
        case SW3:
            return 3;
            break;
        case SW4:
            return 4;
            break;
        case SW5:
            return 5;
            break;
        case SW6:
            return 6;
            break;
        case SW7:
            return 7;
            break;
        default:
            return 255;
            break;
    }
};

uint32 millis(){
    return ticks;
}


void i2cWriteWait(uint32 address, uint8 * buffer, uint32 length, uint32 mode)
{
    I2C_I2CMasterClearStatus();
    I2C_I2CMasterWriteBuf(address, buffer, length, mode);
    for(;;) 
    {    
        if(0u != (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))    
        {              
            /* Transfer complete. Check Master status to make sure that transfer completed without errors. */  
            break;    
        } 
    }
}

void i2cReadWait(uint32 address, uint8 * buffer, uint32 length, uint32 mode)
{
    I2C_I2CMasterReadBuf(address, buffer, length, mode);
    for(;;) 
    {    
        if(0u != (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT))    
        {              
            /* Transfer complete. Check Master status to make sure that transfer completed without errors. */  
            break;    
        } 
    }
}

uint8 getStatus(uint8 *statusArray)
{
    int i;
    uint8 status = 0;
    
    for(i = 0; i < 8; i++)
    {
        if(statusArray[i] > 0)
        {
            status = status | (1 << i);   
        }
    }
    
    return status;
}

void updateLinks(uint8_t index, uint8_t valAddr, uint8_t val)
{
    if(index >= 32 || valAddr >= 4)
        return;
    
    //uint8_t offset = index & 0xF7;  // same as: index - (index % 8)
    
    uint8_t offset = valAddr * 8;
    uint32_t mask = ~(0xFF << offset);
    uint32_t lastLink = settings->switches[index].links;
    
    settings->switches[index].links = (lastLink & mask) | (val << offset);
}

void updateLinks2(uint8_t index, uint8_t val)
{
    updateLinks(index, index/8, val);
}

uint8_t getLinks(uint8_t index, uint8_t addr)
{
    return (settings->switches[index].links >> (addr * 8)) & 0xFF;
}

uint8_t getLinks2(uint8_t index)
{
    return getLinks(index, index /8);
}

void updateButtonStatusLeds() // Blue leds
{
    uint8 buffer[2];
    buffer[0] = 0x02;
    buffer[1] = getStatus(buttonStatus);   // &buttonStatus[8 * settings->sourceAddress]
    i2cWriteWait(ioAddress2, buffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
}

void updateSwitchStatusLeds() // Red leds
{
    uint8 buffer[2];
    buffer[0] = 0x02;
    buffer[1] = getStatus(switchStatus);   // &switchStatus[8 * settings->sourceAddress]
    i2cWriteWait(ioAddress1, buffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
}

void updateLedStatusLeds() // White leds
{
    uint8 buffer[2];
    buffer[0] = 0x03;
    buffer[1] = getStatus(ledStatus);   // &ledStatus[8 * settings->sourceAddress]
    i2cWriteWait(ioAddress1, buffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
}

void updatePWM(uint8_t ch, uint8_t level)
{
    if(ch == AUTO)
    {
        PWM_Red_WriteCompare(settings->backlightIntensity[RED]);
        PWM_Green_WriteCompare(settings->backlightIntensity[GREEN]);
        PWM_Blue_WriteCompare(settings->backlightIntensity[BLUE]);
        PWM_Indicator_WriteCompare(settings->indicatorIntensity);
    }
    else
    {
        if(level > 100)
        {
            level = 100;
        }
        
        switch(ch)
        {
            case RED:
                PWM_Red_WriteCompare(level);
            break;
            case GREEN:
                PWM_Green_WriteCompare(level);
            break;
            case BLUE:
                PWM_Blue_WriteCompare(level);
            break;
            case INDICATOR:
                PWM_Indicator_WriteCompare(level);
            break;
            case LED_ALL:
                PWM_Red_WriteCompare(level);
                PWM_Green_WriteCompare(level);
                PWM_Blue_WriteCompare(level);
                PWM_Indicator_WriteCompare(level);
            break;
            case LED_ONOFF:
                if(level == 0)
                {
                    PWM_Red_Sleep();
                    PWM_Green_Sleep();
                    PWM_Blue_Sleep();
//                    if(settings->isProMode == 0)
//                        PWM_Indicator_Sleep();
                }
                else
                {
                    PWM_Red_Wakeup();
                    PWM_Green_Wakeup();
                    PWM_Blue_Wakeup();
//                    PWM_Indicator_Wakeup();
                }
            break;
        }
    }
}

volatile uint8 wdtIsrCount = 0;

/* ISR prototype declaration */
CY_ISR_PROTO(USER_ISR);

/* User ISR function definition */
CY_ISR(USER_ISR)
{
    ticks++;
    
    if(ticks % 50 == 0)
    {
        updatePWM(AUTO,0);
    }
}

CY_ISR_PROTO(WDT_ISR);

CY_ISR(WDT_ISR)
{     
//    myPrintf("WDT %d: ", wdtIsrCount);
//    write_millis(millis());
    
//    resetWatchdog();
    
//    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
    
    if(wdtIsrCount == 0)
    {
        CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);  
        
//        CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
    }
    else
    {        
        ISR_WDT_Stop();
    }
    wdtIsrCount++;
}

void configureIO(){
    
    int i = 0;
    
    for(i = 0; i < 8; i++)
    {
        switchIsDimming[i] = 0;
        switchWasHeld[i] = 0;
        switchIsFlashing[i] = 0;
        dimDirection[i] = 1;
        switchStatus[i] = 0;
    }
    
    for(i = 0; i < 32; i++)
    {
        dimValue[i] = 0xFE;
    }
    
    //config register
    i2cBuffer[0] = 0x06;
    
    //set port 0 to output
    i2cBuffer[1] = 0x00;
    
    //set port 1 to output
    i2cBuffer[2] = 0x00;
    
    i2cWriteWait(ioAddress1, i2cBuffer, 3, I2C_I2C_MODE_COMPLETE_XFER);
    
    //set port 1 to input
    i2cBuffer[2] = 0xFF;
    
    i2cWriteWait(ioAddress2, i2cBuffer, 3, I2C_I2C_MODE_COMPLETE_XFER);

    //output register
    i2cBuffer[0] = 0x02;
    
    //set port 0 to high
    i2cBuffer[1] = 0x00;
    
    //set port 1 to high
    i2cBuffer[2] = 0x00;
    
    //write to port
    i2cWriteWait(ioAddress1, i2cBuffer, 3, I2C_I2C_MODE_COMPLETE_XFER);
    i2cWriteWait(ioAddress2, i2cBuffer, 3, I2C_I2C_MODE_COMPLETE_XFER);
}

void processButtonUpdate(uint8 index, uint8 status, uint16 heldTime)
{
    uint8 t1 = 0xFF;
    uint8 t2 = 0x00;
    uint8 i;
    uint8 needsUpdate = 0;
    
    uint8_t swIndex = index + 8 * settings->sourceAddress;
    
    /*
    if(settings->switchStrobeOrFlash[index]  > 0)
    {
        if(settings->switchStrobeOrFlash[index]  == 1)
        {
            t1 = 10;
            t2 = 10;
        }
        else
        {
            t1 = 1;
            t2 = 4;
        }
    }
    */
    
    switch(status)
    {
        case RELEASED:
        {
            if(settings->switches[swIndex].type == MOMENTARY)
            {
                switchStatus[index] = 0;
                switchIsFlashing[index] = 0;
                needsUpdate = 1;
            }
            else if(!switchIsDimming[index] && !switchWasHeld[index])
            {
                switchStatus[index] = !switchStatus[index];
                
                if(switchIsFlashing[index])
                    switchIsFlashing[index] = switchStatus[index];
                    
                needsUpdate = 1;
                
            }                       
          
            switchIsDimming[index] = 0;
            switchWasHeld[index] = 0;
            
            break;
        }
        case PRESSED:
        {
                   
            if(settings->switches[swIndex].type == MOMENTARY)
            {
                needsUpdate = 1;
                switchStatus[index] = 1;
            }
            
            switchIsDimming[index] = 0;
            
            break;
        }
        case HELD:
        {
            
            if(heldTime > 1250 && settings->switches[swIndex].isDimmable && !switchIsDimming[index]){
                switchIsDimming[index] = 1;
                dimDirection[index] = !dimDirection[index];
                switchStatus[index] = 1;
                needsUpdate = 1;
                switchIsFlashing[index] = 0;
            
            }
            else
            {
                if(!switchWasHeld[index] && settings->switches[swIndex].isStrobeOrFlash > 0)
                {
                    if(settings->isProMode && settings->switches[swIndex].strobeOff != 0)
                    {
                        t1 = settings->switches[swIndex].strobeOn;
                        t2 = settings->switches[swIndex].strobeOff;
                    }
                    else if(settings->switches[swIndex].isStrobeOrFlash == 1)
                    {
                        t1 = 10;
                        t2 = 10;
                    }
                    else
                    {
                        t1 = 1;
                        t2 = 4;
                    }
                    
                    switchStatus[index] = !switchStatus[index]; 
                            
                    needsUpdate = 1;
          
                    switchIsDimming[index] = 0;
                    switchWasHeld[index] = 1;
                    switchIsFlashing[index] = 1;
                }
            }

            
            
            break;   
        }
    
    }
    
    if(!needsUpdate)
    {
        return;   
    }
        
    uint8 buffer[] = {
        0x80 + settings->sourceAddress, 
        oneHot(index), 
        switchStatus[index] ? (settings->switches[swIndex].isDimmable ? dimValue[swIndex] : 0xFE) : 0x00, 
        t1, 
        t2
    };
    if(oneHot(index) != 0)
        mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer);
    updateSwitchStatusLeds();
    
    if(settings->switches[swIndex].links == 0)
        return;
    
    for(i = 0; i < 32; i++){
        if(i != swIndex)
        {
            if((settings->switches[swIndex].links & (1 << i)) > 0)
            {
                t1 = 0xFF;
                t2 = 0x00;
                
                if(settings->switches[i].isStrobeOrFlash > 0)
                {
                    if(settings->isProMode && settings->switches[i].strobeOff != 0)
                    {
                        t1 = settings->switches[i].strobeOn;
                        t2 = settings->switches[i].strobeOff;
                    }
                    else if(settings->switches[i].isStrobeOrFlash == 1)
                    {
                        t1 = 10;
                        t2 = 10;
                    }
                    else
                    {
                        t1 = 1;
                        t2 = 4;
                    }
                }
                
                if(i/8 == settings->sourceAddress)
                    switchStatus[i%8] = switchStatus[index];
                
                CyDelay(5);
                
                uint8 buffer2[] = {
                    0x80 + (i/8), 
                    oneHot(i%8), 
                    switchStatus[index] ? (settings->switches[i].isDimmable ? dimValue[i] : 0xFE) : 0x00, 
                    t1, 
                    t2
                };
                if(oneHot(i%8) != 0)
                    mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer2);
            }
        }
    }
    
    updateSwitchStatusLeds();
}

void configureCan()
{
    
    STBY_Write(0);
    CyDelay(1);
    uint8 res;
    res = mcpCanInit(CAN_250KBPS);
    if(res != MCP2515_OK)
    {
        UART_PutString("CAN Config Error...\n");
        while(1){};
    }
}

#define WDT_INTERVAL_1S             250u                       /* millisecond */
#define ILO_FREQ                    32768  

void InitWatchdog(uint16 reset_interval)
{
    
    wdtIsrCount = 0;
    
    /*==============================================================================*/
    /* configure counter 0 for system reset                                         */
    /*==============================================================================*/
    /* Counter 0 of watchdog generates peridically interrupt and a 
       reset is generated on the the third unhandled interrupt */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT_RESET);
    /* Set interval as desired value */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, (((uint32)(reset_interval * ILO_FREQ))/1000));
    /* clear counter on match event */
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
    
    /*==============================================================================*/
    /* enable watchdog                                                              */
    /*==============================================================================*/
    /* enable the counter 0 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    /* check if counter 0 is enabled, otherwise keep looping here */
    while(!CySysWdtReadEnabledStatus(CY_SYS_WDT_COUNTER0));
    
        /* connect ISR routine to Watchdog interrupt */
    ISR_WDT_StartEx(WDT_ISR);
    /* set the highest priority to make ISR execute in all condition */
    ISR_WDT_SetPriority(0);
}

void resetWatchdog(void)
{
    CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);  
            /* clearing watchdog counter requires several LFCLK cycles to take effect */
    CyDelayUs(150);
    
    wdtIsrCount = 0;
}

bool resetSleepTimer = false;

#define SLEEP_MIN   120
//#define SLEEP_MIN   (1)

#define     DEEP_SLEEP_MS  21600000 // 6 hrs
//#define     DEEP_SLEEP_MS  120000  // 2 min
//#define     DEEP_SLEEP_MS  15000  // 15 sec

PT_THREAD(checkDeepSleep(struct state *s))
{
    static int32 deepSleepTimerMillis = DEEP_SLEEP_MS;
    static bool inDeepSleep = false;
    
    PT_BEGIN(&s->pt);
    
    while(1)
    {        
        timer_set(&s->timer, 10);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));

        #if (DEEP_SLEEP_ENABLED == 0)
            continue;   
        #endif
        
        if(settings->noDeepSleep != 0)
        {
            continue;
        }
        
        deepSleepTimerMillis -= 10;
        
//        if(isAwake && settings->sleepTimer > 0)
//        {
//            inDeepSleep = false;
//            deepSleepTimerMillis = DEEP_SLEEP_MS;
//        }
        
        if(globalNeedsSleepReset)
        {
            globalNeedsSleepReset = false;
            deepSleepTimerMillis = DEEP_SLEEP_MS;
        }        
        
        if(deepSleepTimerMillis <= 0 && !inDeepSleep)
        {
//            STBY_Write(1);
            
          
            
            i2cBuffer[0] = 0x02;            // turn off red & white leds
            i2cBuffer[1] = 0x00;
            i2cBuffer[2] = 0x00;
            i2cWriteWait(ioAddress1, i2cBuffer, 3, I2C_I2C_MODE_COMPLETE_XFER);
            
            i2cBuffer[0] = 0x02;            // turn off blue leds
            i2cBuffer[1] = 0x00;
            i2cWriteWait(ioAddress2, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
            
            updatePWM(LED_ONOFF, 0);        // sleep pwm modules
            
            PWM_Indicator_Sleep();
            
            mcpCanSleep(true);              // sleep CAN ic
            SPI_Stop();
            
            CyBle_EnterLPM(CYBLE_BLESS_HIBERNATE);
            CyBle_Stop();                   // sleep BLE
            
            
            
            inDeepSleep = true;
            myPrintf("Deep Sleep... %d\n", millis());
            
            CyDelay(10);
            
//            InitWatchdog(WDT_INTERVAL_1S);
            InitWatchdog(50);
            
            while(inDeepSleep)
            {
                resetWatchdog();
                CySysPmDeepSleep();     // Sleep here until wdt trips, then continue
                
                i2cBuffer[0] = 0x01;      
                i2cWriteWait(ioAddress2, i2cBuffer, 1, I2C_I2C_MODE_COMPLETE_XFER);
                i2cReadWait(ioAddress2, &i2cBuffer[1], 1, I2C_I2C_MODE_COMPLETE_XFER);
                
                if(i2cBuffer[1] != 0xFF)    // check if button pressed evey ~250ms
                {
//                    resetWatchdog();
                    inDeepSleep = false;
                }
                
            }
            
            myPrintf("wake/reset %d\n", millis());
            
            settings->isWakeFromDS = 1;     // store flag in flash indicate no led pattern on wake
                saveSettings();
            
            CyDelay(25);
            
            CySoftwareReset();
            
        }
    }
  
    PT_END(&s->pt);
}

PT_THREAD(sendAlivePacket(struct state *s))
{
    static uint8 buffer[] = {0x90, 0x7E, 0x59, 0x00, 0x00};
    
    static int j = 0;
    
    PT_BEGIN(&s->pt);
    while(1)
    {        
        timer_set(&s->timer, 500);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
        
        for(j = 0 ; j < 4 ; j++)
        {
//            buffer[0] = 0x90 + settings->sourceAddress;
            buffer[0] = 0x90 + j;
            mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer);
        
//            myPrintf("A%d(%d): %x %x %x %x %x | ", j, millis(), buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
            
//            timer_set(&s->timer, 500);
            timer_set(&s->timer, 125);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        }
        
//        myPrintf("\n");
        
//        if(isConnected)
//        {
//            buffer[0] = 0x90 + settings->appSourceAddress;
//            mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer);
//        }
        //myPrintf("Send Alive...\n");
    }
  
    PT_END(&s->pt);
}

#define VOLT_WAKE_THRESH     (30)   // ~1.75v

#define VOLT_AVG_SAMPLE_S   (60)    // total samples in seconds
#define VOLT_AVG_SAMPLE_S1  (30)    // number of seconds of oldest samples to avg for compare
#define VOLT_AVG_SAMPLE_S2   (10)    // num of most recent seconds to avg for reported average

bool needsWakeupVavgReset = false;

//#define AVG_VOLT_SAMPLE_SIZE  30 

#define SYSTEM_PACKET   0xA0
#define SWITCH_PACKET   0x80
#define STATUS_PACKET   0xB0
#define DEBUG_PACKET    0x90
#define PRO_PACKET      0xC0

//float batVoltF(uint8_t voltI)
//{
//    return (((double)voltI * 5 / 255) * 3) + 0.7;
//}

uint8_t changeAddr = 0;
uint32_t lastChAddrTime = 0;

PT_THREAD(canRx(struct state *s))
{
    static uint8 rxBuffer[8];
    static uint8 bytesRead = 0;
    static int i;
    static CYBLE_API_RESULT_T                  bleApiResult;
    static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;
    
    static uint8_t voltAvgBuf[VOLT_AVG_SAMPLE_S] = {0};
    
    static uint16_t voltAvg = 0;
    
    static uint16_t voltAvgOlder = 0;
    
    static uint8_t voltAvgSecCnt = 0;
    static uint16_t voltAvgSecSum = 0;
    static uint32_t voltAvgSecLastTime = 0;
    
    static bool voltAvgNeedsInit = true;
    
//    static uint8 voltAvgBuf[AVG_VOLT_SAMPLE_SIZE];
//    static uint16 voltAvg = 0;
//    static uint16 voltAvg2 = 0;
//    static uint8 voltAvgPnt = 0;
    
//    static double voltage = 0;

    
    PT_BEGIN(&s->pt);
    while(1)
    {        
        PT_WAIT_UNTIL(&s->pt, CANINT_Read() == 0);
              

        uint8 status = mcpCanReadMsgBuf(&can, &bytesRead, rxBuffer);

        if(status == CAN_OK)
        {
            
            if(1)// || rxBuffer[0] == 0x80 || rxBuffer[0] == 0x81 || rxBuffer[0] == 0x82 || rxBuffer[0] == 0x83 || rxBuffer[0] == 0xA0)
            {
                if(rxBuffer[0] == (SWITCH_PACKET + settings->sourceAddress))
                {
                    uint8_t srcAdd = rxBuffer[0] & 0x03;
                    uint8_t index = indexPos(rxBuffer[1]);
                    if(index != 255)
                    {
                        //return;
                    
                    uint8_t swAdd = index + 8 * srcAdd;
                    
                    if(rxBuffer[2])
                    {
                        switchStatus[index] = 1;
                        
                        if(switchShort[index])
                        {
                            switchShort[index] = 0;
                            buttonStatus[index] = 0;
                        }
                    }
                    else
                    {
                        switchStatus[index] = 0;
                    }
                    
                    //dimValue[swAdd] = rxBuffer[2];        /// save dimValue from CAN bus? currently dimming from app/touchscreen are overwritten by hd
                    
                    if(!switchShort[index])// && !(changeAddr > 0 && ((changeAddr - 1) == index || lastChAddrTime == 0)))   /// flashing wierdly when changing addresses... 
                    {
                        updateSwitchStatusLeds();
                    }
                    
                    if(rxBuffer[3] != 0 && rxBuffer[4] != 0)
                    {
                        if(settings->switches[swAdd].strobeOn != rxBuffer[3] ||
                           settings->switches[swAdd].strobeOff != rxBuffer[4])
                        {
                            settings->switches[swAdd].strobeOn = rxBuffer[3];
                            settings->switches[swAdd].strobeOff = rxBuffer[4];
                            
                            saveSettings();
                            // save settings
                        }
                    }
                    }
                    
                } 
                else if (rxBuffer[0] == SYSTEM_PACKET)
                {
                    uint8_t localVolt = rxBuffer[1];
                    
                    if(voltAvgNeedsInit || needsWakeupVavgReset)
                    {
                        voltAvgNeedsInit = false;
                        needsWakeupVavgReset = false;
                        
                        for(int i = 0; i < VOLT_AVG_SAMPLE_S; i++)
                        {
                            voltAvgBuf[i] = localVolt;
                        }
                        
                        voltAvgOlder = localVolt;
                        voltAvg = localVolt;
                    }
                    
                    if((voltAvgOlder + VOLT_WAKE_THRESH) < localVolt)
                    {
                        if(settings->isWakeFromIgn)
                        {
                            resetSleepTimer = true;
                        }
                    }
                    
                    voltAvgSecSum += localVolt;
                    voltAvgSecCnt++;
                    
                    if((millis() - voltAvgSecLastTime) > 1000)
                    {
                        voltAvgSecSum /= voltAvgSecCnt;
                        
                        voltAvgOlder = 0;
                        voltAvg = 0;
                        
                        for(int i = 0; i < VOLT_AVG_SAMPLE_S; i++)
                        {
                            if(i < VOLT_AVG_SAMPLE_S - 1)
                            {
                                voltAvgBuf[i] = voltAvgBuf[i + 1];
                            }
                            else
                            {
                                voltAvgBuf[i] = voltAvgSecSum;
                            }
                            
                            if(i < VOLT_AVG_SAMPLE_S1)
                            {
                                voltAvgOlder += voltAvgBuf[i];
                            }
                            
                            if(i >= (VOLT_AVG_SAMPLE_S - VOLT_AVG_SAMPLE_S2))
                            {
                                voltAvg += voltAvgBuf[i];
                            }
                        }
                        
                        voltAvgOlder /= VOLT_AVG_SAMPLE_S1;
                        voltAvg /= VOLT_AVG_SAMPLE_S2;
                        
                        
                        voltAvgSecCnt = 0;
                        voltAvgSecSum = 0;
                        voltAvgSecLastTime = millis();
                    }
                    
                    rxBuffer[1] = voltAvg;      // for ble comm
                    
    			}
                else if (((rxBuffer[0] & 0xF0) == DEBUG_PACKET) && (rxBuffer[1] == 0x0A))
                {
//                    if(rxBuffer[2] == 0x0F)
                        //globalNeedsSleepReset = true;
                    
                }
                else if (rxBuffer[0] == (STATUS_PACKET + settings->sourceAddress))
                {
                    //uint8_t srcAdd = rxBuffer[0] & 0x03;
                    
                    if(rxBuffer[2] == 8)
                    {
                        uint8_t index = indexPos(rxBuffer[1]);
                        if(index != 255)
                            switchShort[index] = 1;
                    }
                }
                else if(rxBuffer[0] == PRO_PACKET)
                {
                    myPrintf("CAN: pro rec -> %x %x %x %x %x\n", rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3], rxBuffer[4]);
                    uint8_t srcAdd = rxBuffer[0] & 0x03;
                    uint8_t index = indexPos(rxBuffer[1]);
                    if(index != 255)
                    {
                        uint8_t swAdd = index + 8 * srcAdd;
                        
                        if(rxBuffer[2] & 0x20)
                        {
                            uint8_t linkPacketIndex = (rxBuffer[2] & 0xC0) == 0x40 ? 1 : 0;
                            
                            if(linkPacketIndex == 0)
                            {
                                updateLinks(swAdd, 1, rxBuffer[3]);
                                updateLinks(swAdd, 0, rxBuffer[4]);
                            }
                            else
                            {
                                updateLinks(swAdd, 3, rxBuffer[3]);
                                updateLinks(swAdd, 2, rxBuffer[4]);
                            }
                        }
                    }
                }
                           
                
                
                if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
                {
                    //iprintf("BLE Connected...\n");
                    
                    /* if stack is free, handle UART traffic */
                    if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
                    {
                        //iprintf("BLE Stack Free...\n");
                        
                        if(sendNotifications){
                            
                            //iprintf("BLE Notification Enabled...\n");
                            
                            bleTxBuffer[0] = 0x55;
                            bleTxBuffer[1] = bytesRead + 5;
                            bleTxBuffer[2] = 0x00;
                            
                            for(i = 0 ; i < bytesRead; i++)
                            {
                                bleTxBuffer[3 + i] = rxBuffer[i];
                            }
                            
                            uint32 crc = crc32(0, bleTxBuffer, bytesRead + 3);
                            
                            uint32 packetLength = bytesRead + 7;
                            
                            bleTxBuffer[packetLength - 4] = crc & 0xFF;
                            bleTxBuffer[packetLength - 3] = crc >> 8;
                            bleTxBuffer[packetLength - 2] = crc >> 16;
                            bleTxBuffer[packetLength - 1] = crc >> 24;
                            
                            
                            characteristicNotify.value.val = bleTxBuffer;
                            characteristicNotify.value.len = bytesRead + 7;
                            characteristicNotify.attrHandle = CYBLE_SPOD_COMM_CHAR_HANDLE;
                            
                            do
                            {
                                
                                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                                CyBle_ProcessEvents();
                            }
                            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                            
                        }
                    }
                }
            }
        }

        
        
        
        if(mcpCanCheckError() != CAN_OK)
        {
            mcpCanClearError();   
        }

    }
  
    PT_END(&s->pt);
}

void setArray8(uint8_t * array, uint8_t val)
{
    for (int i = 0; i < 8; i++)
    {
        array[i] = val;
    }
}

PT_THREAD(updateLeds(struct state *s))
{
    static int i = 0;
    static int flash = 0;
    //static int ledStatus = 0;
    static uint8 ledShort = 0;
    static uint8 updateRB = 0;
    
    //static uint8 passFlag = 0;
    
    PT_BEGIN(&s->pt);
  
    while(1)
    {
        //ledStatus = 0;
        setArray8(ledStatus, 0);
        
        ledShort++;
        ledShort = ledShort % 3;
        
        if(!ledShort)
        {
            flash = ~flash;
        }
        
//        if(passkeyTimer > 0 && !passFlag)
//        {
//            passFlag = 1;
//        }
//        else if(!passkeyTimer && passFlag)
//        {
//            passFlag = 0;    
//        }
        
        uint8_t srcIndex = 8 * settings->sourceAddress;
        
        for(i = 0; i < 8; i ++)
        {
            uint8_t i2 = i + srcIndex;
            
            if(changeAddr > 0 && (changeAddr - 1) == i)     // true until address indicator finished, blocks short pattern
            {
                if(lastChAddrTime == 0)
                {
                    lastChAddrTime = millis();
                    
                    //ledStatus = 0;
                    
                    for(int j = 0 ; j < 8 ; j++)    // clear leds
                    {
                        switchStatus[j] = 0;
                        buttonStatus[j] = 0;
                        ledStatus[j] = 0;
                    }
                }
                
                updateRB = 1;
                
                switchStatus[i] = 1;                // set leds for new addr
                buttonStatus[i] = 1;
                ledStatus[i] = 1;
                //ledStatus = ledStatus | (1 << i);
                
                
                if((millis() - lastChAddrTime) > 750)   // turn off after 0.75s
                {
                    lastChAddrTime = 0;
                    changeAddr = 0;
                    
                    switchStatus[i] = 0;
                    buttonStatus[i] = 0;
                    ledStatus[i] = 0;
                    //ledStatus = 0;
                    
                }
            }
            else if(switchShort[i])
            {
                updateRB = 1;
                
                switch(ledShort)
                {
                    case 0:     // RED
                        switchStatus[i] = 1;
                        buttonStatus[i] = 0;
                        ledStatus[i] = 0;
                    break;
                    case 1:     // BLUE
                        switchStatus[i] = 0;
                        buttonStatus[i] = 1;
                        ledStatus[i] = 0;
                    break;
                    case 2:     // WHITE
                        switchStatus[i] = 0;
                        buttonStatus[i] = 0;
                        ledStatus[i] = 1;
                        //ledStatus = ledStatus | (1 << i);
                    break;
                    default:
                        ledShort = 0;
                }
            }
            else if(switchStatus[i] > 0 && settings->switches[i2].isStrobeOrFlash > 0)
            {
                if(switchIsFlashing[i])
                {
                    if(flash)
                    {
                        //ledStatus = ledStatus | (1 << i);
                        ledStatus[i] = 1;
                    }
                }
                else
                {
                    //ledStatus = ledStatus | (1 << i);
                    ledStatus[i] = 1;
                }
                    
            }
        }
        
        if(passkeyTimer > 0)//passFlag)
        {
            //ledStatus = flash;
            setArray8(ledStatus, flash);
        }
        
//        i2cBuffer[0] = 0x03;
//        i2cBuffer[1] = ledStatus;
//        i2cWriteWait(ioAddress1, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
        
        updateLedStatusLeds();
        
        if(updateRB)        // Red & Blue leds only need to updated here in a short condition
        {
            updateRB = 0;
            updateButtonStatusLeds();
            updateSwitchStatusLeds();
        }
//        myPrintf("ts: %d \n", millis());
        
        timer_set(&s->timer, 300);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
    }
    
    PT_END(&s->pt);
}

void clearButton(uint8_t index)
{
    if(settings->switches[index + settings->sourceAddress * 8].type == MOMENTARY)
    {
        processButtonUpdate(index, RELEASED, 0);
    }
    else
    {
         buttonStatus[index] = 0;                    
    }
}

PT_THREAD(readButtons(struct state *s))
{
    static uint8 lastButtonStatus = 0xFF;
    static uint16 heldButtonCount[8] = {};
    static uint8 currentButtonStatus = 0xFF;
    static uint32 sleepTimerMillis = 0;
    
    static uint8 locked = 0;
    static uint8 lockUpdated = 0;
    
    static uint8 ledSaveR, ledSaveG, ledSaveB;
    uint8 changedBits = 0;
    
    PT_BEGIN(&s->pt);
    
    while(1)
    {        
        timer_set(&s->timer, 10);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        sleepTimerMillis += 10;
        
        i2cBuffer[0] = 0x01;      
        i2cWriteWait(ioAddress2, i2cBuffer, 1, I2C_I2C_MODE_COMPLETE_XFER);
        i2cReadWait(ioAddress2, &i2cBuffer[1], 1, I2C_I2C_MODE_COMPLETE_XFER);

        currentButtonStatus = i2cBuffer[1];
        uint8 i;
        
        //button state change
        changedBits = lastButtonStatus ^ currentButtonStatus;
        
        if(changedBits || resetSleepTimer)
        {
            globalNeedsSleepReset = true;
            resetSleepTimer = false;
            
            sleepTimerMillis = 0;
        }
        
        if(isAwake && (currentButtonStatus == 0x9F ||       // sleep buttons pushed or
                (settings->sleepTimer &&                                        // sleep timer on and
                (sleepTimerMillis > (settings->sleepTimer *  60000)) ) ))       // sleep timer trip
        {
            clearButton(5);
            clearButton(6);
            
//            if(settings->switchType[5] == MOMENTARY)
//            {
//                processButtonUpdate(5, RELEASED, 0);
//            }
//            else
//            {
//                 buttonStatus[5] = 0;                    
//            }
//                
//            if(settings->switchType[6] == MOMENTARY)   
//            {
//                processButtonUpdate(6, RELEASED, 0);
//            }else{
//                 buttonStatus[6] = 0;
//            }
           
            sleepTimerMillis += 10;
            
            updateButtonStatusLeds(); 
                
            myPrintf("Sleep...\n");
            
            if(currentButtonStatus == 0x9F) {
                needsWakeupVavgReset = true;
            }
            currentButtonStatus = 0xFF;
            isAwake = 0;          
            updatePWM(LED_ONOFF, 0);        // put PWM modules to sleep
//            RED_Write(0);               
//            GREEN_Write(0);             
//            BLUE_Write(0);          
//            INDICATORS_Write(0);
            //STBY_Write(1);
            
           // HandleLowPowerMode(DEEPSLEEP);
            
            timer_set(&s->timer, 1000);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        }
                              
        if(currentButtonStatus == 0x3F)         // lockout routine
        {
            clearButton(7);
            clearButton(6);
            
//            if(settings->switchType[7] == MOMENTARY)
//            {
//                processButtonUpdate(7, RELEASED, 0);
//            }
//            else
//            {
//                 buttonStatus[7] = 0;                    
//            }
//                
//            if(settings->switchType[6] == MOMENTARY)   
//            {
//                processButtonUpdate(6, RELEASED, 0);
//            }else{
//                 buttonStatus[6] = 0;
//            }
            
            currentButtonStatus = 0xFF;
            
            updateButtonStatusLeds();
            
            if(!lockUpdated)
            {
                ledSaveR = settings->backlightIntensity[RED];
                ledSaveG = settings->backlightIntensity[GREEN];
                ledSaveB = settings->backlightIntensity[BLUE];
                settings->backlightIntensity[RED] = 75;
                settings->backlightIntensity[GREEN] = 0;
                settings->backlightIntensity[BLUE] = 0;
                
                if(locked)
                {
                    locked = 0;
                    myPrintf("Unlock...\n");
                }
                else
                {
                    locked = 1;
                    myPrintf("Lock...\n");
                }
                
                timer_set(&s->timer, 300);
                PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                
                settings->backlightIntensity[RED] = ledSaveR;
                settings->backlightIntensity[GREEN] = ledSaveG;
                settings->backlightIntensity[BLUE] = ledSaveB;
                
                timer_set(&s->timer, 700);
                PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            }
            
            lockUpdated = 1;
        }
        else if(currentButtonStatus == 0xFF)
        {
            lockUpdated = 0;
        }
        else if(!locked && currentButtonStatus == 0x6F)         // change address
        {
            clearButton(7);
            clearButton(4);
            
//            if(settings->switchType[7] == MOMENTARY)
//            {
//                processButtonUpdate(7, RELEASED, 0);
//            }
//            else
//            {
//                 buttonStatus[7] = 0;                    
//            }
//                
//            if(settings->switchType[4] == MOMENTARY)   
//            {
//                processButtonUpdate(4, RELEASED, 0);
//            }else{
//                 buttonStatus[4] = 0;
//            }
            
            currentButtonStatus = 0xFF;
            changedBits = 0;
                
            if(changeAddr == 0)
            {
                settings->sourceAddress = (settings->sourceAddress + 1) % 4;
                
                changeAddr = settings->sourceAddress + 1;
                
                saveSettings();
                
                
                timer_set(&s->timer, 300);
                PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            }
        }
        
        
        
        if(isAwake && !locked && !lockUpdated)  // normal
        {      
            for(i = 0; i < 8; i++)
            {
                uint8 buttonReleased = (currentButtonStatus >> i) & 0x01;
                uint8 wasButtonChanged = (changedBits >> i) & 0x01;
                
                //uint8_t i2 = i + settings->sourceAddress * 8;      // check only this address
                
                if(wasButtonChanged)
                {
//                    sleepTimerMillis = 0;
                    
                    if(buttonReleased)
                    {
                        //released
                        heldButtonCount[i] = 0;
                        processButtonUpdate(i, RELEASED, heldButtonCount[i]);
                        buttonStatus[i] = 0;
                    }
                    else
                    {
                        //pressed
                        heldButtonCount[i] += 10;
                        processButtonUpdate(i, PRESSED, heldButtonCount[i]);
                        buttonStatus[i] = 1;
                    }
                }
                else if(!buttonReleased)
                {
                    heldButtonCount[i] += 10;
                }
                
                if(heldButtonCount[i] >= 333)
                {
                    processButtonUpdate(i, HELD, heldButtonCount[i]);
                }
                
            }
            
            updateButtonStatusLeds(); 
        }
        else if(!isAwake && ((currentButtonStatus != 0xFF) || !sleepTimerMillis))       // wake
        {
            myPrintf("%x %d ", currentButtonStatus, sleepTimerMillis);
            myPrintf("Wake... %d\n", settings->sleepTimer);
            isAwake = 1;  
            STBY_Write(0);
            
            globalNeedsSleepReset = true;
            
            updatePWM(LED_ONOFF, 1);        // wake PWM modules
            
            currentButtonStatus = 0xFF;
            timer_set(&s->timer, 1000);
            sleepTimerMillis = 0;
            configureCan();
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        }
        
        lastButtonStatus = currentButtonStatus;
        //UART_PutString("Read Buttons...\n");
        
    }
  
    PT_END(&s->pt);
}

PT_THREAD(dim(struct state *s))
{
    
    static int i;
    
    PT_BEGIN(&s->pt);
    
    while(1)
    {        
        timer_set(&s->timer, 20);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
        for(i = 0; i < 8; i++)
        {
            uint8_t i2 = i + 8 * settings->sourceAddress;
            
            if(switchIsDimming[i])
            {
                if(dimDirection[i])
                {
                    if(dimValue[i2] < 254)
                        dimValue[i2]++;
                }
                else
                {
                    if(dimValue[i2] > 0)
                        dimValue[i2]--;
                }
                
                //iprintf("D %d, %d, %d\n", i2, dimDirection[i], dimValue[i2]);
//                if(settings->sourceAddress == (i/8))
//                {
                    uint8 buffer[] = {0x80 + settings->sourceAddress, oneHot(i), dimValue[i2], 0xFF, 0x00};
                    mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer);
//                }
            }
            
        }
    }
  
    PT_END(&s->pt);
}

uint32 checksum(uint8 *addr, int32 count)
{
  int32 sum = 0;

  // Main summing loop
  while(count > 0)
  {    
    sum = sum + *addr;
    count--;
    addr++;
  }

  // Add left-over byte, if any
  if (count > 0)
    sum = sum + *((uint8 *) addr);

  // Fold 32-bit sum to 16 bits
  while (sum>>16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return(~sum);
}

void saveSettings()
{
//    myPrintf("Settings saved %d %x...\n", sizeof(tSettings), settings->sourceAddress);
//    settings->crc = crc32(0, settings, sizeof(tSettings) - 4);
//    CySysFlashWriteRow(1023, (const unsigned char *)settings);
    
    unsigned char tempArray[EEPROM_BYTE_SIZE] = {0};
    
    //CY_FLASH_SIZEOF_ROW
    
    myPrintf("Settings saved: size %d/%d, add %x...\n", sizeof(tSettings), EEPROM_BYTE_SIZE, settings->sourceAddress);
    settings->crc = crc32(0, settings, sizeof(tSettings) - 4);
    
    unsigned char* settingsP = (unsigned char*)settings;
    
    for(unsigned int i = 0; i < sizeof(tSettings) ; i++)
    {
        tempArray[i] = settingsP[i];
    }
    
    for (unsigned int i = 0; i < (EEPROM_BYTE_SIZE / CY_FLASH_SIZEOF_ROW) ; i++)
    {
        CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + i, (const unsigned char *) &tempArray[CY_FLASH_SIZEOF_ROW * i]);
    }
    
    //CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW, (const unsigned char *) tempArray);
}


bool unsecureBleMode = false;

void updateBleSecurity(uint8 turnOn)
{
    
    while (CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_BUSY)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    CyBle_Stop();
    
    while (CyBle_GetState() != CYBLE_STATE_STOPPED)
 	{
 	    CyBle_ProcessEvents();
 	}
       
    if(turnOn)
    {
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_3 );
        
        unsecureBleMode = false;
    }
    else
    {
//        cyBle_authInfo.bonding = CYBLE_GAP_BONDING_NONE;
//        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_1 );
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_1 );
        
        unsecureBleMode = true;
    }
    
    CyBle_Start( StackEventHandler );

    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
 	{
 	    CyBle_ProcessEvents();
 	}
}

#define PASS_NEW    0
#define PASS_CLEAR  1
#define UNSECURED   0
#define SECURED     1

//bool deepSleepWake = false;

PT_THREAD(hardwareTest(struct state *s))
{
    static uint32 i,j,k, red, green, blue, intensity, crc = 0;
    static int address;
    PT_BEGIN(&s->pt);
    
    tSettings* eeprom = (void *)EEPROM_FLASH_ADDR;
    
    while(1)
    {
        crc = crc32(0, "\x01\x02\x03\x04\x05", 5);
//        myPrintf("TCRC: %x\n", crc);
        
        crc = crc32(0, eeprom, sizeof(tSettings) - 4);
//        myPrintf("ECRC: %x\n", crc);
//        myPrintf("SCRC: %x\n", eeprom->crc);
        
        if(crc != eeprom->crc)
        {
            myPrintf("Loading Default Settings...\n");
            settings->backlightIntensity[RED] = 0;
            settings->backlightIntensity[GREEN] = 0;
            settings->backlightIntensity[BLUE] = 30;
            settings->indicatorIntensity = 100;
//            settings->pin = 0;
            settings->appSourceAddress = 0;
            settings->sourceAddress = 0;
            settings->sleepTimer = SLEEP_MIN;
            settings->isSecChng = 0;
            settings->isWakeFromIgn = 1;
            
        #if IS_DEFAULT_PRO
            settings->isProMode = 1;
        #else
            settings->isProMode = 0;
        #endif
            
            settings->noDeepSleep = 0;
            settings->isWritable = 0;
            settings->isWakeFromDS = 0;
            for(i = 0; i < 32; i++)
            {
                settings->switches[i].type = 0;
                
                settings->switches[i].links = 0;
                settings->switches[i].isDimmable = 0;
                
                settings->switches[i].strobeOn = 0xFF;
                settings->switches[i].strobeOff = 0;
                settings->switches[i].isStrobeOrFlash = 0;
                
//                settings->switchStrobeOn[i] = 255;
//                settings->switchStrobeOff[i] = 0;
//                
//                settings->switchIsDimmable[i] = 0;
//                for(j = 0; j < 4; j++)
//                {
//                    settings->switchLinks[(j * 8) + i] = 0;
//                }
//                
//                settings->switchStrobeOrFlash[i] = 0;
//                settings->switchType[i] = 0;
            }
            
            timer_set(&s->timer, 1);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            generatePasskey(PASS_CLEAR);
            updateBleSecurity(UNSECURED);   // turn off security for unauthorized mode
            
            saveSettings();
        }
        else
        {
            myPrintf("Loading Saved Settings %d...\n", sizeof(tSettings));
            
            unsigned char* settingsP = (unsigned char*)settings;
            unsigned char* eepromP = (unsigned char*)eeprom;
            
            for(i = 0; i < sizeof(tSettings); i ++)
            {
                settingsP[i] = eepromP[i];  
            }
        
            if(settings->isProMode)
            {
                myPrintf("PRO MODE!\n");
            }
            
//            if(settings->isSecChng == 0)
//            {
                    timer_set(&s->timer, 1);
                PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
//            }
            
            if(settings->passkey == 0)
            {
                generatePasskey(PASS_NEW);     // every power cycle until one is captured
            }
            else if(settings->passkey < 0)
            {
                updateBleSecurity(UNSECURED);   // turn off security for unauthorized mode
            }
            else
            {
                CyBle_GapFixAuthPassKey(1, settings->passkey);
            }
        }
        
        isTempWritable = settings->isWritable;
        
//        if(settings->isSecChng == 0)
//        {
                timer_set(&s->timer, 1);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
//        }
        
        passkeyTimer = 0;
        
        myPrintf("Passkey: %d\n", settings->passkey);
        
        red = settings->backlightIntensity[RED];
        green = settings->backlightIntensity[GREEN];
        blue = settings->backlightIntensity[BLUE];
        intensity = settings->indicatorIntensity;
        
        settings->indicatorIntensity = 100;       
        
//        updatePWM(INDICATOR, settings->indicatorIntensity);
        // / *
        for(k = 0; k < 4; k++)
        {
            if(settings->isProMode || settings->isSecChng || settings->isWakeFromDS)
            {                
                break;
            }
            
//            if(settings->isWakeFromDS)
//            {
//                settings->isWakeFromDS = 0;
//                saveSettings();
//                
//                break;
//            }
            
            settings->backlightIntensity[RED] = 0;
            settings->backlightIntensity[GREEN] = 0;
            settings->backlightIntensity[BLUE] = 0;
            
            settings->backlightIntensity[k % 3] = 100;
            
//            updatePWM(RED, settings->backlightIntensity[RED]);
//            updatePWM(GREEN, settings->backlightIntensity[GREEN]);
//            updatePWM(BLUE, settings->backlightIntensity[BLUE]);
            
            
            for(i = 0; i < 8; i++)
            {    
                
                
                for(j = 0; j < 3; j++)
                {               
                    
                    switch(j){
                        case 0:
                            address = ioAddress1;
                            i2cBuffer[0] = 0x02;
                        break;
                        case 1:
                            address = ioAddress1;
                            i2cBuffer[0] = 0x03;
                        break;
                        case 2:
                            address = ioAddress2;
                            i2cBuffer[0] = 0x02;
                        break;
                    }
                    
                    i2cBuffer[1] = 0x01 << i;
                    
                    i2cWriteWait(address, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
                    
                    timer_set(&s->timer, 30);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                }        
                
            }
            
            i2cBuffer[0] = 0x02;
            i2cBuffer[1] = 0x00;
            i2cWriteWait(ioAddress1, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
                    
            timer_set(&s->timer, 30);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            i2cBuffer[0] = 0x03;
            i2cBuffer[1] = 0x00;
            i2cWriteWait(ioAddress1, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
                    
            timer_set(&s->timer, 30);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            i2cBuffer[0] = 0x02;
            i2cBuffer[1] = 0x00;
            i2cWriteWait(ioAddress2, i2cBuffer, 2, I2C_I2C_MODE_COMPLETE_XFER);
        }
      //  */
        
        settings->backlightIntensity[RED] = red;
        settings->backlightIntensity[GREEN] = green;
        settings->backlightIntensity[BLUE] = blue;
        settings->indicatorIntensity = intensity;
        
//        updatePWM(RED, settings->backlightIntensity[RED]);
//        updatePWM(GREEN, settings->backlightIntensity[GREEN]);
//        updatePWM(BLUE, settings->backlightIntensity[BLUE]);
//        updatePWM(INDICATOR, settings->indicatorIntensity);
        
        i2cBuffer[0] = 0x01;      
        i2cWriteWait(ioAddress2, i2cBuffer, 1, I2C_I2C_MODE_COMPLETE_XFER);
        i2cReadWait(ioAddress2, &i2cBuffer[1], 1, I2C_I2C_MODE_COMPLETE_XFER);

        uint8 currentButtonStatus = i2cBuffer[1];
        
        myPrintf("Wake... %d (%d), ", settings->sleepTimer, settings->noDeepSleep);
        myPrintf("%x, %d\n", currentButtonStatus, settings->isSecChng);
        
        
        while(settings->isWakeFromDS == 0 && (currentButtonStatus == 0xFE || currentButtonStatus == 0xEF || currentButtonStatus == 0xF7))
        {   
//            tempPin = 1234;     //generateRandom() * 65500;
            
            pairingNeeded = 1; 
            
            if(currentButtonStatus == 0xEF)     //B2
            {
                pinGenerationNeeded = 1;
            }
            
            if(currentButtonStatus == 0xF7)     //B3
            {
                pinClearNeeded = 1;
            }
            
            passkeyTimer = millis();
            
            i2cBuffer[0] = 0x01;      
            i2cWriteWait(ioAddress2, i2cBuffer, 1, I2C_I2C_MODE_COMPLETE_XFER);
            i2cReadWait(ioAddress2, &i2cBuffer[1], 1, I2C_I2C_MODE_COMPLETE_XFER);

            currentButtonStatus = i2cBuffer[1];            
        }   
        
        //settings->isSecChng = 1; //if changed
        
        if(pinGenerationNeeded)
        {
            updateBleSecurity(SECURED);       // set back to secure after being unsecured
            
            generatePasskey(PASS_NEW);
            saveSettings();
            
//            pairingNeeded = 0;
            pinGenerationNeeded = 0;
            
            authGood = false;
            
        }
        else if(pairingNeeded && settings->passkey < 0)
        {
            updateBleSecurity(SECURED);       // set back to secure after being unsecured
            
            generatePasskey(PASS_NEW);
            saveSettings();
            
//            pairingNeeded = 0;
            
            authGood = false;
        }
        
        
        
        if(pinClearNeeded)
        {
            generatePasskey(PASS_CLEAR);     // clear
            pinGenerationNeeded = 0;
//            pairingNeeded = 0;
            pinClearNeeded = 0;
            saveSettings();
            
            passkeyTimer = 0;
            //authgood = 1;
            
            updateBleSecurity(UNSECURED);
        }
        
        if(pairingNeeded == 1 || settings->isSecChng == 1)
        {
            passkeyTimer = millis();
            
            if(settings->passkey < 0)
                passkeyTimer = 0;
            
            pairingNeeded = 0;
            
            settings->isSecChng++;
            settings->isSecChng %= 2;
            saveSettings();
            
            if(settings->isSecChng == 1)
            {
                myPrintf("reset for ble\n");
                CySoftwareReset();
            }
        }
        
        if(settings->isWakeFromDS != 0)
        {
            settings->isWakeFromDS = 0;
                saveSettings();
        }
        
        runHardwareTest = 0;
        PT_WAIT_UNTIL(&s->pt, runHardwareTest);
    }
    PT_END(&s->pt);
}


static struct state hardwareTestState, canRxState, readButtonsState, sendAlivePacketState, updateLedsState, dimState, deepSleepState; // pairDeviceState

int main()
{
#if !defined(__ARMCC_VERSION)
    InitializeBootloaderSRAM();
#endif

    // Checks if Self Project Image is updated and Runs for the First time 
    AfterImageUpdate();
    
    uint8_t resetCause;
    
    resetCause = CySysGetResetReason(CY_SYS_RESET_WDT | CY_SYS_RESET_SW | CY_SYS_RESET_PROTFAULT);
    
    if(resetCause != 0)
    {
        if(resetCause == CY_SYS_RESET_WDT)
        {
            myPrintf("\nReset by watchdog \n\n");
            
        } else if(resetCause == CY_SYS_RESET_SW)
        {
            myPrintf("\nReset: CY_SYS_RESET_SW \n\n");
        } else if(resetCause == CY_SYS_RESET_PROTFAULT)
        {
            myPrintf("\nReset: CY_SYS_RESET_PROTFAULT \n\n");
        }
    
    }
    
    
    uint16 appVer, boardId;//, appId;
    uint32 board;//, icPN;
    char boardRev;
    
    //appId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
    appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    boardId = (board & 0xFFFF0000) >> 16;
    boardRev = (board & 0x0000FFFF) + ('A' - 1);
    
    myPrintf("Compiled: %s @ %s\n", __DATE__, __TIME__);
    myPrintf("Boot App Board ID: %drev%c\n", boardId, boardRev);
    myPrintf("Boot App Version: %d.%d.%d\n", appVer / 0x100, appVer % 0x100 / 0x10, appVer % 0x10);
//    myPrintf("Boot App ID: %x\n", appId);
    
    //appId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
    appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
//    board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
//    
//    boardId = (board & 0xFFFF0000) >> 16;
//    boardRev = (board & 0x0000FFFF) + ('A' - 1);
    
//    myPrintf("Boot Stack Board ID: %drev%c\n", boardId, boardRev);
    myPrintf("Boot Stack Version: %d.%d.%d\n", appVer / 0x100, appVer % 0x100 / 0x10, appVer % 0x10);
//    myPrintf("Boot Stack ID: %x\n", appId);
    
    
    
    PT_INIT(&hardwareTestState.pt);    
    hardwareTestState.name = "hardwareTest\n";
    
//    PT_INIT(&pairDeviceState.pt);    
//    pairDeviceState.name = "pairDevice\n";
    
    PT_INIT(&canRxState.pt);    
    canRxState.name = "canRx\n";
    
    PT_INIT(&readButtonsState.pt);    
    readButtonsState.name = "readButtons\n";
    
    PT_INIT(&sendAlivePacketState.pt);    
    sendAlivePacketState.name = "sendAlivePacket\n";
    
    PT_INIT(&updateLedsState.pt);    
    updateLedsState.name = "updateLeds\n";
    
    PT_INIT(&dimState.pt);    
    dimState.name = "dimState\n";
    
    I2C_Start();
    SPI_Start();
    UART_Start();
//    ADC_Start();
//    ADC_SetChanMask(1);
//    ADC_StartConvert();
    myPrintf("Start...\n\n");
    
    PWM_Red_Start();
    PWM_Green_Start();
    PWM_Blue_Start();
    PWM_Indicator_Start();
    
    /* Map systick ISR to the user defined ISR. SysTick_IRQn is already defined in core_cm0.h file */
	CyIntSetSysVector((SysTick_IRQn + 16), USER_ISR);
    
    CyGlobalIntEnable;
    
    PT_SCHEDULE(hardwareTest(&hardwareTestState));  // setup / load settings
    
    CyBle_Start( StackEventHandler );
    
//    PT_SCHEDULE(hardwareTest(&hardwareTestState)); // to read/initialize passkey
    
    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
 	{
 	    CyBle_ProcessEvents();
 	}
            
    configureIO();  
    
    configureCan();
    
    /* Enable Systick timer with desired period/number of ticks */
	SysTick_Config(NUMBER_OF_TICKS);
    
    CyDelay(10);
    
    myPrintf("Finish Config...\n");
     

    
    while(runHardwareTest)
    {
        PT_SCHEDULE(hardwareTest(&hardwareTestState));
        CyBle_ProcessEvents();
    }
    
//    for(int i = 0 ; i < 8 ; i++)
//    {
//        switchShort[i] = 1;
//    }
    
    for(;;)
    {               
        CyBle_ProcessEvents();
        updateAuth();
        
        PT_SCHEDULE(canRx(&canRxState));
        PT_SCHEDULE(sendAlivePacket(&sendAlivePacketState));
        PT_SCHEDULE(updateLeds(&updateLedsState));
        PT_SCHEDULE(readButtons(&readButtonsState));
        PT_SCHEDULE(dim(&dimState));
        PT_SCHEDULE(checkDeepSleep(&deepSleepState));
        
    }
}

bool bleDisconnect = true;

//bool storeBond = false;

bool sendOtaPacket = false;
uint8_t sendProPacket = 0;

#define NUM_PRO_SWITCH_PACKETS      (8)
#define NUM_PRO_HD_SETTINGS_PACKETS    (1)
//#define NUM_PRO_LINK_PACKETS        (4)
#define NUM_PRO_SW_SETTINGS_PACKETS (32)


#define NUM_PRO_SYNC_PACKETS    (NUM_PRO_SWITCH_PACKETS + NUM_PRO_HD_SETTINGS_PACKETS + NUM_PRO_SW_SETTINGS_PACKETS)//(13)

#define INDEX_SYNC_START        (0)

#define INDEX_SYNC_SW_SETTINGS      (INDEX_SYNC_START)
#define INDEX_SYNC_HD_SETTINGS      (INDEX_SYNC_SW_SETTINGS + NUM_PRO_SW_SETTINGS_PACKETS)
#define INDEX_SYNC_SWITCHES         (INDEX_SYNC_HD_SETTINGS + NUM_PRO_HD_SETTINGS_PACKETS)

#define IS_MOMENTARY_MASK       0x01
#define IS_DIMABLE_MASK         0x02
#define IS_STROBE_MASK          0x04   
#define IS_FLASH_MASK           0x08

void updateAuth(void)
{
//    write_millis(millis());
    
    static bool passOn = 1;
    static bool passOff = 1;
    static CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    if(unsecureBleMode)
    {
        bleDisconnect = false;
        authGood = true;
    }
    else if(passkeyTimer)
    {
        if((millis() - passkeyTimer) > PASSKEY_TIMEOUT)
        {
            passkeyTimer = 0;
        }

        if(passOn) {
            writePassKey(settings->passkey);
            passOn = 0;
            passOff = 1;
            bleDisconnect = false;
        }
    } 
    else
    {
        if(passOff){
            writePassKey(0);
            passOn = 1;
            passOff = 0;
            bleDisconnect = true;
        }
    }
    
    if(saveSettingsFlag)
    {
        saveSettings();
        if(settings->passkey > 0)
        {
            CyBle_GapFixAuthPassKey(1, settings->passkey);
            updateBleSecurity(SECURED);
        }
            
        saveSettingsFlag = 0;
    }
    
    if(cyBle_pendingFlashWrite != 0u)
    {
        // Store Bonding informtation to flash 
        apiResult = CyBle_StoreBondingData(0u);
        if ( apiResult == CYBLE_ERROR_OK)
        {
            passkeyTimer = 0;
            myPrintf("Bonding data stored\r\n");
        }
        else
        {
            myPrintf ("Bonding data storing pending\r\n");
        }
    }
    
    if(sendOtaPacket)
    {
        if(authGood && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            sendOtaPacket = false;
            
            static CYBLE_API_RESULT_T                  bleApiResult;
            static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;
            
            int packetLength = 18;
            
            uint32_t appBoard, stkBoard;
            uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
            
            
            appBoard = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
            stkBoard = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
            
            svdAppId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
            svdAppVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
            svdStkId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
            svdStkVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
            
            
            
            bleTxBuffer[0] = packetLength;
            bleTxBuffer[1] = 0x00;
            
            bleTxBuffer[2] = (appBoard >> 24) & 0xFF;
            bleTxBuffer[3] = (appBoard >> 16) & 0xFF;
            bleTxBuffer[4] = (appBoard >> 8) & 0xFF;
            bleTxBuffer[5] = (appBoard >> 0) & 0xFF;
            
            bleTxBuffer[6] = (svdAppId >> 8) & 0xFF;
            bleTxBuffer[7] = (svdAppId >> 0) & 0xFF;
            
            bleTxBuffer[8] = (svdAppVer >> 8) & 0xFF;
            bleTxBuffer[9] = (svdAppVer >> 0) & 0xFF;
            
            bleTxBuffer[10] = (stkBoard >> 24) & 0xFF;
            bleTxBuffer[11] = (stkBoard >> 16) & 0xFF;
            bleTxBuffer[12] = (stkBoard >> 8) & 0xFF;
            bleTxBuffer[13] = (stkBoard >> 0) & 0xFF;
            
            bleTxBuffer[14] = (svdStkId >> 8) & 0xFF;
            bleTxBuffer[15] = (svdStkId >> 0) & 0xFF;
            
            bleTxBuffer[16] = (svdStkVer >> 8) & 0xFF;
            bleTxBuffer[17] = (svdStkVer >> 0) & 0xFF;
            
            
            characteristicNotify.value.val = bleTxBuffer;
            characteristicNotify.value.len = packetLength;
            characteristicNotify.attrHandle = CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE;
            
            myPrintf("Send OTA Packet\n");
            
            do
            {
                
                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
            
            myPrintf("Success \n");
        }
    }
    else if(sendProPacket != 0)
    {
        if(authGood && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {            
            static CYBLE_API_RESULT_T                  bleApiResult;
//            static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;
            static CYBLE_GATTS_HANDLE_VALUE_IND_T      characteristicIndicate;
            
            int packetLength = 20;
            
            sendProPacket--;
            
            bool sendSwitchPacket = false;
            
            if(sendProPacket >= INDEX_SYNC_SWITCHES)//> 4)
            {
                int index = (INDEX_SYNC_SWITCHES + NUM_PRO_SWITCH_PACKETS - 1) - sendProPacket;//12 - sendProPacket;
                
                sendSwitchPacket = true;
                
                packetLength = 12;
                
                bleTxBuffer[0] = 0x55;
                bleTxBuffer[1] = packetLength - 2;
                bleTxBuffer[2] = 0x00;
                
//                {0x80 + settings->sourceAddress, oneHot(i), switchStatus[i] ? (settings->switchIsDimmable[i] ? dimValue[i] : 0xFE)
                
                uint8_t swIndex = settings->sourceAddress * 8 + index;
                
                bleTxBuffer[3] = 0x80 + settings->sourceAddress;
                bleTxBuffer[4] = oneHot(index);
                if(oneHot(index) == 0)
                    return;
                
                bleTxBuffer[5] = switchStatus[index] ? (settings->switches[swIndex].isDimmable ? dimValue[swIndex] : 0xFF) : 0x00;
                bleTxBuffer[6] = settings->switches[swIndex].strobeOn;
                bleTxBuffer[7] = settings->switches[swIndex].strobeOff;
                
                if(settings->switches[swIndex].strobeOff == 0)
                {
                    return;
                }
                
                uint32 crc = crc32(0, bleTxBuffer, 8);
                
                bleTxBuffer[packetLength - 4] = crc & 0xFF;
                bleTxBuffer[packetLength - 3] = crc >> 8;
                bleTxBuffer[packetLength - 2] = crc >> 16;
                bleTxBuffer[packetLength - 1] = crc >> 24;
                
                
            }
            else if(sendProPacket >= INDEX_SYNC_HD_SETTINGS)//== 4)
            {
                
                packetLength = 15;
                
                uint8_t packetType = 0x02;
                
                bleTxBuffer[0] = packetLength;
                bleTxBuffer[1] = (DEVICE_TYPE << 6) | packetType;
                bleTxBuffer[2] = packetType;
                
                bleTxBuffer[3] = settings->appSourceAddress;
                bleTxBuffer[4] = settings->sourceAddress;
                bleTxBuffer[5] = settings->backlightIntensity[RED];
                bleTxBuffer[6] = settings->backlightIntensity[GREEN];
                bleTxBuffer[7] = settings->backlightIntensity[BLUE];
                bleTxBuffer[8] = settings->indicatorIntensity;
                bleTxBuffer[9] = settings->sleepTimer;
                
                bleTxBuffer[10] = 0x00; // don't care...
                bleTxBuffer[11] = 0x00;
                bleTxBuffer[12] = 0x00;
                bleTxBuffer[13] = 0x00;
                
                
                for(int j = 0; j < 8; j++)
                {
                    int j2 = settings->sourceAddress * 8 + j;
                    
                    bleTxBuffer[10] |= (settings->switches[j2].type != 0) ? 0x01 << j : 0;
                    bleTxBuffer[11] |= (settings->switches[j2].isDimmable != 0) ? 0x01 << j : 0;
                    
                    if(settings->switches[j2].isStrobeOrFlash != 0)
                    {
                        bleTxBuffer[12] |= (0x01 << j);
                        
                        if(settings->switches[j2].isStrobeOrFlash == 1)
                        {
                            bleTxBuffer[13] |= (0x01 << j);
                        }
                    }
                    
//                    momentary |= settings->switchType[j] << j;
//                dimable |= settings->switchIsDimmable[j] << j;
                }
                
                bleTxBuffer[14] = settings->noDeepSleep;
                
                bleTxBuffer[14] |= 0x04;
                bleTxBuffer[14] |= (settings->isWakeFromIgn != 0) ? 0x02 : 0x00;
            
            }
            else if(sendProPacket >= INDEX_SYNC_SW_SETTINGS)
            {
                int index = (INDEX_SYNC_SW_SETTINGS + NUM_PRO_SW_SETTINGS_PACKETS - 1) - sendProPacket;
                
                packetLength = 13;
                
                uint8_t packetType = 0x08;
                
                bleTxBuffer[0] = packetLength;
                bleTxBuffer[1] = (DEVICE_TYPE << 6) | packetType;
                bleTxBuffer[2] = packetType;
                bleTxBuffer[3] = index;
                bleRxBuffer[4] = 0;         // versioning...
                
                bleRxBuffer[5] = dimValue[index];
                bleRxBuffer[6] = 
                    (settings->switches[index].type == MOMENTARY ? IS_MOMENTARY_MASK : 0) | 
                    (settings->switches[index].type == 1 ? IS_DIMABLE_MASK : 0) |
                    (settings->switches[index].isStrobeOrFlash == 2 ? IS_STROBE_MASK : 
                        (settings->switches[index].isStrobeOrFlash == 1 ? IS_FLASH_MASK : 0));
                        
                bleRxBuffer[7] = settings->switches[index].strobeOn;
                bleRxBuffer[8] = settings->switches[index].strobeOff;
                
                bleRxBuffer[9] = (settings->switches[index].links >> 0) & 0xFF;
                bleRxBuffer[10] = (settings->switches[index].links >> 8) & 0xFF;
                bleRxBuffer[11] = (settings->switches[index].links >> 16) & 0xFF;
                bleRxBuffer[12] = (settings->switches[index].links >> 24) & 0xFF;
                
//                uint32 crc = crc32(0, bleTxBuffer, 8);
//                
//                bleTxBuffer[packetLength - 4] = crc & 0xFF;
//                bleTxBuffer[packetLength - 3] = crc >> 8;
//                bleTxBuffer[packetLength - 2] = crc >> 16;
//                bleTxBuffer[packetLength - 1] = crc >> 24;
                
            }
//            else
//            {
//                
//                packetLength = 12;
//                
//                uint8 offset = 3 - sendProPacket;
//                uint8_t packetType = 0x02 + 1 + offset;
//                
//                bleTxBuffer[0] = packetLength;
//                bleTxBuffer[1] = (packetType << 1) | (DEVICE_TYPE << 6);
//                bleTxBuffer[2] = 3;
//                
//                bleTxBuffer[3] = offset;
//                
////                bleTxBuffer[4] = settings->switchLinks[(offset * 8) + 0];           /// uint32_t....
////                bleTxBuffer[5] = settings->switchLinks[(offset * 8) + 1];
////                bleTxBuffer[6] = settings->switchLinks[(offset * 8) + 2];
////                bleTxBuffer[7] = settings->switchLinks[(offset * 8) + 3];
////                bleTxBuffer[8] = settings->switchLinks[(offset * 8) + 4];
////                bleTxBuffer[9] = settings->switchLinks[(offset * 8) + 5];
////                bleTxBuffer[10] = settings->switchLinks[(offset * 8) + 6];
////                bleTxBuffer[11] = settings->switchLinks[(offset * 8) + 7];
//                
//                bleTxBuffer[4] = getLinks2((offset * 8) + 0);
//                bleTxBuffer[5] = getLinks2((offset * 8) + 1);
//                bleTxBuffer[6] = getLinks2((offset * 8) + 2);
//                bleTxBuffer[7] = getLinks2((offset * 8) + 3);
//                bleTxBuffer[8] = getLinks2((offset * 8) + 4);
//                bleTxBuffer[9] = getLinks2((offset * 8) + 5);
//                bleTxBuffer[10] = getLinks2((offset * 8) + 6);
//                bleTxBuffer[11] = getLinks2((offset * 8) + 7);
//                
//            }
            
            
//            characteristicNotify.value.val = bleTxBuffer;
//            characteristicNotify.value.len = packetLength;
//            characteristicNotify.attrHandle = CYBLE_SPOD_PRO_MODE_CHAR_HANDLE;
            
            characteristicIndicate.value.val = bleTxBuffer;
            characteristicIndicate.value.len = packetLength;
            characteristicIndicate.attrHandle = CYBLE_SPOD_PRO_MODE_CHAR_HANDLE;
            
            if(sendSwitchPacket){
                characteristicIndicate.attrHandle = CYBLE_SPOD_COMM_CHAR_HANDLE;
            }
            
            myPrintf("Send PRO Packet: %d  - ", sendProPacket);
            
                for(int k = 0; k < packetLength ; k++)
                {
                    myPrintf("%x ", bleTxBuffer[k]);
                }
                myPrintf("\n");
            
            do
            {
//                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                bleApiResult = CyBle_GattsIndication(cyBle_connHandle, &characteristicIndicate);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
            
//            myPrintf("Success \n");
        }
    }
    
}

uint8 bleFoundDelimiter = 0;
uint8 bleBufferAddress = 0;
int blePacketLength = 0;

uint8_t isSyncing = false;



void ProcessPacket(uint8 packetLength)
{
    static int lastAddrChng = -1;
    
    int i;
    uint32 calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32 tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(tempCrc == calcCrc)
    {
        resetSleepTimer = true;
        
        //myPrintf("Packet Good...\n");
        if(bleRxBuffer[2] == 0) //CAN PACKET
        {
//            uint16 pin = 0;
            int index;//, i = 0;
            //uint8 t1,t2;
//            pin = pin | bleRxBuffer[4];
//            pin = (pin << 8) | bleRxBuffer[3];
            
            //myPrintf("Packet Good... %d\n", pin);
            
            if(true)//settings->pin == 0 || pin == settings->pin)
            {
                mcpCanSendMsgBuf(&can, 0x80, 0, packetLength - 9, &bleRxBuffer[5]);
                
                if((bleRxBuffer[5] & 0xF0) == 0x80)     // switch packet...
                {
                    index = indexPos(bleRxBuffer[6]);
                    if(index == 255)
                        return;
                    uint8_t addr = bleRxBuffer[5] & 0x03;
                    
                    if(addr == settings->sourceAddress)
                        switchStatus[index] = bleRxBuffer[7];
                    
//                    if(settings->appSourceAddress == settings->sourceAddress)
//                    {
//                        switchStatus[index] = bleRxBuffer[7];
//                    }
                
                    /*      // links controlled by app or used on button press, unneeded here
                    for(i = 0; i < 8; i++){
                        if( i != index)
                        {
                            if((settings->switchLinks[(settings->appSourceAddress * 8) + index] & (1 << i)) > 0)
                            {
                                t1 = 0xFF;
                                t2 = 0x00;
                                
//                                if(settings->isProMode && 
                                
                                if(settings->switchStrobeOrFlash[i]  > 0)
                                {
                                    if(settings->isProMode && settings->switchStrobeOff[i] != 0)
                                    {
                                        t1 = settings->switchStrobeOn[i];
                                        t2 = settings->switchStrobeOff[i];
                                    }
                                    else if(settings->switchStrobeOrFlash[i]  == 1)
                                    {
                                        t1 = 10;
                                        t2 = 10;
                                    }
                                    else
                                    {
                                        t1 = 1;
                                        t2 = 4;
                                    }
                                }
                                
                                switchStatus[i] = switchStatus[index];
                                
                                CyDelay(5);
                                uint8 buffer2[] = {bleRxBuffer[5], oneHot(i), switchStatus[i] ? 0xFE : 0x00, t1, t2};
                                mcpCanSendMsgBuf(&can, 0x80, 0, 5, buffer2);
                            }
                        }
                    } */
                }
            }
        }
        else if(bleRxBuffer[2] == 1) //LED PACKET
        {
            if(settings->isProMode != 0 && !isTempWritable) {
                return;
            }
            
//            myPrintf("L: %x %x ", bleRxBuffer[3], bleRxBuffer[4]);
            
            switch(bleRxBuffer[3])
            {
                case RED:{
                    settings->backlightIntensity[RED] = bleRxBuffer[4];
                    break;
                }
                case GREEN:{
                    settings->backlightIntensity[GREEN] = bleRxBuffer[4];
                    break;
                }
                case BLUE:{
                    settings->backlightIntensity[BLUE] = bleRxBuffer[4];
                    break;
                }
                default:{
                    settings->indicatorIntensity = bleRxBuffer[4];
                }
            }
        }
        else if(bleRxBuffer[2] == 2) //SETTINGS PACKET
        {
            if(settings->isProMode != 0 && !isTempWritable) {
                return;
            }
            
            myPrintf("Settings Recieved %x %x...\n", bleRxBuffer[3], bleRxBuffer[4]);
            
            myPrintf("-> %x %x %x %x %x %x %x %x %x %x \n", 
                        bleRxBuffer[5], bleRxBuffer[6], bleRxBuffer[7], bleRxBuffer[8], bleRxBuffer[9],
                        bleRxBuffer[10], bleRxBuffer[11], bleRxBuffer[12], bleRxBuffer[13], bleRxBuffer[14]);
            
            settings->appSourceAddress = bleRxBuffer[3];
            
            int length = bleRxBuffer[1] - 2;
            
            
            if(lastAddrChng != bleRxBuffer[4])
            {
                if(settings->sourceAddress != bleRxBuffer[4])
                {
                    changeAddr = bleRxBuffer[4] + 1;
                }
                
                settings->sourceAddress = bleRxBuffer[4];
                
                lastAddrChng = bleRxBuffer[4];
            }
            
            
            settings->backlightIntensity[RED] = bleRxBuffer[5];
            settings->backlightIntensity[GREEN] = bleRxBuffer[6];
            settings->backlightIntensity[BLUE] = bleRxBuffer[7];
            settings->indicatorIntensity = bleRxBuffer[8];
            settings->sleepTimer = bleRxBuffer[9];
            
            if(length >= 15 && (bleRxBuffer[14] & 0x04))
            {
                settings->isWakeFromIgn = (bleRxBuffer[14] & 0x02) ? 1 : 0;
//                myPrintf("L: %d, wfi: %x\n", length, settings->isWakeFromIgn);
            }
//            else
//            {
//                myPrintf("L: %d\n", length);
//            }
            
//            uint8 momentary = 0;
//            uint8 dimable = 0;
            
            for(i = 0; i < 8; i++){
                
                uint8_t i2 = settings->sourceAddress * 8 + i;
                
                settings->switches[i2].type = ((bleRxBuffer[10] >> i) & 0x01) ? MOMENTARY : TOGGLE;
                settings->switches[i2].isDimmable = ((bleRxBuffer[11] >> i) & 0x01) ? 1 : 0;
                settings->switches[i2].isStrobeOrFlash = ((bleRxBuffer[12] >> i) & 0x01) ? (((bleRxBuffer[13] >> i) & 0x01) ? 1 : 2) : 0;
                
                
//                if((bleRxBuffer[10] >> i) & 0x01) 
//                {
//                    settings->switches[i2].type = 1;   
//                }
//                else
//                {
//                    settings->switches[i2].type = 0;   
//                }
//                
//                if((bleRxBuffer[11] >> i) & 0x01) 
//                {
//                    settings->switches[i2].isDimmable = 1;   
//                }
//                else
//                {
//                    settings->switches[i2].isDimmable = 0;   
//                }
//                
//                if((bleRxBuffer[12] >> i) & 0x01) 
//                {
//                    if((bleRxBuffer[13] >> i) & 0x01)
//                    {
//                        settings->switches[i2].isStrobeOrFlash = 1;
//                    }
//                    else
//                    {
//                        settings->switches[i2].isStrobeOrFlash = 2;
//                    }
//                        
//                }
//                else
//                {
//                    settings->switches[i2].isStrobeOrFlash = 0;   
//                }
                
//                momentary |= settings->switches[i2].type << i;
//                dimable |= settings->switches[i2].isDimmable << i;
                               
            }
            
//            myPrintf("svd: %x %x\n", momentary, dimable);
            
            
            
        }
        else if(bleRxBuffer[2] == 3) //LINK PACKET
        {
            if(settings->isProMode != 0 && !isTempWritable) {
                return;
            }
            
            myPrintf("Links Recieved... %d \n", bleRxBuffer[3]);
            
            uint8 offset = bleRxBuffer[3];
            
            if(offset > 3)
                return;
            
            updateLinks2(offset * 8 + 0, bleRxBuffer[4]);
            updateLinks2(offset * 8 + 1, bleRxBuffer[5]);
            updateLinks2(offset * 8 + 2, bleRxBuffer[6]);
            updateLinks2(offset * 8 + 3, bleRxBuffer[7]);
            updateLinks2(offset * 8 + 4, bleRxBuffer[8]);
            updateLinks2(offset * 8 + 5, bleRxBuffer[9]);
            updateLinks2(offset * 8 + 6, bleRxBuffer[10]);
            updateLinks2(offset * 8 + 7, bleRxBuffer[11]);
            
//            if(offset == currAddr)
//            {
//                needsLinksSend = true;
//            }
            
            /// send via pro CAN, if did receive pro-can packet to know what input settings are...
            
            if(offset == 3)
                saveSettings();
        }
        else if(bleRxBuffer[2] == 8) //SWITCH SETTINGS PACKET
        {
            myPrintf("switchSettingsPacket(): ");
            
            for(int i = 0; i < packetLength; i++)
            {
                myPrintf("%x ", bleRxBuffer[i]);
            }
            
            myPrintf("\n");
            
            if(settings->isProMode != 0 && !isTempWritable) {
                return;
            }
            
            uint8_t swIndex = bleRxBuffer[3];
            
            if(swIndex > 32)
                return;
            
            if(bleRxBuffer[4] != 0)     // for versioning
                return;
            
//            uint8_t srcAddr = swIndex / 8;
//            
//            if(srcAddr == settings->sourceAddress)
//            {
//            }
            
            if(bleRxBuffer[5] > 0 && bleRxBuffer[5] < 0xFE)
            {
                dimValue[swIndex] = bleRxBuffer[5];
            }
            
            settings->switches[swIndex].type = (bleRxBuffer[6] & IS_MOMENTARY_MASK) > 0 ? MOMENTARY : TOGGLE;
            settings->switches[swIndex].isDimmable = (bleRxBuffer[6] & IS_DIMABLE_MASK) > 0 ? 1 : 0;
            
            switch(bleRxBuffer[6] & (IS_FLASH_MASK | IS_STROBE_MASK))
            {
                case 0:
                   settings->switches[swIndex].isStrobeOrFlash = 0; 
                break;
                case IS_FLASH_MASK:
                    settings->switches[swIndex].isStrobeOrFlash = 1;
                break;
                case IS_STROBE_MASK:
                    settings->switches[swIndex].isStrobeOrFlash = 2;
                break;
                default:
                    myPrintf("invalid stobe/flash: %d)%x\n", swIndex, bleRxBuffer[6]);
                break;
            }
            
            
            if(bleRxBuffer[7] != 0 && bleRxBuffer[8] != 0)
            {
                settings->switches[swIndex].strobeOn = bleRxBuffer[7];
                settings->switches[swIndex].strobeOff = bleRxBuffer[8];
            }
            
            settings->switches[swIndex].links = bleRxBuffer[9];
            settings->switches[swIndex].links |= bleRxBuffer[10] << 8;
            settings->switches[swIndex].links |= bleRxBuffer[11] << 16;
            settings->switches[swIndex].links |= bleRxBuffer[12] << 24;
            
            if(swIndex >= 31)
                isSyncing = false;
            
            if(!isSyncing)
                saveSettings();
        }
        
    }


}

//bool sendOtaPacket = false;

void enterBootloader(void)
{
    

    myPrintf("Enter Bootloader Mode \n");
    
//        while(1);
    
    Bootloadable_SetActiveApplication(0);
    Bootloadable_Load();
    CySoftwareReset();
    
}


void printBoard(uint32_t boardRaw)
{
    uint16_t boardId;
    char boardRev, boardRev2;
    
    boardId = (boardRaw & 0xFFFF0000) >> 16;
    
    myPrintf("%drev", boardId);
    
    boardRev = boardRaw & 0x0000FFFF;
    
    if(boardRev > 26)
    {
        boardRev2 = boardRev / 26 + ('A' - 1);
        
        myPrintf("%c", boardRev2);
        
        boardRev = boardRev % 26 + ('A' - 1);
    }
    else
    {
        boardRev = boardRev + ('A' - 1);
    }
    
    myPrintf("%c\n", boardRev);
    
}

void processOtaPacket(void)
{
    uint32_t boardRx, boardRead;//, stkBoardRead;
    uint16_t appId, appVer, stkId, stkVer;
    uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
//    uint16_t boardId;
//    char boardRev;
    
    
    
    boardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
    //stkBoardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    svdAppId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
    svdAppVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    svdStkId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
    svdStkVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
    
    if(bleRxBuffer[1] == 0)
    {
        boardRx = 0;
        
        
        for(int i = 0 ; i < 14 ; i++)
        {
            myPrintf("%x ", bleRxBuffer[i]);
        }
        
        myPrintf("\n");
        
        for(int j = 0 ; j < 4 ; j++)
        {
            boardRx = (bleRxBuffer[2 + j] << (8 * (3 - j))) | boardRx;
        }
        
        if(boardRx != boardRead)
        {
            myPrintf("Will Not Update: Incompatible hardware\n");
            
            myPrintf("Saved Hardware ID: ");
            printBoard(boardRead);
            
            myPrintf("Recieved Hardware ID: ");
            printBoard(boardRx);
            
            return;
        }
        
        myPrintf("Board# good: ");
        printBoard(boardRx);
        
        appId = (bleRxBuffer[6] << 8) | bleRxBuffer[7];
        appVer = (bleRxBuffer[8] << 8) | bleRxBuffer[9];
        stkId = (bleRxBuffer[10] << 8) | bleRxBuffer[11];
        stkVer = (bleRxBuffer[12] << 8) | bleRxBuffer[13];
        
        
        if(appId != svdAppId)
        {
            myPrintf("Will Not Update: Invalid App ID");
            
            return;
        }
        
        if(appVer < svdAppVer)
        {
            myPrintf("Will Not Update: Newer app firmware installed\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New App Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            return;
        }
        else if(appVer > svdAppVer)         
        {
//            if(stkVer < svdStkVer)
//            {
//                myPrintf("Will Not Update: Newer stack firmware installed\n");
//                myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
//                myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
//                return;
//            }
            
            myPrintf("Updating\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New App Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            
            enterBootloader();          // Should never return
            
            return;
        }
        else if(appVer == svdAppVer)
        {
            
        }
        else
        {
            myPrintf("APP VERSION ERROR\n");
            return;
        }
        
        
        if(stkId != svdStkId)
        {
            myPrintf("Will Not Update: Invalid Stack ID");
            
            return;
        }
        
        
        if(stkVer < svdStkVer)
        {
            myPrintf("Will Not Update: Newer stack firmware installed\n");
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            return;
        }
        else if(stkVer > svdStkVer)         
        {
            myPrintf("Updating\n");
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            
            enterBootloader();          // Should never return
            
            return;
        }
        else if(stkVer == svdStkVer)
        {
            myPrintf("Will Not Update: App and Stack versions match installed versions\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
        }
        else
        {
            myPrintf("STACK VERSION ERROR\n");
        }
        
    }
    else if(bleRxBuffer[1] == 0xFF)
    {
        myPrintf("Forcing Bootloader Sequence\n");
        enterBootloader();          // Should never return
    }
    else
    {
        
        sendOtaPacket = true;
        
        
        
        
//        sendOtaPacket();
    }
    
    
}

void processProPacket(void)
{
    
    bool updateFlash = 0;
    bool isPermWrite = 0;
    //uint16_t swVal;
    //uint8_t index;
    
//    bleRxBuffer[0]; //length
    
    myPrintf("Pro rec: ");
    
    for(int i = 0 ; i < bleRxBuffer[0] ; i++)
    {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    
    myPrintf("\n");
    
    if(((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) // check that the app sent the correct type of packet
    {
        if((bleRxBuffer[1] & 0x01) == 1)    // read
        {
            sendProPacket = NUM_PRO_SYNC_PACKETS;
            
            //is32sw = (bleRxBuffer[2]) ? 1 : 0;
            
            return;
        }
        
        if((bleRxBuffer[1] & 0x02) > 0)    // isn't locked
        {
            isPermWrite = true;
        }
        else
        {
            isPermWrite = false;
        }
        
        if(settings->isWritable != isPermWrite)
        {
            settings->isWritable = isPermWrite;
            updateFlash = 1;
            
        }
        
        if((bleRxBuffer[1] & 0x04) > 0)    // is writable until ble disconnect
        {
            isTempWritable = true;
        }
        else
        {
            isTempWritable = false;
        }
        
        if(settings->isWritable != 0)
        {
            isTempWritable = true;
        }
        
        if(settings->isProMode == 0)     // set into "pro" mode
        {
            settings->isProMode = 1;
            
//            writeFlash();
            updateFlash = 1;
            
            myPrintf("PRO Mode enabled!\n");
        }
        
        if((bleRxBuffer[2] & 0x02) != 0 && isTempWritable) // update deep sleep flag
        {
            uint8_t nds = bleRxBuffer[2] & 0x01;
            if(settings->noDeepSleep != nds)
            {
                myPrintf("Deep Sleep disabled %d \n", nds);
                settings->noDeepSleep = nds;
                updateFlash = 1;
            }
        }
        
        isSyncing = (bleRxBuffer[1] & 0x08) ? 1 : 0;
        
        if(updateFlash)
        {
            saveSettings();
        }
        
        
    }
    
    
}

void writePassKey(uint32 thisKey)
{
    uint32 newPasskey = thisKey;
    
//    return;s
    if(newPasskey > 999999)
    {
        newPasskey = 0;
    }
    
    CYBLE_GATT_ERR_CODE_T apiGattErrCode = 0;

    CYBLE_GATT_HANDLE_VALUE_PAIR_T handleValuePair;
    
    handleValuePair.value.val = (uint8 *)&newPasskey;
    handleValuePair.value.len = sizeof(newPasskey);
    handleValuePair.attrHandle = CYBLE_SPOD_PASSKEY_CHAR_HANDLE;
    
    while (CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_BUSY)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    /* To register the service change in the Database of the GATT Server */
    apiGattErrCode = CyBle_GattsWriteAttributeValue(&handleValuePair, 0u, NULL, CYBLE_GATT_DB_LOCALLY_INITIATED);
    
    if (apiGattErrCode != 0)
    {
        myPrintf("Service Changed Attribute DB write failed: %d\n", apiGattErrCode );
    }
     else
    {
        myPrintf("Service Changed Attribute DB write success: ");
        
        if(newPasskey) {
//            myPrintf("PASSKEY ON %d\n", millis());
            myPrintf("PASSKEY ON: %d ", newPasskey);;
            write_millis(millis());
        } else {
            myPrintf("PASSKEY OFF: %d ", newPasskey);
            write_millis(millis());
        }
    }
}

uint8_t localName[] = "sPOD Link #00123456";

#define CYREG_SRSS_TST_DDFT_CTRL 0x40030008

void StackEventHandler( uint32 eventCode, void *eventParam )
{
    switch( eventCode )
    {

        case CYBLE_EVT_HOST_INVALID:
        break;

        case CYBLE_EVT_STACK_ON:
        
        /* Configure the Link Layer to automatically switch PA control pin P3[2] and LNA control pin P3[3] */
        CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_BB_XO_CAPTRIM), 0xD0D0); 
        
            CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLESS_RF_CONFIG), 0x0331); 
            CY_SET_XTND_REG32((void CYFAR *)(CYREG_SRSS_TST_DDFT_CTRL), 0x80000302);
        
            CYBLE_GAP_BD_ADDR_T localAddr;
            
            localAddr.type = 0x00;
            
            CyBle_GetDeviceAddress(&localAddr);
            
            sprintf((char *)&localName[11], "%02X", DEVICE_TYPE);
            
            int index = 0;
            for(int i = 3; i > 0; i--)
            {
               myPrintf("%x ", localAddr.bdAddr[i-1]); 
               sprintf((char *)&localName[13 + index], "%02X", localAddr.bdAddr[i-1]);
               index += 2;
            }
            
            myPrintf("\n");
            
            myPrintf("%s\n", localName);
            
            CyBle_GapFixAuthPassKey(1, settings->passkey);
            CyBle_GapSetLocalName((const char8*)&localName[0]);
            CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST );
        break;

        case CYBLE_EVT_TIMEOUT:
        break;

        case CYBLE_EVT_HARDWARE_ERROR:
        break;

        case CYBLE_EVT_HCI_STATUS:
        break;

        case CYBLE_EVT_STACK_BUSY_STATUS:
        break;

        case CYBLE_EVT_PENDING_FLASH_WRITE:
        break;
        
        case CYBLE_EVT_GAP_AUTH_REQ:
            myPrintf("CYBLE_EVT_GAP_AUTH_REQ \n");
        break;

        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            myPrintf("CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST \n ");

        break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            {
                myPrintf("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST \n");
                
                if(bleDisconnect)
                {
                    myPrintf("Disconnecting...\n");
                    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                    break;
                }
                
                uint32 thisKey = *(uint32*)eventParam;
                
                writePassKey(thisKey);
                
                if(settings->passkey == 0)
                {
                    settings->passkey = thisKey;
                    saveSettingsFlag = 1;
                }
                
                myPrintf("Passkey is: %d.\n", thisKey);
                myPrintf("Please enter the passkey on peer device.\n");
            
            
            }
        break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            myPrintf("CYBLE_EVT_GAP_AUTH_COMPLETE...\n");
            writePassKey(0);
            
            authGood = true;
            
        break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
            myPrintf("CYBLE_EVT_GAP_AUTH_FAILED...\n");
            
            authGood = false;
            
            if(bleDisconnect)
            {
                myPrintf("Disconnecting...\n");
                CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
            }
        break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            myPrintf("BLE Connected...\n");
            isTempWritable = settings->isWritable;
            isConnected = 1;
        break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            myPrintf("BLE Disconnected...\n");
            isConnected = 0;
            sendNotifications = 0;
            isSyncing = false;
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
        break;

        case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        break;

        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
        break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        break;

        case CYBLE_EVT_GAPC_SCAN_START_STOP:
        break;

        case CYBLE_EVT_GATT_CONNECT_IND:
        break;

        case CYBLE_EVT_GATT_DISCONNECT_IND:
        break;

        case CYBLE_EVT_GATTC_ERROR_RSP:
        break;

        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BLOB_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_MULTI_RSP:
        break;

        case CYBLE_EVT_GATTC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
        break;

        case CYBLE_EVT_GATTC_INDICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
        break;

        case CYBLE_EVT_GATTS_WRITE_REQ:                        
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
            {
                CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
                
                //myPrintf("BLE Write...\n");
                
                 /* Handling Notification Enable */
    			if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    	        {
                    //myPrintf("CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE...\n");
                    
    				CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
    				uint8 ccdValue[2];
    				
    	            /* Extract CCCD Notification enable flag */
    	            sendNotifications = wrReqParam->handleValPair.value.val[0];
    				
    				/* Write the present notification status to the local variable */
    				ccdValue[0] = sendNotifications;
    				
    				ccdValue[1] = 0x00;
    				
    				/* Update CCCD handle with notification status data*/
    				notificationCCDHandle.attrHandle = CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    				
    				notificationCCDHandle.value.val = ccdValue;
    				
    				notificationCCDHandle.value.len = 2;
    				
    				/* Report data to BLE component for sending data when read by Central device */
    				CyBle_GattsWriteAttributeValue(&notificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
    	        }
    				
    	        /* Handling Write data from Client */
    	        else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CHAR_HANDLE)
    	        {
    					//myPrintf("CYBLE_SPOD_COMM_CHAR_HANDLE...\n");
                        
                        uint8 * inData = wrReqParam->handleValPair.value.val;
                        int i;
                        for(i = 0; i < wrReqParam->handleValPair.value.len; i++)
                        {
                            if(bleFoundDelimiter)
                            {
                                if(blePacketLength == -1)
                                {
                                    if(inData[i] <= 32)
                                    {
                                        blePacketLength = inData[i] + 2;
                                        bleRxBuffer[bleBufferAddress] = inData[i];
                                        bleBufferAddress++;
                                    }
                                    else
                                    {
                                        bleFoundDelimiter = 0;
                                    }
                                }
                                else
                                {
                                    bleRxBuffer[bleBufferAddress] = inData[i];
                                    bleBufferAddress++;
                                    
                                    if(bleBufferAddress == blePacketLength)
                                    {
                                        //myPrintf("ProcessPacket...\n");
                                        ProcessPacket(blePacketLength);
                                        bleFoundDelimiter = 0;
                                    }
                                }
                            }
                            else
                            {
                                if(inData[i] == 0x55)
                                {
                                    bleBufferAddress = 0;
                                    blePacketLength = -1;
                                    bleFoundDelimiter = 1;
                                    bleRxBuffer[bleBufferAddress] = inData[i];
                                    bleBufferAddress++;
                                }
                            }
                        }
    	        }
                else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PASSKEY_CHAR_HANDLE)
    	        {
                        passkeyTimer = millis();
                        myPrintf("Passkey timer reset %d\n", passkeyTimer);
    	        }
//                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE)
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE)
                {
                    uint8 * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processOtaPacket();
                    
//                    enterBootloader();
                }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PRO_MODE_CHAR_HANDLE)
                {
                    uint8 * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processProPacket();
                }
                    
                if (eventCode == CYBLE_EVT_GATTS_WRITE_REQ)
    			{
                    //myPrintf("CYBLE_EVT_GATTS_WRITE_REQ...\n");
    	            CyBle_GattsWriteRsp(cyBle_connHandle);
    			}
            }
        break;

        case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
        break;

        case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
        break;

        case CYBLE_EVT_GATTS_INDICATION_ENABLED:
        break;

        case CYBLE_EVT_GATTS_INDICATION_DISABLED:
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
        break;

        case CYBLE_EVT_L2CAP_COMMAND_REJ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
        break;

        default:
        break;
    }
}
/* [] END OF FILE */
