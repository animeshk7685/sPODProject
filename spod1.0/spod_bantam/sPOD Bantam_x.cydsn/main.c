/* ========================================
 *
 * Copyright Star Technologies, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF STAR TECHNOLOGIES.
 *
 *  Author: Luke Williams
 *
 *  Version 1.00    12-16-2017
 *  Version 1.01    12-19-2017
 *  Version 1.02    12-26-2017
 *  Version 1.03    01-11-2017
 *  Version 1.04    01-11-2017
 *  Version 1.05    01-15-2017
 *  Version 1.06    01-15-2017
 *
 * ========================================
*/
//#include <project.h>
//#include <stdbool.h>
//
//#include <stdlib.h>
//#include <stdio.h>
//#include "myPrintf.h"
//
//#include "stdDrivers.h"
//
//#include "mcpCan.h"
//#include "pt.h"
//#include "timer.h"
//
//#include "ota_mandatory.h"

#include "common.h"

void saveSettings();

void StackEventHandler( uint32 eventCode, void *eventParam);

//1ms
#define NUMBER_OF_TICKS		 48000

#define IS_PRO_TESTING         (0)     // set to 0 for releases, 1 to default to pro
#define IS_PRO_INPUTLINK_TESTING         (0) 

#define DEEP_SLEEP_FORCE_ON  (false)



static uint32 crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
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

mcpCan can;

struct state {
  char *name;
  struct pt pt;
  struct timer timer;
};


struct canBuffer {
    uint8_t rx_buf[5];
};


enum sPOD_Device_Type {
    BANTAM_V1,
    TOUCHSCREEN_V1,
    SWITCH_HD_V1,
    SOURCE_LT_V1
};

#define DEVICE_TYPE BANTAM_V1

uint8 lastTemp[4] = {0};
uint8 lastBatt[4] = {0};

//uint8_t lastSwitchValue[32] = {0};
//uint8_t lastOutputState[32] = {0};
//uint16_t lastOutputValue[32] = {0};
uint32 passkey = 1234;

//uint8 canState;

//enum open {OUTPUT_OPEN, OUTPUT_OFF, OUTPUT_SHORT, OUTPUT_NOM, OUTPUT_FAULT, OUTPUT_STROBE, OUTPUT_FLASH};
enum open {
    OUTPUT_OFF = 0, 
    OUTPUT_OPEN = 1, 
    OUTPUT_NOM = 2,
    
    OUTPUT_FAULT = 4, 
    OUTPUT_FLASH = 5, 
    OUTPUT_STROBE = 6, 
    OUTPUT_BLINK = 7, 
    
    OUTPUT_SHORT = 8 
};

enum canState {CAN_LISTEN, SYSTEM_UPDATE, OUTPUTS_1, OUTPUTS_2, CAN_DEBUG};

status_t status;

uint8_t sourceConnected[4] = {0};
uint32_t srcConnTimer[4] = {0};

//uint32 ioAddress1 = 0x20;
//uint32 ioAddress2 = 0x24;
//bool isConnected = 0;
bool sendNotifications = 0;
//bool pairingNeeded = 0;
//bool pinGenerationNeeded = 0;
//bool pinClearNeeded = 0;
//uint16 tempPin = 0;
//uint8 i2cBuffer[3];

uint8 bleRxBuffer[32];
uint8 bleTxBuffer[20];

uint32 aliveTimer = 0;
//uint32 switchTimer = 0;
////uint32 switchDelay = 0;
//uint32 debugTimer = 0;
//uint32 systemTimer = 0;
////uint32 proTimer = 0;

bool isAwake = 1;
bool wasAsleep = 0;
bool overheated = 0;
bool is12v_not24 = 1;
//bool catastophic_fault = 0;

bool overheated2 = 0;

volatile uint32 ticks = 0;


#define SWITCH_PACKET   0x80
#define DEBUG_PACKET    0x90
#define SYSTEM_PACKET   0xA0
#define STATUS_PACKET   0xB0
#define PRO_PACKET      0xC0
#define CONNECTED       0x7E
#define SWITCH_ADDR     0x53
#define SWITCH_DATA_BUFFER_SIZE 5

#define FLASH_SPEED     20      // __ms = 1 "tick"
#define SHUTOFF_TEMP    125.0   // in deg C, max 150C


#define WDT_INTERVAL_1S             250u                       /* millisecond */
#define ILO_FREQ                    32768     

volatile uint8 wdtIsrCount = 0;
         

volatile struct canBuffer can_buf[16];
//struct canTxBuffer can_buf[32];

volatile uint8_t canBufLoad = 0;
volatile uint8_t canBufRead = 0;

//uint8_t tx_buffer[5] = {0x00};
//uint8_t rx_buffer[5] = {0x00};
uint8 bytesRead = 0;

void InitBrownout(uint32_t lvd_volt);
void InitWatchdog(uint16 reset_interval);
void resetWatchdog(void);

//uint32_t millis(void);
void printStatus(void);

unsigned int rand_interval(unsigned int min, unsigned int max);
void generatePasskey(void);
//uint32 readPasskey(void);
void writePassKey(uint32 thisKey);
//void write_millis(void);

//int16 adc_off;

void enterBootloader(void);

char ds = '0';      // debug state, to know where program was when wdt triggers

/* ISR prototype declaration */
CY_ISR_PROTO(USER_ISR);


CY_ISR(USER_ISR)
{
    ticks++;
}

CY_ISR_PROTO(CANRX_ISR);

CY_ISR(CANRX_ISR)
{    
    CANINT_ClearInterrupt();

    uint8 canStatus = mcpCanReadMsgBuf(&can, &bytesRead, (uint8_t *)(can_buf[canBufLoad].rx_buf));
    
//    if((can_buf[canBufLoad].rx_buf[0] & 0xF0) == 0x80)
//    {
//    
//        myPrintf("C: (%x %d) %x %x %x %x %x ", canBufLoad, canStatus, can_buf[canBufLoad].rx_buf[0], can_buf[canBufLoad].rx_buf[1],
//                can_buf[canBufLoad].rx_buf[2], can_buf[canBufLoad].rx_buf[3], can_buf[canBufLoad].rx_buf[4]);
//        write_millis();
//    
//    }
    
    if(canStatus == CAN_OK)
    {
        canBufLoad++;
        canBufLoad = canBufLoad % 16;
    }
    
    if(mcpCanCheckError() != CAN_OK)
    {
        mcpCanClearError();   
    } 
    
}


CY_ISR_PROTO(WDT_ISR);

CY_ISR(WDT_ISR)
{     
    if(wdtIsrCount > 0)
    {
        myPrintf("WDT %d(%c): ", wdtIsrCount, ds);
        write_millis(millis());
    }
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


CY_ISR_PROTO(LVD_ISR);

CY_ISR(LVD_ISR)
{     
    myPrintf("LVD: ");
    write_millis(millis());

    
    ISR_LVD_Stop();
    CySysLvdClearInterrupt();   
    
    while(1);       // sys tick is overridden by this
    
}

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

void InitBrownout(uint32_t lvd_volt)
{
    CySysLvdEnable(lvd_volt);
    
    ISR_LVD_StartEx(LVD_ISR);
    
    ISR_LVD_SetPriority(2);
}

void resetWatchdog(void)
{
    CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);  
            /* clearing watchdog counter requires several LFCLK cycles to take effect */
    CyDelayUs(150);
    
    wdtIsrCount = 0;
}


uint32_t millis(){
    return ticks;
}



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


#define PRO_NUM_SYNC_PACKETS    (8) // 8 x switches + ...

CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};

#define PASS_DBG_TIMEOUT    30000       //    (30 * 1000)
#define PASSKEY_TIMEOUT     180000      //(3 * 60 * 1000)

static uint32_t passkeyTimer = 0;
//static bool passkey_init = false;

bool proIsTempWritable = false;

//void writeFlash(void);

void generatePasskey(void)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
   
    myPrintf("Generate passkey - ");
    myPrintf("Prepare to wipe bonding\n");
    
        if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
        {
            CyBle_GappStopAdvertisement ();
        } 
        
        
    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
    
//    write_millis();
    
    while (CyBle_GetState() == CYBLE_STATE_CONNECTED)
 	{
 	    CyBle_ProcessEvents();
 	}
    
//    write_millis();
    
    apiResult = CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
    
    if(apiResult != CYBLE_ERROR_OK)
    {
        myPrintf("ERR: CyBle_GapRemoveDeviceFromWhiteList() %d\n", apiResult);
    }
    
    while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
        

//    CYBLE_GAP_BD_ADDR_T localAddr;
//    localAddr.type = 0x00;
//    CyBle_GetDeviceAddress(&localAddr);
//    
//    for(int i = 3; i > 0; i--)
//    {
//       localAddr.bdAddr[i-1];
//    }
    
    passkey = rand_interval(1, 999999);
    
//    passkey_init = true;
    
    writeFlash();
    
    CyBle_GapFixAuthPassKey(1, passkey);
    writePassKey(passkey);
    
    passkeyTimer = millis();
    
    myPrintf("New passkey: %d\n\n", passkey);
    
    
    
    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
    
//    if(apiResult != CYBLE_ERROR_OK)
//    {
//        myPrintf("ERR: CyBle_GappStartAdvertisement() %d\n", apiResult);
//    }
}

//bool unsecureBleMode = false;

#define UNSECURED   (true)
#define SECURED     (false)

//bool isResetForSecUpdate = false;
//bool isDeepSleepEnabled = false;

void updateBleSecurity(uint8 turnOff)
{   
    static uint8_t lastOnOff = 0;
    
    myPrintf("updateBleSecurity()\n");
    
    if(lastOnOff == turnOff)
    {
        myPrintf("skipping... already ");
        
        if(turnOff)
        {
            myPrintf("unsecured\n\n");
        }
        else
        {
            myPrintf("secured\n\n");
        }
        
        return;
    }
    
    myPrintf("isUnsecured: %d\n", turnOff);
    
    while (CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_BUSY)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    CyBle_Stop();
    
    while (CyBle_GetState() != CYBLE_STATE_STOPPED)
 	{
 	    CyBle_ProcessEvents();
 	}
       
    if(turnOff)
    {
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_1 );
        
//        unsecureBleMode = false;
    }
    else
    {
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_3 );
        
//        unsecureBleMode = true;
    }
    
    CyBle_Start( StackEventHandler );

    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    lastOnOff = turnOff;
    myPrintf("updateBleSecurity completed\n\n");
}

void setSecurityLevel(bool unsecured)
{
    myPrintf("setSecurityLevel(%d/%d)\n", unsecured, status.isUnsecured);
    if(unsecured != status.isUnsecured)
    {
        status.isUnsecured = unsecured;
        status.isResetForSecUpdate = true;
        writeFlash();
        updateBleSecurity(status.isUnsecured);
        
        myPrintf("setSecurityLevel: reseting to apply\n");
        CyDelay(10);
        
        CySoftwareReset();
    }
}

uint32_t lastIgnTog = 0;
uint32_t lastIgnSig = 0;



#define IGNORE_OFF_TIME_MS  (6000)

uint32_t offTimer[8] = {0};
uint32_t globalOffTimer[8] = {0};
uint8_t globalWasOn[8] = {0};

uint32 debugOnTimer = 0;

bool isLockedFault = false;
bool isTempEdgeDisabled =  false;
bool needsInputsUpdate = true;

void turnLedsOn(bool isTrue)
{
    if(isLockedFault)
        return;
    
    PS_LEDS_Write(isTrue);
}

uint32_t lastActivityMs = 0;

// implement prioritized CAN tx fifo?

#define CAN_TX_FIFO_NUM (32)
static struct canBuffer canTxFifoBuffer[CAN_TX_FIFO_NUM] = {0};
static uint8_t canTxFifoHead = 0, canTxFifoTail = 0;

void pushCanFifo(uint8_t* buf) // uint8_t priority)
{
    for(int i = 0; i < 5; i++)
    {
        canTxFifoBuffer[canTxFifoHead].rx_buf[i] = buf[i];
    }
    
    canTxFifoHead++;
    canTxFifoHead %= CAN_TX_FIFO_NUM;
    
    if(canTxFifoHead == canTxFifoTail)      // to avoid overtaking
    {
        canTxFifoTail++;
        canTxFifoTail %= CAN_TX_FIFO_NUM;
    }
}

uint8_t popCanFifo(uint8_t* buf)
{
    if(canTxFifoHead != canTxFifoTail)
    {
        for(int j = 0; j < 5; j++)
        {
            buf[j] = canTxFifoBuffer[canTxFifoTail].rx_buf[j];
        }
        
        canTxFifoTail++;
        canTxFifoTail %= CAN_TX_FIFO_NUM;
        
        return 1;
    }
    return 0;
}

static struct canBuffer linkBuffer[32] = {0};
static uint32_t needsLinksSent = 0;

void addToCanLinkBuf(uint8_t index, uint8_t* buf)
{
    if(index > 31)
        return;
    
    for(int i = 0; i < 5; i++)
    {
        linkBuffer[index].rx_buf[i] = buf[i];
    }
    
    needsLinksSent |= (0x00000001 << index);
}

uint8_t popCanLinkBuf(uint8_t* buf)
{
    static int i = 0;
    
    //if(needsLinksSent){
    //for(int i = 0; i < 32; i++)
    for(; needsLinksSent; ++i >= 32 ? i = 0 : i)
    {
        if(needsLinksSent & (0x00000001 << i))
        {
            needsLinksSent &= ~(0x00000001 << i);
            
            for(int j = 0; j < 5; j++)
            {
                buf[j] = linkBuffer[i].rx_buf[j];
            }
            
            return i + 1;
        }
    }
        
    return 0;
}

