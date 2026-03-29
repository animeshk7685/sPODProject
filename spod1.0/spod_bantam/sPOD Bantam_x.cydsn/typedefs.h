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
#ifndef TYPEDEFS_H
#define TYPEDEFS_H

    #include <stdbool.h>
    #include <project.h>
    
    typedef struct{
        uint16   hex;
        float32  dec;
    }adcVals_t;
    
    typedef struct{
        uint8_t dimVal;
        uint8_t onVal;
        uint8_t offVal;
    } lastVals_t;
    
    typedef struct {
        uint16  outCmd;
        uint16  output;
        uint8   open;
        uint16  blinkOn;
        uint16  blinkOff;
        adcVals_t current;        // in amps
        uint32  timer;
        uint32  onTime;
        uint32  shortDelay;
    }output_t;

    typedef struct {
        bool    isEnabled;
        bool    isWritable;
        bool    isDisableSleep;
        bool    isInputLinking;
        bool    alwaysOn[8];
        bool    isIgnCtrl[8];
        bool    isLockout[8];
        uint8   lastSwVal[8];
        uint16  timers[8];
        uint32  curTime[8];
        bool    isInputLatch[8];
        bool    isCurrentRestart[8];
        uint8_t currentLimit[8];
        bool    needsRestart[8];
        output_t lastOut[8];
        bool    isInputEnabled[8];
        bool    isInputLockout[8];
//        bool    isInputLockInvert[8];
        bool    isInputLockActive[8];
        uint32_t links[32];
        lastVals_t lastRecVals[32];
    }proMode_t;

    

    typedef struct{
        bool        debug;
     //   bool    lowBat;
        bool        ignCh[2];
        uint8_t     address;        // 0x80 - 0x83
        bool        ignSense;
        bool        inputs[8];
        adcVals_t     batVolt;        // in volts
        adcVals_t     temp;
        bool        leds;
        bool        lvBypass;
        bool        tempEdge;
        bool        isUnsecured;
        output_t    out[8];
        proMode_t   pro;
        bool        isResetForSecUpdate;
        bool        isDeepSleepEnabled;
    } status_t;
    
#endif
/* [] END OF FILE */
