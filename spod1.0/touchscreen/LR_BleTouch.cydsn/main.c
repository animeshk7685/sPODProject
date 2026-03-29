/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 *  Version 1.00    02-12-2017
 *  Version 1.01    02-13-2017
 *  Version 1.02    02-13-2017
 *
 * ========================================
*/

#include <project.h>
#include "sPodCanTouch.h"

//1ms
#define NUMBER_OF_TICKS		 48000

static uint32 ticks = 0;

/* ISR prototype declaration */
CY_ISR_PROTO(USER_ISR); 
CY_ISR_PROTO(TOUCH_ISR);

CY_ISR(USER_ISR)
{
    ticks++;
}

CY_ISR(TOUCH_ISR)
{
    AT_INT_ClearInterrupt();
//    CALI_ClearInterrupt();
    
//    myPrintf("i.");
    
//    globalNeedsWake = true;
    globalNeedsSleepReset = true;
}

uint32 millis(){
    return ticks;
}

void resetWatchdog(void);

#define WDT_INTERVAL_1S             125u                       /* millisecond */
#define ILO_FREQ                    32768     

//volatile uint8 wdtIsrCount = 0;
sTouchRaw tw;

CY_ISR_PROTO(WDT_ISR);

CY_ISR(WDT_ISR)
{
//    myPrintf("i");
    
//    resetWatchdog();
    CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
 
    
    CyIntEnable((SysTick_IRQn + 16));
    
    CyDelayUs(50);
    
    FT_GC_HostCommand(FT_ACTIVE);
    
    CyDelayUs(50);  
//    CyDelay(20);
    
    FT_GC_GetRawTouch(&tw);
    
    if(!tw.touch || CALI_Read()){
//        myPrintf(".");
        
        //save flag in flash
        
        setWakeFromHib(true);
        
        CySoftwareReset();
        
    }
    
    CyDelayUs(50);    
    FT_GC_HostCommand(FT_STANDBY);
    
    CyDelayUs(50);
//    CyDelay(5000);
        
    CyIntDisable((SysTick_IRQn + 16));  
//    CyDelayUs(50);
//    CySysPmDeepSleep(); 
}


void InitWatchdog(uint16 reset_interval)
{
    
//    wdtIsrCount = 0;
    
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
    isr_wdt_StartEx(WDT_ISR);
    /* set the highest priority to make ISR execute in all condition */
    isr_wdt_SetPriority(0);
}



int main()
{
#if !defined(__ARMCC_VERSION)
    InitializeBootloaderSRAM();
#endif

    // Checks if Self Project Image is updated and Runs for the First time 
    AfterImageUpdate();    
    
    
    UART_Start();
    
//    myPrintf("W: %x, %x ", CySysPmGetResetReason(), CySysGetResetReason() );
    
    if( CySysPmGetResetReason() == CY_PM_RESET_REASON_WAKEUP_HIB)
    {
//        myPrintf("WAKE FROM: hibernate \n");
//        wakeFromHib = true;
    }

    if( CySysPmGetResetReason() == CY_SYS_RESET_WDT)
    {
//        myPrintf("WAKE FROM: wdt \n");
//        wakeFromHib = true;
    }

    
    if( CySysPmGetResetReason() == CY_SYS_RESET_SW)
    {
//        myPrintf("WAKE FROM: reset \n");
//        wakeFromHib = true;
    }

    
    SCB_Start();
//    UART_Start();
    
    myPrintf("main() \n");
    
//    AT_INT_SetDriveMode(AT_INT_DM_OD_HI);
    isr_touch_StartEx(TOUCH_ISR);
//    isr_cali_StartEx(TOUCH_ISR);
    
    InitWatchdog(WDT_INTERVAL_1S);
    
    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
    
//    isr_touch_Disable();
    
    CyIntSetSysVector((SysTick_IRQn + 16), USER_ISR);
    SysTick_Config(NUMBER_OF_TICKS);
    
    
    CyGlobalIntEnable;
    
    setup();
    for(;;)
    {
        loop();
    }
}

/* [] END OF FILE */
