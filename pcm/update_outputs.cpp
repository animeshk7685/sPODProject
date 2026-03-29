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


static const char* TAG = "OUTPUTS";


bool ignFlag[CIRCUITS] = {0};


static void updateOutput(uint8_t index)
{
    if ((circuit_status[index].blinkOff != 0) && (circuit_status[index].blinkOn != 0)) {  // if any blink mode
    } else {
        circuit_status[index].output = circuit_status[index].outCmd;
    }   
}


bool update_pcm1_outputs(void* unused)
{
    static uint16_t lastOutputs[CIRCUITS];
 
    // TODO: go through all of the switch_status[] and pcm_status[] to update circuit_status[] for the normal
    // update_outputs loop...

    return update_pcmN_outputs(unused);
}


bool update_pcmN_outputs(void* unused)
{
    static uint16_t lastOutputs[CIRCUITS];
    
    for (uint8_t ci = 0; ci < pcm_config[current_pcm].circuit_count; ci++) {
        uint8_t index = current_pcm*CIRCUITS + ci;
        if (pcm_status[current_pcm].overheated || pcm_status[current_pcm].overheated2) {
            // OVERHEATED -- don't turn on any outputs
            if (!pcm_status[current_pcm].overheated2) {
                LOGE(TAG, "Overheated... \r\n");
                pcm_status[current_pcm].overheated2 = 1;
            }
            
            circuit_status[index].outCmd = 0;
            circuit_status[index].output = 0;
            circuit_status[index].blinkOn = 0xff;
            circuit_status[index].blinkOff = 0;
        } else {
            if ((config.ignition || pcm_config[current_pcm].circuits[ci].isIgnCtrl) && !pcm_status[current_pcm].ignSense) {
                // ignition off and configured to turn on only with ignition on
                circuit_status[index].output = 0; 
                ignFlag[ci] = true;
            } else {
                // not overheated and ignition is turned on (or ignored)
                updateOutput(index); 
                ignFlag[ci] = false;
            }
            
            if (config.lockout /*status.pro[i].isInputLockActive*/) {
                circuit_status[index].output = 0; 
                circuit_status[index].outCmd = 0; 
                circuit_status[index].blinkOff = 0;
            }
        }
        
        if (circuit_status[index].outCmd > 0) {   // manage timer to filter out flyback current
            globalOffTimer[ci] = 0;
        } else 
        if (circuit_status[index].outCmd <= 0 && globalWasOn[ci] > 0) {
            globalOffTimer[ci] = millis() - 1;
        } else 
        if (globalOffTimer[ci] > 0 && (millis() - globalOffTimer[ci] > IGNORE_OFF_TIME_MS)) {
            globalOffTimer[ci] = 0;
        }
        
        globalWasOn[ci] = circuit_status[index].outCmd;
        
        if (circuit_status[index].blinkOff == 0) {
            // either OFF or ON and not blinking
            c_pwm(ci, circuit_status[index].output, __LINE__);
            
            if ((lastOutputs[ci] != circuit_status[index].output) && (circuit_status[index].output == 0)) {
                if (circuit_status[index].status != OUTPUT_SHORT) offTimer[ci] = millis() - 1;
            }
            else if(millis() - offTimer[ci] > 10000) {
                offTimer[ci] = 0;
            }
            
            lastOutputs[ci] = circuit_status[index].output;
        }
        
        if (circuit_local[ci].timers > 0) {
            if (circuit_status[index].outCmd > 0 && circuit_local[ci].curTime == 0) {
                circuit_local[ci].curTime = millis();
            }
            else if(circuit_status[index].outCmd == 0 && circuit_local[ci].curTime != 0) {
                circuit_local[ci].curTime = 0;
            }
            else if(circuit_local[ci].curTime != 0 && (millis() - circuit_local[ci].curTime > (circuit_local[ci].timers * 60 * 1000))) {
                // output programmed to be on for only circuit_local[ci].timers minutes
                circuit_status[index].outCmd = 0; 
                circuit_local[ci].curTime = 0;
            }
        }
    }
    
    // handle blinking
    uint32_t time = millis();
    for (uint8_t ci = 0; ci < pcm_config[current_pcm].circuit_count; ci++)  {
        uint8_t index = current_pcm*CIRCUITS + ci;

        if ((circuit_status[index].blinkOff != 0) && (circuit_status[index].blinkOn != 0)) {
            uint32_t time2 = time % (circuit_status[index].blinkOff + circuit_status[index].blinkOn);
            
            circuit_status[index].output = time2 > circuit_status[index].blinkOn? 0 : circuit_status[index].outCmd;
            if (ignFlag[ci]) circuit_status[index].output = 0;
            
            if (circuit_status[index].output != lastOutputs[ci]) {
                lastOutputs[ci] = circuit_status[index].output;
                c_pwm(ci, circuit_status[index].output, __LINE__);
            }
        }
    }

    return true;
}