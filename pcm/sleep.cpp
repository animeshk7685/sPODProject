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


static const char* TAG = "SLEEP";


static void enableDeepSleep(uint8_t needsDeepSleep)    // does not return from sleep
{
    if (needsDeepSleep) {
#if 0 // TODO
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
        
        myPrintf("Going into deep sleep: %fv - %d\n", lastAdc);//, lastCAN);
        
        CyDelay(10);
        
        CyBle_EnterLPM(CYBLE_BLESS_HIBERNATE);
        CyBle_Stop();

        UART_Stop();

        set_watchdog_timeout(DEEP_SLEEP_WDT);
        
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
            
                CyIntEnable((SysTick_IRQn + 16));
                
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
        }
#endif
    } else {
        // wake logic
    }
}


bool do_sleep(void* unused)
{
    static uint32_t lvTimer;
    static uint32_t lvMessageTime = 0;
    
    static float lastBatVolt = 0.0;
    static uint32_t lastIgnCheck = 0;
    
    #if IS_TEST_DEEP_SLEEP
        if (millis() > 60000) enableDeepSleep(1);
    #endif
        
    if (is12v_not24) {
        if (status.batVolt.dec > 11.6) lvTimer = millis();
    } else {
        if (status.batVolt.dec > 23.2) lvTimer = millis();
    }
    
    if (millis() > 10000 && (millis() - lastIgnCheck) > 5000) {
        lastIgnCheck = millis();
        
        adcVals_t batV = vbat_read();
        
        if (lastBatVolt < 1.0) {
            lastBatVolt = batV.dec;
        } else {
            if ((batV.dec - lastBatVolt) > 1.0) {
            }
            lastBatVolt = batV.dec;
        }
    }
    
    if ((millis() - lvTimer) > LV_TIMEOUT) { // && !status.debug)
        if ((millis() - lvMessageTime) > 30000) {
            //LOGD(TAG, "Low voltage timeout: %d\r\n", millis());
            lvMessageTime = millis();
        }
        
        /* TODO -- BYPASS_CTRL_Write(0); */
    // if alternate lv_bypass isn't enabled, this will completely power down the unit and all periphrials
    }
    
    if (millis() > 10000 && !status.debug && !(status.isDisableSleep) && 
    (!isAwake || (
        (millis() - bleAliveTimer) > 30000 && 
        (((millis() - lastActivityMs) < ACTIVITY_TIMEOUT && (millis() - aliveTimer) > ALIVE_TIMEOUT) ||
        ((millis() - lastActivityMs) > ACTIVITY_TIMEOUT && (millis() - aliveTimer) > ALIVE_TIMEOUT_INACTIVE))
    ))) {
        if (!wasAsleep) {     // sleep "initialize"
            LOGI(TAG, "SLEEP %d\r\n", millis());
            
            //myPrintf("timers: %d %d %d\n", millis(), aliveTimer, lastActivityMs);
            
            outputs_off();                      // disable outputs and pwm IC
            outputs_sleep();
            
//                PS_LEDS_Write(0);
            
            for (int i = 0; i < 8; i++) status.out[i].outCmd = 0;
        }
        
        if (status.isDeepSleepEnabled || DEEP_SLEEP_FORCE_ON) enableDeepSleep(1);
    
        isAwake = 0;
        wasAsleep = 1;            
        
        status.batVolt = vbat_read();
    }
                    
    if (isAwake && wasAsleep) {     // awake "initialize
        LOGD(TAG, "AWAKEN %d\r\n", millis());
        
        outputs_wake();                     // renable pwm IC
        
        wasAsleep = 0;
        
        for (int i = 0 ; i < 8 ; i++) {
            status.out[i].outCmd = status.pro[i].alwaysOn ? status.pro[i].lastSwVal : 0x00;
        }
        aliveTimer = millis();
    }

    return true;
}