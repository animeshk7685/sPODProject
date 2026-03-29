// Do not remove the include below



#include "sPodCanTouch.h"

#define ROTATE_SCREEN 1





#ifdef __cplusplus
} // extern "C"
#endif

#define IS_PRO_TESTING         (0)      // set to 1 to default to pro
//#define DEEP_SLEEP_ENABLED     (1)    ->> in common.h
#define DEFAULT_SLEEP_TIMERS    (1)

#if (DEFAULT_SLEEP_TIMERS)
    
#define DEEP_SLEEP_MS (21600000)    // 6 hrs
#define SLEEP_TIMER_MS (600000)     // 10 min
    
#else
    
#define DEEP_SLEEP_MS (120000)      // 2 min
#define SLEEP_TIMER_MS (60000)      // 1 min
    
#endif

//6 hours
//#define DEEP_SLEEP_MS (21600000)

//2 min
//#define DEEP_SLEEP_MS (120000)

//20 sec
//#define DEEP_SLEEP_MS (20000)

//10 min
//#define SLEEP_TIMER_MS (600000)

//1 min
//#define SLEEP_TIMER_MS (60000)

int machineTick(void *_machine);
int machineInit(tMachineStruct *machine);

/* Global object for FT800 Implementation */
//FT800IMPL_SPI FTImpl(8, 12, 13);

tMachineStruct myMachine;
simplehsm_t mainHsm = { NULL };
machineParams params;

unsigned long frameMillis = 40;
unsigned long canMillis = 5;
unsigned long lastTime;
unsigned long currentTime;
unsigned long elapsedTime;

unsigned long machineLastTime;
unsigned long machineElapsedTime;

int caliButton = 0;

tTimer frameTimer;
tTimer tickTimer;

static bool wasOff = false;
tCalibrate calibrate;



stlogger genericLogger(const uint8_t * logString) {
	myPrintf((char*)logString);
}; 


void loadSettings()
{
    settings.cps = 200;
	settings.fps = 25;
	settings.sleepTimer = SLEEP_TIMER_MS;
	//settings.aliveTimer = 1000;//rand_interval(500,1500);
	settings.dayBrightness = 128;
	settings.nightBrightness = 64;
	settings.dayNightThreshold = 128;
	settings.fgColor = 0x000000;
	settings.bgColor = 0x303030;

	settings.color = 0xffffff;
    settings.greyColor = 0xd0d0d0;
	settings.gradColor = 0xffffff;
	settings.highlightColor = 0xEE0000;

	settings.sliderBgColor = 0x101010;
	settings.sliderFgColor = 0xEE0000;
	settings.sliderColor = 0x404040;

	settings.pageBgColor = 0x202020;
    
    myPrintf("loadSettings()\n");
}


unsigned int getConfigCrc(void) {
	unsigned char currentConfigCrc = 0;

	unsigned int i;
	unsigned char *cByte;

	cByte = (uint8_t *) &calibrate;
	unsigned int cLength = sizeof(calibrate);

	for (i = 0; i < cLength - 4; i++) {
		crc8(&currentConfigCrc, cByte[i]);
	}

	return (unsigned int) currentConfigCrc;

}


/* Api to bootup ft800, verify FT800 hardware and configure display/audio pins */
/* Returns 0 in case of success and 1 in case of failure */
int16_t BootupConfigure() {
    FTCS_Write(1);
    SPI_ss0_m_Write(1);
    
	uint32_t chipid = 0;
    
    FT_GC_Init_Soft(FT_DISPLAY_RESOLUTION);
    
	CyDelay(20); //for safer side  
    
	chipid = FT_Transport_SPI_Read32(FT_ROM_CHIPID);

	/* Identify the chip */
	if (FT800_CHIPID != chipid) {
		myPrintf("\nError in chip id read, Attempting hardware reset...\n");
        
        FT_GC_InitA(FT_DISPLAY_RESOLUTION);
    
	    CyDelay(20); //for safer side  
    
	    chipid = FT_Transport_SPI_Read32(FT_ROM_CHIPID);
        
        if (FT800_CHIPID != chipid) {
    		myPrintf("\nError in chip id read, id = %x, should be %x, probably an SPI communication error or maybe a power enable pin...\n\n", chipid, FT800_CHIPID);
            
    		return 1;
    	} else {
            myPrintf("\nChip id read 2 good, id = %x \n\n", chipid);
        }
        
	} else {
        myPrintf("\nChip id read good, id = %x \n\n", chipid);
    }

    FT_GC_SetTouchRzThresh(0xFFFF);
//    FT_GC_SetTouchOversample(15);
    FT_GC_SetTouchCharge(1000);
    
    
    FT_GC_SetDisplayEnablePin(7);
//    FT_GC_SetAudioEnablePin(1);   
	FT_Transport_SPI_Write32(REG_PWM_DUTY, 0);
	FT_GC_DisplayOn(); 
//	  FT_GC_AudioOn(); 
	FT_Transport_SPI_Write32(REG_ROTATE, ROTATE_SCREEN);
	CyDelay(500);
	FT_Transport_SPI_Write32(REG_PWM_DUTY, 128);

	return 0;
}

