/*
 * mainPage.h
 *
 *  Created on: Sep 26, 2014
 *      Author: Jason
 */

#include "common.h"
#include "sPodImageData.h"
#include "pubSub.h"

/*
 #define SPOD_LOGO 0
 #define ADDR1 SPOD_RAWDATA_LENGTH
 #define ADDR2 SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH
 #define ADDR3 SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH + SPOD_ADDRS_2_LENGTH
 #define ADDR4 SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH + SPOD_ADDRS_2_LENGTH + SPOD_ADDRS_3_LENGTH
 */



stnext onMainPage(int signal, void* params) {
	static int activityReset = 40;
	static int activity = 40;
//	static uint8_t i = 0;
	static uint8_t canIndex = 0;
//	static int battCount = 61;

    static uint8_t setupHeld = 0;
	static uint8_t sleepHeld = 0;
	static uint8_t tempHeld = 0;
//	static uint8_t avgTemp = 0;
    
    static uint32_t sPodLogoBMPSize;
    static uint32_t iconBMPSize;
    
    static uint32_t currentTimeout = 0;
    
    static uint32_t lastPageInitTime = 0;
    
    sPodLogoBMPSize = sPod_Bitmap_Header->Stride * sPod_Bitmap_Header->Height;
    
    iconBMPSize = button_Bitmap_Header->Stride * button_Bitmap_Header->Height;
    
    iconGpuBaseAddr = sPodLogoBMPSize;
    
//    static const char blank10[10] = {0};
    
//	static bool switchStates = false;
    
//	static bool flashStatus[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//	static bool flashEnable[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//	static bool strobeStatus[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//	static bool strobeEnable[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    
//    static CY_NOINIT bool flashStatus[32];          // to retain across hibernate/deep sleep
//	static CY_NOINIT bool flashEnable[32];
//	static CY_NOINIT bool strobeStatus[32];
//	static CY_NOINIT bool strobeEnable[32];
    
    static bool mainPgInit = true;
    
    if(mainPgInit && (!wakeFromHib))// || devInit))
    {
        mainPgInit = false;
        
//        for(int i = 0; i < 32 ; i++)
//        {
////            flashStatus[i] = 0;
//            flashEnable[i] = 0;
////            strobeStatus[i] = 0;
//            strobeEnable[i] = 0;
//        }
        myPrintf("woke from rst\n");
    }
    else if(wakeFromHib)
    {
        mainPgInit = false;
        myPrintf("woke from ds\n");
        setWakeFromHib(false);  // set flag in flash
    }


    
//	static bool fahOn = true;

	static sTagXY touch;
	static sTrackTag track;
	static uint16_t sliderValue = 65535 / 2;

//	static uint8_t oneHotVals[8] = { SW0, SW1, SW2, SW3, SW4, SW5, SW6, SW7 };

	char pageId;

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;

	//FT800IMPL_SPI *FTImpl = (FT800IMPL_SPI *) mParams->machine->obj;
    
    static bool logoInit = 1;
    static bool needsProLogoInit = 1;
    
    static bool ignoreTouch = false;

//    myPrintf(".");
    
	switch (signal) {
	case SIG_INIT: {
		activityReset = ((float)settings.fps * 2.0f);
		activity = activityReset;
		touch.tag = 0;
		touch.x = 0;
		touch.y = 0;

        
        
//		switchStates = false;

        logoInit = 1;
        needsProLogoInit = 1;
        
        if(currentPage == onMainPage)
        {
            logoInit = 0;
            needsProLogoInit = 0;
        }
        
		currentPage = onMainPage;
        
        
        if(logoInit)
        {
            logoInit = 0;
        
    		FT_GC_Cmd_Memset(0, 0L,
    				1L * sPodLogoBMPSize + (iconBMPSize * 8));
            
            FT_GC_Cmd_Inflate(0);
            
            FT_GC_WriteCmdfromflash(sPod_Bitmap_RawData,
				1L * SPOD_RAWDATA_LENGTH);  
            
            if(isPro)
            {
                needsProLogoInit = 0;
                
                FT_GC_Cmd_Inflate(0);
                
                FT_GC_WriteCmdfromflash(sPod_Pro_Bitmap_RawData,
    				1L * SPOD_PRO_RAWDATA_LENGTH);
            }
        }
        
        for(int i = 0 ; i < 8 ; i++)
        { 
            if(iconIsOn[(8 * sourceAdrMask) + i] != 0)
            {
                FT_GC_Cmd_Memwrite((iconGpuBaseAddr + (i * iconBMPSize)), 1L * iconBMPSize);
                
        		FT_GC_WriteCmdfromflash(button_Bitmap_RawData + 
                    ((8 * sourceAdrMask) + i) * BUTTON_FLASH_RAWDATA_LENGTH,
        				1L * iconBMPSize);  
            }
        }
        
		FT_GC_Cmd_Track(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, SLIDER);
		FT_GC_Finish();

        
        simplehsm_transition_state(mParams->machine->hsm, canComm, params);
        
//        simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
		
        lastPageInitTime = millis();

        myPrintf("main page init\n");
        
		break;
	}
	case FRAME_TICK_SIG: {
        
//                myPrintf("frm: %d\n", millis());
        
        if((millis() - currentTimeout) > 2000)
        {
            currentTimeout = 0;
        }
        
		if(activity > 0)
			activity--;

//		int32_t tagoption;
		int16_t switchX = SWITCH_X;
		int16_t switchY = SWITCH_Y;
        
		FT_GC_ClearColorRGB_int(settings.pageBgColor);
		FT_GC_ClearCST(1, 1, 1);
        
		FT_GC_TagMask(1);
        
		drawImage(sPod_Bitmap_Header, 5, 5, 0.4, 0.4, touch,
				SPOD_LOGO_TOUCH, 0);
        
        if(isPro)
        {
            if(needsProLogoInit)
            {
                needsProLogoInit = 0;
                
                FT_GC_Cmd_Inflate(0);
                
                FT_GC_WriteCmdfromflash(sPod_Pro_Bitmap_RawData,
    				1L * SPOD_PRO_RAWDATA_LENGTH);
            }
            
            FT_GC_TagMask(1);
            
            drawImage(sPod_Pro_Bitmap_Header, 5, 5, 0.4, 0.4, touch,
				SPOD_LOGO_TOUCH, 0);
        }
        
        

        float buttonIconScale = 0.8;
        float buttonIconScaleText = 0.55;
        
        
		int bIndex = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 4; j++) {
				switchX = SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * j);
				switchY = SWITCH_Y + ((SWITCH_H + SWITCH_SPACE) * i);

				uint8_t switchID = SWITCH_0 + (i * 4) + j;
				uint8_t switchIndex = switchID - 1;

				bIndex = switchIndex + (sourceAdrMask * 8);

				if (!switchStatus[bIndex]) {    // switch off

					FT_GC_Begin(FT_RECTS);
					FT_GC_Tag(0);
					FT_GC_ColorRGB_int(settings.highlightColor);
					FT_GC_LineWidth(5 * 16);
					FT_GC_Vertex2ii(switchX + 6, switchY + 6, 0, 0);
					FT_GC_Vertex2ii(switchX + SWITCH_W - 3,
							switchY + SWITCH_H - 3, 0, 0);
					FT_GC_End();
                    
                    
                    if(iconIsOn[bIndex] != 0)    // switch off, icon on
                    {
                        drawButtonSmall( switchX, switchY, SWITCH_W, SWITCH_H, 26, 0, 0, 0, 
                            touch, switchID,
							switchStatus[bIndex], 
                            (currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );

                        if(buttonLabels[bIndex].line1[0] == 0)
                        {
                            drawImage(button_Bitmap_Header, switchX+5, switchY+3, buttonIconScale, buttonIconScale, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                        }
                        else
                        {
                            drawImage(button_Bitmap_Header, switchX+14, switchY+2, buttonIconScaleText, buttonIconScaleText, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                            
                            drawTriText(switchX, switchY + 34, SWITCH_W, 20, 26, 
                                buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
                        }
                                  
                    }else{     // switch off, icon off
                        
					    drawButtonSmall( switchX, switchY, SWITCH_W, SWITCH_H, 26, 
                            buttonLabels[bIndex].line1,
							buttonLabels[bIndex].line2,
							buttonLabels[bIndex].line3, 
                            touch, switchID,
							switchStatus[bIndex], 
                            (currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
                    }
                    
				} else {        // switch on
					FT_GC_Begin(FT_RECTS);
					FT_GC_Tag(0);
					FT_GC_ColorRGB_int(settings.fgColor);
					FT_GC_LineWidth(5 * 16);
					FT_GC_Vertex2ii(switchX + 5, switchY + 5, 0, 0);
					FT_GC_Vertex2ii(switchX + SWITCH_W - 2,
							switchY + SWITCH_H - 2, 0, 0);
					FT_GC_End();
                    
                    
                    if(iconIsOn[bIndex])    // switch on, icon on
                    {
                        drawButtonSmall( switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, 26, 0, 0, 0, 
                            touch, switchID,
							switchStatus[bIndex], 
                            (currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
                        
                        if(buttonLabels[bIndex].line1[0] == 0)
                        {
                            FT_GC_ColorRGB_int(0x000000);
                        
                            drawImage(button_Bitmap_Header, switchX+5, switchY+3, buttonIconScale, buttonIconScale, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                        
                            FT_GC_ColorRGB_int(settings.color);
                            
                            drawImage(button_Bitmap_Header, switchX+6, switchY+4, buttonIconScale, buttonIconScale, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                        }
                        else
                        {
                            FT_GC_ColorRGB_int(0x000000);
                        
                            drawImage(button_Bitmap_Header, switchX+14, switchY+2, buttonIconScaleText, buttonIconScaleText, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                            
                            FT_GC_ColorRGB_int(settings.color);
                            
                            drawImage(button_Bitmap_Header, switchX+15, switchY+3, buttonIconScaleText, buttonIconScaleText, touch,
                            TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
                            
                            drawTriText(switchX+1, switchY+35, SWITCH_W, 20, 26, 
                                buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
                        }
                                  
                    }else{     // switch on, icon off
    					drawButtonSmall( switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, 26, 
                            buttonLabels[bIndex].line1,
							buttonLabels[bIndex].line2,
							buttonLabels[bIndex].line3, 
                            touch, switchID,
							switchStatus[bIndex], 
                            (currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
                    }
				}
			}
		}

		bIndex = currentSwitchIndex + (sourceAdrMask * 8);

		if (currentSwitchIndex < 255) {

			if (switchIsDimmable[bIndex] && switchStatus[bIndex]) {
				drawSlider( SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H,
						sliderValue, track, SLIDER);
				drawPercentGauge( 205, 12, "", 29,
						normalizeSlider(sliderValue));
			}

			if (!isOnRoad) {
//                if (strobeStatus[bIndex]) {
				if (switchIsStrobe[bIndex]) {
					drawButton(
					SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 3), 200,
					SWITCH_W, 35, 26, "Strobe", "", "", touch, MSTROBE,
							switchStrobeSet[bIndex]);
				}

//				if (flashStatus[bIndex]) {
                if (switchIsFlash[bIndex]) {    
					drawButton(
					SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 3), 200,
					SWITCH_W, 35, 26, "Flash", "", "", touch, MFLASH,
							switchFlashSet[bIndex]);
				}
			}
		}

        if(isLockoutOn)
        {
		    drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Locked", "", "",
				touch, SETUP, false);
        }
        else
        {
            drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Setup", "", "",
				touch, SETUP, false);
        }
//        drawInvisibleButton( 0,0,//SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 0), 7,
//    		61, 42, 26, "", "", "", touch, SPOD_LOGO_TOUCH, false);
        
        
        
		drawButton( SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 1), 7,
		SWITCH_W, 35, 26, "Off", "", "", touch, SLEEP, false);

		if (isOnRoad)
        {
			drawButtonColor( SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 2),
					7, SWITCH_W, 35, 26, "On", "Road", "", touch, ON_OFF_ROAD,
					!isOnRoad, 0x0000EE, 0x009900);
        }
		else
        {
			drawButtonColor( SWITCH_X + ((SWITCH_W + SWITCH_SPACE) * 2),
					7, SWITCH_W, 35, 26, "Off", "Road", "", touch, ON_OFF_ROAD,
					isOnRoad, 0x009900, 0x0000EE);
        }

        FT_GC_Tag(0);
		drawSourceNum( 58, 31, "", 26, (sourceAdrMask + 1));   
            
		FT_GC_Tag(0);
		drawTriText( 160, 160, 0, 55, 20, "Battery", "", "", false, 0,
				settings.greyColor, true);
        
        double voltScale;
        
        if(is24v_not12){
            voltScale = voltage * 2.0;
        } else {
            voltScale = voltage;
        }
        
		FT_GC_Tag(0);
		drawBatteryVoltage( 160, 226, "", 20, voltScale);

		FT_GC_Tag(0);
		drawTriText( 95, 160, 0, 55, 20, "Temp", "", "", false, 0,
				settings.greyColor, true);
        
		FT_GC_Tag(0);
		drawTempVal( 95, 226, "", 20, temperature, fahOn);

		//drawImage( sPod_Bitmap_Header, 0, 0, 0.5, 0.5, touch, SPOD_LOGO_TOUCH, 0);
		FT_GC_Tag(0);
		drewBatteryMeter( voltage);
		//FT_GC_Tag(TEMP_SEL);
		drawTemperatureMeter( temperature, fahOn);
        
        FT_GC_ColorRGB(196,196,196);
        FT_GC_Begin(FT_LINE_STRIP);
			FT_GC_LineWidth(1 *16);
			FT_GC_Vertex2f(5*16,185*16);
			FT_GC_Vertex2f(270*16,185*16);
			FT_GC_End();

        FT_GC_LineWidth(4);
//                FT_GC_ColorMask(0, 0, 0, 0);
//		FT_GC_ColorARGB(0);

//        FT_GC_Begin(FT_RECTS);
//		FT_GC_Tag(SPOD_LOGO_TOUCH);
//        FT_GC_ColorMask(0, 0, 0, 0);
//        FT_GC_Vertex2f(0,0);
//        FT_GC_Vertex2f(62*16, 42*16);
        
		// Invisible Temp button
		FT_GC_Begin(FT_POINTS);
		FT_GC_Tag(TEMP_SEL);
		FT_GC_PointSize(300);
		FT_GC_ColorMask(0, 0, 0, 0);
		FT_GC_Vertex2ii(105, 215, 0, 0);
      

        
		break;

	}
    case RAW_TOUCH_SIG: {
        
        ignoreTouch = false;
        
        if(isLockoutOn)
            break;
        
        if(!isSwipeOn)
            break;
        
                sTouchRaw raw;
        static bool touched, updated;
        bool refresh = 0;
        static uint16_t xStart, xLast;
//        static uint32_t touchTime, touchTimer;
        
//        FT_GC_GetRawTouch(&raw);
        
        raw = *((sTouchRaw *) mEvt->asInts[0]);
        
//        touchTime = 1000;
        
        if(raw.touch != RAW_TOUCHED)
        {
            if(touched)
            {
                touched = 0;
                xLast = 0;
                xStart = 0;
                
            }
            
            
        }
        else
        {
            if(!touched)
            {
                touched = 1;
                
//                myPrintf("Touch start: x %d, y %d\n", raw.x, raw.y);
                xStart = raw.x;

//                touchTimer = millis();
                
                updated = 0;
            }
            
            xLast = raw.x;
            
//            touchTime = millis() - touchTimer;
            
            
            if(!updated && abs(xLast - xStart) > 150)
                {
                    updated = 1;
                    
                    if(xLast < xStart) {
                        if(sourceAdrMask != 0) {
                            sourceAdrMask--;
                            refresh = 1;
                        }
                    } else {
                        if(sourceAdrMask < 3) {
                            sourceAdrMask++;
                            refresh = 1;
                        }
                    }

                    if(refresh)
                    {
                        refresh = 0;
                    
                        updateSourceAddress();
                        
                        pageId = 1;

        				machineEvent evt;
        				evt.asChars[0] = pageId;
        				mParams->mEvt = &evt;
        				publish(GOTO_PAGE_SIG, mParams);
                    
                    
                    }
                }
        }
        
        
        
        if(abs(xLast - xStart) > 25)
        {
            ignoreTouch = true;
        }
        
//        if(touchTime < 1)
//        {
//            ignoreTouch = true;
//        }
        
        break;
    }
	case TOUCH_SIG: {

//        static sTagXY touchTemp;
//            touchTemp = *((sTagXY *) mEvt->asInts[0]);
//            
//            touch.tag = 0;
//        }
//        else
//        {
//        
//        if(touchTemp.tag == 0)
//        {
//            touchTemp = *((sTagXY *) mEvt->asInts[0]);
//        }
//            
//        touch = touchTemp;
//        
//        touchTemp.tag = 0;
        
        touch = *((sTagXY *) mEvt->asInts[0]);
        
        
        if(isLockoutOn)
        {
            if(touch.tag == SETUP) {
                setupHeld = 0;
            }
            
            touch.tag = 0;
            
            break;
        }

        if(ignoreTouch)        // from swipe
            break;
//        {

		if (touch.tag <= SWITCH_7 && touch.tag > MAIN_PAGE_NONE) {

			uint8_t buttonIndex = touch.tag - 1;

			uint8_t switchIndex = buttonIndex + (sourceAdrMask * 8);

			switchStatus[switchIndex] = !switchStatus[switchIndex];

//			uint8_t link = switchIsLinked[switchIndex];             /// todo: implement sending/displaying links x32
            uint32_t link = switchIsLinked[switchIndex];

//            for (int index = 0; index < 8; index++) {
//				int linkIndex = index + (sourceAdrMask * 8);
			for (int index = 0; index < 32; index++) {
				int linkIndex = index;

				if (linkIndex == switchIndex || (0x00000001 & (link >> index))) {
					switchStatus[linkIndex] = switchStatus[switchIndex];

					if (switchStatus[linkIndex]) {
						sliderValue = switchVal[switchIndex];

						if (switchIsStrobe[linkIndex] && !isOnRoad) {
							if (!switchStrobeSet[linkIndex]) {
								onTime_buffer[linkIndex] = 255;
								offTime_buffer[linkIndex] = 0;
							} else {
								if(isPro)
                                {
                                    onTime_buffer[linkIndex] = switchStrobeOn[linkIndex];
                					offTime_buffer[linkIndex] = switchStrobeOff[linkIndex];;
                                }
                                else
                                {
                                    onTime_buffer[linkIndex] = 1;
                					offTime_buffer[linkIndex] = 4;
                                }
							}
//							strobeStatus[linkIndex] = true;
//							flashStatus[linkIndex] = false;
						} else if (switchIsFlash[linkIndex] && !isOnRoad) {
							if (!switchFlashSet[linkIndex]) {
								onTime_buffer[linkIndex] = 255;
								offTime_buffer[linkIndex] = 0;
							} else {
								onTime_buffer[linkIndex] = 10;
								offTime_buffer[linkIndex] = 10;
							}
//							strobeStatus[linkIndex] = false;
//							flashStatus[linkIndex] = true;
						} else {
							onTime_buffer[linkIndex] = 255;
							offTime_buffer[linkIndex] = 0;
//							strobeStatus[linkIndex] = false;
//							flashStatus[linkIndex] = false;
						}
					} else {
//						strobeStatus[linkIndex] = false;
//						flashStatus[linkIndex] = false;
					}

					machineEvent evt;

					evt.asPtrs[0] = (int) &switchVal;
					evt.asPtrs[1] = (int) &switchStatus;
					evt.asPtrs[2] = (int) &switchIsDimmable;
					evt.asPtrs[3] = (int) &onTime_buffer;
					evt.asPtrs[4] = (int) &offTime_buffer;

					evt.asChars[20] = linkIndex;

					mParams->mEvt = &evt;

					activity = activityReset;


					publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
					CyDelay(5);

//					if(index == 0)
                    if(index%8 == 0 && index/8 == sourceAdrMask)
					{
						publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
						CyDelay(5);
					}

				}
			}

			currentSwitchIndex = buttonIndex;

		} else if (touch.tag == 0
				&& switchIsMomentary[currentSwitchIndex + (sourceAdrMask * 8)]) { 

			int switchIndex = currentSwitchIndex + (sourceAdrMask * 8);
            
            //myPrintf("momRelease: %d+%d = %d\n", sourceAdrMask, currentSwitchIndex, switchIndex);

//			uint8_t link = switchIsLinked[switchIndex];        /// todo: implement sending/displaying links x32
            uint32_t link = switchIsLinked[switchIndex];        /// todo: implement sending/displaying links x32

//			for (int index = 0; index < 8; index++) {
//				int linkIndex = index + (sourceAdrMask * 8);
            for (int index = 0; index < 32; index++) {
				int linkIndex = index;

//                if ((index == currentSwitchIndex) || (0x00000001 & (link >> index))) {
				if ((index == switchIndex) || (0x00000001 & (link >> index))) {
					switchStatus[linkIndex] = false;

					machineEvent evt;

					evt.asPtrs[0] = (int) &switchVal;
					evt.asPtrs[1] = (int) &switchStatus;
					evt.asPtrs[2] = (int) &switchIsDimmable;
					evt.asPtrs[3] = (int) &onTime_buffer;
					evt.asPtrs[4] = (int) &offTime_buffer;

					evt.asChars[20] = linkIndex;

					mParams->mEvt = &evt;

					publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
					activity = activityReset;
					CyDelay(5);

				}
			}

		}
		else if (touch.tag == 0 && tempHeld > 0) {
			//myPrintf("rs th");
			tempHeld = 0;
		}
		else 
        {
			int switchIndex;

			switch (touch.tag) {
                
			case (ON_OFF_ROAD): {
				isOnRoad = !isOnRoad;
				int bIndex;

                if(isOnRoad)
                {
                    eepromLocalArray[SW_STATE_OFFSET] &= ~IS_OFF_ROAD_MASK;
                }
                else
                {
                    eepromLocalArray[SW_STATE_OFFSET] |= IS_OFF_ROAD_MASK;
				}
                
                saveSettingRow(SW_STATE_OFFSET / CY_FLASH_SIZEOF_ROW);
//				if (currentSwitchIndex < 255) {
//					
//                    bIndex = currentSwitchIndex + (sourceAdrMask * 8);
//                    
//                    if (!isOnRoad) {
//						
//						if (switchIsFlash[bIndex])
//							flashStatus[bIndex] = true;
//						else if (switchIsStrobe[bIndex])
//							strobeStatus[bIndex] = true;
//                            
//					} else {
//						flashStatus[bIndex] = false;
//						strobeStatus[bIndex] = false;
//					}
//
//				}
//                for (int index = 0; index < 8; index++) {
//					bIndex = index + (sourceAdrMask * 8);
				for (int index = 0; index < 32; index++) {

					bIndex = index;

					if (isOnRoad) 
                    {
						if (switchStatus[bIndex]) {
							onTime_buffer[bIndex] = 255;
							offTime_buffer[bIndex] = 0;
						}
					} 
                    else 
                    {
						if (switchStatus[bIndex]) 
                        {
							if (switchStrobeSet[bIndex]) 
                            {
								if(isPro) {
                                    onTime_buffer[bIndex] = switchStrobeOn[bIndex];
                					offTime_buffer[bIndex] = switchStrobeOff[bIndex];;
                                } else {
                                    onTime_buffer[bIndex] = 1;
                					offTime_buffer[bIndex] = 4;
                                }
							}

							if (switchFlashSet[bIndex]) 
                            {
								onTime_buffer[bIndex] = 10;
								offTime_buffer[bIndex] = 10;
							}
						}
					}

					machineEvent evt;

					evt.asPtrs[0] = (int) &switchVal;
					evt.asPtrs[1] = (int) &switchStatus;
					evt.asPtrs[2] = (int) &switchIsDimmable;
					evt.asPtrs[3] = (int) &onTime_buffer;
					evt.asPtrs[4] = (int) &offTime_buffer;

					evt.asChars[20] = bIndex;

					mParams->mEvt = &evt;

					publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
					activity = activityReset;
					CyDelay(5);

//					if(index == 0)
                    if(index%8 == 0 && index/8 == sourceAdrMask)
					{
						publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
						CyDelay(5);
					}
				}
				break;
			}
			case (SETUP):

				pageId = 2;

				machineEvent evt;

				evt.asChars[0] = pageId;

				mParams->mEvt = &evt;

				publish(GOTO_PAGE_SIG, mParams);
				break;
            
			case (SLEEP):{
				//DisplaySleep(true);
                sleepHeld = 1;
				myPrintf("sleep pressed...");
				//CyDelay(1000);
				break;
            }
			case (MSTROBE): {
				switchIndex = currentSwitchIndex + (sourceAdrMask * 8);

				switchStrobeSet[switchIndex] = !switchStrobeSet[switchIndex];
                
                
				if (switchStrobeSet[switchIndex]) {
                    
                    if(isPro)
                    {
                        onTime_buffer[switchIndex] = switchStrobeOn[switchIndex];
    					offTime_buffer[switchIndex] = switchStrobeOff[switchIndex];;
                    }
                    else
                    {
                        onTime_buffer[switchIndex] = 1;
    					offTime_buffer[switchIndex] = 4;
                    }
					
                    eepromLocalArray[SW_STATE_OFFSET + switchIndex] |= IS_STROBE_SET_MASK;
                    
				} else {
					onTime_buffer[switchIndex] = 255;
					offTime_buffer[switchIndex] = 0;
                    
                    eepromLocalArray[SW_STATE_OFFSET + switchIndex] &= ~IS_STROBE_SET_MASK;
				}
                
                saveSettingRow(SW_STATE_OFFSET / CY_FLASH_SIZEOF_ROW);

				evt.asPtrs[0] = (int) &switchVal;
				evt.asPtrs[1] = (int) &switchStatus;
				evt.asPtrs[2] = (int) &switchIsDimmable;
				evt.asPtrs[3] = (int) &onTime_buffer;
				evt.asPtrs[4] = (int) &offTime_buffer;

				evt.asChars[20] = switchIndex;

				mParams->mEvt = &evt;

				publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
                
				activity = activityReset;

				break;
			}
			case (MFLASH): {
				switchIndex = currentSwitchIndex + (sourceAdrMask * 8);

				switchFlashSet[switchIndex] = !switchFlashSet[switchIndex];

				if (switchFlashSet[switchIndex]) {
					onTime_buffer[switchIndex] = 10;
					offTime_buffer[switchIndex] = 10;
                    
                    eepromLocalArray[SW_STATE_OFFSET + switchIndex] |= IS_FLASH_SET_MASK;
                    
				} else {
					onTime_buffer[switchIndex] = 255;
					offTime_buffer[switchIndex] = 0;
                    
                    eepromLocalArray[SW_STATE_OFFSET + switchIndex] &= ~IS_FLASH_SET_MASK;
				}
                
                saveSettingRow(SW_STATE_OFFSET / CY_FLASH_SIZEOF_ROW);

				evt.asPtrs[0] = (int) &switchVal;
				evt.asPtrs[1] = (int) &switchStatus;
				evt.asPtrs[2] = (int) &switchIsDimmable;
				evt.asPtrs[3] = (int) &onTime_buffer;
				evt.asPtrs[4] = (int) &offTime_buffer;

				evt.asChars[20] = switchIndex;

				mParams->mEvt = &evt;

				publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
				activity = activityReset;

				break;
			}
			}
		}
        
        if (touch.tag == 0 && sleepHeld > 0) {
			sleepHeld = 0;
			DisplaySleep(true);
			CyDelay(1000);
		}
        
//        }
        
//        touchTemp = *((sTagXY *) mEvt->asInts[0]);  
        
		break;
	}
	case SWITCH_STATUS_FROM_CAN_SIG: {
        
		uint8_t *receive_buffer;

		receive_buffer = (uint8_t *) mEvt->asPtrs[0];

		//Serial.print("CAN index: ");

		canIndex = indexPos(receive_buffer[1]);
        
        uint8_t srcAddr = receive_buffer[0] & 0x03;
        int switchIndex = canIndex + (srcAddr * 8);

//        myPrintf("c%d %x\n", canIndex +1, oneHot(canIndex % 8));
//		myPrintf("SWITCH_STATUS_FROM_CAN_SIG %d (%x)\n", canIndex + 1, receive_buffer[0]);
		//	Serial.print("buffer: ");
		//	myPrintf(receive_buffer[2]);
		if((activity > 0) && (millis() - lastPageInitTime) > 7500 && srcAddr == sourceAdrMask)      // resend if initial send didn't work?
		{
			machineEvent evt;
//			int switchIndex = canIndex + (srcAddr * 8);
			bool isOn = receive_buffer[2] > 0 ? 1 : 0;
            
//            if(isPro && receive_buffer[4] != 0)
//            {
//                switchStrobeOn[switchIndex] = receive_buffer[3];
//                switchStrobeOff[switchIndex] = receive_buffer[4];
//            }
            
			if(switchStatus[switchIndex] != isOn)
			{
				evt.asPtrs[0] = (int) &switchVal;
				evt.asPtrs[1] = (int) &switchStatus;
				evt.asPtrs[2] = (int) &switchIsDimmable;
				evt.asPtrs[3] = (int) &onTime_buffer;
				evt.asPtrs[4] = (int) &offTime_buffer;

				evt.asChars[20] = switchIndex;

				mParams->mEvt = &evt;

				publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
                
			}
		}
		else
		{
			if (receive_buffer[2] > 0) {
				switchStatus[switchIndex] = 1;
//                switchStatus[canIndex + (sourceAdrMask * 8)] = 1;
			} else {
                switchStatus[switchIndex] = 0;
//				switchStatus[canIndex + (sourceAdrMask * 8)] = 0;
			}
		}

		/*	for(i=0; i<8; i++) {
		 if((oneHotVals[i] & receive_buffer[2]) == oneHotVals[i]){
		 switchStatus[i] = 1;
		 }
		 else {
		 switchStatus[i] = 0;
		 }
		 }
		 */

		break;
	}
    case SWITCH_STATUS2_FROM_CAN_SIG: {

		uint8_t *receive_buffer;

		receive_buffer = (uint8_t *) mEvt->asPtrs[0];

		//Serial.print("CAN index: ");

		canIndex = indexPos(receive_buffer[1]);
        
        switchMode[canIndex + (sourceAdrMask * 8)] = receive_buffer[2];
        
        switchCurrent[canIndex + (sourceAdrMask * 8)] = ((float) ((receive_buffer[3] << 8) | receive_buffer[4])) * CURRENT_FACTOR;
        
//        myPrintf("index: %x  mode: %d  current1: %x current2: %f  ", (canIndex + (sourceAdrMask * 8)), switchMode[canIndex + (sourceAdrMask * 8)], ((receive_buffer[3] << 8) | receive_buffer[4]), switchCurrent[canIndex + (sourceAdrMask * 8)]);
        
        currentTimeout = millis();
        
		break;
	}
	case TRACK_SIG: {
		track = *((sTrackTag *) mEvt->asInts[0]);
		int switchIndex;

        if(isLockoutOn)
        {
            if(track.tag == SETUP)
            {
                setupHeld++;
                
                if(setupHeld > 40)
                {
                    pageId = 2;

    				machineEvent evt;

    				evt.asChars[0] = pageId;

    				mParams->mEvt = &evt;

    				publish(GOTO_PAGE_SIG, mParams);
                    
                }
            }
            
            break;
        }
        
		switch (track.tag) {
		case (SLIDER): {
			switchIndex = currentSwitchIndex + (sourceAdrMask * 8);

			switchVal[switchIndex] = track.track;
			sliderValue = switchVal[switchIndex];
            
//			if (switchStatus[switchIndex]
//					&& normalizeSlider(switchVal[switchIndex]) > 655) {
            if (switchStatus[switchIndex]){
                
                if(normalizeSlider(switchVal[switchIndex]) <= 655) {
                    switchVal[switchIndex] = 64881;
				    sliderValue = switchVal[switchIndex];
                }
                
				machineEvent evt;

				evt.asPtrs[0] = (int) &switchVal;
				evt.asPtrs[1] = (int) &switchStatus;
				evt.asPtrs[2] = (int) &switchIsDimmable;
				evt.asPtrs[3] = (int) &onTime_buffer;
				evt.asPtrs[4] = (int) &offTime_buffer;

				evt.asChars[20] = switchIndex;

				mParams->mEvt = &evt;

				publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
				activity = activityReset;
            }
//			} else {
//				switchVal[switchIndex] = 64881;
//				sliderValue = switchVal[switchIndex];
//			}
            
            eepromLocalArray[DIM_VALUE_OFFSET + switchIndex] = switchVal[switchIndex] / 255;
            saveSettingRow(DIM_VALUE_OFFSET / CY_FLASH_SIZEOF_ROW);

			break;
		}
		case (SLEEP): {
			myPrintf("sleep held");
			sleepHeld++;
			if (sleepHeld > 40) {
				for (int j = 0; j < 8; j++) {
					int linkIndex = j + (sourceAdrMask * 8);
					switchVal[linkIndex] = 0;	// pwm value
					switchStatus[linkIndex] = 0;
					onTime_buffer[linkIndex] = 0xFF;
					offTime_buffer[linkIndex] = 0;
				}
				DisplaySleep(true);

				machineEvent evt;

				for (int k = 0; k < 8; k++) {
					int linkIndex = k + (sourceAdrMask * 8);

					evt.asPtrs[0] = (int) &switchVal;
					evt.asPtrs[1] = (int) &switchStatus;
					evt.asPtrs[2] = (int) &switchIsDimmable;
					evt.asPtrs[3] = (int) &onTime_buffer;
					evt.asPtrs[4] = (int) &offTime_buffer;

					evt.asChars[20] = linkIndex;

					mParams->mEvt = &evt;

					publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
					activity = activityReset;
					CyDelay(5);

				}
				sleepHeld = 0;
				myPrintf("reset values");
				CyDelay(1000);

			}
			break;
		}
		case (TEMP_SEL): {
            
            myPrintf("temperature Button\n");
            
			tempHeld++;
			if (tempHeld > 40) {
				tempHeld = 0;
				fahOn = !fahOn;

                eepromLocalArray[IS_FAHREN_OFFSET] = fahOn;
                saveSettingRow((IS_FAHREN_OFFSET)/CY_FLASH_SIZEOF_ROW); 
                
			}
			break;
		}
        break;
		}

		break;
	}

	}

	return (void *) mainIdle;

}
