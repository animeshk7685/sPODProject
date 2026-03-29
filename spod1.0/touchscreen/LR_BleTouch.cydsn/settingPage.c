/*
 * settingPage.h
 *
 *  Created on: Oct 22, 2014
 *      Author: drewrawlings
 */

#include "common.h"
#include "sPodImageData.h"
#include "pubSub.h"



#define SET_SWITCH_X 45
#define SET_SWITCH_Y 45
#define SET_SWITCH_W 49
#define SET_SWITCH_H 49

//extern bool pairOn = false;

int iSqrt(int x)
{
    int i;
    
    for(i = 0; i <= 256 ; i++)
    {
        if((i * i) > x)
        {
            return (i - 1); 
        }
    }
    
    return i;
}

static int linkStatus = 0;
static int linkIndex = -1;
static bool isLinked[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

void endLinking(bool isClearStat)
{
//    myPrintf("endLinking()\n");
    if(isClearStat) {
        linkStatus = 0;
    }
    
    for(int i = 0; i < 32; i++) {
    	isLinked[i] = 0;
    }
    
    linkIndex = -1;
}

stnext onSettingsPage(int signal, void* params) {
	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;

	static sTagXY touch;

    static sTagXY nullTag;
    
	static uint16_t bSliderValue = 0;
    static uint16_t tSliderValue = 52572;
	static sTrackTag track;
	static int offset = -20;
//	static int linkStatus = 0;
//    static int blePairStatus = 0;
//    static int bleResetStatus = 0;
//	static char linkText[] = "Link";
//	static char unLink[] = "Unlink";
//	static char* linktextPtr;
//	static int linkIndex = -1;
//	static bool isLinked[] = {0,0,0,0,0,0,0,0};

    static uint16_t appVer;
    static char boardRev;
    
	char pageId;
    
    static uint32_t resetHeld = 0;
    static bool resetSent = false;
    

    static bool screenTimerOn = 0;
    
    static int lockoutHeld = 0;
    
    static char strSelectedSw[] = "Linking to Source 0 Switch 0";
    static char strLinkedSw[] = "01234567 89012345 67890123 45678901";
    
//    static int switchIndex = -1;
    
//    static bool swipeOn = 1;
    
//    static bool needsPrint = false;
//    static int cnt = 0;

	switch (signal) {
		case SIG_INIT: {
			touch.tag = 0;
			touch.x = 0;
			touch.y = 0;

            bSliderValue = 65535 - (settings.dayBrightness * 511);
            if(bSliderValue < 0)
                bSliderValue = 0;
            
            
            tSliderValue = 65535 - (iSqrt(settings.sleepTimer / 236) * 256);
            if(tSliderValue < 0)
                tSliderValue = 0;
            
//            slTime = (65535 - tSliderValue) / 256;
//                        
//                        settings.sleepTimer = (slTime * slTime);
            
            nullTag.tag = 0;
            
			myPrintf("settings page\n");
			currentPage = onSettingsPage;

            
			FT_GC_Cmd_Track(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, BRIGHT_SLIDER);
			FT_GC_Finish();

//            simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
			simplehsm_transition_state(mParams->machine->hsm, canComm, params);
			linkStatus = 0;
            endLinking(1);
            
//			linktextPtr = linkText;

            resetHeld = 0;
            
//            uint16_t boardId;
                
            uint32_t board;

            appVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
            board = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);

            boardRev = (board & 0x0000FFFF) + ('A' - 1);
            
//            appVer = 0x0FFF;


			break;
		}
		case FRAME_TICK_SIG: {
//            needsPrint = false;
//            cnt++;
//            if(cnt > 100)
//            {
//                cnt = 0;
//                needsPrint = true;
//            }
            
//			int32_t tagoption;
			int16_t switchX = SET_SWITCH_X;
			int16_t switchY = SET_SWITCH_Y;
            

			FT_GC_ClearColorRGB_int(settings.pageBgColor);
			FT_GC_Clear(1, 1, 1);

			FT_GC_TagMask(1);

			//drawImage( sPod_Bitmap_Header, 0, 0, 0.5, 0.5, touch, SPOD_LOGO_TOUCH, SPOD_LOGO);
			//drawImage( sPod_Addr1Bitmap_Header, 100, 185, 0.35, 0.35, touch, ADDR1_TOUCH, 160000);
			//drawImage( sPod_Addr2Bitmap_Header, 155, 185, 0.35, 0.35, touch, ADDR2_TOUCH, SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH);
			//drawImage( sPod_Addr3Bitmap_Header, 210, 185, 0.35, 0.35, touch, ADDR3_TOUCH, SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH + SPOD_ADDRS_2_LENGTH);
			//drawImage( sPod_Addr4Bitmap_Header, 265, 185, 0.35, 0.35, touch, ADDR4_TOUCH, SPOD_RAWDATA_LENGTH + SPOD_ADDRS_1_LENGTH + SPOD_ADDRS_2_LENGTH + SPOD_ADDRS_3_LENGTH);


//			drawTriText( switchX + offset, -20, 0, 55, 26, "Select Switch", "", "", false, 0, settings.color, true);
            drawTriText( switchX + offset, -25, 0, 55, 26, "Select", "", "", false, 0, settings.color, true);
            drawTriText( switchX + offset, -10, 0, 55, 26, "Switch", "", "", false, 0, settings.color, true);
//			drawTriText( 205 + offset, -20, 0, 55, 26, "Brightness ->", "", "", false, 0, settings.color, true);
//			drawTriText( 127 + offset, 140, 0, 55, 26, "Select Source", "", "", false, 0, settings.color, true);
            
            
            drawTriText( 147 + offset, 161, 0, 55, 26, "Select Source", "", "", false, 0, settings.color, true);
            
            FT_GC_ColorRGB(196,196,196);
            FT_GC_Begin(FT_LINE_STRIP);
			FT_GC_LineWidth(1 *16);
			FT_GC_Vertex2f((offset + 98)*16,205*16);
			FT_GC_Vertex2f((offset + 272)*16,205*16);
			FT_GC_End();

			drawButton( 100 + offset, 210, 22, 22, 26, "1", "", "", touch, SOURCE1, sourceAddr[0]);
			drawButton( 130 + offset, 210, 22, 22, 26, "2", "", "", touch, SOURCE2, sourceAddr[1]);
			drawButton( 160 + offset, 210, 22, 22, 26, "3", "", "", touch, SOURCE3, sourceAddr[2]);
			drawButton( 190 + offset, 210, 22, 22, 26, "4", "", "", touch, SOURCE4, sourceAddr[3]);
            
            if(isSwipeOn)
            {
                drawButton( 220 + offset, 210, SET_SWITCH_W, 22, 20, "Swipe on", "", "", touch, SWIPE_TOGGLE, true); 
            }
            else
            {
                drawButton( 220 + offset, 210, SET_SWITCH_W, 22, 20, "Swipe off", "", "", touch, SWIPE_TOGGLE, false); 
            }
            
            
            
            
//            drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);
            
//			drawButton( SET_SWITCH_X + offset, 150, SET_SWITCH_W, 35, 20, linktextPtr, "", "", touch, LINK, linkStatus);
//            drawButton(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W) * 3 / 2, 4, SET_SWITCH_W, 26, 20, linktextPtr, "", "", touch, LINK, linkStatus);
            
            if(isPro)
            {
                drawButton(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W), 4, SET_SWITCH_W, 26, 26, "Link", "", "", touch, LINK, linkStatus);
                drawButton(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W) * 2, 4, SET_SWITCH_W, 26, 26, "Inputs", "", "", touch, INPUTS, false);
            }
            else
            {
                drawButton(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W), 4, SET_SWITCH_W *2 + SWITCH_SPACE, 26, 26, "Switch Linking", "", "", touch, LINK, linkStatus);
            }
            
            if(0 && linkStatus)
            {
                if(linkIndex < 0)// || switchIndex < 0)
                {
                    drawTriText( 50 + offset, 125, 0, 55, 21, "Select switch to link", "", "", false, 0, settings.color, true);
                }
                else
                {
//                    int swInd = linkIndex;
//                    isLinked[]
                    
//                    char selectedSw[] = "Linking to Source 0 Switch 0";
                    
//                    int normIndex = linkIndex + 1;
                    uint8_t normSrc = linkIndex / 8 + 1;
                    uint8_t normSw = linkIndex % 8 + 1;
                    uint8_t swPos = sizeof(strSelectedSw) - 2;
                    uint8_t srcPos = sizeof(strSelectedSw) - 11;
                    
                    strSelectedSw[srcPos] = '0' + normSrc;
                    strSelectedSw[swPos] = '0' + normSw;
                    
//                    uint32_t tempLinks = isLinked[linkIndex];
                    int j = 0;
                    
                    for(uint8_t i = 0; i < (32 + 3) ; i++)
                    {
                        if(j == linkIndex)
                        {
                            strLinkedSw[i] = '|';
                        }
                        else if(isLinked[j])//tempLinks & (0x00000001 << j))
                        {
                            strLinkedSw[i] = 'X';
                        }
                        else
                        {
                            strLinkedSw[i] = '-';
                        }
                        
                        if(i == 7 || i == 16 || i == 25) // account for spaces
                        {
                            i++;
                        }
                        
                        j++;
                        
                    }
                    
                    drawTriText( 50 + offset, 122, 0, 55, 20, strSelectedSw, "", "", false, 0, settings.color, true);
                    drawTriText( 50 + offset, 140, 0, 55, 20, strLinkedSw, "", "", false, 0, settings.color, true);
                }
            }
            else
            {
                if(isLockoutOn)
                {
                    drawButton(SET_SWITCH_X + offset, 163, SET_SWITCH_W, 22, 20, "Locked", "", "", touch, LOCKOUT_TOGGLE, true); 
                }
                else
                {
                    drawButton(SET_SWITCH_X + offset, 163, SET_SWITCH_W, 22, 20, "Unlocked", "", "", touch, LOCKOUT_TOGGLE, false); 
                }
            
                drawButton( SET_SWITCH_X + offset + 1*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                    "Pair", "", "", touch, BLE_PAIR, pairOn);
                
                if(resetSent)
                {
                    drawButton( SET_SWITCH_X + offset + 2*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                       "Reset", "", "", touch, BLE_RESET, false); 
                }
                else
                {
                    drawButton( SET_SWITCH_X + offset + 2*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                       "Reset", "", "", nullTag, BLE_RESET, false); 
                }
                
                drawTriText( 55 + offset, 119, 0, 55, 20, "Screen", "", "", false, 0, settings.color, true);
                drawTriText( 134 + offset, 119, 0, 55, 20, "Bluetooth", "", "", false, 0, settings.color, true);
                drawTriText( 220 + offset, 119, 0, 55, 20, "Backlight", "", "", false, 0, settings.color, true);
                
                if(screenTimerOn)
                {
                    drawButton( SET_SWITCH_X + offset + 3*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                       "Timer", "", "", touch, BACKLIGHT_TOGGLE, false); 
                }
                else
                {
                    drawButton( SET_SWITCH_X + offset + 3*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                       "Intensity", "", "", touch, BACKLIGHT_TOGGLE, false); 
                }
            }
            
            
            FT_GC_Tag(0);
		    drawVersionNum( 315, 226, "", 20, appVer, boardRev);
            