/* API for calibration on ft800 */
void Calibrate() {
    
    myPrintf("Calibrating...\n");
	/*************************************************************************/
	/* Below code demonstrates the usage of calibrate function. Calibrate    */
	/* function will wait untill user presses all the three dots. Only way to*/
	/* come out of this api is to reset the coprocessor bit.                 */
	/*************************************************************************/

    sTouchRaw check;
    FT_GC_GetRawTouch(&check);
            
    if(check.touch == RAW_TOUCHED)
    {
        FT_GC_DLStart();
    
        FT_GC_ClearColorRGB(64, 64, 64);
    	FT_GC_ClearCST(1, 1, 1);
        FT_GC_ColorRGB(0xff, 0xff, 0xff);    
    	FT_GC_Cmd_Text((FT_DISPLAYWIDTH / 2), (FT_DISPLAYHEIGHT / 2), 27, FT_OPT_CENTER, "Please release the screen");
        
        FT_GC_DLEnd();
        
        while(check.touch == RAW_TOUCHED)
        {
            CyDelay(25);
            FT_GC_GetRawTouch(&check);
        }
        
        CyDelay(250);
    }
    
//    FT_GEStatus status0;
    
	/* Construct the display list with grey as background color, informative string "Please Tap on the dot" followed by inbuilt calibration command */
	FT_GC_DLStart();
    
    FT_GC_ClearColorRGB(64, 64, 64);
	FT_GC_ClearCST(1, 1, 1);
    FT_GC_ColorRGB(0xff, 0xff, 0xff);    
	FT_GC_Cmd_Text((FT_DISPLAYWIDTH / 2), (FT_DISPLAYHEIGHT / 2), 27, FT_OPT_CENTER, "Please Tap on the dot");
    FT_GC_Cmd_Calibrate(0);

	/* Wait for the completion of calibration - either finish can be used for flush and check can be used */
	FT_GC_Finish();

    myPrintf("done calibration\n");

	calibrate.regTouchTransformA = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_A);
	calibrate.regTouchTransformB = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_B);
	calibrate.regTouchTransformC = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_C);
	calibrate.regTouchTransformD = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_D);
	calibrate.regTouchTransformE = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_E);
	calibrate.regTouchTransformF = FT_Transport_SPI_Read32 (REG_TOUCH_TRANSFORM_F);
    
    calibrate.initialized = 1;
    
	saveConfig();
}

void sendEventWithObj(int event, machineParams *mParams, void *obj)
{
	machineEvent evt;
	evt.asInts[0] = (int) obj;
	mParams->mEvt = &evt;
	publish(event, mParams);
}

bool isDaytime()
{
	return true;
}


bool globalIsSleep = false;
bool globalNeedsSleep = false;
//bool globalNeedsWake = false;

//void touchISR() {
//
////	detachInterrupt(1);
//
//	myPrintf("touchISR...INT\n");
//	//globalNeedsWake = true;
//
//}




