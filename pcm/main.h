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
 *  Version 1.03    01-11-2018
 *  Version 1.04    01-11-2018
 *  Version 1.05    01-15-2018
 *  Version 1.06    01-15-2018
 *
 * ========================================
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "pcm_pins.h"
#include "status.h"

#define CAN_ENABLED 0

#define NUMBER_OF_TICKS		 48000      // 1ms

#define IS_PRO_TESTING         (0)     // set to 0 for releases, 1 to default to pro
#define IS_PRO_INPUTLINK_TESTING         (0) 

#define DEEP_SLEEP_FORCE_ON  (false)

#define DEVICE_TYPE 0

#define CONNECTED       0x7E
#define SWITCH_ADDR     0x53

#define FLASH_SPEED     20      // __ms = 1 "tick"
#define SHUTOFF_TEMP    125.0   // in deg C, max 150C

#define WDT_INTERVAL_1S             250u                       /* millisecond */
#define ILO_FREQ                    32768     
#define PRO_NUM_SYNC_PACKETS    (8) // 8 x switches + ...

#define PASS_DBG_TIMEOUT    30000       //    (30 * 1000)
#define PASSKEY_TIMEOUT     180000      //(3 * 60 * 1000)

#define UNSECURED   (true)
#define SECURED     (false)

#define IGNORE_OFF_TIME_MS  (6000)
#define CAN_TX_FIFO_NUM (32)

#define INPUT_LINK_FLAG_CAN_MASK  (0x80)
#define INPUT_LINK_CAN_MASK (0x40)
#define LINKS_FLAG_MASK (0x20)

#define INPUT_EN_CAN_MASK   (0x04)
#define INPUT_LCK_CAN_MASK  (0x02)
#define INPUT_INV_CAN_MASK  (0x01)

#define LV_TIMEOUT      120000  // 2min
#define ALIVE_TIMEOUT   3000  // 3s
#define ACTIVITY_TIMEOUT   18000000  // 30min   30*60*1000
#define ALIVE_TIMEOUT_INACTIVE   30000  // 30s

#define IS_TEST_DEEP_SLEEP  (0)

#define DEEP_SLEEP_WDT  (500)
#define LV_TIMEOUT_CNT  (LV_TIMEOUT / DEEP_SLEEP_WDT)
#define ADC_BUF_TIME_MS    (60000)
#define ADC_BUF_SIZE    (ADC_BUF_TIME_MS / DEEP_SLEEP_WDT)


struct canBuffer {
    uint8_t rx_buf[5];
};


enum output_status_flags {
    OUTPUT_OFF = 0, 
    OUTPUT_OPEN = 1, 
    OUTPUT_NOM = 2,
    
    OUTPUT_FAULT = 4, 
    OUTPUT_FLASH = 5, 
    OUTPUT__STROBE = 6, 
    OUTPUT_BLINK = 7, 
    
    OUTPUT_SHORT = 8 
};

enum canState {CAN_LISTEN, SYSTEM_UPDATE, OUTPUTS_1, OUTPUTS_2, CAN_DEBUG};


#if CAN_ENABLED
mcpCan can;
#endif

//CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};

extern uint8_t our_mac[6];
extern uint8_t lastTemp[PCMS];
extern uint8_t lastBatt[PCMS];
extern uint8_t sourceConnected[PCMS];
extern uint32_t srcConnTimer[PCMS];

extern bool sendNotifications;

extern bool ble_client_connected;
extern uint8_t bleTxBuffer[20];

extern uint32_t aliveTimer;

extern bool isAwake;
extern bool wasAsleep;
extern bool overheated;
extern bool is12v_not24;
extern bool overheated2;

extern volatile uint32_t ticks;
extern volatile uint8_t wdtIsrCount;
         
extern volatile struct canBuffer can_buf[16];

extern uint8_t bytesRead;
extern char ds; // debug state, to know where program was when wdt triggers
extern uint32_t passkeyTimer;

extern uint32_t lastIgnTog;
extern uint32_t lastIgnSig;

extern uint32_t offTimer[CIRCUITS];
extern uint32_t globalOffTimer[CIRCUITS];
extern uint8_t globalWasOn[CIRCUITS];

extern uint32_t debugOnTimer;

extern bool isTempEdgeDisabled;
extern bool needsInputsUpdate;

extern uint32_t lastActivityMs;

extern bool sendPacket;
extern bool sendBlePacket;
extern bool sendSecPacket;
extern bool sendSettingsPacket;
extern uint8_t sendProPacket;
extern bool authGood;

extern bool globalUpdateProCanPackets;

extern uint32_t bleAliveTimer;

//uint32_t ioAddress1 = 0x20;
//uint32_t ioAddress2 = 0x24;

extern uint8_t bleRxBuffer[256];
extern uint8_t tx_buffer[5];

extern uint8_t currSwitchValue[64];
extern uint8_t currBlinkOn[64];
extern uint8_t currBlinkOff[64];

extern uint8_t currCurrentHigh[64];
extern uint8_t currCurrentLow[64];
extern uint8_t currStatusValue[64];

extern uint8_t currTemp[4];
extern uint8_t currBatt[4];
extern bool currBatt12_24[4];

extern void StackEventHandler( uint32_t eventCode, void *eventParam);
extern void generatePasskey(void);
extern void updateAuth(void);
extern void InitBrownout(uint32_t lvd_volt);
extern void resetWatchdog(void);

extern void printStatus(void);
extern void writePassKey(uint32_t thisKey);
extern void enterBootloader(size_t total_length);
extern void updateBleSecurity(uint8_t turnOff);
extern void setSecurityLevel(bool unsecured);

extern void turnLedsOn(bool isTrue);

extern void pushCanFifo(uint8_t* buf);
extern uint8_t popCanFifo(uint8_t* buf);
extern void addToCanLinkBuf(uint8_t index, uint8_t* buf);
extern uint8_t popCanLinkBuf(uint8_t* buf);

extern uint8_t TEMP_TRIG_Read();

extern void do_send_sec_packet();
extern void do_send_ble_packet();
extern void do_send_ota_packet();
extern void do_send_settings_packet();
extern void do_send_pro_packet();

extern void ble_ack_proxy_ota_packet(uint8_t* data, uint8_t leng);
extern void ble_ack_proxy_firmware_packet(uint8_t* data, uint8_t leng);

extern void canListen();
extern bool canBroadcast(void* unused);
extern bool readInputs(void* unused);
extern bool readCurrents(void* unused);
extern bool update_pcm1_outputs(void* unused);
extern bool update_pcmN_outputs(void* unused);
extern bool do_sleep(void* unused);
extern bool debug(void* unused);


#endif
/* [] END OF FILE */
