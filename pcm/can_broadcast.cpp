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
#include "pkt_process.h"
#include "ota.h"
#include "rs485.h"
#include "status.h"


static const char* TAG = "CANTX";

extern bool ignFlag[8];

static bool updateSystemPacket = 0;
static bool updateSwitchPacket = 0;
static bool updateStatusPacket = 0;
static bool updateProPackets = 0;
static bool updateDebugPacket = 0;
static bool updateHeartbeatPacket = 0;
static uint32_t heartbeatUpdateTimer = 0;

bool sendPacket = 0;
bool sendBlePacket = 0;
static bool onlyBle = 0;

static bool ignToggle = 0;

static uint8_t bleUpdateCount = 0;
static uint32_t bleUpdateTimer = 0;
static bool bleUpdateBridge = 0;

uint8_t tx_buffer[5] = {0x00};


static void do_update_system_packet()
{
    uint8_t address = status.address & (PCMS-1);
    tx_buffer[0] = SYSTEM_PACKET;
    tx_buffer[1] = status.batVolt.hex;
    tx_buffer[2] = status.temp.hex;     
    tx_buffer[3] = 0x55;
    tx_buffer[4] = address | (~is12v_not24 << 7);    // can error bit counter thing "d1"
    
    sendPacket = 1;
    
    if (authGood) {
        if (lastBatt[address] != status.batVolt.hex) {
            int diff = (int)status.batVolt.hex - (int)lastBatt[address];
            if (diff > 3 || diff < -3) {
                lastBatt[address] = status.batVolt.hex;
                sendBlePacket = 1;
            }
        }
        
        if (lastTemp[address] != status.temp.hex) {
            int diff = (int)status.temp.hex - (int)lastTemp[address];
            if (diff > 2 || diff < -2) {
                lastTemp[address] = status.temp.hex;
                sendBlePacket = 1;
            }
        }
    } else {
        lastBatt[address] = 0;
        lastTemp[address] = 0;
    }
    
    if (bleUpdateCount) {
        bleUpdateCount--;
        sendBlePacket = 1;
    }
    
    updateSystemPacket = 0;
}


static void do_update_debug_packet()
{
    tx_buffer[0] = ((status.address&3) | DEBUG_PACKET);
    tx_buffer[1] = 0x0A;     
    tx_buffer[2] = 0x00;
    tx_buffer[3] = 0x00;
    tx_buffer[4] = 0x00;
    
    sendPacket = 1;
    sendBlePacket = 0;
    updateDebugPacket = 0;
}


static void do_update_switch_packet()
{
    static uint8_t j = 0;
    static uint8_t lastSwitchValue[32] = {0};

    tx_buffer[0] = ((status.address&3) | SWITCH_PACKET);
    tx_buffer[1] = iToCan(j);     
    tx_buffer[2] = status.out[j].outCmd;
    tx_buffer[3] = status.out[j].blinkOn / FLASH_SPEED;
    tx_buffer[4] = status.out[j].blinkOff / FLASH_SPEED;
        //LOGD(TAG, "%d | %x | %x | %x | %x | %x\r\n", j, tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3], tx_buffer[4]);
    
    if (ignToggle && ignFlag[j]) {  // 1500 ms period (TODO: no, this only happens once at beginning of the world)
        tx_buffer[2] = 0;
    }
    
    if (status.pro[j].isIgnCtrl && ignFlag[j]) {
        sendBlePacket = 1;
    }
    
    if (lastSwitchValue[((status.address&3) * 8) + j] != status.out[j].output) {
        lastSwitchValue[((status.address&3) * 8) + j] = status.out[j].output;
        sendBlePacket = 1;
    } 
    
    if (bleUpdateCount) {
        bleUpdateCount--;
        sendBlePacket = 1;
    }
    
    if (++j > 7) {
        j=0;

        updateSwitchPacket = 0;
        updateStatusPacket = 1;
        if (ignToggle) {
            ignToggle = 0;
        } else {
            ignToggle = 1;
            lastIgnTog = millis();
        }
    }
    
    sendPacket = 1;
}


