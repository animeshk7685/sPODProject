#include "touchscreen.h"
#include "main.h"
#include "timer.h"
#include "crc32.h"
#include "config.h"
#include <stdio.h>
#include <GD23Z.h>


static const char* TAG = "SPOD";

#define ROTATE_SCREEN 0

#define IS_PRO_TESTING         (0)      // set to 1 to default to pro
#define DEFAULT_SLEEP_TIMERS    (1)

#if (DEFAULT_SLEEP_TIMERS)

#define DEEP_SLEEP_MS (6*60*60*1000UL)    // 6 hrs
#define SLEEP_TIMER_MS (10*60*1000UL)     // 10 min

#else

#define DEEP_SLEEP_MS (2*60*1000UL)      // 2 min
#define SLEEP_TIMER_MS (60*1000UL)      // 1 min

#endif


static tMachineStruct myMachine;
static simple_hsm_t mainHsm = { NULL };
static machineParams params;

static unsigned long lastTime;

static unsigned long machineLastTime;
static unsigned long machineElapsedTime;

static tTimer tickTimer;

static bool wasOff = false;
switch_config_t saved_switch_config;
pcm_config_t saved_pcm_config[PCMS];
uint8_t current_switch = SWITCHES;

static void isr_touch_Enable() {} // TODO: necessary?

void InitWatchdog(uint16_t reset_interval)
{
	set_watchdog_timeout(reset_interval);
}


void resetWatchdog(void)
{
	pet_watchdog(NULL);
}

stlogger genericLogger(const uint8_t * logString)
{
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
    
	settings.isLockoutOn = false;
    myPrintf("loadSettings()\n");
}


unsigned int getConfigCrc(void) 
{
	unsigned char crc = 0;
	unsigned char *cByte = (uint8_t *) &calibrate;
	unsigned int cLength = sizeof(calibrate);

	for (unsigned int i = 0; i < cLength - 4; i++) {
		crc8(&crc, cByte[i]);
	}

	return (unsigned int) crc;
}


/* Api to bootup ft81x, verify FT81x hardware and configure display/audio pins */
/* Returns 0 in case of success and 1 in case of failure */
static int16_t BootupConfigure()
{
	LOGD(TAG, "%s() -- using GD!!\r\n", __FUNCTION__);
	GD.begin();
	GD.gt911();
	GC_SetDisplayEnablePin(7);
	GD.wr32(REG_PWM_DUTY, 0);
	GC_DisplayOn();
	if (ROTATE_SCREEN) GD.wr32(REG_ROTATE, ROTATE_SCREEN);
	CyDelay(500);
	GD.wr32(REG_PWM_DUTY, 128);
	return 0;
}

/* API for calibration on ft800 */
static void Calibrate()
{
    LOGD(TAG, "Calibrating...\r\n");
	/*************************************************************************/
	/* Below code demonstrates the usage of calibrate function. Calibrate    */
	/* function will wait untill user presses all the three dots. Only way to*/
	/* come out of this api is to reset the coprocessor bit.                 */
	/*************************************************************************/

    sTouchRaw check;
    GC_GetRawTouch(&check);
   
	LOGD(TAG, "check.touch==RAW_TOUCHED? (%d)\r\n", check.touch);
    if (check.touch == RAW_TOUCHED) {
        GC_DLStart();
    
        GC_ClearColorRGB(64, 64, 64);
    	GC_ClearCST(1, 1, 1);
        GC_ColorRGB(0xff, 0xff, 0xff);
		LOGD(TAG, "GC_Cmd_test(x=%d, y=%d)\r\n", FT_DISPLAYWIDTH/2, FT_DISPLAYHEIGHT/2);  
    	GC_Cmd_Text((FT_DISPLAYWIDTH / 2), (FT_DISPLAYHEIGHT / 2), 27, FT_OPT_CENTER, "Please release the screen");
        
        GC_DLEnd();
		LOGD(TAG, "Entering check.touch==RAW_TOUCHED loop...\r\n");
        while (check.touch == RAW_TOUCHED) {
            CyDelay(25);
            GC_GetRawTouch(&check);
        }
        
        CyDelay(250);
    }
    
	/* Construct the display list with grey as background color, informative string "Please Tap on the dot" followed by inbuilt calibration command */
	GC_DLStart();
    
    GC_ClearColorRGB(64, 64, 64);
	GC_ClearCST(1, 1, 1);
    GC_ColorRGB(0xff, 0xff, 0xff);    
	GC_Cmd_Text((FT_DISPLAYWIDTH / 2), (FT_DISPLAYHEIGHT / 2), 27, FT_OPT_CENTER, "Please Tap on the dot");
    GC_Cmd_Calibrate(0);

	/* Wait for the completion of calibration - either finish can be used for flush and check can be used */
	GD.finish();
    LOGD(TAG, "done calibration\r\n");

	calibrate.regTouchTransformA = GD.rd32 (REG_TOUCH_TRANSFORM_A);
	calibrate.regTouchTransformB = GD.rd32 (REG_TOUCH_TRANSFORM_B);
	calibrate.regTouchTransformC = GD.rd32 (REG_TOUCH_TRANSFORM_C);
	calibrate.regTouchTransformD = GD.rd32 (REG_TOUCH_TRANSFORM_D);
	calibrate.regTouchTransformE = GD.rd32 (REG_TOUCH_TRANSFORM_E);
	calibrate.regTouchTransformF = GD.rd32 (REG_TOUCH_TRANSFORM_F);
    calibrate.initialized = 1;
    
	saveCalibration();
}