void SystemSleep()
{
//    uint8_t count = 0;
    
	globalNeedsSleep = false;
//    FT_GC_Reset();
    
    CyDelay(25);
    
    FT_Transport_SPI_Write32(REG_PWM_DUTY, 0);
////	FTImpl.DisplayOff();
    FT_GC_DisplayOff();
    
//    CyDelay(100);
    
//    FT_GC_Reset();
    
    CyDelay(100);
    
//	FTImpl.HostCommand(FT_SLEEP);    
//    FT_GC_HostCommand(FT_SLEEP);
    
    FT_GC_HostCommand(FT_STANDBY );
    
//    do {
//        AT_INT_SetDriveMode(AT_INT_DM_OD_HI);
//        CyDelay(100);
//        AT_INT_Write(1);        // bias int pin and set for wakeup read
//        CyDelay(200);
//        AT_INT_Write(0);
//        CyDelay(100);
////        AT_INT_SetDriveMode(AT_INT_DM_RES_DWN);
//        AT_INT_SetDriveMode(AT_INT_DM_DIG_HIZ);
//        CyDelay(100);
        
//        count++;
//        if(count > 10)
//        {
//            CySoftwareReset();
//        }
//    } while (AT_INT_Read() != 1);
//    
    
//    AT_INT_ClearInterrupt();
    isr_touch_Enable();
    
    globalNeedsWake = false;
    
    mcpCanSleep(true);
//    STBY_Write(1);
//	digitalWrite(38, HIGH);		//PE6 STB Put can controller to sleep

    globalIsSleep = true;
  
    CyDelay(100);
   
    myPrintf("system sleep... %d (%d)\n", millis(), AT_INT_Read());

    CyDelay(100);
   
    CyBle_EnterLPM(CYBLE_BLESS_HIBERNATE);
    CyBle_Stop();
    
//    SCB_Stop();
    UART_Stop();
    
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    CyIntDisable((SysTick_IRQn + 16));
    
//    if(AT_INT_Read() != 1)
//    {
//         CySoftwareReset();
//    }
//    
//    CySysPmHibernate();         // wake up into reset from interrupt trigger
    while(1)
    {
        CySysPmDeepSleep();         
    }
    
    
//    CyIntEnable((SysTick_IRQn + 16));
    
	// Choose our preferred sleep mode:
//	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//
//
//
//	noInterrupts();
//
//	attachInterrupt(1, touchISR, LOW);
//
//	// Set sleep enable (SE) bit:
//	sleep_enable();
//
    
//
//	// Put the device to sleep:
//	interrupts();
//	sleep_cpu();
//
//	//DOES NOT SLEEP HERE FOR LONG.. immedetly wakes up
//
//	// Upon waking up, sketch continues from this point.
//	sleep_disable();

//	FTImpl.Reset();
//    FT_GC_Reset();
//    
//
//	myPrintf("system wake... %d\n", millis());

//        CyDelay(30000);
    
//	wdt_enable(WDTO_120MS);
    
//        globalNeedsWake = true;
    
//    isr_touch_Enable();

}

bool isWaking = false;

void SystemWake()
{
    isr_touch_Disable();

//	wdt_enable(WDTO_120MS);

    mcpCanSleep(false);
//    STBY_Write(0);
//	digitalWrite(38, LOW);

//	FTImpl.ActiveInternalClock();
//    FT_GC_ActiveInternalClock();
//	FTImpl.DisplayConfigExternalClock(FT_DISPLAY_RESOLUTION);
//    FT_GC_DisplayConfigExternalClock(FT_DISPLAY_RESOLUTION);
    
    
    
//    FT_GC_WriteCmd(FT_ACTIVE);
    FT_GC_HostCommand(FT_ACTIVE);

	CyDelay(100);


	CyDelay(100);

	DisplaySleep(false);


	globalIsSleep = false;
	globalNeedsSleep = false;

	CyDelay(500);

	globalNeedsWake = false;
	isWaking = false;

    globalNeedsSleepReset = true;
    
	myPrintf("system full wake... %d\n", millis());
}

void DisplaySleep(bool isSleep)
{


	if(isSleep)
	{
		FT_Transport_SPI_Write32(REG_PWM_DUTY, 0);
		FT_GC_DisplayOff();
		wasOff = true;

        myPrintf("display sleep... %d\n", millis());
	}
	else
	{
//        if(settings.sleepTimer > 0)
//        {
//			sleepTimer.currentTime = settings.sleepTimer;
//            sleepTimer.rollover = settings.sleepTimer;
//        }
        
//        myPrintf("display wake... %d\n", millis());
		FT_GC_DisplayOn();

		if(wasOff)
			CyDelay(250);

		if(isDaytime())
		{
			FT_Transport_SPI_Write32(REG_PWM_DUTY, settings.dayBrightness);
		}
		else
		{
			FT_Transport_SPI_Write32(REG_PWM_DUTY, settings.nightBrightness);
		}

		if(wasOff)
		{
			//wasOff = false;
			CyDelay(500);
		}
	}
}

