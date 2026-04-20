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

#include "spod.h"
#include "crc32.h"
#include "main.h"
#include "stdDrivers.h"
#include "ota.h"
#include "rs485.h"


static const char* TAG = "MAIN";

uint8_t lastTemp[PCMS] = {0}; // last temperature reading from this PCM board that was transmitted
uint8_t lastBatt[PCMS] = {0}; // last battery voltage reading from this PCM board that was transmitted

uint8_t currTemp[PCMS] = {0}; // temperature reading from last system packet received from this PCM
uint8_t currBatt[PCMS] = {0}; // current reading from last system packet received from this PCM 
bool currBatt12_24[PCMS] = {0}; // v12 vs v24 reading from last system packet received from this PCM

uint8_t sourceConnected[PCMS] = {0}; // 0 -- no packet for 30 seconds, 1 -- saw system packet, or 2 -- saw status packet 
uint32_t srcConnTimer[PCMS] = {0}; // millis() count when last system packet arrived, used to set sourceConnected[x] to 0

bool sendNotifications = 1; // if BLE client does not have notifications enabled, set this to zero

bool ble_client_connected = false; // true if BLE client connected
uint8_t bleRxBuffer[256];
uint8_t bleTxBuffer[20];

bool isAwake = 1;
bool overheated = 0;
bool overheated2 = 0;
bool is12v_not24 = 1;

uint32_t passkeyTimer = 0;

uint32_t lastIgnTog = 0;

uint32_t offTimer[CIRCUITS] = {0};
uint32_t globalOffTimer[CIRCUITS] = {0};
uint8_t globalWasOn[CIRCUITS] = {0};

uint32_t debugOnTimer = 0;

bool isTempEdgeDisabled =  false;
bool needsInputsUpdate = true;     // true when inputs have changed

uint32_t lastActivityMs = 0; // used to decide when to go into deep sleep mode

static struct canBuffer canTxFifoBuffer[CAN_TX_FIFO_NUM] = {0};
static uint8_t canTxFifoHead = 0, canTxFifoTail = 0;

uint8_t currSwitchValue[CONFIGS*SWITCHES] = {0};
uint8_t currBlinkOn[CONFIGS*SWITCHES] = {0};
uint8_t currBlinkOff[CONFIGS*SWITCHES] = {0};

uint8_t currCurrentHigh[CONFIGS*SWITCHES] = {0};
uint8_t currCurrentLow[CONFIGS*SWITCHES] = {0};
uint8_t currStatusValue[CONFIGS*SWITCHES] = {0};

uint32_t shortDelay[CIRCUITS]; // msecs -- used to timeout output when short occurs
lastVals_t lastRecVals[PCMS*CIRCUITS]; // output set to these values when turned on -- last received values

bool sendSecPacket = false;
bool sendSettingsPacket = false;
uint8_t sendProPacket = false;
bool authGood = false;

bool globalUpdateProCanPackets = false;

uint32_t bleAliveTimer = 0;


uint8_t TEMP_TRIG_Read() {return 0;} // TODO


void resetWatchdog(void)
{
    pet_watchdog(NULL);
}


static unsigned int rand_interval(unsigned int min, unsigned int max) 
{
	unsigned int r;
	const unsigned int range = 1 + max - min;
	const unsigned int buckets = RAND_MAX/range;
	const unsigned int limit = buckets * range;

	do r = rand(); while (r >= limit);
	return min + (r/buckets);
}


void generatePasskey(void)
{
#if 0 // TODO
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
   
    myPrintf("Generate passkey - ");
    myPrintf("Prepare to wipe bonding\n");
    
    if(CyBle_GetState() == CYBLE_STATE_ADVERTISING) CyBle_GappStopAdvertisement ();
    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
    while (CyBle_GetState() == CYBLE_STATE_CONNECTED) CyBle_ProcessEvents();
    
    apiResult = CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
    if(apiResult != CYBLE_ERROR_OK) myPrintf("ERR: CyBle_GapRemoveDeviceFromWhiteList() %d\n", apiResult);
    while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
        
    passkey = rand_interval(1, 999999);
    
    writeFlash();
    
    CyBle_GapFixAuthPassKey(1, passkey);
    writePassKey(passkey);
    
    passkeyTimer = millis();
    myPrintf("New passkey: %d\n\n", passkey);
    
    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
#endif
}


void updateBleSecurity(uint8_t turnOff)
{
#if 0
    static uint8_t lastOnOff = 0;
    
    myPrintf("updateBleSecurity()\n");
    if(lastOnOff == turnOff) {
        myPrintf("skipping... already ");
        myPrintf(turnOff? "unsecured\n\n":"secured\n\n");
        return;
    }
    
    myPrintf("isUnsecured: %d\n", turnOff);
    while (CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_BUSY) CyBle_ProcessEvents();
    
    CyBle_Stop();
    while (CyBle_GetState() != CYBLE_STATE_STOPPED) CyBle_ProcessEvents();
       
    if(turnOff) {
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_1 );
    } else {
        cyBle_authInfo.bonding = CYBLE_GAP_BONDING;
        cyBle_authInfo.security = (CYBLE_GAP_SEC_MODE_1 | CYBLE_GAP_SEC_LEVEL_3 );
    }
    
    CyBle_Start( StackEventHandler );

    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING) CyBle_ProcessEvents();
    
    lastOnOff = turnOff;
    myPrintf("updateBleSecurity completed\n\n");