static void do_update_status_packet()
{
    static uint8_t k = 0;
    static uint8_t lastOutputState[32] = {0};
    static uint16_t lastOutputValue[32] = {0};

    int netIndex = ((status.address&3) * 8) + k;
    
    //status
    tx_buffer[0] = ((status.address&3) | STATUS_PACKET);
    tx_buffer[1] = iToCan(k);                           // Pin mask
    tx_buffer[2] = status.out[k].status_flags;          // mode pin is in (eg open, short, etc)
    tx_buffer[3] = status.out[k].current.hex >> 8;      // first byte of current    
    tx_buffer[4] = status.out[k].current.hex & 0x00FF;  // second byte of current, 
        // current value right justified 11 bit e.g. between 0x0000 - 0x07FF

    if (lastOutputState[netIndex] != status.out[k].status_flags) {
        lastOutputState[netIndex] = status.out[k].status_flags;
        sendBlePacket = 1;
    }
    
    if (lastOutputValue[netIndex] != status.out[k].current.hex) {
        int diff = (int)status.out[k].current.hex - (int)lastOutputValue[netIndex];
        if (diff > 3 || diff < -3) {
            lastOutputValue[netIndex] = status.out[k].current.hex;
            sendBlePacket = 1;
        }
    }
    if (bleUpdateCount) {
        bleUpdateCount--;
        sendBlePacket = 1;
    }
    
    if (++k > 7) {
        k=0;
        updateStatusPacket = 0;
    }
    
    sendPacket = 1;
}


static void do_update_pro_packet()
{
    static uint8_t m = 0;

    uint8_t index = m >> 1;
    uint8_t inputByte = 0x00 /*|
            (status.pro[index].isInputEnabled ? INPUT_EN_CAN_MASK : 0x00) |
            (status.pro[index].isInputLockout ? INPUT_LCK_CAN_MASK : 0x00)*/;
    
    //status
    tx_buffer[0] = ((status.address&3) | PRO_PACKET);
    tx_buffer[1] = iToCan(index);                              // Pin mask
    tx_buffer[2] = inputByte;
    
    sendBlePacket = 1;
    updateProPackets = 0;
    sendPacket = 1;
}


static void do_update_bridge_packet()
{
    static int currBufEl = -1;
    static uint8_t bleBridgeBuffer[52][5] = {0};

    if (bleUpdateBridge && (currBufEl == -1)) {
        bleUpdateBridge = 0;
        
        for (uint8_t b = 0; b < 4; b++) {
            if ((b == (status.address&3)) || (sourceConnected[b] == 0)) continue; // either us or bridge PDB not present
            
            // not this PDB or bridged PDB broadcast status or system packet, send battery and temperature
            currBufEl++;
            bleBridgeBuffer[currBufEl][0] = SYSTEM_PACKET; 
            bleBridgeBuffer[currBufEl][1] = currBatt[b]; 
            bleBridgeBuffer[currBufEl][2] = currTemp[b];
            bleBridgeBuffer[currBufEl][3] = 0x55;
            bleBridgeBuffer[currBufEl][4] = b | (currBatt12_24[b] << 7);
            
            // report PWM for 8 circuits
            for (int c = 0; c < 8 ; c++) {
                currBufEl++;
                bleBridgeBuffer[currBufEl][0] = SWITCH_PACKET | b; 
                bleBridgeBuffer[currBufEl][1] = iToCan(c);
                bleBridgeBuffer[currBufEl][2] = currSwitchValue[b * 8 + c];
                bleBridgeBuffer[currBufEl][3] = currBlinkOn[b * 8 + c];
                bleBridgeBuffer[currBufEl][4] = currBlinkOff[b * 8 + c];
            }
            
            if (sourceConnected[b] < 2) continue; // status packet not received from bridged PDB
            
            // status packet received from bridged PDB, report status and current for 8 circuits
            for (int c = 0; c < 8 ; c++) {
                currBufEl++;
                bleBridgeBuffer[currBufEl][0] = STATUS_PACKET | b; 
                bleBridgeBuffer[currBufEl][1] = iToCan(c);
                bleBridgeBuffer[currBufEl][2] = currStatusValue[b * 8 + c];
                bleBridgeBuffer[currBufEl][3] = currCurrentHigh[b * 8 + c];
                bleBridgeBuffer[currBufEl][4] = currCurrentLow[b * 8 + c];
            }
        }
    }
    
    if (currBufEl >= 0) {
        memcpy(tx_buffer, bleBridgeBuffer[currBufEl], sizeof(tx_buffer));
        sendPacket = 1;
        onlyBle = 1;
        sendBlePacket = 1;
        currBufEl--;
    }
}