bool needsProInputsRequest = true;

stnext mainIdle(int signal, void* params) {

	static bool wasTouched = false;
    static bool wasRawTouched = false;
	static int holdCount = 0;

	static tTimer sleepTimer;
    static tTimer deepSleepTimer;
	static tTimer aliveTimer;

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
	char pageId;

    static unsigned long localLast = 0;
    static unsigned long localElapsed = 0;
    
    
	switch (signal) {
	case SIG_INIT:
        
        myPrintf("Main Idle Init \n");
        
//        localLast = millis();
        
		settings.aliveTimer = rand_interval(500,1500);

		sleepTimer.currentTime = settings.sleepTimer;
		sleepTimer.rollover = settings.sleepTimer;
        
//        myPrintf("sleep init %d\n", settings.sleepTimer);
        
        deepSleepTimer.currentTime = DEEP_SLEEP_MS;
		deepSleepTimer.rollover = DEEP_SLEEP_MS;

		aliveTimer.currentTime = settings.aliveTimer;
		aliveTimer.rollover = settings.aliveTimer;

		//first page -- eventually needs to goto warning page...
		simplehsm_transition_state(mParams->machine->hsm, onMainPage, params);
//        simplehsm_transition_state(mParams->machine->hsm, onSettingsPage, params);
        
//        

		break;

	case GOTO_PAGE_SIG:
 
	 	pageId = mEvt->asChars[0]; //page id

        myPrintf(" goto page %d; \n", pageId);
        
		switch(pageId)
		{
		case 0: //warning page

			break;
		case 1: //main page
			simplehsm_transition_state(mParams->machine->hsm, onMainPage, params);
			break;
		case 2: //settings page
			simplehsm_transition_state(mParams->machine->hsm, onSettingsPage, params);
			break;

		case 3: //switch edit page
			simplehsm_transition_state(mParams->machine->hsm, onConfigurePage, params);
			break;

		case 4:
			simplehsm_transition_state(mParams->machine->hsm, onSwitchOptionsPage, params);
			break;
            
        case 5: 
//			simplehsm_transition_state(mParams->machine->hsm, onConfigureTextPage, params);
//			break;
//            
//        case 6: 
			simplehsm_transition_state(mParams->machine->hsm, onInputsPage, params);
			break;

		default:
			simplehsm_transition_state(mParams->machine->hsm, onMainPage, params);
			break;
		}

		break;
        
	case TICK_SIG:
        {
            
            
    		sTagXY touch;
            static sTagXY touchLast;
    		sTrackTag track;
            sTouchRaw swipe;


    		caliButton = CALI_Read();

    		if(caliButton){
    			Calibrate();
    			myPrintf("read cali..\n");
    		}
            
            
            localElapsed = millis() - localLast;
            localLast = millis();
            
//            myPrintf("sys: %d %d \n", millis(), localElapsed);//mParams->machine->elapsedTime);
            

            if(globalNeedsSleepReset)
    		{
    			//Serial.println("globalNeedsSleepReset");
                globalNeedsSleep = false;
    			globalNeedsSleepReset = false;
                deepSleepTimer.currentTime = DEEP_SLEEP_MS;
                if(settings.sleepTimer > 0)
                    sleepTimer.currentTime = settings.sleepTimer;
    		}
            
    		if(timerDidFire(&aliveTimer, localElapsed))//mParams->machine->elapsedTime)) 
            {

    			//settings.aliveTimer = rand_interval(500,1500);
    			settings.aliveTimer = rand_interval(125,375);
    			aliveTimer.currentTime = settings.aliveTimer;
    			publish(SEND_ALIVE_CAN_PACKET,mParams);

    		}
            
            if(needsProInputsRequest)
            {
                needsProInputsRequest = false;
                
                if(isPro)
                {
                    machineEvent evt;

                    evt.asPtrs[0] = (int) &inputIsEnabled;
                	evt.asPtrs[1] = (int) &inputIsLockout;
                	evt.asPtrs[2] = (int) &inputIsInvert;
                    evt.asPtrs[3] = (int) &switchIsLinked;
                    
                	evt.asChars[31] = 255;

                	mParams->mEvt = &evt;

                	publish(SEND_PRO_CAN_PACKET, mParams);
                    
//                    CyDelay(5);
                }
            }

    		if(timerDidFire(&sleepTimer, localElapsed))// mParams->machine->elapsedTime))
    		{
                if(settings.sleepTimer > 0)
                {
    			    DisplaySleep(true);
                }
    		}
            
            //#if (DEEP_SLEEP_ENABLED != 0)
            
            if(!isDeepSleepDisabled && timerDidFire(&deepSleepTimer, localElapsed))
    		{
    			globalNeedsSleep = true;
    		}

    		//#endif

            
            static uint32_t touchTimer, touchTime;
            
//            static bool notSent = false;
//            static bool rawReleased = false;
            


    		FT_GC_GetTagXY(&touch);

            
    		FT_GC_GetTrackTag(&track);
            FT_GC_GetRawTouch(&swipe);
            
//            static bool isTouched = false;
//            static uint32_t touchHeldTime = 0;
            static uint32_t touchHeldStart = 0;
            
//            myPrintf("swipe.. %d\n", swipe.touch);
            
            if(swipe.touch == RAW_TOUCHED)
            {
//                myPrintf("|\n");
                if(touchHeldStart == 0)
                {
//                    isTouched = true;
                    touchHeldStart = millis() - 1;
//                    myPrintf("start touchHeldStart %d\n", touchHeldStart);
                }
            }
            else
            {
//                myPrintf("-\n");
                touchHeldStart = 0;
            }
            
            if(touchHeldStart > 0 && millis() - touchHeldStart > 30000)
            {
                touchHeldStart = 0;
                Calibrate();
    			myPrintf("read cali2..\n");
            }
            
            if(currentPage == onMainPage)
            {
                touchTime = 1000;
                
                if(swipe.touch == RAW_TOUCHED)
                {
                    sendEventWithObj(RAW_TOUCH_SIG, mParams, &swipe);
                    
                    wasRawTouched = true;
//                    rawReleased = false;
                    
                    
                    DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
        			if(settings.sleepTimer > 0)
                    {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }
                    
                    if (wasOff){
        				wasOff = false;
        				break;
        			}
                    
                    if(!wasRawTouched)
                    {
                        touchTimer = millis();
//                        myPrintf("start touchTimer\n");
                    }
                    
                    touchTime = millis() - touchTimer;
                }
                else if(wasRawTouched)
                {
                    wasRawTouched = false;
                    
                    sendEventWithObj(RAW_TOUCH_SIG, mParams, &swipe);
                }
                else
                {
//                    rawReleased = true;
                }
                
                    
                if (touch.tag) {

        			DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
        			if(settings.sleepTimer > 0)
                    {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }
                    

        			if (wasOff){
        				wasOff = false;
        				break;
        			}

        			if (!wasTouched) {

        				wasTouched = true;
//                        notSent = true;
                        touchLast = touch;
//            				sendEventWithObj(TOUCH_SIG, mParams, &touch);

        			} 
//                    else if(0){
//        				holdCount++;
//        				if (holdCount >= 15 && holdCount % 1 == 0) {
//        					sendEventWithObj(TOUCH_SIG, mParams, &touch);
//        				}
//        			}
                    
        		}
        		else if(wasTouched)// && rawReleased)
        		{
//                    rawReleased = false;
        			wasTouched = false;
//                    notSent = false;
        			holdCount = 0;
        			sendEventWithObj(TOUCH_SIG, mParams, &touch);
        			sendEventWithObj(TRACK_SIG, mParams, &track);

        		}
                
//                    if(wasTouched && notSent && touchTime > 0)
                if((touchLast.tag != 0) && touchTime > 200)
                {
//                    notSent = false;
                    touch = touchLast;
                    sendEventWithObj(TOUCH_SIG, mParams, &touch);
                    touchLast.tag = 0;
                }                    
                
//                }
                
                if(track.tag)
        		{
        			wasTouched = true;
        			holdCount = 0;
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        			//myPrintf("held down...");
        		}
                
            }
            else
            {
                
        		if (touch.tag) {

        			DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
                    if(settings.sleepTimer > 0)
                    {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }

        			if (wasOff){
        				wasOff = false;
        				break;
        			}

        			if (!wasTouched) {

        				wasTouched = true;
        				sendEventWithObj(TOUCH_SIG, mParams, &touch);

        			} else {
        				holdCount++;
        				if (holdCount >= 15 && holdCount % 1 == 0) {
        					sendEventWithObj(TOUCH_SIG, mParams, &touch);
        				}
        			}
        		}
                
        		else if(wasTouched)
        		{
        			wasTouched = false;
        			holdCount = 0;
        			sendEventWithObj(TOUCH_SIG, mParams, &touch);
        			sendEventWithObj(TRACK_SIG, mParams, &track);

        		}
                
                if(track.tag)
        		{
        			wasTouched = true;
        			holdCount = 0;
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        			//myPrintf("held down...");
        		}
            
            }
          
    		FT_GC_DLStart();

    		//publish(PRE_FRAME_TICK_SIG, params);

    		FT_GC_TagMask(0);
    		FT_GC_Cmd_FGColor(settings.fgColor);
    		FT_GC_Cmd_BGColor(settings.bgColor);
    		FT_GC_Cmd_GradColor(settings.gradColor);
    		FT_GC_ColorRGB_int(settings.color);

    		publish(FRAME_TICK_SIG, params);

    		//publish(POST_FRAME_TICK_SIG, params);

    		FT_GC_DLEnd();
    		FT_GC_Finish();

    		break;
        }

	}

	return stnone;
}

