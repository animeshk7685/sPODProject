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


static const char* TAG = "CANRX";

static uint8_t i;
static uint8_t _index = 0;

static uint8_t rx_buffer[5] = {0x00};


static void rx_debug_packet()
{
    if (rx_buffer[1] == 0x7E) {
        isAwake = 1;
        if (millis() > 10000) {
            turnLedsOn(false);
        }
    }
}


static void rx_system_packet()
{
    srcConnTimer[rx_buffer[4] & 0x03] = millis();
    if (sourceConnected[rx_buffer[4] & 0x03] < 2) {
        sourceConnected[rx_buffer[4] & 0x03] = 1;
    }
    
    currBatt[rx_buffer[4] & 0x03] = rx_buffer[1];
    currTemp[rx_buffer[4] & 0x03] = rx_buffer[2];
    currBatt12_24[rx_buffer[4] & 0x03] = rx_buffer[4] & 0x80 >> 7;
}


static void rx_switch_packet()
{
    _index = canToI(rx_buffer[1]) + 8 * (rx_buffer[0] & 0x0F);
        
    if (rx_buffer[2] > 0) {
        lastRecVals[_index].dimVal = rx_buffer[2];
        lastRecVals[_index].onVal = rx_buffer[3];
        lastRecVals[_index].offVal = rx_buffer[4];
    }
    
    if (rx_buffer[0] == (SWITCH_PACKET | (status.address&3))) {
        lastActivityMs = millis();
        
        isAwake = 1;
        
        i = canToI(rx_buffer[1]);
        if (status.pro[i].isLockout) {
        } else 
        if (shortDelay[i] == 0) {
            status.out[i].outCmd = rx_buffer[2]; 
            if(rx_buffer[2] == 0xFE) {           // correct for legacy values
                status.out[i].outCmd = 0xFF;
            }
            if (status.out[i].outCmd != 0) {
                status.pro[i].lastSwVal = status.out[i].outCmd;
            }
            status.out[i].blinkOn = rx_buffer[3] * FLASH_SPEED; 
            status.out[i].blinkOff = rx_buffer[4] * FLASH_SPEED;

            // Sync to circuit_status[] which is read by update_pcmN_outputs()
            uint8_t ci_index = current_pcm * CIRCUITS + i;
            if (ci_index < PCMS * CIRCUITS) {
                circuit_status[ci_index].outCmd  = status.out[i].outCmd;
                circuit_status[ci_index].blinkOn = status.out[i].blinkOn;
                circuit_status[ci_index].blinkOff = status.out[i].blinkOff;
            }
        }
    } else {
        currSwitchValue[_index] = rx_buffer[2];
        currBlinkOn[_index] = rx_buffer[3];
        currBlinkOff[_index] = rx_buffer[4];
    }
}


static void rx_status_packet()
{
    sourceConnected[rx_buffer[0] & 0x03] = 2;
    _index = canToI(rx_buffer[1]) + 8 * (rx_buffer[0] & 0x0F);
        
    currStatusValue[_index] = rx_buffer[2];
    currCurrentHigh[_index] = rx_buffer[3];
    currCurrentLow[_index] = rx_buffer[4];
}


static void rx_pro_packet()
{
    if ((rx_buffer[0] & 0x0F) == (status.address&3) && (rx_buffer[1] == 0 || canToI(rx_buffer[1]) != 255)) {
        _index = canToI(rx_buffer[1]);
        
        if(rx_buffer[1] != 0) {  // else ping request
            #if 0
            status.pro[_index].isInputEnabled = (rx_buffer[2] & INPUT_EN_CAN_MASK) ? true : false;
            status.pro[_index].isInputLockout = (rx_buffer[2] & INPUT_LCK_CAN_MASK) ? true : false;
            #endif
            writeFlash();
            needsInputsUpdate = true;
        } else {
            globalUpdateProCanPackets = true;
        }
    }
}


void canListen()
{
    uint8_t owner;

    if (pkt_get(&owner)) {
        if ((pkt_type(NULL) & MASK_PKT_TYPE) != CAN_PKT_TYPE) {
            pkt_process(owner);
        } else {
            memcpy(rx_buffer, pkt_data(NULL), sizeof(rx_buffer));
            
            if (rx_buffer[0] == (DEBUG_PACKET | (status.address&3)))                   rx_debug_packet();   else
            if ((rx_buffer[0] & 0xF0) == SYSTEM_PACKET)                                rx_system_packet();  else
            if ((rx_buffer[0] & 0xF0) == SWITCH_PACKET && canToI(rx_buffer[1]) != 255) rx_switch_packet();  else
            if ((rx_buffer[0] & 0xF0) == STATUS_PACKET && canToI(rx_buffer[1]) != 255) rx_status_packet();  else
            if ((rx_buffer[0] & 0xF0) == PRO_PACKET)                                   rx_pro_packet();
        }
        pkt_release();
    }
}