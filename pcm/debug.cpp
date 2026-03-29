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

static const char* TAG = "DEBUG";

//#define DEBUG_ON_TIME   30000      // 30 s
#define DEBUG_ON_TIME   600000      // 10 min

bool debug(void* unused)
{
    static uint8_t i;
    static bool lastDebug = 0;
    
    status.debug = read_inputs(0) & DEBUG_MASK;
    if (status.debug) {
        aliveTimer = millis();
        isAwake = 1;
    }
    
    if (status.debug) {
        if (!lastDebug) debugOnTimer = millis();
    } else {
        debugOnTimer = 0;
    }
    
    lastDebug = status.debug;
    
    if (debugOnTimer > 0 && (millis() - debugOnTimer) < DEBUG_ON_TIME) {
        LOGD(TAG, "Source status: %d\r\n", millis());
            
        myPrintf("Switches:\n");
        myPrintf("   Debug  |  ignCh0  |  ignCh1  |  Address  \n");
        myPrintf("     %d          %d          %d         %x   \n", 
            status.debug, /*status.ignCh[0], status.ignCh[1],*/0,0, status.address);
        myPrintf("Outputs: \n");
        myPrintf("  Channel  |  Output  |  Current  |  Select  |  Mode\n");
        myPrintf("--------------------------------------------------------\n");
        
        for (i=0; i<8; i++) {
            myPrintf("     %d     |     %d        %fA        %d        ", (i+1), 
                (status.out[i].output * 100 / 0xFF), status.out[i].current.dec,  status.inputs[i]);
            
            switch(status.out[i].status_flags) {
            case OUTPUT_OPEN:   myPrintf("open \n"); break;
            case OUTPUT_OFF:    myPrintf("off\n"); break;            
            case OUTPUT_SHORT:  myPrintf("short \n"); break;            
            case OUTPUT_NOM:    myPrintf("nominal \n"); break;            
            case OUTPUT_FAULT:  myPrintf("fault \n"); break;
            case OUTPUT_FLASH:  myPrintf("flash \n"); break;
            //case OUTPUT_STROBE: myPrintf("strobe \n"); break;
            default:            myPrintf("error \n");
            }
        }
        
        myPrintf("Other:\n");    
        myPrintf("  ignition  |  BatVoltage  |  lvBypass  |  LEDs  |    Temp\n");
        myPrintf("     %d        (%d)%fV           %d          %d       (%d) %fC   \n", 
            status.ignSense, is12v_not24, status.batVolt.dec, status.lvBypass, 0/*status.leds*/, status.tempEdge, status.temp.dec);
        myPrintf("\n");
        
        myPrintf("PRO: \n");
        myPrintf("  Channel  | ao | ig | lo | il | ar | cl | tim\n");
        myPrintf("----------------------------------------------------\n");
#if 0      
        for (i=0; i<8; i++) {
            myPrintf("     %d     |  %d    %d    %d    %d    %d    %d    %d\n", (i+1), 
                status.pro[i].alwaysOn, status.pro[i].isIgnCtrl, status.pro[i].isLockout, status.pro[i].isInputLatch, 
                status.pro[i].isCurrentRestart, status.pro[i].currentLimit, status.pro[i].timers);
        }
#endif
    }
    return true;
}