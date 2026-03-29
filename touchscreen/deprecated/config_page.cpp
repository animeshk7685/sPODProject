/*
 * configurePage.h
 *
 *  Created on: Oct 23, 2014
 *      Author: drewrawlings
 */

#include "touchscreen.h"
#include <math.h>

static const char* TAG = "CFGPG";

enum tConfigurePageButtonIds {
	CONFIGURE_PAGE_NONE, LINE1, LINE2, LINE3, DEL, SHIFT, SAVE, SW_OPTIONS//, SW_TEXT, INPUT_ENABLED, INPUT_LOCKOUT, INPUT_LOCK_INVERT
};

#define SWITCH_X 98
#define SWITCH_Y 62
#define SWITCH_W 62
#define SWITCH_H 70

#define SLIDER_X 160
#define SLIDER_Y 250
#define SLIDER_W 140
#define SLIDER_H 12

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

static const int16_t TextFont = 29;
static const int16_t ButtonW = 30;
static const int16_t ButtonH = 30;
static const int16_t yBtnDst = 5;

static sTagXY touch;
static tButtonLabel textLine[1] = {{"","",""}};
static uint8_t lineIndex = CONFIGURE_PAGE_NONE;
static int cursorCount = 0;
static bool cursor = true;


static uint8_t CurrTag = 0;//, lastTag=0;
static int32_t CurrTextIdx = 0;
static uint8_t shiftState = 1;


static void sig_init(machineParams* mParams, machineEvent* mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	touch.tag = 0;
	touch.x = 0;
	touch.y = 0;

	textLine[0] = buttonLabels[configureIndex + (sourceAdrMask * 8)];
	lineIndex = 0;

	LOGD(TAG, "configure page");
	currentPage = onConfigurePage;
	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void frame_tick_sig(machineParams* mParams, machineEvent* mEvt)
{
	int16_t yOffset;

	if (cursorCount <= 25) {
		cursorCount++;
	} else {
		char dash;

		cursorCount = 0;
		cursor = !cursor;
		dash = cursor? '|' : 0;

		switch (lineIndex) {
		case LINE1:
			CurrTextIdx = getTextIndex(textLine[0].line1);
			myPrintf("%d\n", CurrTextIdx);
			if(CurrTextIdx >=9)
				break;
			else
				textLine[0].line1[CurrTextIdx] = dash;
			break;
		case LINE2:
			CurrTextIdx = getTextIndex(textLine[0].line2);
			myPrintf("%d\n", CurrTextIdx);
			if(CurrTextIdx >=9)
				break;
			else
				textLine[0].line2[CurrTextIdx] = dash;
			break;
		case LINE3:
			CurrTextIdx = getTextIndex(textLine[0].line3);
			myPrintf("%d\n", CurrTextIdx);
			if(CurrTextIdx >=9)
				break;
			else
				textLine[0].line3[CurrTextIdx] = dash;
			break;
		}
	}

	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_Clear(/*1, 1, 1*/);

	GC_TagMask(1);

	drawTriText( 10, -10, 0, 55, 26, "Line 1:", "", "", false, 0, settings.color, true);
	drawTriText( 10,  15, 0, 55, 26, "Line 2:", "", "", false, 0, settings.color, true);
	drawTriText( 10,  40, 0, 55, 26, "Line 3:", "", "", false, 0, settings.color, true);

	drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);
	drawButton( 250, 165, 40, 30, 26, "Del", "", "", touch, DEL, false);
	drawButton( 12, 165, 47, 30, 26, "Shift", "", "", touch, SHIFT, false);
	drawButton( 250, 200, SWITCH_W, 35, 26, "Save", "", "", touch, SAVE, false);
	
	if (iconId[configureIndex + (sourceAdrMask * 8)] == 255) {
		drawButton( 215, 25, 65, 50, 26, "Switch", "Features", "", touch, SW_OPTIONS, false);
	} else {
		bool icon_on = iconIsOn[configureIndex + (sourceAdrMask * 8)];
		drawButton( 205, 45, 85, 40, 26, "Switch", "Features", "", touch, SW_OPTIONS, false);
		drawButton( 205, 10, 85, 25, 26, icon_on? "Icon ON" : "Icon OFF", "", "", touch, ICON_TOGGLE, false); 
	}
		
	for (int i = 0; i<3; i++) {
		char* line;

		switch(i){
		case 0: line = textLine[0].line1; break;
		case 1: line = textLine[0].line2; break;
		case 2: line = textLine[0].line3; break;
		}
		drawButton(LINE_X, LINE_Y + ((LINE_H + LINE_SPACE) * i), LINE_W, LINE_H, 26, line, "", "", touch, LINE1+i, false);
	}

	if (shiftState == 1) {
		yOffset = 80 + 15;
		// Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "qwertyuiop");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "asdfghjkl");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "zxcvbnm");//highlight the z button
		yOffset += ButtonH + yBtnDst;
		GC_Tag(' ');
		if (' ' == CurrTag) {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		} else {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		}
	} else if (shiftState == 2) {
		yOffset = 80 + 15;
		// Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "QWERTYUIOP");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "ASDFGHJKL");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "ZXCVBNM");//highlight the z button
		yOffset += ButtonH + yBtnDst;
		GC_Tag(' ');
		if (' ' == CurrTag) {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		} else {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		}
	} else {
		yOffset = 80 + 15;
		// Construct a simple keyboard - note that the tags associated with the keys are the character values given in the arguments 
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "0123456789");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), "-@#$%^&*(");
		yOffset += ButtonH + yBtnDst;
		GC_Cmd_Keys(yBtnDst, yOffset, 10*ButtonW, ButtonH, TextFont, (FT_OPT_CENTER | CurrTag), ")_+[]{}");//highlight the z button
		yOffset += ButtonH + yBtnDst;
		GC_Tag(' ');
		if (' ' == CurrTag) {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER | FT_OPT_FLAT, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		} else {
			GC_Cmd_Button(85, yOffset, 5*ButtonW, ButtonH, TextFont, FT_OPT_CENTER, " ");//mandatory to give '\0' at the end to make sure coprocessor understands the end of the string
		}
	}
}