PT_THREAD(readInputs(struct state *s))
{
//    static adcVals_t localCur;
    static uint16 read;
//    static uint8 faults, newVal, lastVal;
    static uint8 newVal, lastVal;
    static int i;//, j;
//    static double avgCurrentsDec[8];
//    static double avgCurrentsHex[8];
//    static uint32_t proShortTimer[8] = {0};
    
//    static uint8 avgF = 3;
    
    static uint8_t debugCycle = 0;
    static bool passDebug = true;
//    static bool outOn = false;
    
    static bool isSecListen = true;
    static uint8_t lastSec1 = 0xff;
    static uint8_t lastSec2 = 0xff;
    static uint8_t secCycle1 = 0;
    static uint8_t secCycle2 = 0;
    
//    static uint8_t lastWasErr[8] = {0};
    
    PT_BEGIN(&s->pt);
    while(1)                // 
    {        
        timer_set(&s->timer, 5);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
        read = (uint16_t)read_inputs(0); 
        
        status.debug = read & DEBUG_MASK;
        
        if((debugCycle % 2) != status.debug) {
            debugCycle++;
            
            if(status.debug) {
                passkeyTimer = millis();        // reset on rising edge of debug switch
//                writePassKey(passkey);
            }
        }
        
        if(passDebug) {  
            if(debugCycle > 3) {
                generatePasskey();              // only regenerate if switch it tripped 4 times in the first 30s
                passDebug = false; 
            }
            
            if(millis() > PASS_DBG_TIMEOUT) {
                passDebug = false;              
            }
        }
        
        if(isSecListen)
        {
            if(status.isResetForSecUpdate)
            {
                passkeyTimer = 1;
                status.isResetForSecUpdate = false;
                writeFlash();
            }
            
            uint8_t addRead1 = read & SW5_MASK ? 1 : 0;
            uint8_t addRead2 = read & SW6_MASK ? 1 : 0;
            
            if(lastSec1 == 0xff || lastSec2 == 0xff)
            {
                lastSec1 = addRead1;
                lastSec2 = addRead2;
            }
            else if(lastSec1 != addRead1)
            {
                lastSec1 = addRead1;
                
                secCycle1++;
                
                if(secCycle1 == 2)
                {
                    setSecurityLevel(UNSECURED);
                }
            }
            else if(lastSec2 != addRead2)
            {
                lastSec2 = addRead2;
                
                secCycle2++;
                
                if(secCycle2 == 2)
                {
                    setSecurityLevel(SECURED);
                }
            }
            
            if(millis() > PASS_DBG_TIMEOUT) {
                isSecListen = false;              
            }
        }

        status.ignCh[0] = read & IGNCH0_MASK;
        status.ignCh[1] = read & IGNCH1_MASK;

        bool isIgn = (read & IGN_CTRL_MASK) ? 1 : 0;
        
        if(status.ignSense != isIgn)
        {
            lastIgnSig = millis();
            status.ignSense = isIgn;
        }
            
        if(isAwake && (lastVal != (read & INPUTS_MASK) || needsInputsUpdate))     // if an input has changed, update the corresponding output
        {
            bool turnOutOn = false;
            bool turnOutOff = false;
            
            lastActivityMs = millis();
            
            for(i = 0 ; i < 8 ; i++)
            {
                newVal = (read >> (7-i)) & 0x01;
                
                if(needsInputsUpdate || (status.out[i].shortDelay == 0 && newVal != status.inputs[i]))
                {
                    if(status.pro.isEnabled)
                    {
                        if(status.pro.isInputEnabled[i])
                        {
                            if(status.pro.isInputLatch[i])
                            {
                                if(status.inputs[i] < newVal) {
                                    turnOutOn = true;
                                }
                                
                                if(status.inputs[i] > newVal) {
                                    turnOutOff = true;
                                }
                            }
                            else
                            {
                                if(status.inputs[i] < newVal)   // toggle on "rising" edge
                                {
                                    if(status.out[i].outCmd == 0) {
                                        turnOutOn = true;
                                    } else {
                                        turnOutOff = true;
                                    }
                                }
                            }
                            
                            if(status.pro.isInputLockout[i])
                            {
                                turnOutOn = false;
                                
//                                if(newVal > 0)//status.inputs[i] == 0)//< newVal)
//                                {
//                                   status.pro.isInputLockActive[i] = !status.pro.isInputLockInvert[i];
//                                }
//                                else
//                                {
//                                    status.pro.isInputLockActive[i] = status.pro.isInputLockInvert[i];
//                                }
                                
                                status.pro.isInputLockActive[i] = newVal > 0 ? true : false;
                                
                                if(status.pro.isInputLockActive[i])
                                {
                                    turnOutOff = true;
                                }
                            }
                            else
                            {
                                status.pro.isInputLockActive[i] = false;
                            }
                            
                        }
                        else
                        {
                            status.pro.isInputLockActive[i] = false;
                        }
                    }
                    else
                    {
                        if(status.inputs[i] < newVal) {
                            turnOutOn = true;
                        } else {
                            turnOutOff = true;
                        }
                        
                    }
                    
                    if(turnOutOn || turnOutOff)
                    {
                        status.out[i].outCmd = (turnOutOn ? 0xFF : 0x00);
                        status.out[i].blinkOff = 0x00; 
                        
                        if(turnOutOn)
                        {
                            status.out[i].outCmd = status.pro.lastRecVals[i + 8*status.address].dimVal;
                            status.out[i].blinkOn = status.pro.lastRecVals[i + 8*status.address].onVal; 
                            status.out[i].blinkOff = status.pro.lastRecVals[i + 8*status.address].offVal;
                        }
                        else
                        {
                            status.out[i].outCmd = 0x00;
                            status.out[i].blinkOff = 0x00; 
                        }
                        
                        if(status.pro.isEnabled && status.pro.isInputLinking && status.pro.links[i] != 0)
                        {
                            for(uint8_t j = 0; j < 32; j++)
                            {
                                if(status.pro.links[i] & (0x00000001 << j) || j == (status.address*8 + i))
                                {
                                    uint8_t txBuf[5];
                                        
                                    txBuf[0] = 0x80 | (j/8);
                                    txBuf[1] = iToCan(j%8);
                                    
                                    if(turnOutOn)
                                    {
                                        txBuf[2] = status.pro.lastRecVals[j].dimVal;
                                        txBuf[3] = status.pro.lastRecVals[j].onVal; 
                                        txBuf[4] = status.pro.lastRecVals[j].offVal;
                                    }
                                    else
                                    {
                                        txBuf[2] = 0x00;
                                        txBuf[3] = 0xff;
                                        txBuf[4] = 0x00;
                                    }
                                    
                                    addToCanLinkBuf(j, txBuf);
                                    
                                        
                                    if(j / 8 == status.address)
                                    {
                                        if(j%8 == i)
                                            continue;
                                        
                                        if(turnOutOn)
                                        {
                                            status.out[j%8].outCmd = status.pro.lastRecVals[j].dimVal;
                                            status.out[j%8].blinkOn = status.pro.lastRecVals[j].onVal; 
                                            status.out[j%8].blinkOff = status.pro.lastRecVals[j].offVal;
                                        }
                                        else
                                        {
                                            status.out[j%8].outCmd = 0x00;
                                            status.out[j%8].blinkOff = 0x00; 
                                        }
                                    }
                                        
                                }
                                
                                
//                                if(status.pro.links[i] & (0x01 << j))
//                                {
//                                    status.out[j].outCmd = (turnOutOn ? 0xFF : 0x00);
//                                    status.out[j].blinkOff = 0x00; 
//                                }
                            }
                        }
                    }
                    
//                    if(turnOutOn)
//                    {
//                        status.out[i].outCmd = 0xFF;
//                        status.out[i].blinkOff = 0x00;
//                    }
//                    
//                    if(turnOutOff)
//                    {
//                        status.out[i].outCmd = 0x00;
//                        status.out[i].blinkOff = 0x00;
//                    }
                    
                }
                
                turnOutOn = false;
                turnOutOff = false;
                
                status.inputs[i] = newVal;
            }
            
            lastVal = read & INPUTS_MASK;
            
            needsInputsUpdate = 0;
        }
        
//        status.tempEdge = read & TEMP_EDGE_MASK;
        status.tempEdge = TEMP_TRIG_Read();
        
        
        timer_set(&s->timer, 5);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));        
        
        
        status.batVolt = vbat_read();
        
//        if((status.batVolt.dec >= 15.6 && status.batVolt.hex < 0xC0) || status.batVolt.dec >= 16.0)
        if(status.batVolt.dec > 15.7)
        {
            is12v_not24 = 0;
        }
        else
        {
            is12v_not24 = 1;
        }
        
        
        status.temp = temp_read();
        

        if(status.temp.dec >= SHUTOFF_TEMP)
        {
            overheated = 1;
        }
        
        if(!isTempEdgeDisabled && !status.tempEdge)
        {
            overheated2 = 1;
        }
        
    }
    PT_END(&s->pt);
}
   
PT_THREAD(readCurrents(struct state *s))
{
    static adcVals_t localCur;
    
    static uint8_t faults;
    static int i, j;
    static bool outOn = false;
    
    static uint8_t avgF = 3;
    
    static double avgCurrentsDec[8];
    static double avgCurrentsHex[8];
    static uint32_t proShortTimer[8] = {0};
    
    static uint8_t lastWasErr[8] = {0};
    
    PT_BEGIN(&s->pt);
    while(1)
    {     
        faults = 0;
        
        timer_set(&s->timer, 10);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
        for(i = 0; i < 8 ; i++)
        {
            outOn = 0;
            
            timer_set(&s->timer, 10);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            pre_read(i);        // takes ~3ms
            
            if((status.out[i].blinkOff != 0) && (status.out[i].outCmd != 0))
            {
                j = 0;
                
                while(status.out[i].output == 0)                // if blink mode, wait for output to be on
                {
                    j++;
                    
                    timer_set(&s->timer, 1);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                    
                    if(j > 500)
                    {
                        continue;
                    }
                }
                
                outOn = 1;
            }
        
            timer_set(&s->timer, 10);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            
            localCur = out_current(i);      // read current
            
            
            if((outOn == 1) && (status.out[i].output == 0))     // check blink didn't turn off while reading
            {
                i--;
                continue;
            }
            
           
            if(status.out[i].blinkOff == 0 && ((status.out[i].outCmd == 0) || (status.out[i].outCmd == 0xFF))) // std (not blink or pwm)
            {
                
                if((offTimer[i] == 0) || (millis() - offTimer[i] > 6000))
                {
                    avgF = 3;
                }
                else
                {
                    if(localCur.dec > 0.5)                  // filter out back emf fault
                    {
                        avgF = 100;
                    }
                    else
                    {
                        avgF = 3;
                    }
                }
                
            }
            else    // non-standard: flash, strobe, pwm
            {
                if(status.out[i].outCmd != 0)
                { 
                    if(localCur.dec > 70.0f && status.out[i].outCmd < 0xFE)
                    {
                        if(lastWasErr[i] < 2)
                        {
                            localCur.dec = 0;
                            localCur.hex = 0;
                            
                            lastWasErr[i]++;
                        }
                    }
                    else
                    {
                        lastWasErr[i] = 0;
                    }
                    
                    if(status.out[i].outCmd < 0xFE)         // scale pwm currents
                    {
                        uint16_t scaledCurr = localCur.hex * status.out[i].outCmd / 0xFF;
                        
                        localCur.hex = approxRollingAverage(scaledCurr, localCur.hex, 4);   // 3:1
                    }
                    
                    
                    if(status.out[i].blinkOff != 0 && status.out[i].blinkOn > 0)         // scale blink mode currents/avoid /0
                    {
                        if(status.out[i].blinkOff / status.out[i].blinkOn == 4){
                            localCur.hex = localCur.hex / 4;
                        }else{
                            localCur.hex = localCur.hex / 2;
                        }
                    }
                    
                    
                    if(localCur.dec > 0.5)                  // filter out transient
                    {
                        avgF = 10;
                    }
                    else
                    {
                        avgF = 75;
                    }
                    
                    if(status.out[i].outCmd < 0x40)         // ignore short protection on pwm < %25
                    {
                        localCur.dec = 0;
                    }
                }
                else
                {
                    avgF = 3;
                }
                
            }
            
            if(globalOffTimer[i] > 0 && localCur.dec > 0.5) // supress flyback current spike
            {
                avgF = 100;
            }
            
            bool isCurrError = false;       // overCurrent/overTemperature/output overVoltage/etc...
            
            if(localCur.dec > 74.0f)
            {
                isCurrError = true;
                //error code...
            }
            
            if(globalOffTimer[i] > 0 && isCurrError)    // ignore flyback
            {
                localCur.dec = 0.0f;
                localCur.hex = 0;
            }
            else
            {
                avgCurrentsDec[i] = approxRollingAverage(avgCurrentsDec[i], localCur.dec, avgF);
                avgCurrentsHex[i] = approxRollingAverage(avgCurrentsHex[i], localCur.hex, avgF);
            }
            status.out[i].current.dec = avgCurrentsDec[i];
            status.out[i].current.hex = avgCurrentsHex[i];
            
            bool needsHandleShort = false;
            bool needsShortRestart = false;
            
//            if(i == 4)
//            {
//                myPrintf("curr 4: %d %d %x/%x %f/%f\n", avgF, lastWasErr[i], localCur.hex, status.out[i].current.hex, fl(localCur.dec), fl(status.out[i].current.dec));
//            }
            
//            float currLimit = ((float)status.pro.currentLimit[i]);
            
            float currLimit = 45.0f;
            
            if(status.pro.isEnabled)
            {
                currLimit = ((float)status.pro.currentLimit[i]);
                
                if(status.pro.currentLimit[i] >= 30)
                {
                    currLimit = 45.0;
                }
            }
            
//            myPrintf("(%d)%f ", status.pro.currentLimit[i], fl(currLimit));
//            if(i == 7)
//            {
//                myPrintf("\n");
//            }
            if(false && status.out[i].outCmd > 0 && status.out[i].outCmd < 0xFE) // ignore pwm
            {
            }
            else if(globalOffTimer[i] > 0)   // ignore off (flyback) current for IGNORE_OFF_TIME_MS 
            {
                
            }
            else if(status.pro.isEnabled)
            {
                
                if (approxRollingAverage(localCur.dec, avgCurrentsDec[i], 2) < currLimit)//45.0)
                {
                    proShortTimer[i] = millis();
                }
                
                if(
                    (approxRollingAverage(localCur.dec, avgCurrentsDec[i], 2) > 65.0) || 
                    (
                        (status.out[i].blinkOff == 0 && status.out[i].outCmd >= 0xFE) && 
                        ( (offTimer[i] == 0) || (millis() - offTimer[i] > 2000) ) &&
                        (approxRollingAverage(avgCurrentsDec[i], localCur.dec, 3) > currLimit && (millis() - proShortTimer[i] > 3000))
                    )
                  )
                {
                    needsHandleShort = true;
                    
                    if(status.pro.isCurrentRestart[i] && !status.pro.needsRestart[i])
                    {
                        status.pro.lastOut[i].outCmd = status.out[i].outCmd;
                        status.pro.lastOut[i].blinkOn = status.out[i].blinkOn;
                        status.pro.lastOut[i].blinkOff = status.out[i].blinkOff;
                        
                        status.pro.needsRestart[i] = true;
                    }
                }
                
            }
            else
            {
                if(
                    (approxRollingAverage(localCur.dec, avgCurrentsDec[i], 2) > 65.0) || 
                    (
                        (status.out[i].blinkOff == 0 && status.out[i].outCmd >= 0xFE) && 
                        ( (offTimer[i] == 0) || (millis() - offTimer[i] > 2000) ) &&
                        (approxRollingAverage(avgCurrentsDec[i], localCur.dec, 4) > 45.0)
                    )
                  )
                {
                    needsHandleShort = true;
                }
            }
            
            
            if(needsHandleShort)
            {
                myPrintf("Fault/Short on SW%d: %f \n", i, fl(localCur.dec));
                
                 status.out[i].outCmd = 0;
    //                status.out[i].output = 0;
    //                status.out[i].blinkOn = 0xff;
                    status.out[i].blinkOff = 0;
                    status.out[i].open = OUTPUT_SHORT;
                    
                    offTimer[i] = 0;
                    
                    status.out[i].shortDelay = millis();
            }
            else if((status.out[i].shortDelay != 0) && (millis() - status.out[i].shortDelay > 3500))
            {
                status.out[i].shortDelay = 0;
                
//                if(status.pro.needsRestart[i])
//                {
//                    needsShortRestart = true;
//                    
//                }
            }
            else if(status.pro.needsRestart[i] && approxRollingAverage(localCur.dec, avgCurrentsDec[i], 2) < currLimit)
            {
                //status.out[i].shortDelay = 0;
                needsShortRestart = true;
            }
            
            
            if(needsShortRestart)
            {
                status.out[i].outCmd = status.pro.lastOut[i].outCmd;
                status.out[i].blinkOn = status.pro.lastOut[i].blinkOn;
                status.out[i].blinkOff = status.pro.lastOut[i].blinkOff;
                    
                status.pro.needsRestart[i] = false;
            }
            
            
            
            
            if(status.out[i].current.dec < 0.05) // Check open/short conditions
            {
                if(status.out[i].current.hex <= 2)
                {
                    status.out[i].current.hex = 0;
                }
                status.out[i].current.dec = 0;
                
                if(status.out[i].outCmd) 
                {
                    if(!status.out[i].shortDelay)
                    status.out[i].open = OUTPUT_OPEN;      // no current & powered
                } 
                else 
                {
                    if(!status.out[i].shortDelay)
                    status.out[i].open = OUTPUT_OFF;     // not powered
                }
            } 
            else if(status.out[i].outCmd) 
            {
                if(status.out[i].blinkOff != 0 && status.out[i].blinkOn != 0)// check if strobe or flash
                {
                    if(!status.out[i].shortDelay)
                    status.out[i].open = OUTPUT_FLASH;
                }
                else
                {
                    if(status.out[i].current.dec > 35.0 && status.out[i].outCmd > 200) // short
                    {
//                        status.out[i].open = OUTPUT_SHORT;         // over current
                        
                        myPrintf("Over current on SW%d \n", i);
                        
//                        status.out[i].outCmd = 0;
////                        status.out[i].output = 0;
////                        status.out[i].blinkOn = 0;
//                        status.out[i].blinkOff = 0;
                    }
                    else // nominal
                    {
                      if(!status.out[i].shortDelay) // || (millis() - status.out[i].shortDelay) > 1500)
                      status.out[i].open = OUTPUT_NOM;           // nominal operation  
                    }
                }
            } 
            else // fault
            {
                if(!status.out[i].shortDelay)
                status.out[i].open = OUTPUT_FAULT;
                
                faults++;
                
//                myPrintf("Fault on SW%d \n", i);
                
//                status.out[i].outCmd = 0;
////                status.out[i].output = 0;
////                status.out[i].blinkOn = 0;
//                status.out[i].blinkOff = 0;
                
                //fault_handler(i);  //i, status.out[i].current.hex))

                
                
            }  
        }
        
        
        if(faults > 2){
            adc_c_calibrate();
            faults = 0;
        }
        
    }
    PT_END(&s->pt);
}