static void sendEventWithObj(int event, machineParams *mParams, void *obj)
{
	machineEvent evt;
	evt.asInts[0] = (int) obj;
	mParams->mEvt = &evt;
	publish(event, mParams);
}


static bool globalIsSleep = false;
static bool globalNeedsSleep = false;
static bool isWaking = false;


static void SystemSleep()
{
	globalNeedsSleep = false;
#if 0 // TODO: SLEEP
    CyDelay(25);
    
    GD.wr32(REG_PWM_DUTY, 0);
    GC_DisplayOff();
    
    CyDelay(100);
    
    GC_HostCommand(FT_STANDBY );
    
    isr_touch_Enable();
    
    globalNeedsWake = false;
    
    mcpCanSleep(true);

    globalIsSleep = true;
  
    CyDelay(100);
   
    CyDelay(100);
   
    CyBle_EnterLPM(CYBLE_BLESS_HIBERNATE);
    CyBle_Stop();
    
    UART_Stop();
    
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    CyIntDisable((SysTick_IRQn + 16));
    
    while (true) CySysPmDeepSleep();
#endif
}


static void SystemWake()
{
#if 0 // TODO: WAKEUP
    isr_touch_Disable();

    mcpCanSleep(false);
    GC_HostCommand(FT_ACTIVE);

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
#endif
}


void DisplaySleep(bool isSleep)
{
	if (isSleep){
		GD.wr32(REG_PWM_DUTY, 0);
		GC_DisplayOff();
		wasOff = true;

        myPrintf("display sleep... %d\n", millis());
	} else {
		GC_DisplayOn();
		if (wasOff) CyDelay(250);
		GD.wr32(REG_PWM_DUTY, settings.dayBrightness);
		if (wasOff) CyDelay(500);
	}
}


