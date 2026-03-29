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
#include "xi.h"

static const char* TAG = "INPUTS";


bool readInputs(void* unused)
{
    static uint32_t last_sent = 0;
    static uint8_t debugCycle = 0;
    static bool passDebug = true;
    static bool isSecListen = true;
    static uint8_t lastSec1 = 0xff;
    static uint8_t lastSec2 = 0xff;
    static uint8_t secCycle1 = 0;
    static uint8_t secCycle2 = 0;
    uint8_t inputs = read_inputs(0); 
    uint8_t changed = inputs ^ pcm_status[current_pcm].inputs;

    #if 0
    status.debug = read & DEBUG_MASK; // TODO: I think these are hardcoded -- but setup by installer/customer
    
    if ((debugCycle % 2) != status.debug) {
        debugCycle++;
        
        if(status.debug) {
            passkeyTimer = millis();        // reset on rising edge of debug switch
        }
    }
    
    if (passDebug) {  
        if (debugCycle > 3) {
            generatePasskey();              // only regenerate if switch it tripped 4 times in the first 30s
            passDebug = false; 
        }
        
        if (millis() > PASS_DBG_TIMEOUT) {
            passDebug = false;              
        }
    }
    
    if (isSecListen) {
        if (status.isResetForSecUpdate) {
            passkeyTimer = 1;
            status.isResetForSecUpdate = false;
            writeFlash();
        }
        
        uint8_t addRead1 = read & SW5_MASK ? 1 : 0;
        uint8_t addRead2 = read & SW6_MASK ? 1 : 0;
        
        if (lastSec1 == 0xff || lastSec2 == 0xff) {
            lastSec1 = addRead1;
            lastSec2 = addRead2;
        }
        else if (lastSec1 != addRead1) {
            lastSec1 = addRead1;
            if (++secCycle1 == 2)
            {
                setSecurityLevel(UNSECURED);
            }
        } else if(lastSec2 != addRead2) {
            lastSec2 = addRead2;
            if (++secCycle2 == 2) {
                setSecurityLevel(SECURED);
            }
        }
        
        if (millis() > PASS_DBG_TIMEOUT) {
            isSecListen = false;              
        }
    }
    #endif

    if (isAwake) {
        if (changed) {
            lastActivityMs = millis();
            pcm_status[current_pcm].changed = changed;
            pcm_status[current_pcm].inputs = inputs;

            for (uint8_t ti = 0; ti < TRIGGERS; ++ti) {
                if (changed & (1<<ti)) {
                    uint8_t bits = pcm_config[current_pcm].triggers[ti].bits;
                    bool set = false;
                    if (bits & TRIGGER_ACTIVE_LOW) {
                        set = (inputs & (1<<ti)) == 0;
                    } else {
                        set = (inputs & (1<<ti)) != 0;
                    }
                    if (set) {
                        pcm_status[current_pcm].triggers |= (1<<ti);
                    } else {
                        pcm_status[current_pcm].triggers &= ~(1<<ti);
                    }
                }
            }
        }

        bool ignSense = ign_sense();
        bool tempEdge = TEMP_TRIG_Read();
        adcVals_t batVolt = vbat_read();
        bool is12v_not24 = batVolt.dec <= 15.7;
        if (!changed) {
            float prev = pcm_status[current_pcm].batVolt.dec;
            float delta = (batVolt.dec < prev)? prev - batVolt.dec : batVolt.dec - prev;

            changed = ignSense != pcm_status[current_pcm].ignSense ||
                      tempEdge != pcm_status[current_pcm].tempEdge ||
                      is12v_not24 != pcm_status[current_pcm].is12v_not24 ||
                      (delta >= 2.0);
        }
        pcm_status[current_pcm].ignSense = ignSense;
        pcm_status[current_pcm].tempEdge = tempEdge;
        pcm_status[current_pcm].batVolt = batVolt;
        pcm_status[current_pcm].is12v_not24 = is12v_not24;
        
        pcm_status[current_pcm].temp = temp_read();
        if (pcm_status[current_pcm].temp.dec >= SHUTOFF_TEMP) {
            LOGE(TAG, "%d -- %s(): OVERHEATED!!!\r\n", millis(), __FUNCTION__);
            pcm_status[current_pcm].overheated = 1;
            changed = 1;
        }
        if (!isTempEdgeDisabled && !pcm_status[current_pcm].tempEdge) {
            LOGE(TAG, "%d -- %s(): EDGE OVERHEATED!!!\r\n", millis(), __FUNCTION__);
            pcm_status[current_pcm].overheated2 = 1;
            changed = 1;
        }

        if (last_sent == 0) last_sent = millis();
        if (changed || millis() - last_sent > 10000) {
            last_sent = millis();
            send_pcm_status_pkt(current_pcm);
        }
    }
    return true;
}