#endif
}


void setSecurityLevel(bool unsecured)
{
    LOGD(TAG, "setSecurityLevel(want=%d have=%d))\r\n", unsecured, status.isUnsecured);
    if (unsecured != status.isUnsecured) {
        status.isUnsecured = unsecured;
        status.isResetForSecUpdate = true;
        writeFlash();
        updateBleSecurity(status.isUnsecured);
        
        LOGD(TAG, "setSecurityLevel: reseting to apply\r\n");
        reboot("SWITCH SECURITY LEVEL");
    }
}


void pushCanFifo(uint8_t* buf)
{
    memcpy(canTxFifoBuffer[canTxFifoHead].rx_buf, buf, sizeof(canTxFifoBuffer[canTxFifoHead].rx_buf));
    canTxFifoHead = (canTxFifoHead + 1) % CAN_TX_FIFO_NUM;
    if (canTxFifoHead == canTxFifoTail) {    // FIFO full — drop oldest item to avoid overtaking
        canTxFifoTail = (canTxFifoTail + 1) % CAN_TX_FIFO_NUM;
    }
}


uint8_t popCanFifo(uint8_t* buf)
{
    if (canTxFifoHead != canTxFifoTail) {
        memcpy(buf, canTxFifoBuffer[canTxFifoTail].rx_buf, sizeof(canTxFifoBuffer[canTxFifoTail].rx_buf));
        canTxFifoTail = (canTxFifoTail + 1) % CAN_TX_FIFO_NUM;
        return 1;
    }
    return 0;
}


static void process_pcm_config_pkt(uint8_t* data, uint8_t leng)
{
    pcm_config_t pc; // TODO
}


static void process_pcm_status_pkt(uint8_t* data, uint8_t leng)
{
    // TODO
}


static void process_switch_config_pkt(uint8_t* data, uint8_t leng)
{
    // TODO
}


static void process_switch_status_pkt(uint8_t* data, uint8_t leng)
{
    switch_status_t s;
    memcpy(&s, data, sizeof(s));
    if (leng >= sizeof(s)) {
        pcm_activate_circuits(&s);
    }
}


static void process_circuit_status_pkt(uint8_t* data, uint8_t leng)
{
    // TODO
}


static void packet_cb(uint8_t ptype, uint8_t* data, uint8_t leng)
{
    // TODO: fill this in as needed...
    switch (ptype) {
    case PCM_CONFIG_PKT_TYPE:     process_pcm_config_pkt(data, leng); break;
    case PCM_STATUS_PKT_TYPE:     process_pcm_status_pkt(data, leng); break;
    case SWITCH_CONFIG_PKT_TYPE:  process_switch_config_pkt(data, leng); break;
    case SWITCH_STATUS_PKT_TYPE:  process_switch_status_pkt(data, leng); break;
    case CIRCUIT_STATUS_PKT_TYPE: process_circuit_status_pkt(data, leng); break;
    }
}


int main_init(void)
{
    timer.every(10, readInputs);
    timer.every(20, readCurrents);
    timer.every(10, pcm1? update_pcm1_outputs : update_pcmN_outputs);
    timer.every(25, canBroadcast);
    //timer.every(100, do_sleep);
    //timer.every(2500, debug);

    pinMode(TEMP_TRIGGER_PIN, ANALOG);
    pinMode(TEMP_DETECT_PIN, ANALOG);
    pinMode(C_SENSE_IN_PIN, ANALOG);
    pinMode(LV_DETECT_PIN, ANALOG);

    srand(analogRead(TEMP_DETECT_PIN) + 1);

    if (readPasskey() == 0) {
        loadFlashDefaults();
        generatePasskey();
    }
    
    updateBleSecurity(status.isUnsecured);
    
    if (status.address != rs485_getaddr()) {
        status.address = rs485_getaddr();
        writeFlash();
    }
    LOGI(TAG, "Source Address: 0x%x\r\n", status.address);
    ms_delay(5 + (3 * (status.address&(PCMS-1))));     // different delays for each source address to offset communications

    isTempEdgeDisabled = TEMP_TRIG_Read() == 0;
    if (isTempEdgeDisabled) LOGD(TAG, "Temperature edge sensor issue: isTempEdgeDisabled\r\n");
    
    adc_c_calibrate();
    pkt_register_callback(packet_cb);

    LOGD(TAG, "Finish Config...\r\n\r\n");
    return 0;
}


void main_loop(void)
{
    canListen();
    resetWatchdog();
    updateAuth();
}