static void tag_selected(machineParams* mParams, machineEvent* mEvt)
{
	static machineEvent evt;

	switch (touch.tag) {
	case BACK:
		evt.asChars[0] = 2;
		mParams->mEvt = &evt;
		publish(GOTO_PAGE_SIG, mParams);
		break;

	case SW_OPTIONS:
		evt.asChars[0] = 4;
		mParams->mEvt = &evt;
		publish(GOTO_PAGE_SIG, mParams);
		break;

	case DEL:
		switch(lineIndex) {
		case LINE1:
			strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
			CurrTextIdx = getTextIndex(textLine[0].line1);
			textLine[0].line1[CurrTextIdx] = CurrTag;
			if (CurrTextIdx > 0) CurrTextIdx--;
			textLine[0].line1[CurrTextIdx] = 0 ;
			break;
		case LINE2:
			strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
			CurrTextIdx = strlen(textLine[0].line2);
			textLine[0].line2[CurrTextIdx] = CurrTag;
			if (CurrTextIdx > 0) CurrTextIdx--;
			textLine[0].line2[CurrTextIdx] = 0 ;
			break;
		case LINE3:
			strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
			CurrTextIdx = strlen(textLine[0].line3);
			textLine[0].line3[CurrTextIdx] = CurrTag;
			if (CurrTextIdx > 0) CurrTextIdx--;
			textLine[0].line3[CurrTextIdx] = 0 ;
			break;
		}
		break;

	case SHIFT:
		if (++shiftState > 3) shiftState = 1;
		break;
		
	case ICON_TOGGLE:     
		iconIsOn[configureIndex + (sourceAdrMask * 8)] = !iconIsOn[configureIndex + (sourceAdrMask * 8)];
		eepromLocalArray[IS_ICON_OFFSET + configureIndex + (sourceAdrMask * 8)] = iconIsOn[configureIndex + (sourceAdrMask * 8)] ;
		saveSettingRow((IS_ICON_OFFSET)/CY_FLASH_SIZEOF_ROW);        
		break;
		
	case SAVE:
		strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
		strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
		strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));

		buttonLabels[configureIndex + (sourceAdrMask * 8)] = textLine[0];

		myPrintf("save \n");
		
		uint8_t *tempArray = (uint8_t *) buttonLabels;
		uint16_t tempPos = ((configureIndex + (sourceAdrMask * 8)) * 30);
		memcpy(&eepromLocalArray[BUTTON_LABELS_OFFSET + tempPos], &tempArray[tempPos], 30);
		saveSettingRow((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW);          // save the first row
		if (((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW) != ((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW)) {
			saveSettingRow((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW);     // if it wraps around to the next, save the second row
		}
		
		break;
	}
}


static void touch_sig(machineParams* mParams, machineEvent* mEvt)
{
	touch = *((sTagXY *) mEvt->asInts[0]);
	if (touch.tag <= 126 && touch.tag >= 32) {
		CurrTag = touch.tag;
		switch (lineIndex) {
		case LINE1:
			CurrTextIdx = getTextIndex(textLine[0].line1);
			if (CurrTextIdx >=9)
				break;
			textLine[0].line1[CurrTextIdx] = CurrTag;
			break;
		case LINE2:
			CurrTextIdx = getTextIndex(textLine[0].line2);
			if (CurrTextIdx >=9)
				break;
			textLine[0].line2[CurrTextIdx] = CurrTag;
			break;
		case LINE3:
			CurrTextIdx = getTextIndex(textLine[0].line3);
			if (CurrTextIdx >=9)
				break;
			textLine[0].line3[CurrTextIdx] = CurrTag;
			break;
		}

		cursorCount = 26;
		cursor = false;
	} else 
	if (touch.tag <= 3 && touch.tag >= 1) {
		switch (touch.tag) {
		case LINE1:
			lineIndex = LINE1;
			strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
			strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
			break;
		case LINE2:
			lineIndex = LINE2;
			strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
			strcpy(textLine[0].line3, strtok(textLine[0].line3,"|"));
			break;
		case LINE3:
			lineIndex = LINE3;
			strcpy(textLine[0].line2, strtok(textLine[0].line2,"|"));
			strcpy(textLine[0].line1, strtok(textLine[0].line1,"|"));
			break;
		default:
			break;
		}
	} else 
	if (touch.tag == 0) {
		CurrTag = touch.tag;
	} else {
		tag_selected(mParams, mEvt);
	}
}


stnext onConfigurePage(int signal, void* params) 
{
	if (signal != SIG_NULL) {
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;

		switch (signal) {
		case SIG_INIT:       sig_init(mParams, mEvt); break;
		case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
		case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}