stnext mainIdle(int signal, void* params)
{
	static bool wasTouched = false;
    static bool wasRawTouched = false;
	static int holdCount = 0;

	static tTimer sleepTimer;
    static tTimer deepSleepTimer;
	static tTimer aliveTimer;

LOGV(TAG, "%s(signal=%d, params=%p)\r\n", __FUNCTION__, signal, params);
	if (params == NULL) return stnone;

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
	char pageId;

    static unsigned long localLast = 0;
    static unsigned long localElapsed = 0;

	switch (signal) {
	case SIG_INIT:
        LOGV(TAG, "Main Idle Init \r\n");
        
		settings.aliveTimer = rand_interval(500,1500);

		sleepTimer.currentTime = settings.sleepTimer;
		sleepTimer.rollover = settings.sleepTimer;
        
        deepSleepTimer.currentTime = DEEP_SLEEP_MS;
		deepSleepTimer.rollover = DEEP_SLEEP_MS;

		aliveTimer.currentTime = settings.aliveTimer;
		aliveTimer.rollover = settings.aliveTimer;

		//first page -- eventually needs to goto warning page...
		simple_hsm_transition_state(mParams->machine->hsm, onHomePage, params);
		break;

	case GOTO_PAGE_SIG:
	 	pageId = mEvt->asChars[0]; //page id

        LOGD(TAG, "%s(): GOTO PAGE %d, asChars[1]=%d \r\n", __FUNCTION__, pageId, mEvt->asChars[1]);
        
		switch(pageId) {
		case WARNING_PAGE:         break; //warning page
		case HOME_PAGE:            simple_hsm_transition_state(mParams->machine->hsm, onHomePage, params); break;
		case CONFIG_SETUP_PAGE:    simple_hsm_transition_state(mParams->machine->hsm, onConfigSetupPage, params); break;
		case SWITCH_SETUP_PAGE:    simple_hsm_transition_state(mParams->machine->hsm, onSwitchSetupPage, params); break;
		case SETTINGS_PAGE:        simple_hsm_transition_state(mParams->machine->hsm, onSettingsPage, params); break;
		case ADVANCED_PAGE:        simple_hsm_transition_state(mParams->machine->hsm, onOutputSetupPage, params); break;
		case TEST_PAGE:            simple_hsm_transition_state(mParams->machine->hsm, onTestPage, params); break;
		case FACTORY_RESET_PAGE:   simple_hsm_transition_state(mParams->machine->hsm, onFactoryResetPage, params); break;
		case TRIGGERS_SELECT_PAGE: simple_hsm_transition_state(mParams->machine->hsm, onAutoTriggerSelectPage, params); break;
		case TRIGGERS_OUTPUT_PAGE: simple_hsm_transition_state(mParams->machine->hsm, onAutoTriggerOutputPage, params); break;
		case CONFIG_TRIGGERS_PAGE: simple_hsm_transition_state(mParams->machine->hsm, onConfigTriggersPage, params); break;
		default:                   simple_hsm_transition_state(mParams->machine->hsm, onHomePage, params); break;
		}
		break;
        
	case TICK_SIG:
        {
    		sTagXY touch;
            static sTagXY touchLast;
    		sTrackTag track;
            sTouchRaw swipe;

            localElapsed = millis() - localLast;
            localLast = millis();

            if (globalNeedsSleepReset) {
                globalNeedsSleep = false;
    			globalNeedsSleepReset = false;
                deepSleepTimer.currentTime = DEEP_SLEEP_MS;
                if (settings.sleepTimer > 0)
                    sleepTimer.currentTime = settings.sleepTimer;
    		}
            
    		if (timerDidFire(&aliveTimer, localElapsed)) {
    			settings.aliveTimer = 5000/*rand_interval(125,375)*/;
    			aliveTimer.currentTime = settings.aliveTimer;
    			publish(SEND_ALIVE_CAN_PACKET,mParams);
    		}
            
    		if (timerDidFire(&sleepTimer, localElapsed)) {// mParams->machine->elapsedTime))
                if (settings.sleepTimer > 0) {
    			    DisplaySleep(true);
                }
    		}
            
            if (!isDeepSleepDisabled && timerDidFire(&deepSleepTimer, localElapsed)) {
    			globalNeedsSleep = true;
    		}
            
            static uint32_t touchTimer, touchTime;

    		GC_GetTagXY(&touch);
    		GC_GetTrackTag(&track);
            GC_GetRawTouch(&swipe);
            
            static uint32_t touchHeldStart = 0;
            
            if (swipe.touch == RAW_TOUCHED) {
                if (touchHeldStart == 0) {
                    touchHeldStart = millis() - 1;
					//LOGD(TAG, "%s(): touchHeldStart=%d\r\n", __FUNCTION__, touchHeldStart);
                }
            } else {
                touchHeldStart = 0;
            }
            
            if (touchHeldStart > 0 && millis() - touchHeldStart > 20000) {
				LOGD(TAG, "%s(): touch held for 20 seconds, calibrating!!!\r\n", __FUNCTION__);
                touchHeldStart = 0;
                Calibrate();
            }
 
            if (currentPage == onHomePage) {
                touchTime = 1000;
                
                if (swipe.touch == RAW_TOUCHED) {
                    sendEventWithObj(RAW_TOUCH_SIG, mParams, &swipe);
                    
                    wasRawTouched = true;
                    DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
        			if (settings.sleepTimer > 0) {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }
                    
                    if (wasOff) {
        				wasOff = false;
        				break;
        			}
                    
                    if (!wasRawTouched) {
                        touchTimer = millis();
                    }
                    
                    touchTime = millis() - touchTimer;
                } else 
				if (wasRawTouched) {
                    wasRawTouched = false;
                    sendEventWithObj(RAW_TOUCH_SIG, mParams, &swipe);
                } else {
                }
                    
                if (touch.tag) {
        			DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
        			if (settings.sleepTimer > 0) {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }

        			if (wasOff) {
        				wasOff = false;
        				break;
        			}

					#if 0
					// TODO: GRH commented this -- see #else comments...
        			if (!wasTouched) {
        				wasTouched = true;
                        touchLast = touch;
        			} 
					#else
					// TODO: GRH added this to make LONG_PRESS work on main page 6/7/25
					if (!wasTouched) {
        				wasTouched = true;
                        touchLast = touch;
        				sendEventWithObj(TOUCH_SIG, mParams, &touch);
        			} else {
        				holdCount++;
        				if (holdCount >= 15 && holdCount % 1 == 0) {
        					sendEventWithObj(TOUCH_SIG, mParams, &touch);
        				}
        			}
					#endif
        		} else 
				if (wasTouched) { // && rawReleased)
        			wasTouched = false;
        			holdCount = 0;
        			sendEventWithObj(TOUCH_SIG, mParams, &touch);
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        		}
                
                if ((touchLast.tag != 0) && touchTime > 200) {
                    touch = touchLast;
                    sendEventWithObj(TOUCH_SIG, mParams, &touch);
                    touchLast.tag = 0;
                }                    
                
                if (track.tag) {
        			wasTouched = true;
        			holdCount = 0;
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        		}
            } else { /* not main page */
        		if (touch.tag) {
        			DisplaySleep(false);
                    globalNeedsSleepReset = true;
                    
                    if (settings.sleepTimer > 0) {
            			sleepTimer.currentTime = settings.sleepTimer;
                        sleepTimer.rollover = settings.sleepTimer;
                    }

        			if (wasOff) {
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
        		} else 
				if (wasTouched) {
        			wasTouched = false;
        			holdCount = 0;
        			sendEventWithObj(TOUCH_SIG, mParams, &touch);
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        		}
                
                if (track.tag) {
        			wasTouched = true;
        			holdCount = 0;
        			sendEventWithObj(TRACK_SIG, mParams, &track);
        		}
            }
          
    		GC_DLStart();
    		GC_TagMask(0);
    		GC_Cmd_FGColor(settings.fgColor);
    		GC_Cmd_BGColor(settings.bgColor);
    		GC_Cmd_GradColor(settings.gradColor);
    		GC_ColorRGB_int(settings.color);

    		publish(FRAME_TICK_SIG, params);

    		GC_DLEnd();
    		GD.finish();
    		break;
        }
	}

	return stnone;
}

stnext mainInitial(int signal, void* params)
{
	simple_hsm_t *hsm = ((machineParams *) params)->machine->hsm;

	switch (signal) {
	case SIG_INIT: {
		if (BootupConfigure()) {
			LOGE(TAG, "--Boot Config Error--\r\n");
		} else {
			// Calibrate();
			bool isValidConfig = false;

			loadCalibration();

			unsigned int calculatedCrc = getConfigCrc();
			unsigned int loadedCrc = calibrate.crc;

			if ((calibrate.initialized != 0) && (calculatedCrc == loadedCrc)){
				LOGD(TAG, "CRC is good, loading saved config...\r\n");
				isValidConfig = true;
				GD.wr32 (REG_TOUCH_TRANSFORM_A, calibrate.regTouchTransformA) ;
				GD.wr32 (REG_TOUCH_TRANSFORM_B, calibrate.regTouchTransformB) ;
				GD.wr32 (REG_TOUCH_TRANSFORM_C, calibrate.regTouchTransformC) ;
				GD.wr32 (REG_TOUCH_TRANSFORM_D, calibrate.regTouchTransformD) ;
				GD.wr32 (REG_TOUCH_TRANSFORM_E, calibrate.regTouchTransformE) ;
				GD.wr32 (REG_TOUCH_TRANSFORM_F, calibrate.regTouchTransformF) ;
				loadSavedValues();
                
                GD.wr32(REG_PWM_DUTY, settings.dayBrightness);
			} else {
				LOGE(TAG, "CRC is bad...(%d) \r\n", calibrate.initialized);
			}

			unsigned char* bChar = (unsigned char*)&settings.buttonLabels[0];
			for(unsigned int i = 0; i < sizeof(settings.buttonLabels); i++)
			{
				char temp = *bChar;

				if(i % 10 == 9 && temp != 0)
				{
					isValidConfig = false;
					LOGE(TAG, "Missing line termination...\r\n");
					break;
				}
				else if(temp != 0 && (temp < 32 || temp > 126))
				{
					isValidConfig = false;
					LOGE(TAG, "Unknown character...\r\n");
					break;
				}

				bChar++;
			}

			if(!isValidConfig)
			{
                GC_Reset();
                BootupConfigure();
                
				Calibrate();
                LOGD(TAG, "Saving defaults...\r\n");
				loadDefaults();
				//loadSavedValues();
			}

			subscribe(hsm, TICK_SIG);
			subscribe(hsm, FRAME_TICK_SIG);
            
            subscribe(hsm, CAN_TICK_SIG);
            
			subscribe(hsm, TOUCH_SIG);
            
            subscribe(hsm, RAW_TOUCH_SIG);
            
			subscribe(hsm, TRACK_SIG);
			subscribe(hsm, PRE_FRAME_TICK_SIG);
			subscribe(hsm, POST_FRAME_TICK_SIG);
            subscribe(hsm, SWITCH_STATUS2_FROM_CAN_SIG);
			subscribe(hsm, GOTO_PAGE_SIG);
			subscribe(hsm, SEND_ALIVE_CAN_PACKET);
            
			simple_hsm_init_transition_state(hsm, mainIdle, params);
		}
	}
	}

	return stnone;
}


//33ms tick
static int machineTick(void *_machine)
{
	static uint8_t canTickCount = 0;

	tMachineStruct *machine = (tMachineStruct *) _machine;

	machineParams mParams;
	mParams.machine = machine;
	mParams.mEvt = NULL;

    publish(CAN_TICK_SIG, &mParams);
    canTickCount++;

    if (canTickCount % 8 == 0) publish(TICK_SIG, &mParams);
	return 0;
}


static int machineInit(tMachineStruct *machine) {

	machine->hsm = &mainHsm;
	machine->tick = &machineTick;
	machine->isRunning = true;

	params.machine = machine;
	params.mEvt = NULL;

	simple_hsm_initialize(machine->hsm, mainInitial, &params, genericLogger);

	return 0;
}


void main_setup()
{
    LOGD(TAG, "\r\n -------- Begin -------- \r\n");
    LOGD(TAG, "Compiled: %s @ %s\r\n", __DATE__, __TIME__);
    LOGD(TAG, "VERSION: %s\r\n\r\n", VERSION);
    
	loadSettings();
    unsigned long canMillis = (float) (1000 / settings.cps);

	tickTimer.currentTime = canMillis;
	tickTimer.rollover = canMillis;

	LOGD(TAG, "\r\nStarting fps = %d, cps = %d\r\n", 1000/canMillis);

	lastTime = millis();
	machineLastTime = lastTime;

	machineInit(&myMachine);

	lastTime = millis();
	machineLastTime = lastTime;

	// CONFIGS/SWITCHES is used for auto-trigger, initialize the
	// saved_switch_config to bad values so it will be initialized with
	// the correct switch_config[][] in setup switch page
	saved_switch_config.config_index = CONFIGS+PCMS+1;
	saved_switch_config.switch_index = SWITCHES+1;
}


void main_loop() 
{
	unsigned long currentTime  = millis();
	unsigned long elapsedTime = currentTime - lastTime;
	lastTime = currentTime;
    
	if (timerDidFire(&tickTimer, elapsedTime)) {
		machineElapsedTime = currentTime - machineLastTime;
		machineLastTime = currentTime;
		myMachine.time = currentTime;
		myMachine.elapsedTime = machineElapsedTime;
        
        if (globalNeedsSleep) {
			SystemSleep();
		} else
		if (!globalIsSleep) {
			myMachine.tick(&myMachine);
		} else
		if (globalNeedsWake) {
			if (!isWaking) {
				isWaking = true;
				LOGD(TAG, "isWaking...\r\n");
				SystemWake();
			}
		}
	}
}