//bool ignFlag0 = 0;
//bool ignFlag1 = 0;

bool ignFlag[8] = {0};

void updateOutput(uint8_t i)
{
    if((status.out[i].blinkOff != 0) && (status.out[i].blinkOn != 0))   // if any blink mode
    {
//        if((millis() - status.out[i].timer) <= (status.out[i].blinkOn))
//        {
//            status.out[i].output = status.out[i].outCmd;
//        }
//        else if((millis() - status.out[i].timer) <= (status.out[i].blinkOn + status.out[i].blinkOff))
//        {
//            status.out[i].output = 0x00;
//        }
//        else
//        {
//            status.out[i].timer = millis();
//        }
    }
    else
    {
        status.out[i].output = status.out[i].outCmd;
    }   
}

PT_THREAD(updateOutputs(struct state *s))
{
    static uint8 i;
//    static uint16 out;
//    static int32 onTimers[8];
    static uint16 lastOutputs[8];
    

    PT_BEGIN(&s->pt);
    while(1)
    {         
        
        timer_set(&s->timer, 4);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
//            PS_LEDS_Write(status.leds);

       
        for(i = 0; i < 8 ; i++)
        {
            
        
            if(overheated || overheated2)
            {
                
                if(!overheated2)
                {
                    myPrintf("Overheated... \n");
                    overheated2 = 1;
                }
                
                turnLedsOn(true);
                isLockedFault = true;
                
                status.out[i].outCmd = 0;
                status.out[i].output = 0;
                status.out[i].blinkOn = 0xff;
                status.out[i].blinkOff = 0;
                
            }
            else
            {
                if(status.pro.isEnabled)
                {
                    if(status.pro.isIgnCtrl[i] && ! status.ignSense)
                    {
//                        if(status.ignSense)
//                        {
//                            updateOutput(i); 
//                            ignFlag[i] = 0;
//                        }else
//                        {
                            status.out[i].output = 0; 
                            ignFlag[i] = 1;
//                        }
                    }
                    else
                    {
                        updateOutput(i); 
                        ignFlag[i] = 0;
                    }
                    
                    if(status.pro.isInputLockActive[i])
                    {
                        status.out[i].output = 0; 
                        status.out[i].outCmd = 0; 
                        status.out[i].blinkOff = 0;
                    }
                    
                }
                else 
                {
                    if(i <= 1)  // check ign ctrl for sw0/1
                    {
                        if(status.ignCh[i] && !status.ignSense){
//                            if(status.ignSense)
//                            {
//                                updateOutput(i); 
//                                ignFlag[i] = 0;
//                            }else
//                            {
                                status.out[i].output = 0; 
                                ignFlag[i] = 1;
//                                }
                        }else{
                            updateOutput(i); 
                            ignFlag[i] = 0;
                        }
                    }
                    else
                    {
                        updateOutput(i); 
                        ignFlag[i] = 0;
                    }
                }
            }
            
            if(status.out[i].outCmd > 0)    // manage timer to filter out flyback current
            {
//                if(globalOffTimer[i] > 0)
//                {
//                    myPrintf("Switch reset on: %d\n", i);
//                }
//                else if(globalWasOn[i] == 0)
//                {
//                    myPrintf("Switch on: %d\n", i);
//                }
                
                globalOffTimer[i] = 0;
            }
            else if(status.out[i].outCmd <= 0 && globalWasOn[i] > 0)
            {
                globalOffTimer[i] = millis() - 1;
//                myPrintf("Switch off: %d -> init timer\n", i);
            }
            else if(globalOffTimer[i] > 0 && (millis() - globalOffTimer[i] > IGNORE_OFF_TIME_MS))
            {
                globalOffTimer[i] = 0;
//                myPrintf("timer %d timout -> reset timer\n", i);
            }
            
            globalWasOn[i] = status.out[i].outCmd;
            
            if(status.out[i].blinkOff == 0)
            {
                c_pwm(i, status.out[i].output);
                
                if((lastOutputs[i] != status.out[i].output) && (status.out[i].output == 0))
                {
                    
                    if(status.out[i].open != OUTPUT_SHORT)
                    {
                        offTimer[i] = millis() - 1;
                    }
                }
                else if(millis() - offTimer[i] > 10000)
                {
                    offTimer[i] = 0;
                }
                
                lastOutputs[i] = status.out[i].output;
            }
            
            if(status.pro.isEnabled && status.pro.timers[i] > 0)
            {
                if(status.out[i].outCmd > 0 && status.pro.curTime[i] == 0)
                {
                    status.pro.curTime[i] = millis();
                }
                else if(status.out[i].outCmd == 0 && status.pro.curTime[i] != 0)
                {
                    status.pro.curTime[i] = 0;
                }
                else if(status.pro.curTime[i] != 0 && (millis() - status.pro.curTime[i] > (status.pro.timers[i] * 60 * 1000)))
                {
                    status.out[i].outCmd = 0; 
                    status.pro.curTime[i] = 0;
                }
            }
            
            timer_set(&s->timer, 1);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
            
            uint32_t time = millis();
            uint32_t time2;
//            int j = 0;
//            
//            if(0)
            for(uint8_t j = 0 ; j < 8 ; j++)    // handle blinking quickly
            {
                if((status.out[j].blinkOff != 0) && (status.out[j].blinkOn != 0))
                {
//                    if(status.out[j].outCmd == 0)
//                    {
//                    }
                    
                    time2 = time;
                    
                    time2 = time2 % (status.out[j].blinkOff + status.out[j].blinkOn);
                    
                    if(time2 > status.out[j].blinkOn)
                    {
                        status.out[j].output = 0;
                    }
                    else
                    {
                        status.out[j].output = status.out[j].outCmd;
                    }
                    
                    if(status.pro.isEnabled || j <= 1)
                    {
                        if(ignFlag[j])
                        {
                            status.out[j].output = 0;
                        }
                        
                    }
//                    else
//                    {
//                        if(j <= 1 && ignFlag[j])
//                        {
//                            status.out[j].output = 0;
//                        }
////                        if(ignFlag0 && (j == 0))
////                        {
////                            status.out[j].output = 0;
////                        }
////                        
////                        if(ignFlag1 && (j == 1))
////                        {
////                            status.out[j].output = 0;
////                        }
//                    }
                    
                    
                    if(status.out[j].output != lastOutputs[j])
                    {
                        lastOutputs[j] = status.out[j].output;
                        c_pwm(j, status.out[j].output);
                    }
                    
                }
                
                CyDelayUs(50);
                
            }
            
            
//            myPrintf("\n");
        }
        

    }
    PT_END(&s->pt);
}


void enterBootloader(void)
{
    myPrintf("Enter Bootloader Mode \n");
    
    outputs_off();
    
    Bootloadable_SetActiveApplication(0);
    Bootloadable_Load();
    CySoftwareReset();
    
}

static uint8_t currSwitchValue[32] = {0};
static uint8_t currBlinkOn[32] = {0};
static uint8_t currBlinkOff[32] = {0};

static uint8_t currCurrentHigh[32] = {0};
static uint8_t currCurrentLow[32] = {0};
static uint8_t currStatusValue[32] = {0};


static uint8_t currTemp[4] = {0};
//static uint8_t lastTemp[4] = {0};
static uint8_t currBatt[4] = {0};
//static uint8_t lastBatt[4] = {0};
static bool currBatt12_24[4] = {0};



bool sendOtaPacket = false;
bool sendSecPacket = false;
bool sendSettingsPacket = false;
uint8_t sendProPacket = false;
bool authGood = false;
//bool misInt = false;
//bool misErr = false;

bool globalUpdateProCanPackets = false;

#define INPUT_LINK_FLAG_CAN_MASK  (0x80)
#define INPUT_LINK_CAN_MASK (0x40)
#define LINKS_FLAG_MASK (0x20)

#define INPUT_EN_CAN_MASK   (0x04)
#define INPUT_LCK_CAN_MASK  (0x02)
#define INPUT_INV_CAN_MASK  (0x01)

