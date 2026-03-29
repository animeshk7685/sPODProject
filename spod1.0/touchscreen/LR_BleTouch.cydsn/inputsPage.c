/*
 * settingPage.h
 *
 *  Created on: Oct 22, 2014
 *      Author: drewrawlings
 */

#include "common.h"
#include "sPodImageData.h"
#include "pubSub.h"



#define SET_SWITCH_X 35
#define SET_SWITCH_Y 30
#define SET_SWITCH_W 59
#define SET_SWITCH_H 59

#define SET_SWITCH_H2 31
#define SWITCH_SPACE3   6

#define SWITCH_SPACE_V 35
#define SWITCH_SPACE_H 15


enum tInputsPageButtonIds {
    INPUT_PAGE_NONE, BCK, SRC1, SRC2, SRC3, SRC4, INPUT_ID 
//	CONFIGURE_PAGE_NONE, LINE1, LINE2, LINE3, DEL, SHIFT, SAVE
};


stnext onInputsPage(int signal, void* params) {
	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;

	static sTagXY touch;

//    static sTagXY nullTag;
    
//	static sTrackTag track;
	static int offset = -20;
    
    
	char pageId;
    
//    static uint32_t resetHeld = 0;
//    static bool resetSent = false;
    

//    static bool screenTimerOn = 0;
    
//    static int lockoutHeld = 0;
    
//    static bool swipeOn = 1;

	switch (signal) {
		case SIG_INIT: {
			touch.tag = 0;
			touch.x = 0;
			touch.y = 0;

            
//            nullTag.tag = 0;
            
			myPrintf("inputs page\n");
			currentPage = onInputsPage;

            
			FT_GC_Cmd_Track(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, BRIGHT_SLIDER);
			FT_GC_Finish();

//            simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
			simplehsm_transition_state(mParams->machine->hsm, canComm, params);
            
            

			break;
		}
		case FRAME_TICK_SIG: {
//			int32_t tagoption;
			int16_t switchX = SET_SWITCH_X;
			int16_t switchY = SET_SWITCH_Y;
            

			FT_GC_ClearColorRGB_int(settings.pageBgColor);
			FT_GC_Clear(1, 1, 1);

			FT_GC_TagMask(1);

            
//            drawTriText( switchX + offset, -20, 0, 55, 26, "Input Settings", "", "", false, 0, settings.color, true);
            drawTriText(235, 163, 0, 55, 27, "Input", "", "", false, 0, settings.color, false);
            drawTriText(225, 185, 0, 55, 27, "Settings", "", "", false, 0, settings.color, false);
            
            drawTriText( 113 + offset, 161, 0, 55, 26, "Select Source", "", "", false, 0, settings.color, true);
            
            FT_GC_ColorRGB(196,196,196);
            FT_GC_Begin(FT_LINE_STRIP);
			FT_GC_LineWidth(1 *16);
			FT_GC_Vertex2f((offset + 98)*16, 205*16);
			FT_GC_Vertex2f((offset + 215)*16, 205*16);
			FT_GC_End();

            
			drawButton( 100 + offset, 210, 22, 22, 26, "1", "", "", touch, SRC1, sourceAddr[0]);
			drawButton( 130 + offset, 210, 22, 22, 26, "2", "", "", touch, SRC2, sourceAddr[1]);
			drawButton( 160 + offset, 210, 22, 22, 26, "3", "", "", touch, SRC3, sourceAddr[2]);
			drawButton( 190 + offset, 210, 22, 22, 26, "4", "", "", touch, SRC4, sourceAddr[3]);
            

            char inputLabel[] = {"Input 0"};

			for(int i = 0; i < 2; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					switchX = SET_SWITCH_X + ((SET_SWITCH_W + SWITCH_SPACE_H) * j) + offset;
					switchY = SET_SWITCH_Y + ((SET_SWITCH_H + SWITCH_SPACE_V) * i) - 10;

                    uint8_t switchIndex = (i * 4) + j;
//					uint8_t switchID = BUTTON_ID_LAST + (switchIndex + 8 * sourceAdrMask) * 3;
                    uint8_t switchID = INPUT_ID + (switchIndex + 8 * sourceAdrMask) * 3;
            
                    
                        inputLabel[6] = '1' + switchIndex;
                    
                            drawTriText(switchX + 8, switchY - 44, 0, 55, 26, inputLabel, "", "", false, 0, settings.color, true);
                    
                        drawButton( switchX, switchY, SET_SWITCH_W, SET_SWITCH_H2, 20, 
                           "Enabled", "" , "", touch, switchID, inputIsEnabled[switchIndex + (sourceAdrMask * 8)]);
                        
                        drawButton( switchX, switchY + (SET_SWITCH_H2 + SWITCH_SPACE3), SET_SWITCH_W, SET_SWITCH_H2, 20, 
                            "Lockout", "" , "", touch, switchID + 1, inputIsLockout[switchIndex + (sourceAdrMask * 8)]);
                        
//                        drawButton( switchX, switchY + (SET_SWITCH_H2 + SWITCH_SPACE3)*2, SET_SWITCH_W, SET_SWITCH_H2, 20, 
//                            "Invert Lock", "" , "", touch, switchID + 2, inputIsInvert[switchIndex + (sourceAdrMask * 8)]);
                         

				}
			}


			drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BCK, false);


			break;
		}
		case TOUCH_SIG: {

			touch = *((sTagXY *) mEvt->asInts[0]);

//			if (touch.tag <= SWITCH_7 && touch.tag > MAIN_PAGE_NONE)
            if (touch.tag >= INPUT_ID)
			{
				uint8_t swIndex = (touch.tag - INPUT_ID) / 3;
                uint8_t type = (touch.tag - INPUT_ID) % 3;

//                swIndex = swIndex + 8 * sourceAdrMask;
                
                if(swIndex < 32)
                {
                    switch(type)
                    {
                        case 0:
                            inputIsEnabled[swIndex] = !inputIsEnabled[swIndex];
                        break;
                        case 1:
                            inputIsLockout[swIndex] = !inputIsLockout[swIndex];
                        break;
//                        case 2:
//                            inputIsInvert[swIndex] = !inputIsInvert[swIndex];
//                        break;
                    }
                    
//                    uint8_t lastVal = eepromLocalArray[SW_OPTIONS_OFFSET + swIndex];
//        
//                    lastVal = lastVal & 0x0f;
//                    
//                    eepromLocalArray[SW_OPTIONS_OFFSET + swIndex] = lastVal |
//                        (inputIsEnabled[swIndex] ? IS_INPUT_ENABLED_MASK : 0) |
//                        (inputIsLockout[swIndex] ? IS_INPUT_LOCKOUT_MASK : 0) |
//                        (inputIsInvert[swIndex] ? IS_INPUT_INVERT_MASK : 0);
//                    
//                    myPrintf("sv sw %d: %x\n", swIndex, eepromLocalArray[SW_OPTIONS_OFFSET + swIndex]);
//        
//                    saveSettingRow(SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW);
                    
    
                    machineEvent evt;

                    evt.asPtrs[0] = (int) &inputIsEnabled;
                	evt.asPtrs[1] = (int) &inputIsLockout;
                	evt.asPtrs[2] = (int) &inputIsInvert;
                    evt.asPtrs[3] = (int) &switchIsLinked;
                    
                	evt.asChars[31] = swIndex;

                	mParams->mEvt = &evt;

                	publish(SEND_PRO_CAN_PACKET, mParams);
                    
                    CyDelay(5);
                    
    
    
                }
                
                    
				break;


			} else {
				switch (touch.tag) {
				
				case (BCK):
                
                    //saveSettingRow(BRIGHTNESS_OFFSET / CY_FLASH_SIZEOF_ROW);

                    //needsCanUpdate(swIndex, 0, mParams);
                
					pageId = 2;

					newEvt.asChars[0] = pageId;
					mParams->mEvt = &newEvt;
					publish(GOTO_PAGE_SIG, mParams);


					break;
				case (SRC1):
					sourceAdrMask = 0x00;
					sourceAddr[0] = true;
					sourceAddr[1] = false;
					sourceAddr[2] = false;
					sourceAddr[3] = false;
                    
                    eepromLocalArray[ADR_MASK_OFFSET] = sourceAdrMask;
					///EEPROM.write(ADR_MASK_OFFSET,sourceAdrMask);

					for(int i = 1; i <= 4; i++){
                        eepromLocalArray[ADR_MASK_OFFSET+i] = (uint8_t)sourceAddr[i - 1];
						///EEPROM.write(ADR_MASK_OFFSET+i,(uint8_t)sourceAddr[i -1]);
					}

//					linkStatus = 0;
//
//					for(int i = 0; i < 8; i++)
//						isLinked[i] = 0;
//
//					linkIndex = -1;

					break;
				case (SRC2):
					sourceAdrMask = 0x01;
					sourceAddr[0] = false;
					sourceAddr[1] = true;
					sourceAddr[2] = false;
					sourceAddr[3] = false;
                    
                    eepromLocalArray[ADR_MASK_OFFSET] = sourceAdrMask;
					///EEPROM.write(ADR_MASK_OFFSET,sourceAdrMask);

					for(int i = 1; i <= 4; i++){
                        eepromLocalArray[ADR_MASK_OFFSET+i] = (uint8_t)sourceAddr[i - 1];
						///EEPROM.write(ADR_MASK_OFFSET+i,(uint8_t)sourceAddr[i - 1]);
					}
//					linkStatus = 0;
//
//					for(int i = 0; i < 8; i++)
//						isLinked[i] = 0;
//
//					linkIndex = -1;

					break;
				case (SRC3):
					sourceAdrMask = 0x02;
					sourceAddr[0] = false;
					sourceAddr[1] = false;
					sourceAddr[2] = true;
					sourceAddr[3] = false;
                    
                    eepromLocalArray[ADR_MASK_OFFSET] = sourceAdrMask;
					///EEPROM.write(ADR_MASK_OFFSET,sourceAdrMask);

					for(int i = 1; i <= 4; i++){
                        eepromLocalArray[ADR_MASK_OFFSET+i] = (uint8_t)sourceAddr[i - 1];
						///EEPROM.write(ADR_MASK_OFFSET+i,(uint8_t)sourceAddr[i - 1]);
					}
//					linkStatus = 0;
//
//					for(int i = 0; i < 8; i++)
//						isLinked[i] = 0;
//
//					linkIndex = -1;

					break;
				case (SRC4):
					sourceAdrMask = 0x03;
					sourceAddr[0] = false;
					sourceAddr[1] = false;
					sourceAddr[2] = false;
					sourceAddr[3] = true;
                    eepromLocalArray[ADR_MASK_OFFSET] = sourceAdrMask;
					///EEPROM.write(ADR_MASK_OFFSET,sourceAdrMask);

					for(int i = 1; i <= 4; i++){
                        eepromLocalArray[ADR_MASK_OFFSET+i] = (uint8_t)sourceAddr[i - 1];
						///EEPROM.write(ADR_MASK_OFFSET+i,(uint8_t)sourceAddr[i - 1]);
					}
//					linkStatus = 0;
//
//					for(int i = 0; i < 8; i++)
//						isLinked[i] = 0;
//
//					linkIndex = -1;

					break;

				}
			}

            saveSettingRow(ADR_MASK_OFFSET / CY_FLASH_SIZEOF_ROW);
            
			break;
		}
		case TRACK_SIG:
			{
//				track = *((sTrackTag *) mEvt->asInts[0]);

//                myPrintf("%d \n", track.tag);
                
//				switch(track.tag)
//				{
//                }
			}
            break;

		}

	return (void *) mainIdle;
}