stnext mainInitial(int signal, void* params) {
	///static int tickCount = 0;

	simplehsm_t *hsm = ((machineParams *) params)->machine->hsm;

	switch (signal) {
	case SIG_INIT: {

		if (BootupConfigure()) {
			myPrintf("--Boot Config Error--");
		} else {
//			Calibrate();
			bool isValidConfig = false;

            
            if(IS_PRO_TESTING)
            {
                isPro = true;
            }
            
			loadConfig();

			unsigned int calculatedCrc = getConfigCrc();
			unsigned int loadedCrc = calibrate.crc;

//            myPrintf("Calc: %x , Loaded: %x \n", calculatedCrc, loadedCrc);
            
			if ((calibrate.initialized != 0) && (calculatedCrc == loadedCrc)){
				myPrintf("CRC is good, loading saved config...\n");
				isValidConfig = true;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_A, calibrate.regTouchTransformA) ;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_B, calibrate.regTouchTransformB) ;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_C, calibrate.regTouchTransformC) ;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_D, calibrate.regTouchTransformD) ;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_E, calibrate.regTouchTransformE) ;
				FT_Transport_SPI_Write32 (REG_TOUCH_TRANSFORM_F, calibrate.regTouchTransformF) ;
				loadSavedValues();
                
                FT_Transport_SPI_Write32(REG_PWM_DUTY, settings.dayBrightness);
                
                devInit = true;
			} else {
				myPrintf("CRC is bad...(%d) \n", calibrate.initialized);
			}

			unsigned char* bChar = (unsigned char*)&buttonLabels[0];
            
			for(unsigned int i = 0; i < sizeof(buttonLabels); i++)
			{
				char temp = *bChar;

//                if((i != 0) && ((i%10) == 0))
//                myPrintf(" ");
//                
//                myPrintf("%x", temp);
                
				if(i % 10 == 9 && temp != 0)
				{
					isValidConfig = false;
					myPrintf("Missing line termination...");
					break;
				}
				else if(temp != 0 && (temp < 32 || temp > 126))
				{
					isValidConfig = false;
					myPrintf("Unknown character...");
					break;
				}

				bChar++;
			}
            
			

			if(!isValidConfig)
			{
                FT_GC_Reset();
                BootupConfigure();
                
				Calibrate();
                myPrintf("Saving defaults...\n");
				loadDefaults();
				loadSavedValues();
			}

			subscribe(hsm, TICK_SIG);
			subscribe(hsm, FRAME_TICK_SIG);
            
            subscribe(hsm, CAN_TICK_SIG);
            
            
			subscribe(hsm, TOUCH_SIG);
            
            subscribe(hsm, RAW_TOUCH_SIG);
            
			subscribe(hsm, TRACK_SIG);
			subscribe(hsm, PRE_FRAME_TICK_SIG);
			subscribe(hsm, POST_FRAME_TICK_SIG);
			subscribe(hsm, SWITCH_STATUS_FROM_TS_SIG);
			subscribe(hsm, SWITCH_STATUS_FROM_CAN_SIG);
            subscribe(hsm, SWITCH_STATUS2_FROM_CAN_SIG);
            subscribe(hsm, SWITCH_STATUS_FROM_BLE_SIG);
			subscribe(hsm, GOTO_PAGE_SIG);
			subscribe(hsm, SEND_ALIVE_CAN_PACKET);
            subscribe(hsm, SEND_PRO_CAN_PACKET);
            
            subscribe(hsm, BLE_PAIR_SIG);
            subscribe(hsm, BLE_SEND_SIG);
            subscribe(hsm, BLE_SEND_OTA_SIG);
            subscribe(hsm, BLE_SEND_PRO_SIG);

			simplehsm_init_transition_state(hsm, mainIdle, params);
		}
	}
	}

	return stnone;
}

