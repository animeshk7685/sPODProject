/*
 * configurePage.h
 *
 *  Created on: Oct 23, 2014
 *      Author: drewrawlings
 */

#include "common.h"
#include <math.h>


enum tConfigurePageButtonIds {
	CONFIGURE_PAGE_NONE, LINE1, LINE2, LINE3, DEL, SHIFT, SAVE, SW_OPTIONS//, SW_TEXT, INPUT_ENABLED, INPUT_LOCKOUT, INPUT_LOCK_INVERT
};



#define SET_SWITCH_X 45
#define SET_SWITCH_Y 45
#define SET_SWITCH_W 49
#define SET_SWITCH_H 49
#define SWITCH_SPACE 8

#define LINE_X 55
#define LINE_Y 7
#define LINE_W 130
#define LINE_H 23
#define LINE_SPACE 5


#define INPUT_TOG_X     (200)
#define INPUT_TOG_Y     (138)
#define INPUT_TEXT_X    (235)
#define INPUT_TEXT_Y    (115)
#define INPUT_SPACE     (30)


#define SAMAPP_COPRO_WIDGET_KEYS_INTERACTIVE_TEXTSIZE (512)

stnext onConfigurePage(int signal, void* params) {

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
	//FT800IMPL_SPI *FTImpl = (FT800IMPL_SPI *) mParams->machine->obj;

	static sTagXY touch;

	static tButtonLabel textLine[1] = {{"","",""}};

	char pageId;
	static uint8_t lineIndex = CONFIGURE_PAGE_NONE;
	static int cursorCount = 0;
	static bool cursor = true;
//	char* token;
	char dash;

	//drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);

	int16_t TextFont = 29,ButtonW = 30,ButtonH = 30,yBtnDst = 5,yOffset;//,xOffset;

//	char DispText[SAMAPP_COPRO_WIDGET_KEYS_INTERACTIVE_TEXTSIZE];
	static uint8_t CurrTag = 0;//, lastTag=0;
	static int32_t CurrTextIdx = 0;
//	static bool textOn=false;
//	uint8_t touchedTag=0;

//	uint16_t textX, textY;
	static uint8_t shiftState = 1;

	switch (signal) {
		case SIG_INIT: {
			touch.tag = 0;
			touch.x = 0;
			touch.y = 0;

			textLine[0] = buttonLabels[configureIndex + (sourceAdrMask * 8)];
			lineIndex = 0;

			myPrintf("configure page");
			currentPage = onConfigurePage;

//            simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
			simplehsm_transition_state(mParams->machine->hsm, canComm, params);

			break;
		}
		case FRAME_TICK_SIG: {
//			int32_t tagoption;
//			int16_t lineX = LINE_X;
			int16_t lineY = LINE_Y;

			if(cursorCount > 25){
				cursorCount = 0;
				cursor = !cursor;

				if(cursor){
					dash = '|';

				}else {
					dash = 0;

				}
				switch(lineIndex) {
					case (LINE1):

							CurrTextIdx = getTextIndex(textLine[0].line1);
							myPrintf("%d\n", CurrTextIdx);
							if(CurrTextIdx >=9)
								break;
							else
								textLine[0].line1[CurrTextIdx] = dash;
							break;
					case (LINE2):
							CurrTextIdx = getTextIndex(textLine[0].line2);
							myPrintf("%d\n", CurrTextIdx);
							if(CurrTextIdx >=9)
								break;
							else
								textLine[0].line2[CurrTextIdx] = dash;
							break;
					case (LINE3):
							CurrTextIdx = getTextIndex(textLine[0].line3);
							myPrintf("%d\n", CurrTextIdx);
							if(CurrTextIdx >=9)
								break;
							else
								textLine[0].line3[CurrTextIdx] = dash;
							break;
					}

			}else{
				cursorCount++;
			}




			FT_GC_ClearColorRGB_int(settings.pageBgColor);
			FT_GC_Clear(1, 1, 1);

			FT_GC_TagMask(1);



			//drawImage( sPod_Bitmap_Header, 0, 0, 0.5, 0.5, touch, SPOD_LOGO_TOUCH, SPOD_LOGO);

			drawTriText( 10, -10, 0, 55, 26, "Line 1:", "", "", false, 0, settings.color, true);
			drawTriText( 10,  15, 0, 55, 26, "Line 2:", "", "", false, 0, settings.color, true);
			drawTriText( 10,  40, 0, 55, 26, "Line 3:", "", "", false, 0, settings.color, true);

			drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);
			drawButton( 250, 165, 40, 30, 26, "Del", "", "", touch, DEL, false);
			drawButton( 12, 165, 47, 30, 26, "Shift", "", "", touch, SHIFT, false);
			drawButton( 250, 200, SWITCH_W, 35, 26, "Save", "", "", touch, SAVE, false);
            
            if(iconId[configureIndex + (sourceAdrMask * 8)] == 255)
            {
            
			    drawButton( 215, 25, 65, 50, 26, "Switch", "Features", "", touch, SW_OPTIONS, false);
                
            }
            else
            {
            
                drawButton( 205, 45, 85, 40, 26, "Switch", "Features", "", touch, SW_OPTIONS, false);
                
                if(iconIsOn[configureIndex + (sourceAdrMask * 8)])
                {
                    drawButton( 205, 10, 85, 25, 26, 
                       "Icon ON", "", "", touch, ICON_TOGGLE, false); 
                }
                else
                {
                    drawButton( 205, 10, 85, 25, 26,  
                       "Icon OFF", "", "", touch, ICON_TOGGLE, false); 
                }
            }
                
                
			for(int i = 0; i<3; i++)
			{

				lineY = LINE_Y + ((LINE_H + LINE_SPACE) * i);

				uint8_t lineID = LINE1 + i;

				switch(i){
				case(0):
					drawButton( LINE_X, lineY, LINE_W, LINE_H, 26, textLine[0].line1, "", "", touch, lineID, false);
					break;
				case(1):
					drawButton( LINE_X, lineY, LINE_W, LINE_H, 26, textLine[0].line2, "", "", touch, lineID, false);
					break;
				case(2):
					drawButton( LINE_X, lineY, LINE_W, LINE_H, 26, textLine[0].line3, "", "", touch, lineID, false);
					break;

				}
			}

			if(shiftState == 1){
				yOffset = 80 + 15;
			   // Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
				//FT_GC_Cmd_FGColor(0x404080);
				//FT_GC_Cmd_GradColor(0x00ff00);
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "qwertyuiop");
				//FT_GC_Cmd_GradColor(0x00ffff);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "asdfghjkl");
				//FT_GC_Cmd_GradColor(0xffff00);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "zxcvbnm");//highlight the z button
				yOffset += ButtonH + yBtnDst;
				FT_GC_Tag(' ');
				if(' ' == CurrTag)
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
				else
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
			} else if (shiftState == 2) {
				yOffset = 80 + 15;
			   // Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
				//FT_GC_Cmd_FGColor(0x404080);
				//FT_GC_Cmd_GradColor(0x00ff00);
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "QWERTYUIOP");
				//FT_GC_Cmd_GradColor(0x00ffff);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "ASDFGHJKL");
				//FT_GC_Cmd_GradColor(0xffff00);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "ZXCVBNM");//highlight the z button
				yOffset += ButtonH + yBtnDst;
				FT_GC_Tag(' ');
				if(' ' == CurrTag)
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
				else
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
			} else {
				yOffset = 80 + 15;
			   // Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
				//FT_GC_Cmd_FGColor(0x404080);
				//FT_GC_Cmd_GradColor(0x00ff00);
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "0123456789");
				//FT_GC_Cmd_GradColor(0x00ffff);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "-@#$%^&*(");
				//FT_GC_Cmd_GradColor(0xffff00);
				yOffset += ButtonH + yBtnDst;
				FT_GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), ")_+[]{}");//highlight the z button
				yOffset += ButtonH + yBtnDst;
				FT_GC_Tag(' ');
				if(' ' == CurrTag)
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
				else
				{
					FT_GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
				}
                
			}
            
			break;
		}

		case TOUCH_SIG: {

			touch = *((sTagXY *) mEvt->asInts[0]);


			if (touch.tag <= 126 && touch.tag >= 32) //|| touch.tag == 0)
			{
				CurrTag = touch.tag;
				switch(lineIndex) {
				case (LINE1):
						CurrTextIdx = getTextIndex(textLine[0].line1);

						if(CurrTextIdx >=9)
							break;
						else
							textLine[0].line1[CurrTextIdx] = CurrTag;
						break;
				case (LINE2):
						CurrTextIdx = getTextIndex(textLine[0].line2);
						if(CurrTextIdx >=9)
							break;
						else
							textLine[0].line2[CurrTextIdx] = CurrTag;
						break;
				case (LINE3):
						CurrTextIdx = getTextIndex(textLine[0].line3);
						if(CurrTextIdx >=9)
							break;
						else
							textLine[0].line3[CurrTextIdx] = CurrTag;
						break;
				}

				cursorCount = 26;
				cursor = false;

			}
			else if (touch.tag <= 3 && touch.tag >= 1)
			{
//				textOn = true;
				switch(touch.tag){
				case (LINE1):
					lineIndex = LINE1;
					strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
					strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
					break;
				case (LINE2):
					lineIndex = LINE2;
					strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
					strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
					break;
				case (LINE3):
					lineIndex = LINE3;
					strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
					strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
					break;
				default:
					break;
				}
			}
			else if (touch.tag == 0)
			{
				CurrTag = touch.tag;
			}
			else {

				switch (touch.tag) {
				case (BACK):

					pageId = 2;

					machineEvent evt;

					evt.asChars[0] = pageId;

					mParams->mEvt = &evt;

					publish(GOTO_PAGE_SIG, mParams);
					break;

				case (SW_OPTIONS):
					pageId = 4;

					//machineEvent evt;

					evt.asChars[0] = pageId;

					mParams->mEvt = &evt;

					publish(GOTO_PAGE_SIG, mParams);
					break;
                    

				case (DEL):
				switch(lineIndex) {
						case (LINE1):
								strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
								CurrTextIdx = getTextIndex(textLine[0].line1);
								textLine[0].line1[CurrTextIdx] = CurrTag;
								if(CurrTextIdx > 0) {
									CurrTextIdx--;
								}

								textLine[0].line1[CurrTextIdx] = 0 ;
								break;
						case (LINE2):
								strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
								CurrTextIdx = strlen(textLine[0].line2);
								textLine[0].line2[CurrTextIdx] = CurrTag;
								if(CurrTextIdx > 0) {
									CurrTextIdx--;
								}

								textLine[0].line2[CurrTextIdx] = 0 ;
								break;
						case (LINE3):
								strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
								CurrTextIdx = strlen(textLine[0].line3);
								textLine[0].line3[CurrTextIdx] = CurrTag;
								if(CurrTextIdx > 0) {
									CurrTextIdx--;
								}

								textLine[0].line3[CurrTextIdx] = 0 ;
								break;
						}
					break;

				case (SHIFT):
					shiftState++;
					if(shiftState > 3){
						shiftState = 1;
					}
//					myPrintf("%d", shiftState);
					break;
                    
                case (ICON_TOGGLE):     

                    if(iconIsOn[configureIndex + (sourceAdrMask * 8)])
                    {
                        iconIsOn[configureIndex + (sourceAdrMask * 8)] = 0;
                    }
                    else
                    {
                        iconIsOn[configureIndex + (sourceAdrMask * 8)] = 1;
                    }
                
                    
                    
                    eepromLocalArray[IS_ICON_OFFSET + configureIndex + (sourceAdrMask * 8)] = iconIsOn[configureIndex + (sourceAdrMask * 8)] ;
                    
                    saveSettingRow((IS_ICON_OFFSET)/CY_FLASH_SIZEOF_ROW);        
                    
    				break;
				case (SAVE):
					strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
					strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
					strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));

					buttonLabels[configureIndex + (sourceAdrMask * 8)] = textLine[0];

					myPrintf("save \n");
//					myPrintf("%d \n", sizeof(buttonLabels[configureIndex + (sourceAdrMask * 8)]));

					///EEPROM_writeAnything(BUTTON_LABELS_OFFSET + ((configureIndex + (sourceAdrMask * 8)) * 30),buttonLabels[configureIndex + (sourceAdrMask * 8)]);
                    
                    uint8 *tempArray = (uint8 *) buttonLabels;
                    
                    uint16_t tempPos = ((configureIndex + (sourceAdrMask * 8)) * 30);
                    
//                    myPrintf("CI %d SAM %d TP %d ", configureIndex, sourceAdrMask, tempPos);
                    
                    for(unsigned int i = 0 ; i < 30 ; i ++)
                    {
                        eepromLocalArray[BUTTON_LABELS_OFFSET + tempPos + i] = tempArray[tempPos + i];
                    }
                    
                    saveSettingRow((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW);          // save the first row
                    
//                    myPrintf("row1: %d row2: %d \n", ((BUTTON_LABELS_OFFSET + tempPos)/128), ((BUTTON_LABELS_OFFSET + tempPos + 29)/128));
                    
                    if(((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW) != ((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW))
                    {
                        saveSettingRow((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW);     // if it wraps around to the next, save the second row
                    }
                    
					break;

				}
            
			}

			break;
		}

		}

	return (void *) mainIdle;
}