static void do_send_can_packet()
{
    //Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
    pkt_put(pkt_broadcast, CAN_PKT_TYPE, tx_buffer, sizeof(tx_buffer), -1);
}


static void check_packet_timers()
{
    static uint32_t switchTimer = 0;
    static uint32_t debugTimer = 0;
    static uint32_t systemTimer = 0;

    uint32_t now = millis();

    if ((now - systemTimer) > 900/*535*/) {
        systemTimer = now;
        updateSystemPacket = 1;
    }

#if 0
    if ((now - debugTimer) > 600) {
        debugTimer = now;
        updateDebugPacket = 1;
    }
#endif

    if ((now - switchTimer) > 1000 /*750*/) {
        switchTimer = now;
        updateSwitchPacket = 1;
    }

    if ((now - bleUpdateTimer) > 2000) {
        bleUpdateTimer = now;
        bleUpdateCount = 17;
        bleUpdateBridge = 1;
    }

    if ((now - heartbeatUpdateTimer) > 5000) {
        heartbeatUpdateTimer = now;
        updateHeartbeatPacket = true;
    }
}


static void do_send_heartbeat_packet()
{
    if (!pkt_ota_in_progress()) send_heartbeat_pkt(rs485_getaddr());
    updateHeartbeatPacket = false;
}


bool canBroadcast(void* unused)
{
    //Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
    if (isAwake) {
  #if 0      
        if (ble_client_connected && (sendSecPacket || sendSettingsPacket)) {   
            // ignore CAN if OTA info is needed
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
        } else 
        if (popCanFifo(tx_buffer)) {
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            sendPacket = 1;
        } else 
        if (updateSystemPacket) {  
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            do_update_system_packet(); // temp/voltage
        } else 
        if (updateDebugPacket) { 
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            do_update_debug_packet(); // tell shields/periphs debug mode/leds are on
        } else 
        if (updateSwitchPacket) {
            do_update_switch_packet();
        } else 
        if (updateStatusPacket) {
            //Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            do_update_status_packet();
        } else 
        if (updateProPackets) {
            //Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            do_update_pro_packet();
        } else 
        if (authGood) {
            //Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
            do_update_bridge_packet();
        } else 
#endif
        if (updateHeartbeatPacket) {
            do_send_heartbeat_packet();
        }
        
        if (sendPacket) {
            sendPacket = 0;

            if (onlyBle) {
                onlyBle = 0;
            } else {
                do_send_can_packet();
            }
            
            if (sendBlePacket && ble_client_connected) do_send_ble_packet();
        }
        else 
        if(sendSecPacket && ble_client_connected) {
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();

            do_send_sec_packet();  
        } else 
        if(sendSettingsPacket && ble_client_connected) {
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();

            do_send_settings_packet();
        } else 
        if(sendProPacket && ble_client_connected) {
            Serial.printf("%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();

            do_send_pro_packet();
        }
        
        check_packet_timers();
        
        if (globalUpdateProCanPackets) {
            globalUpdateProCanPackets = false;
            updateProPackets = true;
        }
        
        // if a system packet has not been received from one of the other 3 PDB
        // boards, mark it as no longer present
        for (int s = 0; s < 4; s++) {
            if ((sourceConnected[s] > 0) && ((millis() - srcConnTimer[s]) > 30000)) {
                sourceConnected[s] = 0;
            }
        }
    }

    return true;
}