int machineInit(tMachineStruct *machine) {

	machine->hsm = &mainHsm;
	machine->tick = &machineTick;
	machine->isRunning = true;

	params.machine = machine;
	params.mEvt = NULL;

	simplehsm_initialize(machine->hsm, mainInitial, &params, genericLogger);

	return 0;
}

uint8_t canTickCount = 0;

//33ms tick
int machineTick(void *_machine) {

	tMachineStruct *machine = (tMachineStruct *) _machine;

	machineParams mParams;
	mParams.machine = machine;
	mParams.mEvt = NULL;

    publish(CAN_TICK_SIG, &mParams);
    canTickCount++;
    
    if(canTickCount % 8 == 0){
    	publish(TICK_SIG, &mParams);
    }
    
	return 0;
}

void setup() {

    myPrintf("\n -------- Begin -------- \n");
    myPrintf("Compiled: %s @ %s\n", __DATE__, __TIME__);

//    uint16_t appId;
    uint16_t appVer, boardId;
    uint32_t board;//, icPN;
    char boardRev;
    
//    appId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
    appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    boardId = (board & 0xFFFF0000) >> 16;
    boardRev = (board & 0x0000FFFF) + ('A' - 1);
    
    
    myPrintf("Boot App Board ID: %drev%c\n", boardId, boardRev);
    myPrintf("Boot App Version: %d.%d.%d\n", appVer / 0x100, appVer % 0x100 / 0x10, appVer % 0x10);
//    myPrintf("Boot App ID: %x\n", appId);
    
//    appId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
    appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
    board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    boardId = (board & 0xFFFF0000) >> 16;
    boardRev = (board & 0x0000FFFF) + ('A' - 1);
    
//    myPrintf("Boot Stack Board ID: %drev%c\n", boardId, boardRev);
    myPrintf("Boot Stack Version: %d.%d.%d\n\n", appVer / 0x100, appVer % 0x100 / 0x10, appVer % 0x10);
//    myPrintf("Boot Stack ID: %x\n", appId);
    
	///digitalWrite(36, LOW);
//	CyDelay(500);
	///digitalWrite(36, HIGH);

	loadSettings();
    
//	frameMillis = (1.0f / (float) settings.fps) * 1000.0;
    frameMillis = (float) (1000 / settings.fps);
    canMillis = (float) (1000 / settings.cps);
    

	tickTimer.currentTime = canMillis;
	tickTimer.rollover = canMillis;

//	frameTimer.currentTime = frameMillis;
//	frameTimer.rollover = frameMillis;

	myPrintf("\nStarting... \n");
	myPrintf("fps = %d\n", 1000/frameMillis);
    myPrintf("cps = %d\n", 1000/canMillis);

	lastTime = millis();
	machineLastTime = lastTime;

	machineInit(&myMachine);

	lastTime = millis();
	machineLastTime = lastTime;
}

bool needsSleep = false;

void loop() {

	currentTime = millis();
	elapsedTime = currentTime - lastTime;
	lastTime = currentTime;

    
	if (timerDidFire(&tickTimer, elapsedTime)) {

		machineElapsedTime = currentTime - machineLastTime;
		machineLastTime = currentTime;

		myMachine.time = currentTime;
		myMachine.elapsedTime = machineElapsedTime;
//		myMachine.tick(&myMachine);
        
        
        if(globalNeedsSleep)
		{
			SystemSleep();

		}else if(!globalIsSleep){

//			if(needsSleep)
//			{
//				needsSleep = false;
//				myPrintf("x");
//			}

		myMachine.tick(&myMachine);

		}
        else if(globalNeedsWake)
		{

			if(!isWaking){
				isWaking = true;
				myPrintf("isWaking...\n");
				SystemWake();
			}
		}
	}
}