PT_THREAD(canListen(struct state *s))
{
    static uint8 i;
//    static uint8 bytesRead = 0;
    static uint8 index = 0;
    
    static uint8_t rx_buffer[5] = {0x00};
    
    static uint32_t lastRec = 0;
    
    static int canResCnt = 0;
//    static uint32 debTime = 0;
//    static uint32 flshTime = 0;
    
//    static uint8 isShowErr = 0;
    static bool ledsOn = 0;
    static uint32 ledTime = 0;
    static uint32 ledTimeE = 0;
    
    static uint32 awakeLedTime = 0;
    
    static uint32 lastAlivePacket = 0;
    
    static bool misInt = false;
//    static bool misErr = false;
    
    PT_BEGIN(&s->pt);
    while(1)
    {     
        ds = 'L';

        timer_set(&s->timer, 1); 
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
        
        if(canBufRead != canBufLoad)    // if packet received
        {
            lastRec = millis();
            
            ds = 'M';
            
            isr_canint_Disable();
            
            for(uint8_t j = 0 ; j < 5 ; j++)
            {
                rx_buffer[j] = can_buf[canBufRead].rx_buf[j];
                can_buf[canBufRead].rx_buf[j] = 0;
            }
            
            canBufRead++;
            canBufRead = canBufRead % 16;
            
            isr_canint_Enable();
//        }
//        
//        if(canStatus == CAN_OK)
//        {
            ds = 'N';

            
            if(rx_buffer[0] == (DEBUG_PACKET | status.address)){
                
                if(rx_buffer[1] == 0x7E)
                {
                    lastAlivePacket = millis();
                    aliveTimer = millis();
                    isAwake = 1;
                    
//                    myPrintf("A\n");
                    
                    if(millis() > 10000)
                    {
                        turnLedsOn(!PS_LEDS_Read());
                        awakeLedTime = millis();
                    }
                   // myPrintf("alive packet\n");
                    // > addressed alive packet: reset timout timer    
                }
            }
            
            
            if((rx_buffer[0] & 0xF0) == SYSTEM_PACKET){
                
                srcConnTimer[rx_buffer[4] & 0x0F] = millis();
                
                if(sourceConnected[rx_buffer[4] & 0x0F] < 2)
                {
                    sourceConnected[rx_buffer[4] & 0x0F] = 1;
                }
                
                currBatt[rx_buffer[4] & 0x0F] = rx_buffer[1];
                currTemp[rx_buffer[4] & 0x0F] = rx_buffer[2];
                currBatt12_24[rx_buffer[4] & 0x0F] = rx_buffer[4] & 0x80 >> 7;
            }
            
            
            if((rx_buffer[0] & 0xF0) == SWITCH_PACKET && canToI(rx_buffer[1]) != 255){
                
//                myPrintf("SWITCH_PACKET: %x %x %x %x %x ", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4]);
//                write_millis();
                
                index = canToI(rx_buffer[1]) + 8 * (rx_buffer[0] & 0x0F);
                    
                if(rx_buffer[2] > 0) {
                    status.pro.lastRecVals[index].dimVal = rx_buffer[2];
                    status.pro.lastRecVals[index].onVal = rx_buffer[3];
                    status.pro.lastRecVals[index].offVal = rx_buffer[4];
                }
                
                if(rx_buffer[0] == (SWITCH_PACKET | status.address)){
                
                    lastActivityMs = millis();
                    
                    aliveTimer = millis();
                    isAwake = 1;
                    
                    i = canToI(rx_buffer[1]);
                    
                    if(status.pro.isEnabled && status.pro.isLockout[i])
                    {
                        
                    }
                    else if(status.out[i].shortDelay == 0)
                    {
//                        if(ignFlag[i] && (millis() - lastIgnSig) > 1000 && (millis() - lastIgnTog) > 200)
//                        {
                            
                            status.out[i].outCmd = rx_buffer[2]; 
                            
                            if(rx_buffer[2] == 0xFE){           // correct for legacy values
                                status.out[i].outCmd = 0xFF;
                            }
                        
//                        }
                        
                        if(status.pro.isEnabled && status.out[i].outCmd != 0)
                        {
//                            status.out[i].outCmd = status.pro.alwaysOn[i] ? status.pro.lastSwVal[i] : 0x00;
                            status.pro.lastSwVal[i] = status.out[i].outCmd;
                        }
                        status.out[i].blinkOn = rx_buffer[3] * FLASH_SPEED; 
                        status.out[i].blinkOff = rx_buffer[4] * FLASH_SPEED;
                    }
                    
                    
                }
                else
                {
                    //index = canToI(rx_buffer[1]) + 8 * (rx_buffer[0] & 0x0F);
                    
                    currSwitchValue[index] = rx_buffer[2];
                    currBlinkOn[index] = rx_buffer[3];
                    currBlinkOff[index] = rx_buffer[4];
                    
                }
                
                
                
            }
            
            if((rx_buffer[0] & 0xF0) == STATUS_PACKET && canToI(rx_buffer[1]) != 255){
            
                sourceConnected[rx_buffer[0] & 0x0F] = 2;
                
                index = canToI(rx_buffer[1]) + 8 * (rx_buffer[0] & 0x0F);
                    
                currStatusValue[index] = rx_buffer[2];
                currCurrentHigh[index] = rx_buffer[3];
                currCurrentLow[index] = rx_buffer[4];
            
            
            }
            
            if((rx_buffer[0] & 0xF0) == PRO_PACKET){
                
                myPrintf("rec Pro: %x %x %x %x %x \n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4]);
                
                if((rx_buffer[0] & 0x0F) == status.address && (rx_buffer[1] == 0 || canToI(rx_buffer[1]) != 255)){
                    
                    uint8_t index = canToI(rx_buffer[1]);
                    
                    if(rx_buffer[1] != 0)   // else ping request
                    {
                        status.pro.isInputEnabled[index] = (rx_buffer[2] & INPUT_EN_CAN_MASK) ? true : false;
                        status.pro.isInputLockout[index] = (rx_buffer[2] & INPUT_LCK_CAN_MASK) ? true : false;
//                        status.pro.isInputLockInvert[index] = (rx_buffer[2] & INPUT_INV_CAN_MASK) ? true : false;
                        
                        bool linkIndex = 0;
                        
                        if((rx_buffer[2] & 0xC0) == INPUT_LINK_CAN_MASK)
                        {
                            linkIndex = 1;
                        }
                        
                        if(rx_buffer[2] & LINKS_FLAG_MASK)
                        {
                            uint32_t linkHalf = (rx_buffer[3] << 8) | rx_buffer[4];
                            
                            if(linkIndex == 0)
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0xffff0000) | linkHalf;
                            }
                            else
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0x0000ffff) | (linkHalf << 16);
                            }
                            
                            myPrintf("links[%d] = %x\n", index, status.pro.links[index]);
                        }
                        
//                        if(rx_buffer[3] & (0x01 << index))
//                        {
//                            status.pro.links[index] = rx_buffer[3] & ~(0x01 << index);
//                        }
                        
                        writeFlash();
                        
                        needsInputsUpdate = true;
                    }
                    else
                    {
                        globalUpdateProCanPackets = true;
                    }
                    
//                    myPrintf("i: %d, e: %d, L: %d, i: %d \n", index, status.pro.isInputEnabled[index], status.pro.isInputLockout[index], status.pro.isInputLockInvert[index]);
                    //globalUpdateProCanPackets = true;
                }
                
                
                
//                srcConnTimer[rx_buffer[4] & 0x0F] = millis();
//                
//                if(sourceConnected[rx_buffer[4] & 0x0F] < 2)
//                {
//                    sourceConnected[rx_buffer[4] & 0x0F] = 1;
//                }
//                
//                currBatt[rx_buffer[4] & 0x0F] = rx_buffer[1];
//                currTemp[rx_buffer[4] & 0x0F] = rx_buffer[2];
//                currBatt12_24[rx_buffer[4] & 0x0F] = rx_buffer[4] & 0x80 >> 7;
            }
            
            ds = 'O';
        }
        else if(millis() > 30000 && lastRec != 0 && (millis() - lastRec) > 100)
        {
            lastRec = millis();
            
            if(CANINT_Read() == 0)  // packet rec interrupt missed
            {
                mcpCanReadMsgBuf(&can, &bytesRead, rx_buffer);
                
                misInt = true;
                
                myPrintf("_");
            }
            
            isr_canint_Disable();
            
            uint8 isCanErr = mcpCanCheckError();
            
            if(isCanErr != CAN_OK)    // CAN error needs clear
            {
                mcpCanClearError();  
                
//                misErr = 1;
                
//                myPrintf("e");
                
                CyDelay(5);
                
                isCanErr = mcpCanCheckError();
                
                if(isCanErr != CAN_OK)    // CAN error needs clear
                {
                    do{
                        CyDelay(5);
                        
                    }while(configureCan2() != 0);
                    
                    canResCnt++;
//                    myPrintf("e%d", canResCnt);
                }
            }     
            else if((millis() - lastAlivePacket) > 10000)
            {
                lastAlivePacket = millis();
                
                do{
                        
                    CyDelay(5);
                    
                }while(configureCan2() != 0);
            }
            
            isr_canint_Enable();
            
            if(misInt)  // debug
            {
                myPrintf("I");
                
                misInt = false;
            }
        }
        
        if((millis() - debugOnTimer) < 30000)
        {
            if(millis() > 10000 && (millis() - ledTimeE) > 10)
            {
                ledTimeE = millis();
                
                if(awakeLedTime > 0)
                {
                    if((millis() - awakeLedTime) > 25)
                    {
                        turnLedsOn(!PS_LEDS_Read());
                        awakeLedTime = 0;
                    }
                }
                else if((millis() - ledTime) > (isAwake ? 2500 : 5000))
                {
                    ledTime = millis();
                    ledsOn = !ledsOn;
                    turnLedsOn(ledsOn);
                }
            }
        }
        else
        {
            turnLedsOn(0);
        }
        
        
        
//        if(debTime == 0 || (millis() - debTime) > 7200000)
//        {
//            debTime = millis();
//            
//            myPrintf("pCycle:%d WDT:%d Err:%d(+%d) CanRst%d(+%d/%d) (%d)\n"
//                , canErr.rebootCnt - 1, canErr.wdtCnt, canErr.canErrCnt, canErr.canErrMore
//                , canErr.canRstCnt, canErr.canRstTryCnt, canErr.canRstMore, misErr);
//        
//            
//        }
        
//        if(mcpCanCheckError() != CAN_OK)
//        {
//            mcpCanClearError();   
//        }    
        ds = 'F';
    }
    PT_END(&s->pt);
}