//            drawTriText( 205 + offset, -20, 0, 55, 26, "Brightness ->", "", "", false, 0, settings.color, true);
//			drawSlider( SLIDER_X, SLIDER_Y+5, SLIDER_W, SLIDER_H-10, sliderValue, track, BRIGHT_SLIDER);

			for(int i = 0; i < 2; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					switchX = SET_SWITCH_X + ((SET_SWITCH_W + SWITCH_SPACE) * j) + offset;
					switchY = SET_SWITCH_Y + ((SET_SWITCH_H + SWITCH_SPACE) * i) - 10;

					uint8_t switchID = SWITCH_0 + (i * 4) + j;
					uint8_t switchIndex = switchID -1;


						FT_GC_Begin(FT_RECTS);
						FT_GC_ColorRGB_int(settings.highlightColor);
						FT_GC_LineWidth(5 *16);
						FT_GC_Vertex2ii(switchX + 6, switchY + 6, 0 , 0);
						FT_GC_Vertex2ii(switchX + SET_SWITCH_W - 3, switchY + SET_SWITCH_H - 3, 0 , 0);
						FT_GC_End();
						drawButton( switchX, switchY, SET_SWITCH_W, SET_SWITCH_H, 20, 
                            buttonLabels[switchIndex + (sourceAdrMask * 8)].line1, 
                            buttonLabels[switchIndex + (sourceAdrMask * 8)].line2 , 
                            buttonLabels[switchIndex + (sourceAdrMask * 8)].line3, 
                            touch, switchID, isLinked[switchIndex + (sourceAdrMask * 8)]);
                        
//                        if(needsPrint)
//                        {
//                            myPrintf("sw%d: %d, %d/%d\n", switchIndex, isLinked[switchIndex + (sourceAdrMask * 8)], touch.tag, switchID);
//                        }
				}
			}


			drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);
            
            if(screenTimerOn)
            {
                drawSlider( SLIDER_X, SLIDER_Y+5, SLIDER_W, SLIDER_H-15, tSliderValue, track, BRIGHT_SLIDER);
                
                FT_GC_Tag(0);
        		drawBacklightVal( 210 + offset, 4, "", 28, settings.sleepTimer / 60000, screenTimerOn);
            }
            else
            {
                drawSlider( SLIDER_X, SLIDER_Y+5, SLIDER_W, SLIDER_H-15, bSliderValue, track, BRIGHT_SLIDER);
            
                FT_GC_Tag(0);
        		drawBacklightVal( 230 + offset, 4, "", 28, settings.dayBrightness, screenTimerOn);
            }

			break;
		}
		case TOUCH_SIG: {

			touch = *((sTagXY *) mEvt->asInts[0]);

			if (touch.tag <= SWITCH_7 && touch.tag > MAIN_PAGE_NONE)
			{
//				uint8_t index = touch.tag - 1;
                uint8_t index = touch.tag - 1 + (sourceAdrMask * 8);

                if(index >= 32)
                {
                    myPrintf("index out of range %d\n", index);
                    break;
                }
//                else
//                {
//                    myPrintf("index: %d\n", index);
//                }

				if(linkStatus)
				{
					if(linkIndex == -1)
					{
						linkIndex = index;
						isLinked[index] = 1;
                        
//						uint8_t link = switchIsLinked[linkIndex + (sourceAdrMask * 8)];
//
//						for(int i = 0; i < 8; i++)
//						{
//							if(i != linkIndex)
//							{
//								isLinked[i] = 0x01 & (link >> i);
//							}
//						}

						uint32_t link = switchIsLinked[linkIndex];
                        
//                        myPrintf("Start linking %d(%x)\n", linkIndex, link);

						for(int i = 0; i < 32; i++)
						{
							if(i != linkIndex)
							{
								isLinked[i] = 0x01 & (link >> i);
							}
						}
					}

					if(linkIndex != index)
					{
//						int switchIndex = linkIndex + (sourceAdrMask * 8);

						isLinked[index] = !isLinked[index];

//						if(isLinked[index])
//							switchIsLinked[switchIndex] = switchIsLinked[switchIndex] | (0x01 << index);
//						else
//							switchIsLinked[switchIndex] = switchIsLinked[switchIndex] & ~(0x01 << index);
                        
                        if(isLinked[index]) {
							switchIsLinked[linkIndex] = switchIsLinked[linkIndex] | (0x00000001 << index);
						} else {
							switchIsLinked[linkIndex] = switchIsLinked[linkIndex] & ~(0x00000001 << index);
                        }
                        
//                        myPrintf("link %d[%d/%d]: %x\n", isLinked[index], linkIndex, index, switchIsLinked[linkIndex]);
                        
//                        eepromLocalArray[IS_LINKED_OFFSET + switchIndex] = switchIsLinked[switchIndex];
                        eepromLocalArray32[IS_LINKED_OFFSET/4 + linkIndex] = switchIsLinked[linkIndex];
					}
				}
				else
				{
					configureIndex = index % 8;
					pageId = 3;
					newEvt.asChars[0] = pageId;
					mParams->mEvt = &newEvt;
					publish(GOTO_PAGE_SIG, mParams);
				}
				break;


			} else {
				switch (touch.tag) {
				case (LINK):

					linkStatus = !linkStatus;

                    if(!linkStatus && linkIndex > 0) {
//                        myPrintf("saving linkL %d: %x %x\n", linkIndex, switchIsLinked[linkIndex]);
                        saveSettingRow(IS_LINKED_OFFSET / CY_FLASH_SIZEOF_ROW);
                        
                        if(isPro)
                        {
                            //myPrintf("\ntry publish SEND_PRO_CAN_PACKET: (%d) L0x%x\n", linkIndex, switchIsLinked[linkIndex]);
                            
                            machineEvent evt;

                            evt.asPtrs[0] = (int) &inputIsEnabled;
                        	evt.asPtrs[1] = (int) &inputIsLockout;
                        	evt.asPtrs[2] = (int) &inputIsInvert;
                            evt.asPtrs[3] = (int) &switchIsLinked;
                            
                        	evt.asChars[31] = linkIndex;

                        	mParams->mEvt = &evt;

                        	publish(SEND_PRO_CAN_PACKET, mParams);
                        }
                    }
                    
                    endLinking(0);
                    
					break;
                case (BLE_PAIR):
                
                    newEvt.asChars[0] = BLE_PAIR;
					mParams->mEvt = &newEvt;
					publish(BLE_PAIR_SIG, mParams);
                
    				break;        
                case (BLE_RESET):     // moved to track for held instead of touch
                
                    resetSent = false;
                    resetHeld = 0;
                
    				break;
                case (BACKLIGHT_TOGGLE):    

                    if(screenTimerOn) {
                        screenTimerOn = 0;
                    } else {
                        screenTimerOn = 1;
                    }
                
                
    				break;
                case (SWIPE_TOGGLE):    

                    if(isSwipeOn) {
                        isSwipeOn = 0;
                    } else {
                        isSwipeOn = 1;
                    }
                
                    eepromLocalArray[IS_SWIPE_ENABLED_OFFSET] = isSwipeOn;
                    
                    saveSettingRow(IS_SWIPE_ENABLED_OFFSET / CY_FLASH_SIZEOF_ROW);
                
    				break;
                case (LOCKOUT_TOGGLE):    
                    
                    lockoutHeld = 0;
                    
                    touch.tag = 0;
                
                    if(isLockoutOn)
                    {
                        myPrintf("Unlock");
                        isLockoutOn = 0;
                        
                        eepromLocalArray[IS_LOCKOUT_OFFSET] = isLockoutOn;
                    }
                    
                    saveSettingRow(IS_LOCKOUT_OFFSET / CY_FLASH_SIZEOF_ROW);
                
    				break;
				case (BACK):
                
                    saveSettingRow(BRIGHTNESS_OFFSET / CY_FLASH_SIZEOF_ROW);
                    
                    if(linkStatus && linkIndex > 0) {
//                        myPrintf("saving linkB %d: %x\n", linkIndex, switchIsLinked[linkIndex]);
                        saveSettingRow(IS_LINKED_OFFSET / CY_FLASH_SIZEOF_ROW);
                        
                        if(isPro)
                        {
                            machineEvent evt;

                            evt.asPtrs[0] = (int) &inputIsEnabled;
                        	evt.asPtrs[1] = (int) &inputIsLockout;
                        	evt.asPtrs[2] = (int) &inputIsInvert;
                            evt.asPtrs[3] = (int) &switchIsLinked;
                            
                        	evt.asChars[31] = linkIndex;

                        	mParams->mEvt = &evt;

                        	publish(SEND_PRO_CAN_PACKET, mParams);
                        }
                    }
                    endLinking(1);
                    
					pageId = 1;
                    

					newEvt.asChars[0] = pageId;
					mParams->mEvt = &newEvt;
					publish(GOTO_PAGE_SIG, mParams);
                    
					break;
                case (INPUTS):
                
//                    saveSettingRow(BRIGHTNESS_OFFSET / CY_FLASH_SIZEOF_ROW);
                
                    if(linkStatus && linkIndex > 0) {
                        saveSettingRow(IS_LINKED_OFFSET / CY_FLASH_SIZEOF_ROW);
                        
                        if(isPro)
                        {
                            machineEvent evt;

                            evt.asPtrs[0] = (int) &inputIsEnabled;
                        	evt.asPtrs[1] = (int) &inputIsLockout;
                        	evt.asPtrs[2] = (int) &inputIsInvert;
                            evt.asPtrs[3] = (int) &switchIsLinked;
                            
                        	evt.asChars[31] = linkIndex;

                        	mParams->mEvt = &evt;

                        	publish(SEND_PRO_CAN_PACKET, mParams);
                        }
                    }
                    endLinking(1);

					pageId = 5;

					newEvt.asChars[0] = pageId;
					mParams->mEvt = &newEvt;
					publish(GOTO_PAGE_SIG, mParams);

					break;
				case (SOURCE1):
                
					sourceAdrMask = 0x00;
                    updateSourceAddress();
                    
//                    endLinking(1);

					break;
				case (SOURCE2):
                
					sourceAdrMask = 0x01;
                    updateSourceAddress();
                    
//                    endLinking(1);

					break;
				case (SOURCE3):
                
					sourceAdrMask = 0x02;
                    updateSourceAddress();
                    
//                    endLinking(1);

					break;
				case (SOURCE4):
                
					sourceAdrMask = 0x03;
                    updateSourceAddress();
                    
//                    endLinking(1);

					break;

				}
			}

//            saveSettingRow(ADR_MASK_OFFSET / CY_FLASH_SIZEOF_ROW);
            
			break;
		}
		case TRACK_SIG:
			{
				track = *((sTrackTag *) mEvt->asInts[0]);

//                myPrintf("%d \n", track.tag);
                
				switch(track.tag)
				{
				case (BRIGHT_SLIDER): {
						
                    
//                    myPrintf("sliderValue: %d\n", sliderValue);
                    
                    if(screenTimerOn)
                    {
                        tSliderValue = track.track;
                        
                        uint32_t slTime;
                        
                        slTime = (65535 - tSliderValue) / 256;
                        
                        settings.sleepTimer = (slTime * slTime) * 393 / 100000 * 60000;
                        
//                        sleepTimer.currentTime = settings.sleepTimer;
//                		sleepTimer.rollover = settings.sleepTimer;
                        
                        eepromLocalArray[TIMEOUT_OFFSET] = settings.sleepTimer / 60000;
                    }
                    else
                    {
                        bSliderValue = track.track;
                        
						settings.dayBrightness = (65535 - bSliderValue) / 511;

						if(settings.dayBrightness < 9)
							settings.dayBrightness = 9;
						else if(settings.dayBrightness > 120)
							settings.dayBrightness = 128;

						//myPrintf(settings.dayBrightness);

						FT_Transport_SPI_Write32(REG_PWM_DUTY, settings.dayBrightness);
                        
                        eepromLocalArray[BRIGHTNESS_OFFSET] = settings.dayBrightness;

                    }

					break;
				}
                case (BLE_RESET): {
                    
//                    myPrintf("ble Reset button \n");
                    
                    resetHeld++;
                    if(resetHeld > 40)
                        {
                            resetHeld = 0;
            				resetSent = true;
                            
                            newEvt.asChars[0] = BLE_RESET;
        					mParams->mEvt = &newEvt;
        					publish(BLE_PAIR_SIG, mParams);
            			}
//                    }
        			break;
    		    }
                case (LOCKOUT_TOGGLE):
                {
//                    myPrintf(".");
                    lockoutHeld++;
                    
                    if(!isLockoutOn && lockoutHeld > 15)
                    {
                        myPrintf("Lock\n");
                        isLockoutOn = 1;   
                        
                        eepromLocalArray[IS_LOCKOUT_OFFSET] = isLockoutOn;
                    }
                    
                    break;
                }
                
                }
			}
            break;

		}

	return (void *) mainIdle;
}