PT_THREAD(canBroadcast(struct state *s))
{
    static uint8 k = 0;
    static uint8 j = 0;
    static uint8 m = 0;
//    static uint8 n = 0;
    static uint8 b = 0;
    
    static CYBLE_API_RESULT_T                  bleApiResult;
    static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;

    static uint32_t switchTimer = 0;
    static uint32_t debugTimer = 0;
    static uint32_t systemTimer = 0;
    
    static bool updateSystemPacket = 0;
    static bool updateSwitchPacket = 0;
    static bool updateStatusPacket = 0;
    static bool updateProPackets = 0;
    static bool updateDebugPacket = 0;
    static bool sendPacket = 0;
    static bool sendBlePacket = 0;
//    static bool wasConnected = 0;
    static bool onlyBle = 0;
    
    static bool ignToggle = 0;
    
    static uint8_t bleUpdateCount = 0;
    static uint32_t bleUpdateTimer = 0;
    
    static bool bleUpdateBridge = 0;
    static uint8_t bleBridgeBuffer[52][5] = {0};
    static int currBufEl = -1;
    
//    static uint32 ignTimer = 0;
    static uint8_t lastSwitchValue[32] = {0};
    static uint8_t lastOutputState[32] = {0};
    static uint16_t lastOutputValue[32] = {0};
    
    static uint8_t tx_buffer[5] = {0x00};
    
    PT_BEGIN(&s->pt);
    while(1)
    {    
        
        timer_set(&s->timer, 25);     // for proper CAN timing
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
//        myPrintf("B1");
        //myPrintf("ping out\n");
        
        if((sendOtaPacket || sendSecPacket || sendSettingsPacket) && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {   
            // ignore CAN if OTA info is needed
        }
        else if(popCanFifo(tx_buffer))
        {
            sendPacket = 1;
            //sendBlePacket = 1; //from ble...
        }
        else if(updateSystemPacket) 
        {    // temp/voltage
            tx_buffer[0] = SYSTEM_PACKET;
            tx_buffer[1] = status.batVolt.hex;
            tx_buffer[2] = status.temp.hex;     
            tx_buffer[3] = 0x55;
            tx_buffer[4] = status.address | (~is12v_not24 << 7);    // can error bit counter thing "d1"
            
            sendPacket = 1;
            
            if(authGood)
            {
                if(lastBatt[status.address] != status.batVolt.hex)
                {
                    int diff = (int)status.batVolt.hex - (int)lastBatt[status.address];
                    
                    if(diff > 3 || diff < -3)
                    {
                        lastBatt[status.address] = status.batVolt.hex;
                        sendBlePacket = 1;
                    }
                }
                
                if(lastTemp[status.address] != status.temp.hex)
                {
                    int diff = (int)status.temp.hex - (int)lastTemp[status.address];
                    
                    if(diff > 2 || diff < -2)
                    {
                        lastTemp[status.address] = status.temp.hex;
                        sendBlePacket = 1;
                    }
                }
            }
            else
            {
                lastBatt[status.address] = 0;
                lastTemp[status.address] = 0;
            }
            
            if(bleUpdateCount)
            {
                bleUpdateCount--;
                sendBlePacket = 1;
            }
            
            updateSystemPacket = 0;
            
//            myPrintf("SYSTEM PACKET \n");
            
        }
        else if(updateDebugPacket)
        {             // telling shields/periphrials that debug mode/leds are on
            
            tx_buffer[0] = (status.address | DEBUG_PACKET);
            tx_buffer[1] = 0x0A;     
            tx_buffer[2] = 0x00;
            tx_buffer[3] = 0x00;
            tx_buffer[4] = 0x00;
            
            if(status.leds){     
                tx_buffer[2] = 0x0F;
            }
            
            
            sendPacket = 1;
            sendBlePacket = 0;
            updateDebugPacket = 0;
            
//            myPrintf("DEBUG PACKET \n");
            
        }
        else if(popCanLinkBuf(tx_buffer))
        {
            //printByteArray("popCanLinkBuf():", tx_buffer, 5);
            
            sendPacket = 1;
            sendBlePacket = 1;
        }
        else if(updateSwitchPacket)
        {
            // update
            tx_buffer[0] = (status.address | SWITCH_PACKET);
            tx_buffer[1] = iToCan(j);     
            tx_buffer[2] = status.out[j].outCmd;
            tx_buffer[3] = status.out[j].blinkOn / FLASH_SPEED;
            tx_buffer[4] = status.out[j].blinkOff / FLASH_SPEED;
               //myPrintf("%d | %x | %x | %x | %x | %x\n", j, tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3], tx_buffer[4]);
            
            if(ignToggle)   // 1500 ms period
            {
                if(status.pro.isEnabled)
                {
//                    if(status.pro.isIgnCtrl[j] && ignFlag[j])
                    if(ignFlag[j])
                    {
                        tx_buffer[2] = 0;
                    }
                }
                else
                {
//                    if(j <= 1 && status.ignCh[j] && ignFlag[j])
                    if(j <= 1 && ignFlag[j]) // flash the button if the ignition is off for that channel (ignFlag handled in updateOutputs)
                    {
                            tx_buffer[2] = 0;
                    }
                
                }
            }
            
            if(status.pro.isEnabled && status.pro.isIgnCtrl[j] && ignFlag[j])
            {
                sendBlePacket = 1;
            }
            
            if(lastSwitchValue[(status.address * 8) + j] != status.out[j].output)
            {
                lastSwitchValue[(status.address * 8) + j] = status.out[j].output;
                sendBlePacket = 1;
            } 
            
            if(bleUpdateCount)
            {
                bleUpdateCount--;
                sendBlePacket = 1;
            }
            
            j++;
            
            if (j > 7) {
    		    j=0;

                //canState = OUTPUTS_2;

                updateSwitchPacket = 0;
                updateStatusPacket = 1;
                
//                if(millis() - ignTimer > 1500)
//                {
//                    ignTimer = millis();   
                if(ignToggle){
                    ignToggle = 0;
                }else{
                    ignToggle = 1;
                    lastIgnTog = millis();
                }
                
//                myPrintf("SWITCH PACKETS \n");
    		}
            
            sendPacket = 1;
            
        }
        else if(updateStatusPacket)
        {
            
            int netIndex = (status.address * 8) + k;
            
            //status
            tx_buffer[0] = (status.address | STATUS_PACKET);
            tx_buffer[1] = iToCan(k);                              // Pin mask
            tx_buffer[2] = status.out[k].open;                  // mode pin is in (eg open, short, etc)
            tx_buffer[3] = status.out[k].current.hex >> 8;      // first byte of current    
            tx_buffer[4] = status.out[k].current.hex & 0x00FF;  // second byte of current, 
                // current value right justified 11 bit e.g. between 0x0000 - 0x07FF
            
            
            
            if(lastOutputState[netIndex] != status.out[k].open)
            {
                lastOutputState[netIndex] = status.out[k].open;
                sendBlePacket = 1;
            }
            
            if(lastOutputValue[netIndex] != status.out[k].current.hex)
            {
                int diff = (int)status.out[k].current.hex - (int)lastOutputValue[netIndex];
                
                if(diff > 3 || diff < -3)
                {
                    lastOutputValue[netIndex] = status.out[k].current.hex;
                    sendBlePacket = 1;
                }
            }
            if(bleUpdateCount)
            {
                bleUpdateCount--;
                sendBlePacket = 1;
            }
                
            
            
            
            k++;
            
//            myPrintf("%d\n", k);
            
            if (k > 7) 
            {
    		    k=0;
                
                updateStatusPacket = 0;
                
//                if(status.pro.isEnabled)
//                {
//                    updateProPacket = 1;  
//                }
                
//                myPrintf("STATUS PACKETS \n");
    		}
            
            sendPacket = 1;
            
            
            
        }
        else if(updateProPackets && status.pro.isEnabled)
        {
            uint8_t index = m >> 1;
            
            uint8_t inputByte = LINKS_FLAG_MASK | //INPUT_LINK_FLAG_CAN_MASK | (status.pro.isInputLinking ? INPUT_LINK_CAN_MASK : 0x00) |
                    (status.pro.isInputEnabled[index] ? INPUT_EN_CAN_MASK : 0x00) |
                    (status.pro.isInputLockout[index] ? INPUT_LCK_CAN_MASK : 0x00);
                    // |(status.pro.isInputLockInvert[m] ? INPUT_INV_CAN_MASK : 0x00);
            
            //status
            tx_buffer[0] = (status.address | PRO_PACKET);
            tx_buffer[1] = iToCan(index);                              // Pin mask
            
            uint32_t myLink = status.pro.links[(status.address * 8) + index];
            
            if(m & 0x01)
            {
                tx_buffer[2] = inputByte | 0x40;
                tx_buffer[3] = (myLink >> 24) & 0xFF;
                tx_buffer[4] = (myLink >> 16) & 0xFF;
            }
            else
            {
                tx_buffer[2] = inputByte;
                tx_buffer[3] = (myLink >> 8) & 0xFF;
                tx_buffer[4] = (myLink >> 0) & 0xFF;
            }
            
                
            sendBlePacket = 1;
                
            m++;
            
            if (m > 15) 
            {
    		    m=0;
                
                updateProPackets = 0;
//                myPrintf("STATUS PACKETS \n");
    		}
            
            sendPacket = 1;
            
        }
        else if(authGood)
        {
            if(bleUpdateBridge && (currBufEl == -1))
            {
                bleUpdateBridge = 0;
                
                
                for(b = 0 ; b < 4 ; b++)
                {
                    if((b == status.address) || (sourceConnected[b] == 0))
                        continue;
                    
                    currBufEl++;
                    
                    bleBridgeBuffer[currBufEl][0] = SYSTEM_PACKET; 
                    bleBridgeBuffer[currBufEl][1] = currBatt[b]; 
                    bleBridgeBuffer[currBufEl][2] = currTemp[b];
                    bleBridgeBuffer[currBufEl][3] = 0x55;
                    bleBridgeBuffer[currBufEl][4] = b | (currBatt12_24[b] << 7);
                    
                    
                    for(int c = 0; c < 8 ; c++)
                    {
                        currBufEl++;
                        
                        bleBridgeBuffer[currBufEl][0] = SWITCH_PACKET | b; 
                        bleBridgeBuffer[currBufEl][1] = iToCan(c);
                        bleBridgeBuffer[currBufEl][2] = currSwitchValue[b * 8 + c];
                        bleBridgeBuffer[currBufEl][3] = currBlinkOn[b * 8 + c];
                        bleBridgeBuffer[currBufEl][4] = currBlinkOff[b * 8 + c];
                        
                    }
                    
                    if(sourceConnected[b] < 2)
                        continue;
                    
                    for(int c = 0; c < 8 ; c++)
                    {
                        currBufEl++;
                        
                        bleBridgeBuffer[currBufEl][0] = STATUS_PACKET | b; 
                        bleBridgeBuffer[currBufEl][1] = iToCan(c);
                        bleBridgeBuffer[currBufEl][2] = currStatusValue[b * 8 + c];
                        bleBridgeBuffer[currBufEl][3] = currCurrentHigh[b * 8 + c];
                        bleBridgeBuffer[currBufEl][4] = currCurrentLow[b * 8 + c];
                        
                    }
                }
                
            }
            
            if(currBufEl >= 0)
            {
                for(int b = 0; b < 5 ; b++)
                {
                    tx_buffer[b] = bleBridgeBuffer[currBufEl][b];
                }
                
                sendPacket = 1;
                onlyBle = 1;
                sendBlePacket = 1;
                
                currBufEl--;
            }
            
            
        }
        
       
        if(sendPacket)
        {
            if(onlyBle) {
                onlyBle = 0;
            } else {
                
                isr_canint_Disable();
                
                mcpCanSendMsgBuf(&can, 0x80, 0, 5, tx_buffer);
                
                if(mcpCanCheckError() != CAN_OK)
                {
                    mcpCanClearError();   
                }

                isr_canint_Enable();

            }
            
            sendPacket = 0;
            
            if(sendBlePacket && CyBle_GetState() == CYBLE_STATE_CONNECTED)
            {
               
                //myPrintf("BLE Connected...\n");
          
                /* if stack is free, handle UART traffic */
                if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
                {
                    //myPrintf("BLE Stack Free...\n");
                    sendBlePacket = 0;
                    
                    if(authGood && sendNotifications){
                        
                        //myPrintf("BLE Notification Enabled...\n");
                        int txLength = 5;
                        int i = 0;
                        
                        bleTxBuffer[0] = 0x55;
                        bleTxBuffer[1] = txLength + 5;
                        bleTxBuffer[2] = 0x00;
                        
                        for(i = 0 ; i < txLength; i++)
                        {
                            bleTxBuffer[3 + i] = tx_buffer[i];
                        }
                        
                        uint32 crc = crc32(0, bleTxBuffer, txLength + 3);
                        
                        uint32 packetLength = txLength + 7;
                        
                        bleTxBuffer[packetLength - 4] = crc & 0xFF;
                        bleTxBuffer[packetLength - 3] = crc >> 8;
                        bleTxBuffer[packetLength - 2] = crc >> 16;
                        bleTxBuffer[packetLength - 1] = crc >> 24;
                        
                        
                        characteristicNotify.value.val = bleTxBuffer;
                        characteristicNotify.value.len = packetLength;
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
        else if(sendSecPacket && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            sendSecPacket = false;
            
            int packetLength = 9;
            
            
            bleTxBuffer[0] = packetLength;
            bleTxBuffer[1] = 0x00;
            bleTxBuffer[2] = status.isUnsecured;
            bleTxBuffer[3] = authGood;
            bleTxBuffer[4] = passkeyTimer > 0 ? 1 : 0;
            
//            if(passkeyTimer && passkey > 0)
//            {
//                bleTxBuffer[5] = passkey & 0xFF;
//                bleTxBuffer[6] = passkey >> 8;
//                bleTxBuffer[7] = passkey >> 16;
//                bleTxBuffer[8] = passkey >> 24;
//                
//                packetLength = 13;
//                bleTxBuffer[0] = packetLength;
//            }
            
            uint32 crc = crc32(0, bleTxBuffer, packetLength - 4);
            
            bleTxBuffer[packetLength - 4] = crc & 0xFF;
            bleTxBuffer[packetLength - 3] = crc >> 8;
            bleTxBuffer[packetLength - 2] = crc >> 16;
            bleTxBuffer[packetLength - 1] = crc >> 24;
            
            characteristicNotify.value.val = bleTxBuffer;
            characteristicNotify.value.len = packetLength;
            characteristicNotify.attrHandle = CYBLE_SPOD_UNSECURED_STATUS_CHAR_HANDLE;
            
            myPrintf("Send Security Packet: ");
            
            do
            {
                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
            
            myPrintf("Success \n");
        }
        else if(sendOtaPacket && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            sendOtaPacket = false;
            
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
//            bleTxBuffer[1] = 0x00;
            bleTxBuffer[1] = 0x04 | status.address;
            
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
            
            myPrintf("Send OTA Packet: ");
            
            do
            {
                
                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
            
            myPrintf("Success \n");
        }
        else if(sendSettingsPacket && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            sendSettingsPacket = false;
            
            if(authGood){
                
                //myPrintf("BLE Notification Enabled...\n");
                int txLength = 1;
//                int i = 0;
                
                bleTxBuffer[0] = 0x55;
                bleTxBuffer[1] = txLength + 5;
                bleTxBuffer[2] = 0x02;
                
                bleTxBuffer[3] = status.isDeepSleepEnabled ? 0x01 : 0x00;
                
                uint32 crc = crc32(0, bleTxBuffer, txLength + 3);
                
                uint32 packetLength = txLength + 7;
                
                bleTxBuffer[packetLength - 4] = crc & 0xFF;
                bleTxBuffer[packetLength - 3] = crc >> 8;
                bleTxBuffer[packetLength - 2] = crc >> 16;
                bleTxBuffer[packetLength - 1] = crc >> 24;
                
                
                characteristicNotify.value.val = bleTxBuffer;
                characteristicNotify.value.len = packetLength;
                characteristicNotify.attrHandle = CYBLE_SPOD_COMM_CHAR_HANDLE;
                
                myPrintf("Send Setting Packet: %d ", status.isDeepSleepEnabled);
                
                do
                {   
                    bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                    CyBle_ProcessEvents();
                }
                while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                
                myPrintf("Success \n");
            }
        }
        else if(sendProPacket && CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            sendProPacket--;    // = false;
            
            if(sendProPacket < 0 || sendProPacket >= PRO_NUM_SYNC_PACKETS)
            {
                sendProPacket = 0;
            }
            else 
            {
                int packetLength = 9;
                
                if(sendProPacket == 8)
                {
                    packetLength = 4;
                    
                    bleTxBuffer[0] = packetLength;
                    bleTxBuffer[1] = (0x00) | (DEVICE_TYPE << 6);
                    bleTxBuffer[2] = 0x00;
                    
//                    bleTxBuffer[3] = (
//                            (status.pro.isDisableSleep ? 0x01 : 0) | 
//                            (status.pro.isInputLinking ? 0x02 : 0)
//                            );
                    
                    bleTxBuffer[3] = (status.pro.isDisableSleep ? 0x01 : 0);
                }
                else
                {
                    packetLength = 13;
                        
                    uint8_t index = 7 - sendProPacket;      //0;
                    
                    bleTxBuffer[0] = packetLength;
                    bleTxBuffer[1] = (0x03) | (DEVICE_TYPE << 6);
                    bleTxBuffer[2] = 0x00;
                    
                    bleTxBuffer[3] = status.address << 3 | index;
                    
                    bleTxBuffer[4] = (
                            (status.pro.alwaysOn[index] ? 0x10 : 0) | 
                            (status.pro.isIgnCtrl[index] ? 0x08 : 0) | 
                            (status.pro.isLockout[index] ? 0x04 : 0) | 
                            (status.pro.isInputLatch[index] ? 0x02 : 0) | 
                            (status.pro.isCurrentRestart[index] ? 0x01 : 0) 
                            );
                    
                    bleTxBuffer[5] = status.pro.timers[index] >> 8;
                    bleTxBuffer[6] = status.pro.timers[index] & 0xFF;
                    
                    bleTxBuffer[7] = status.pro.currentLimit[index];
                    
                    
                    bleTxBuffer[8] = 0x20 | 
                            status.pro.isInputEnabled[index] ? INPUT_EN_CAN_MASK : 0 | 
                            status.pro.isInputLockout[index] ? INPUT_LCK_CAN_MASK : 0; 
//                            status.pro.isInputLockInvert[index] ? INPUT_INV_CAN_MASK : 0;
                            
                    bleTxBuffer[9] = (status.pro.links[index] >> 0) & 0xFF;
                    bleTxBuffer[10] = (status.pro.links[index] >> 8) & 0xFF;
                    bleTxBuffer[11] = (status.pro.links[index] >> 16) & 0xFF;
                    bleTxBuffer[12] = (status.pro.links[index] >> 24) & 0xFF;
                    
                    //bleTxBuffer[8] = status.pro.links[index];
                }
                
                characteristicNotify.value.val = bleTxBuffer;
                characteristicNotify.value.len = packetLength;
                characteristicNotify.attrHandle = CYBLE_SPOD_PRO_MODE_CHAR_HANDLE;
                
                myPrintf("Send Pro Packet (%d): ", sendProPacket);
                
                do
                {
                    bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                    CyBle_ProcessEvents();
                }
                while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                
                myPrintf("Success \n");
            }
        }
        
        isr_canint_Disable();
        
        if(mcpCanCheckError() != CAN_OK)
        {
            mcpCanClearError();   
        }
        
        isr_canint_Enable();
        
        if((millis() - systemTimer) > 535)
        {
            systemTimer = millis();
            updateSystemPacket = 1;
        }
        
        if((millis() - debugTimer) > 600)
        {
            debugTimer = millis();
            updateDebugPacket = 1;
        }
        
        if((millis() - switchTimer) > 750)
        {
            switchTimer = millis();
            updateSwitchPacket = 1;
        }
        
        if((millis() - bleUpdateTimer) > 2000)
        {
            bleUpdateTimer = millis();
            bleUpdateCount = 17;
            bleUpdateBridge = 1;
        }
        
        if(status.pro.isEnabled)
        {
            if(globalUpdateProCanPackets)
            {
                globalUpdateProCanPackets = false;
                updateProPackets = true;
            }
        }
        
        for(int s = 0 ; s < 4 ; s++)
        {
            if((sourceConnected[s] > 0) && ((millis() - srcConnTimer[s]) > 30000)){
                sourceConnected[s] = 0;
            }
        }
        
    }
    PT_END(&s->pt);
}

#define LV_TIMEOUT      120000  // 2min
#define ALIVE_TIMEOUT   3000  // 3s
#define ACTIVITY_TIMEOUT   18000000  // 30min   30*60*1000
#define ALIVE_TIMEOUT_INACTIVE   30000  // 30s

#define IS_TEST_DEEP_SLEEP  (0)

uint32 bleAliveTimer = 0;

#define DEEP_SLEEP_WDT  (500)
#define LV_TIMEOUT_CNT  (LV_TIMEOUT / DEEP_SLEEP_WDT)
#define ADC_BUF_TIME_MS    (60000)
#define ADC_BUF_SIZE    (ADC_BUF_TIME_MS / DEEP_SLEEP_WDT)


void enableDeepSleep(uint8_t needsDeepSleep)    // does not return from sleep
{
    if(needsDeepSleep)
    {
        adcVals_t batV = vbat_read();
        float lastAdc = batV.dec;
        
        outputs_off();                      // disable outputs and pwm IC
        outputs_sleep();
        
        PS_LEDS_Write(0);
        
        CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
        
        CyDelay(10);
        
        mcpCanSleep(1);
        
        CyDelay(10);
        
        SPI_Stop();
        I2C_Stop();
        
        //uint16_t lastCAN = CANINT_Read();// canBufLoad;
        
        myPrintf("Going into deep sleep: %fv - %d\n", fl(lastAdc));//, lastCAN);
        
        CyDelay(10);
        
        CyBle_EnterLPM(CYBLE_BLESS_HIBERNATE);
        CyBle_Stop();

        UART_Stop();

//        CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
        InitWatchdog(DEEP_SLEEP_WDT);
        
        CyIntDisable((SysTick_IRQn + 16));
        isr_canint_Disable();
        
        
        //int wdtCnt = 0;
        int lvCnt = 0;
        
        float vbatBuf[ADC_BUF_SIZE];
        for (int i = 0; i < ADC_BUF_SIZE; i++) {
            vbatBuf[i] = lastAdc;
        }
        float vinAvgF = lastAdc;
        float vMin = lastAdc;
        int vBatBufPoint = 0;
        
        
        
        while(1)
        {
            resetWatchdog();
            CySysPmDeepSleep();
            
            //wdtCnt++;
            
//            if(wdtCnt >= 5)
//            {
                CyIntEnable((SysTick_IRQn + 16));
//                UART_Start();
                
                
                //PS_LEDS_Write(!PS_LEDS_Read());
                
                CyDelay(5);
                
                batV = vbat_read();
                
                
                
                if(CANINT_Read() == 0)
                {
                    UART_Start();
                    CyDelay(1);
                    myPrintf("CAN restart!!\n");
                    
                    CyDelay(1);
                    CySysPmFreezeIo();
                    CySoftwareReset();
                }
                
//                uint8_t readCan = CANINT_Read();
//                
//                if(lastCAN != readCan)   //canBufLoad
//                {
//                    UART_Start();
//                    CyDelay(1);
//                    
//                    myPrintf("readCan %d\n", readCan);
//                    
//                    CyDelay(1);
//                    UART_Stop();
//                    
//                    lastCAN = readCan;
//                }
                
                
                float vinF = batV.dec;
                
                if(vinF > vinAvgF + 0.8f || (vinAvgF > 13.8f && vinAvgF > vMin + 0.2f))
                {
                    UART_Start();
                    CyDelay(1);
                    myPrintf("vbat Restart! %fv/%fv\n", fl(vinF), fl(vinAvgF));
                    
                    CyDelay(1);
                    CySysPmFreezeIo();
                    CySoftwareReset();
                }
                
                vbatBuf[vBatBufPoint] = vinF;
                
                vBatBufPoint++;
                vBatBufPoint %= ADC_BUF_SIZE;
                
                //int32_t avgSum = 0;
                
                vinAvgF = 0;
                
                for(int i = 0 ; i < ADC_BUF_SIZE ; i++)
                {
                    vinAvgF += vbatBuf[i];
                }
                
                vinAvgF /= ADC_BUF_SIZE;
                
                vMin = vinAvgF < vMin ? vinAvgF : vMin;
                
                //vinAvgF = (float)avgSum * 5.0 / 0x07ff * 11.0;
                
                
                
                
                lvCnt++;
                
                if((is12v_not24 != 0 && vinF > 11.6) || (is12v_not24 == 0 && vinF > 23.2))
                {
                    lvCnt = 0;
                }
                else if(lvCnt > LV_TIMEOUT_CNT)     // 2min
                {
                    myPrintf("LV Restart: %v\n", fl(vinF));
                    CyDelay(20);
                    
                    BYPASS_CTRL_Write(0); 
                }
                
                CyDelay(2);
                
                CyIntDisable((SysTick_IRQn + 16));
//                UART_Stop();
                
//                wdtCnt = 0;
//            }
        }
    }
    else
    {
        // wake logic
    }
}

PT_THREAD(sleep(struct state *s))
{
    static uint32 lvTimer;
    static uint32 lvMessageTime = 0;
    
    static float lastBatVolt = 0.0;
    static uint32 lastIgnCheck = 0;
    
    
    PT_BEGIN(&s->pt);
    while(1)
    {        
        timer_set(&s->timer, 100);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        
    #if IS_TEST_DEEP_SLEEP
        
        if(millis() > 60000)
        {
            enableDeepSleep(1);
        }
            
    #endif
        
        if(is12v_not24)
        {
            if(status.batVolt.dec > 11.6)
            {
                lvTimer = millis();
            }
        }
        else
        {
            if(status.batVolt.dec > 23.2)
            {
                lvTimer = millis();
            }
        }
        
        if(millis() > 10000 && (millis() - lastIgnCheck) > 5000)    // 
        {
            lastIgnCheck = millis();
            
            adcVals_t batV = vbat_read();
            
            if(lastBatVolt < 1.0)
            {
                lastBatVolt = batV.dec;
            }
            else
            {
                if((batV.dec - lastBatVolt) > 1.0)
                {
//                    CySoftwareReset();
                    
                    isr_canint_Disable();
                    
                    do{
                        
                        CyDelay(5);
                        
                    }while(configureCan2() != 0);
                    
                
                    isr_canint_Enable();
                    
                }
                
                lastBatVolt = batV.dec;
            }
        }
        
        if((millis() - lvTimer) > LV_TIMEOUT)// && !status.debug)
        {
            if((millis() - lvMessageTime) > 30000)
            {
                myPrintf("Low voltage timeout: ");
//                myPrintf("TURN OFF: ");
                write_millis(millis());
                
                lvMessageTime = millis();
            }
            
            BYPASS_CTRL_Write(0); 
        // if alternate lv_bypass isn't enabled, this will completely power down the unit and all periphrials
        }
        
        if(millis() > 10000 && 
        !status.debug && 
        !(status.pro.isEnabled && status.pro.isDisableSleep) && 
        (!isAwake || (
            (millis() - bleAliveTimer) > 30000 && 
            (((millis() - lastActivityMs) < ACTIVITY_TIMEOUT && (millis() - aliveTimer) > ALIVE_TIMEOUT) ||
            ((millis() - lastActivityMs) > ACTIVITY_TIMEOUT && (millis() - aliveTimer) > ALIVE_TIMEOUT_INACTIVE))
        )))
        {
            if(!wasAsleep)     // sleep "initialize"
            {
                myPrintf("SLEEP ");// %d\n", millis());
                write_millis(millis());
                
                //myPrintf("timers: %d %d %d\n", millis(), aliveTimer, lastActivityMs);
                
                outputs_off();                      // disable outputs and pwm IC
                outputs_sleep();
                
//                PS_LEDS_Write(0);
                
                for(int i = 0; i < 8; i++)
                {
                    status.out[i].outCmd = 0;
                }
                
            }
            
            if(status.isDeepSleepEnabled || DEEP_SLEEP_FORCE_ON)
            {
                enableDeepSleep(1);
            }
        
            isAwake = 0;
            wasAsleep = 1;            
            
            timer_set(&s->timer, 1000);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer)); 
            
            status.batVolt = vbat_read();
            
//            myPrintf(" B:%f ", fl(status.batVolt.dec));
            
//            myPrintf("batvolt %f \n", fl(status.batVolt.dec));
        }
                        
        if(isAwake && wasAsleep)        // awake "initialize
        {
            myPrintf("AWAKEN ");// %d\n", millis());
            write_millis(millis());
            
            outputs_wake();                     // renable pwm IC
            
            wasAsleep = 0;
            
            if(status.pro.isEnabled)
            {
                for(int i = 0 ; i < 8 ; i++)
                {
                    status.out[i].outCmd = status.pro.alwaysOn[i] ? status.pro.lastSwVal[i] : 0x00;
                }
                
//                globalUpdateProCanPackets = true;
            }
            aliveTimer = millis();
        }
    }
    PT_END(&s->pt);
}

//#define DEBUG_ON_TIME   30000      // 30 s
#define DEBUG_ON_TIME   600000      // 10 min

PT_THREAD(debug(struct state *s))
{
    static uint8 i;
//    static bool tog = 0;
    
    static bool lastDebug = 0;
    
    PT_BEGIN(&s->pt);
    while(1)
    {           
        timer_set(&s->timer, 2500);
        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));

        status.debug = (uint16_t)read_inputs(0) & DEBUG_MASK;
        
        if(status.debug)
        {
            aliveTimer = millis();
            isAwake = 1;
        }
        
//        myPrintf("\n");
//        write_millis();
        
        status.leds = 0;
//        status.leds = tog;
//        status.out[7].outCmd = tog * 0xFF;
//        
//        tog = !tog;
        
        if(isLockedFault)
        {
            status.leds = 1;
        }
        
        
        if(status.debug)
        {
            if(!lastDebug)
            {
                debugOnTimer = millis();
            }
        }
        else
        {
            debugOnTimer = 0;
        }
        
        lastDebug = status.debug;
        
        //if(status.debug && (millis() - debugOnTimer) < DEBUG_ON_TIME)
        if(debugOnTimer > 0 && (millis() - debugOnTimer) < DEBUG_ON_TIME)
        {
                status.leds = status.debug;
//            printStatus();

                
                myPrintf("Source status: ");
                write_millis(millis());
                
                timer_set(&s->timer, 10);//2500);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                
                myPrintf("Switches:\n");
                myPrintf("   Debug  |  ignCh0  |  ignCh1  |  Address  \n");
                myPrintf("     %d          %d          %d         %x   \n", 
                    status.debug, status.ignCh[0], status.ignCh[1], status.address);
                myPrintf("Outputs: \n");
                myPrintf("  Channel  |  Output  |  Current  |  Select  |  Mode\n");
                myPrintf("--------------------------------------------------------\n");
                
                for(i=0 ; i<8 ; i++)
                {
                    timer_set(&s->timer, 10);//2500);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                    
                    myPrintf("     %d     |     %d        %fA        %d        ", (i+1), 
                        (status.out[i].output * 100 / 0xFF), fl(status.out[i].current.dec),  status.inputs[i]);
                    
                    switch(status.out[i].open) 
                    {
                        case OUTPUT_OPEN:
                            myPrintf("open \n");
                        break;
                        case OUTPUT_OFF:
                            myPrintf("off\n");
                        break;            
                        case OUTPUT_SHORT:
                            myPrintf("short \n");
                        break;            
                        case OUTPUT_NOM:
                            myPrintf("nominal \n");
                        break;            
                        case OUTPUT_FAULT:
                            myPrintf("fault \n");
                        break;
                        case OUTPUT_FLASH:
                            myPrintf("flash \n");
                        break;
                        case OUTPUT_STROBE:
                            myPrintf("strobe \n");
                        break;
                        default:
                            myPrintf("error \n");
                    }
                }
                
                timer_set(&s->timer, 10);//2500);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                
                myPrintf("Other:\n");    
                myPrintf("  ignition  |  BatVoltage  |  lvBypass  |  LEDs  |    Temp\n");
                myPrintf("     %d        (%d)%fV           %d          %d       (%d) %fC   \n", 
                    status.ignSense, is12v_not24, fl(status.batVolt.dec), status.lvBypass, status.leds, status.tempEdge, fl(status.temp.dec));
                myPrintf("\n");
                
                timer_set(&s->timer, 10);//2500);
                    PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                
                if(status.pro.isEnabled)
                {
                    myPrintf("PRO: \n");
                    myPrintf("  Channel  | ao | ig | lo | il | ar | cl | tim\n");
                    myPrintf("----------------------------------------------------\n");
                    
                    for(i=0 ; i<8 ; i++)
                    {
                        timer_set(&s->timer, 10);//2500);
                        PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
                        
                        myPrintf("     %d     |  %d    %d    %d    %d    %d    %d    %d\n", (i+1), 
                            status.pro.alwaysOn[i], status.pro.isIgnCtrl[i], status.pro.isLockout[i], status.pro.isInputLatch[i], 
                            status.pro.isCurrentRestart[i], status.pro.currentLimit[i], status.pro.timers[i]
                        );
                        
                        
                    }
                }
        }

        
    }
    PT_END(&s->pt);
}


void updateAuth(void);

static struct state readInputsState, readCurrentsState, updateOutputsState, canListenState, canBroadcastState, debugState, sleepState;//, pairDeviceState;


int main()
{
#if !defined(__ARMCC_VERSION)
    InitializeBootloaderSRAM();
#endif

    // Checks if Self Project Image is updated and Runs for the First time 
    AfterImageUpdate();
    
    uint8_t resetCause = CySysGetResetReason(CY_SYS_RESET_WDT | CY_SYS_RESET_SW | CY_SYS_RESET_PROTFAULT);
    
    if(resetCause != 0) {
//        if(resetCause == CY_SYS_RESET_WDT) {
//            myPrintf("\nReset by watchdog \n\n");
//        } else if(resetCause == CY_SYS_RESET_SW) {
//            myPrintf("\nReset: CY_SYS_RESET_SW \n\n");
//        } else if(resetCause == CY_SYS_RESET_PROTFAULT) {
//            myPrintf("\nReset: CY_SYS_RESET_PROTFAULT \n\n");
//        }
    
        turnLedsOn(1);
    } else {
        turnLedsOn(0);
    }
    
    
    PT_INIT(&readInputsState.pt);
    readInputsState.name = "readInputs\n";
    
    PT_INIT(&readCurrentsState.pt);
    readInputsState.name = "readCurrents\n";
    
    PT_INIT(&updateOutputsState.pt);
    updateOutputsState.name = "updateOutputs\n";
    
    PT_INIT(&canListenState.pt);
    canListenState.name = "canListen\n";
        
    PT_INIT(&canBroadcastState.pt);
    canBroadcastState.name = "canBroadcast\n";
    
    PT_INIT(&debugState.pt);
    debugState.name = "debug\n";
        
    PT_INIT(&sleepState.pt);
    sleepState.name = "sleep\n";
     
    turnLedsOn(0);
    BYPASS_CTRL_Write(1);
    CySysPmUnfreezeIo();

    I2C_Start(); 
    SPI_Start();
    UART_Start();
    ADC_Start();
    ADC_SetChanMask(0b00000111);        // channel 0 - 2
    ADC_StartConvert();

    
    myPrintf("\nStart... \n\n");
    myPrintf("Compiled: %s @ %s\n\n", __DATE__, __TIME__);
    
    uint16_t appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    uint16_t appVer2 = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
    uint32_t board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    uint16_t boardId = (board & 0xFFFF0000) >> 16;
    char boardRev = (board & 0x0000FFFF) + ('A' - 1);
    
    myPrintf("Boot App Board ID: %drev%c\n", boardId, boardRev);
    myPrintf("Boot App Version: %d.%d.%d\n", appVer / 0x100, appVer % 0x100 / 0x10 , appVer % 0x10);
    myPrintf("Boot Stack Version: %d.%d.%d\n\n", appVer2 / 0x100, appVer2 % 0x100 / 0x10 , appVer2 % 0x10);
    
    // Map systick ISR to the user defined ISR. SysTick_IRQn is already defined in core_cm0.h file 
	CyIntSetSysVector((SysTick_IRQn + 16), USER_ISR);
    SysTick_Config(NUMBER_OF_TICKS);
   
    InitWatchdog(WDT_INTERVAL_1S);
    InitBrownout(CY_LVD_THRESHOLD_3_00_V);
    isr_canint_StartEx(CANRX_ISR);
    
    CyGlobalIntEnable; 
    
    CyBle_Start( StackEventHandler );
    
    
    srand(ADC_GetResult16(TEMP_CHAN) + 1);
    
    if(readPasskey() == 0) {
        loadFlashDefaults();
        generatePasskey();
    }
    
//    if(status.isUnsecured) {
        updateBleSecurity(status.isUnsecured);
//    }
    
    readProFlash();
    proIsTempWritable = status.pro.isWritable;
    
    if(!status.pro.isEnabled && IS_PRO_TESTING) {
        myPrintf("Manual pro enable\n");
        status.pro.isEnabled = true;
        
        status.pro.isInputLinking = IS_PRO_INPUTLINK_TESTING;
            
        writeFlash();
    }
    
    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING) {
 	    CyBle_ProcessEvents();
 	}
    
    isTempEdgeDisabled = TEMP_TRIG_Read() == 0 ? true : false;
    if(isTempEdgeDisabled)
        myPrintf("Temperature edge sensor issue: isTempEdgeDisabled\n");
    
    i2c_init();
    configureCan();
    adc_c_calibrate();
    
    status.address = getAddr(read_inputs(0));    // read address for use in CAN bus
    myPrintf("Source Address: %d\n", status.address);
    
    CyDelay(10 + (5 * status.address));     // different delays for each source address to offset communications

    myPrintf("Finish Config...\n\n");
  
    
    
    
    for(;;)
    {
        ds = 'w';
        resetWatchdog();
        ds = 'a';
        updateAuth();
        ds = 'b';
        CyBle_ProcessEvents();
        ds = 'l';
        PT_SCHEDULE(canListen(&canListenState));
        
        if(isAwake)
        {
            ds = 'i';
            PT_SCHEDULE(readInputs(&readInputsState));
            ds = 'c';
            PT_SCHEDULE(readCurrents(&readCurrentsState));
            ds = 'l';
            PT_SCHEDULE(canListen(&canListenState));
            ds = 'o';
            PT_SCHEDULE(updateOutputs(&updateOutputsState));
            ds = 'l';
            PT_SCHEDULE(canListen(&canListenState));
            ds = 'c';
            PT_SCHEDULE(canBroadcast(&canBroadcastState));
            ds = 'l';
            PT_SCHEDULE(canListen(&canListenState));
        }

        ds = 'd';
        PT_SCHEDULE(debug(&debugState));
        ds = 'l';
        PT_SCHEDULE(canListen(&canListenState));
        ds = 's';
        PT_SCHEDULE(sleep(&sleepState));
    }
}


uint8 bleFoundDelimiter = 0;
uint8 bleBufferAddress = 0;
int blePacketLength = 0;

bool bleDisconnect = true;

void updateAuth(void)
{
//    write_millis();
    
    static bool passOn = 1;
    static bool passOff = 1;
    static CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    if(status.isUnsecured)
    {
        bleDisconnect = false;
//        authGood = true;
    }
    
    if(passkeyTimer)
    {
        if((millis() - passkeyTimer) > PASSKEY_TIMEOUT)
        {
            passkeyTimer = 0;
        }
        
        if(passOn) {
            writePassKey(passkey);
            passOn = 0;
            passOff = 1;
            bleDisconnect = false;
        }
        
    } else {
        if(passOff){
            writePassKey(0);
            passOn = 1;
            passOff = 0;
            
            if(!status.isUnsecured) {
                bleDisconnect = true;
            }
            
        }
    }
    
    if(!status.isUnsecured && cyBle_pendingFlashWrite != 0u)
    {
        // Store Bonding informtation to flash 
        apiResult = CyBle_StoreBondingData(0u);
        if ( apiResult == CYBLE_ERROR_OK)
        {
            myPrintf("Bonding data stored\r\n");
        }
        else
        {
            myPrintf ("Bonding data storing pending\r\n");
        }
    }
}

#define IS_MOMENTARY_MASK       0x01
#define IS_DIMABLE_MASK         0x02
#define IS_STROBE_MASK          0x04   
#define IS_FLASH_MASK           0x08

void ProcessPacket(uint8 packetLength)
{
    
    myPrintf("ProcessPacket(): ");
    for(int i = 0 ; i < packetLength ; i++) {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    myPrintf("\n");
    
   // int i;
    uint32 calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32 tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(tempCrc == calcCrc)
    {
        //myPrintf("Packet Good...\n");
        if(bleRxBuffer[2] == 0) //CAN PACKET
        {
            
            //static uint8_t canTxBuf[5];
            
//            uint16 pin = 0;
////            int index, i = 0;
////            uint8 t1,t2;
//            pin = pin | bleRxBuffer[4];
//            pin = (pin << 8) | bleRxBuffer[3];
//            
            
            //myPrintf("Packet Good... %d\n", pin);
            int index = (bleRxBuffer[5] & 0x0F) * 8 + canToI(bleRxBuffer[6]);
            
            if((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255 && bleRxBuffer[7] > 0) {
            
                status.pro.lastRecVals[index].dimVal = bleRxBuffer[7];
                status.pro.lastRecVals[index].onVal = bleRxBuffer[8];
                status.pro.lastRecVals[index].offVal = bleRxBuffer[9];
            }
            
            if((bleRxBuffer[5] & 0x0F) == status.address)
            {
                if((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255)
                {
                    uint8 i = canToI(bleRxBuffer[6]);
                    
                    status.out[i].outCmd = bleRxBuffer[7];
                    
                    if(status.out[i].outCmd == 0xFE)
                    {
                        status.out[i].outCmd = 0xFF;
                    }
                    
                    if(status.pro.isEnabled && status.out[i].outCmd != 0)
                    {
                        status.pro.lastSwVal[i] = status.out[i].outCmd;
                    }
                    
                    status.out[i].blinkOn = bleRxBuffer[8] * FLASH_SPEED; 
                    status.out[i].blinkOff = bleRxBuffer[9] * FLASH_SPEED;
                    
                    isAwake = 1;
                    bleAliveTimer = millis();
                }
                else if(status.pro.isEnabled && (bleRxBuffer[5] & 0xF0) == PRO_PACKET && 
                        (bleRxBuffer[6] == 0 || canToI(bleRxBuffer[6]) != 255))
                {
                    uint8 i = canToI(bleRxBuffer[6]);
                    
                    if(bleRxBuffer[6] != 0)
                    {
                        status.pro.isInputEnabled[i] = (bleRxBuffer[7] & INPUT_EN_CAN_MASK) ? true : false;
                        status.pro.isInputLockout[i] = (bleRxBuffer[7] & INPUT_LCK_CAN_MASK) ? true : false;
                        //status.pro.isInputLockInvert[i] = (bleRxBuffer[7] & INPUT_INV_CAN_MASK) ? true : false;
                        
                        uint8_t linkPacketIndex = 0;
                        
                        if((bleRxBuffer[7] & 0xC0) == INPUT_LINK_CAN_MASK)
                        {
                            linkPacketIndex = 1;
                        }
                        
                        if(bleRxBuffer[7] & LINKS_FLAG_MASK)
                        {
                            uint32_t linkHalf = (bleRxBuffer[8] << 8) | bleRxBuffer[9];
                            
                            if(linkPacketIndex == 0)
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0xffff0000) | linkHalf;
                            }
                            else
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0x0000ffff) | (linkHalf << 16);
                            }
                        }
                        
//                        if(bleRxBuffer[8] & (0x01 << i) && (bleRxBuffer[7] & 0xC0) != 0x40)
//                        {
//                            status.pro.links[i] = bleRxBuffer[8] & ~(0x01 << i);
//                        }
                    }
                    
                    writeFlash();
                    
                    //globalUpdateProCanPackets = true;
                    pushCanFifo(&bleRxBuffer[5]);   // just transmit through rather than try to reassemble/index...
                    
                    needsInputsUpdate = true;
                    
//                    myPrintf("rec proCAN: %d %d %d \n", bleRxBuffer[5], bleRxBuffer[6], bleRxBuffer[7]);
                }
            }
            else        // dump onto CAN
            {
                pushCanFifo(&bleRxBuffer[5]);       // move to canBroadcast to avoid possibility of contention
                
//                for( int i = 0 ; i < 5 ; i++)
//                {
//                    canTxBuf[i] = bleRxBuffer[5 + i];
//                }
//                
//                isr_canint_Disable();
//                
//                mcpCanSendMsgBuf(&can, 0x80, 0, 5, canTxBuf);
//                
//                if(mcpCanCheckError() != CAN_OK)
//                {
//                    mcpCanClearError();   
//                }
//                
//                isr_canint_Enable();
            }
        }
        else if(bleRxBuffer[2] == 1) //LED PACKET
        {
        }
        else if(bleRxBuffer[2] == 2) //SETTINGS PACKET
        {
//            if(settings->isProMode != 0 && !isTempWritable) {
//                return;
//            }   
            
            
            if((bleRxBuffer[3] & 0x02) != 0)
            {
                sendSettingsPacket = true;
            }
            else
            {
                status.isDeepSleepEnabled = bleRxBuffer[3] & 0x01;
                
                writeFlash();  
            }
        }
        else if(bleRxBuffer[2] == 3) //LINK PACKET
        {
//            if(status.pro.isEnabled != 0 && !proIsTempWritable) {       // if pro but not writable
//                return;
//            }
//            
//            uint8 offset = bleRxBuffer[3];
//            
//            myPrintf("Links Recieved... %d \n", offset);
//            
//            if(offset == status.address)
//            {
//                for(uint8_t i = 0; i < 8; i++)
//                {
//                    status.pro.links[i] = bleRxBuffer[i+4];       // mulit-source... no longer valid
//                }
//            }
        }
        else if(bleRxBuffer[2] == 4) //PAIR PACKET
        {
                        
//            if(pairingNeeded)
//            {
//                uint16 blePin = bleRxBuffer[4];
//                blePin = (blePin << 8) | bleRxBuffer[3];
//                myPrintf("Pair Recieved... %d\n", blePin);
//                
//                if(blePin == tempPin)
//                {
//                    settings->pin = blePin; 
//                   // saveSettings();
//                    pairingNeeded = 0;
//                }
//            }            
            
        }
        else if(bleRxBuffer[2] == 5) //altSettings
        {
            
        }
        else if(bleRxBuffer[2] == 8) //SWITCH SETTINGS PACKET
        {
            printByteArray("switchSettingsPacket():", &bleRxBuffer[0], packetLength);
//            myPrintf("switchSettingsPacket(): ");
//            for(int i = 0; i < packetLength; i++)
//            {
//                myPrintf("%x ", bleRxBuffer[i]);
//            }
//            myPrintf("\n");
            
            if(status.pro.isEnabled != 0 && !proIsTempWritable) {
                return;
            }
            
            uint8_t swIndex = bleRxBuffer[3];
            
            if(swIndex > 32)
                return;
            
            if(bleRxBuffer[4] != 0)     // for versioning
                return;
            
            uint8_t srcAddr = swIndex / 8;
            uint8_t swAddr = swIndex % 8;
            
            uint8_t isDim = bleRxBuffer[6] & (IS_DIMABLE_MASK);
            
            if(isDim && bleRxBuffer[5] > 0 && bleRxBuffer[5] < 0xFE)
            {
                status.pro.lastRecVals[swIndex].dimVal = bleRxBuffer[5];
                //status.out[swIndex].outCmd = bleRxBuffer[5];
            }
            
//            uint8_t isMom = bleRxBuffer[6] & (IS_MOMENTARY_MASK);
            uint8_t isBlink = bleRxBuffer[6] & (IS_FLASH_MASK | IS_STROBE_MASK);
            
            if(isBlink && bleRxBuffer[7] != 0 && bleRxBuffer[8] != 0)
            {
                status.pro.lastRecVals[swIndex].onVal = bleRxBuffer[7] * FLASH_SPEED;
                status.pro.lastRecVals[swIndex].offVal = bleRxBuffer[8] * FLASH_SPEED;
                
                if(srcAddr == status.address)
                {
                    status.out[swAddr].blinkOn = bleRxBuffer[7] * FLASH_SPEED;
                    status.out[swAddr].blinkOff = bleRxBuffer[8] * FLASH_SPEED;
                }
            }
            
            status.pro.links[swIndex] = bleRxBuffer[9];
            status.pro.links[swIndex] |= bleRxBuffer[10] << 8;
            status.pro.links[swIndex] |= bleRxBuffer[11] << 16;
            status.pro.links[swIndex] |= bleRxBuffer[12] << 24;
            
//            if(swIndex / 8 != status.address)
//            {
//                //send can packet?
//                
//                uint8_t buf[5];
//                
//                buf[0] = PRO_PACKET | srcAddr;
//                buf[1] = iToCan(swAddr);
//                buf[2] = LINKS_FLAG_MASK | 
//                    (status.pro.isInputEnabled[swIndex] ? INPUT_EN_CAN_MASK: 0) |   // isInputEnabled only [8]...
//                    (status.pro.isInputLockout[swIndex] ? INPUT_LCK_CAN_MASK: 0);   // isInputLockout only [8]...
//                buf[3] = bleRxBuffer[9];
//                buf[4] = bleRxBuffer[10];
//                
//                pushCanFifo(buf);
//                
//                buf[2] = buf[2] | 0x40;
//                buf[3] = bleRxBuffer[11];
//                buf[4] = bleRxBuffer[12];
//                
//                pushCanFifo(buf);
//            }
            
//            if(swIndex >= 31)
//                isSyncing = false;
//            
//            if(!isSyncing)
//                saveSettings();
        }
    
    }
    else
    {
       myPrintf("CRC Error... %d/%d\n", calcCrc, tempCrc);    
    }


}


void processOtaPacket(void)
{
    uint32_t boardRx, boardRead;
//    unit32_t stkBoardRead;
    uint16_t appId, appVer, stkId, stkVer;
    uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
//    uint16_t boardId;
//    char boardRev;
    
    
    
    boardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
//    stkBoardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
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
//    uint16_t swVal;
    uint8_t index;
//    uint8_t address;
    
//    bleRxBuffer[0]; //length
    
    myPrintf("processProPacket(): ");
    
    for(int i = 0 ; i < bleRxBuffer[0] ; i++)
    {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    
    myPrintf("\n");
    
    if(((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) // check that the app sent the correct type of packet
    {   
        if((bleRxBuffer[1] & 0x01) != 0)
        {
            // read 
            sendProPacket = PRO_NUM_SYNC_PACKETS;
            globalUpdateProCanPackets = true;        // to update inputs settings
//            myPrintf("needs pro packet sent\n");
            return;         //ignore the rest of the packet
        }
        
        if(status.pro.isEnabled == 0)     // set into "pro" mode
        {
            status.pro.isEnabled = 1;
            
//            writeFlash();
            updateFlash = 1;
            
            myPrintf("PRO Mode enabled!\n");
        }
        
//        if((bleRxBuffer[1] & 0x02) != 0)
//        {
//            proIsTempWritable = true;
//        }
        proIsTempWritable = false;
        
        if((bleRxBuffer[1] & 0x02) != 0)
        {
            if(status.pro.isWritable != true)
            {
                status.pro.isWritable = true;
                updateFlash = true;
            }
            
            proIsTempWritable = true;
        }
        else
        {
            if(status.pro.isWritable != false)
            {
                status.pro.isWritable = false;
                updateFlash = true;
            }
        }
        
        if((bleRxBuffer[1] & 0x04) != 0)
        {
            proIsTempWritable = true;
        }
        
        
        
        uint8_t len = bleRxBuffer[0];
        
        if(len <= 3)
        {
            if((bleRxBuffer[2] & 0x02) != 0 && proIsTempWritable) // update deep sleep flag
            {
                uint8_t nds = bleRxBuffer[2] & 0x01;
                if(status.pro.isDisableSleep != nds)
                {
                    myPrintf("Deep Sleep disabled %d \n", nds);
                    status.pro.isDisableSleep = nds;
                    updateFlash = 1;
                }
            }
            
            if((bleRxBuffer[2] & 0x08) != 0 && proIsTempWritable) // update input linking flag
            {
                uint8_t il = (bleRxBuffer[2]) & 0x04 ? 1 : 0;
                if(status.pro.isInputLinking != il)
                {
                    myPrintf("Input linking enabled %d \n", il);
                    status.pro.isInputLinking = il;
                    updateFlash = 1;
                }
            }
            
            if(updateFlash)
            {
                writeFlash();
            }
            
            return;
        }
        else if(len >= 8 && bleRxBuffer[2] == 0x00)
        {
            uint8_t addr = bleRxBuffer[3] >> 3;
        
            if(addr == status.address)
            {
                updateFlash = 1;
                
                index = bleRxBuffer[3] & 0x07;
                
                status.pro.alwaysOn[index] = ((bleRxBuffer[4] & 0x10) != 0) ? 1 : 0;
                status.pro.isIgnCtrl[index] = ((bleRxBuffer[4] & 0x08) != 0) ? 1 : 0;
                status.pro.isLockout[index] = ((bleRxBuffer[4] & 0x04) != 0) ? 1 : 0;
                status.pro.isInputLatch[index] = ((bleRxBuffer[4] & 0x02) != 0) ? 1 : 0;
                status.pro.isCurrentRestart[index] = ((bleRxBuffer[4] & 0x01) != 0) ? 1 : 0;
                
                if(status.pro.alwaysOn[index])
                {
                    status.out[index].outCmd = 0xFF;
                }
                
                status.pro.timers[index] = (bleRxBuffer[5] << 8) | bleRxBuffer[6];
                
                if(status.pro.curTime[index] > 0)
                {
                    status.pro.curTime[index] = millis(); // reset timer if running
                }
                
                status.pro.currentLimit[index] = bleRxBuffer[7];
                
                if(status.pro.currentLimit[index] == 0) // error...
                {
                    status.pro.currentLimit[index] = 30;
                }
                
                if(len >= 13)
                {
                    status.pro.isInputEnabled[index] = (bleRxBuffer[8] & INPUT_EN_CAN_MASK) ? true : false;
                    status.pro.isInputLockout[index] = (bleRxBuffer[8] & INPUT_LCK_CAN_MASK) ? true : false;
                    //status.pro.isInputLockInvert[i] = (bleRxBuffer[8] & INPUT_INV_CAN_MASK) ? true : false;
                    
                    if(bleRxBuffer[8] & LINKS_FLAG_MASK)
                    {
                        status.pro.links[index] = bleRxBuffer[9];
                        status.pro.links[index] |= bleRxBuffer[10] << 8;
                        status.pro.links[index] |= bleRxBuffer[11] << 16;
                        status.pro.links[index] |= bleRxBuffer[12] << 24;
                    }
                }
            }
        }
        
        if(updateFlash)
        {
            writeFlash();
        }
        
    }
    
    
}

enum securityPacketType {SEC_READ_STATUS, SEC_SET_MODE, SEC_WRITE_PASSKEY};

void processSecurityPacket(void)
{
    uint8_t packetLength = bleRxBuffer[0];
    
    myPrintf("processSecurityPacket(): ");
    for(int i = 0 ; i < packetLength ; i++) {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    myPrintf("\n");
    
    uint32 calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32 tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(tempCrc == calcCrc)
    {
        if(bleRxBuffer[1] == SEC_READ_STATUS)
        {
            sendSecPacket = true;
        }
        
        if (authGood)
        {
            if(bleRxBuffer[1] == SEC_SET_MODE)
            {
            // set into unsecured/secured mode
//            switch(bleRxBuffer[1])
//            {
//                case SEC_SET_MODE:
                
                    if((bleRxBuffer[2] & 0x01) == 0)
                    {
//                        status.isUnsecured = false;
                        setSecurityLevel(SECURED);
                    }
                    else
                    {
//                        status.isUnsecured = true;
                        setSecurityLevel(UNSECURED);
                    }
//                break;
            }
        }
        else
        {
            if(status.isUnsecured)
            {
                // listen for passkey -> set authGood
                
                if(bleRxBuffer[1] == SEC_WRITE_PASSKEY)
                {
                    uint32_t readPasskey = 0;
                    
                    readPasskey = readPasskey | bleRxBuffer[5];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[4];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[3];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[2];
                    
                    myPrintf("try passkey: %x/%x - ", readPasskey, passkey);
                    
                    if(readPasskey == passkey)
                    {
                        authGood = true;
                        myPrintf("accepted!!\n");
                    }
                    else
                    {
                        myPrintf("failed...\n");
//                        CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                    }
                    
                    sendSecPacket = true;
                }
            }
//            else
//            {
//                // allow read of isUnsecured
//                
//                if(bleRxBuffer[1] == SEC_READ_STATUS)
//                {
//                    sendSecPacket = true;
//                }
//                
//            }
        }
        
    }
    else
    {
        myPrintf(" -> crc bad\n");
    }
}


void writePassKey(uint32 thisKey)
{
    CYBLE_GATT_ERR_CODE_T apiGattErrCode = 0;

    CYBLE_GATT_HANDLE_VALUE_PAIR_T handleValuePair;
    
    handleValuePair.value.val = (uint8 *)&thisKey;
    handleValuePair.value.len = sizeof(thisKey);
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
        
        if(thisKey) {
//            myPrintf("PASSKEY ON %d\n", millis());
            myPrintf("PASSKEY ON ");
            write_millis(millis());
        } else {
            myPrintf("PASSKEY OFF ");
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
        myPrintf("CYBLE_EVT_HOST_INVALID:...\n");
        break;

        case CYBLE_EVT_STACK_ON:
        {
//            myPrintf("CYBLE_EVT_STACK_ON:...\n");
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
            
//            if(status.isUnsecured)
//            {
//                CyBle_GapFixAuthPassKey(1, 0);
//            }
//            else
//            {
                CyBle_GapFixAuthPassKey(1, passkey);
//            }
            CyBle_GapSetLocalName((const char8*)&localName[0]);
            CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST );
        }
        break;

        case CYBLE_EVT_TIMEOUT:
//        myPrintf("CYBLE_EVT_TIMEOUT:...\n");
        break;

        case CYBLE_EVT_HARDWARE_ERROR:
//        myPrintf("CYBLE_EVT_HARDWARE_ERROR:...\n");
        break;

        case CYBLE_EVT_HCI_STATUS:
//        myPrintf("CYBLE_EVT_HCI_STATUS:...\n");
        break;

        case CYBLE_EVT_STACK_BUSY_STATUS:
//        myPrintf("CYBLE_EVT_STACK_BUSY_STATUS:...\n");
        break;

        case CYBLE_EVT_PENDING_FLASH_WRITE:
//        myPrintf("CYBLE_EVT_PENDING_FLASH_WRITE:...\n");
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
//            myPrintf("CYBLE_EVT_GAP_DEVICE_CONNECTED:...\n");
            myPrintf("BLE Connected...\n");
//            isConnected = 1;
        break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
//            myPrintf("CYBLE_EVT_GAP_DEVICE_DISCONNECTED:...\n");
            myPrintf("BLE Disconnected...\n");
//            isConnected = 0;
            sendNotifications = 0;
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
//            myPrintf("CYBLE_EVT_GAP_ENCRYPT_CHANGE:...\n");
        break;

        case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
//            myPrintf("CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:...\n");
        break;

        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
//            myPrintf("CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:...\n");
        break;


        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
//            myPrintf("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:...\n");
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;


        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
//                myPrintf("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:...\n");
        break;

        case CYBLE_EVT_GAPC_SCAN_START_STOP:
//                myPrintf("CYBLE_EVT_GAPC_SCAN_START_STOP:...\n");
        break;



        case CYBLE_EVT_GATT_CONNECT_IND:
//                myPrintf("CYBLE_EVT_GATT_CONNECT_IND:...\n");
        break;

        case CYBLE_EVT_GATT_DISCONNECT_IND:
//                myPrintf("CYBLE_EVT_GATT_DISCONNECT_IND:...\n");
        break;


        case CYBLE_EVT_GATTC_ERROR_RSP:
//                myPrintf("CYBLE_EVT_GATTC_ERROR_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
//                myPrintf("CYBLE_EVT_GATTC_XCHNG_MTU_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
//                myPrintf("CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
//                myPrintf("CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
//                myPrintf("CYBLE_EVT_GATTC_FIND_INFO_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
//                myPrintf("CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_READ_RSP:
//                myPrintf("CYBLE_EVT_GATTC_READ_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_READ_BLOB_RSP:
//                myPrintf("CYBLE_EVT_GATTC_READ_BLOB_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_READ_MULTI_RSP:
//                myPrintf("CYBLE_EVT_GATTC_READ_MULTI_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_WRITE_RSP:
//                myPrintf("CYBLE_EVT_GATTC_EXEC_WRITE_RSP:...\n");
        break;

        case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
//                myPrintf("...\n");
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
//                myPrintf("CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:...\n");
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
//                myPrintf("CYBLE_EVT_GATTC_HANDLE_VALUE_IND:...\n");
        break;
                

        case CYBLE_EVT_GATTC_INDICATION:
//                myPrintf("CYBLE_EVT_GATTC_INDICATION:...\n");
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
//                myPrintf("CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:...\n");
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:
//                myPrintf("CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:...\n");
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:
//                myPrintf("CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:...\n");
        break;

        case CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:
//                myPrintf("CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:...\n");
        break;

        case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
//                myPrintf("CYBLE_EVT_GATTC_SRVC_DUPLICATION:...\n");
        break;

        case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
//                myPrintf("CYBLE_EVT_GATTC_CHAR_DUPLICATION:...\n");
        break;

        case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
//                myPrintf("CYBLE_EVT_GATTC_DESCR_DUPLICATION:...\n");
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
//                myPrintf("CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:...\n");
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
//                myPrintf("CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:...\n");
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
//                myPrintf("CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:...\n");
        break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
//                myPrintf("CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:...\n");
        break;



        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
//                myPrintf("CYBLE_EVT_GATTS_XCNHG_MTU_REQ:...\n");
        break;

        case CYBLE_EVT_GATTS_WRITE_REQ:                        
//                myPrintf("CYBLE_EVT_GATTS_WRITE_REQ:  ...\n");
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
          {
                CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
                
                lastActivityMs = millis();
//                myPrintf("BLE Write...\n");
                
                // Handling Notification Enable 
    			if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    	        {
                    //myPrintf("CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE...\n");
                    
    				CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
    				uint8 ccdValue[2];
    				
    	            // Extract CCCD Notification enable flag 
    	            sendNotifications = wrReqParam->handleValPair.value.val[0];
    				
    				// Write the present notification status to the local variable 
    				ccdValue[0] = sendNotifications;
    				
    				ccdValue[1] = 0x00;
    				
    				// Update CCCD handle with notification status data
    				notificationCCDHandle.attrHandle = CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    				
    				notificationCCDHandle.value.val = ccdValue;
    				
    				notificationCCDHandle.value.len = 2;
    				
    				// Report data to BLE component for sending data when read by Central device 
    				CyBle_GattsWriteAttributeValue(&notificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
    	        }
    	        // Handling Write data from Client 
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
                else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_UNSECURED_STATUS_CHAR_HANDLE)
                {
                    uint8 * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processSecurityPacket();
                }
                    
                if (eventCode == CYBLE_EVT_GATTS_WRITE_REQ)
    			{
                    //myPrintf("CYBLE_EVT_GATTS_WRITE_REQ...\n");
    	            CyBle_GattsWriteRsp(cyBle_connHandle);
    			}
            }  //*/
        break;

        case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
//        myPrintf("CYBLE_EVT_GATTS_PREP_WRITE_REQ:...\n");
        break;

        case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
//        myPrintf("CYBLE_EVT_GATTS_EXEC_WRITE_REQ:...\n");
        break;

        case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
//        myPrintf("CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:...\n");
        break;

        case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
//        myPrintf("CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:...\n");
        break;


        case CYBLE_EVT_GATTS_INDICATION_ENABLED:
//        myPrintf("CYBLE_EVT_GATTS_INDICATION_ENABLED:...\n");
        break;

        case CYBLE_EVT_GATTS_INDICATION_DISABLED:
//        myPrintf("CYBLE_EVT_GATTS_INDICATION_DISABLED:...\n");
        break;


        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
//        myPrintf("CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:...\n");
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
//        myPrintf("CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:...\n");
        break;

        case CYBLE_EVT_L2CAP_COMMAND_REJ:
//        myPrintf("CYBLE_EVT_L2CAP_COMMAND_REJ:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_CONN_IND:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_CONN_CNF:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_DATA_READ:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND...\n");
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
//        myPrintf("CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND...\n");
        break;

        default:
        break;
    }
}

/* [] END OF FILE */
