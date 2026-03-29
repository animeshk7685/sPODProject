/*
 * switchOptionsPage.h
 *
 *  Created on: Mar 11, 2015
 *      Author: drewrawlings
 */

#include "common.h"
#include "pubSub.h"

enum tSwOptionsPageButtonIds {
	SW_OPTIONS_PAGE_NONE, DIMABLE, MOMENTARY, STROBE, FLASH, SPODLOGO_TOUCH,
};

#define OPTIONS_X 240
#define OPTIONS_Y 55
#define OPTIONS_Y_SPACE 60

#define TEXT_X 200
#define TEXT_Y 5
#define TEXT_Y_SPACE 50

#define RADIO_X 190
#define RADIO_Y 70
#define RADIO_W 40
#define RADIO_H 40
#define RADIO_SPACE 60

//int iSqrt(int x)
//{
//    int i;
//    
//    for(i = 0; i <= 256 ; i++)
//    {
//        if((i * i) > x)
//        {
//            return (i - 1); 
//        }
//    }
//    
//    return i;
//}

uint8_t scaleSliderU8(uint16_t sliderVal)
{
    float stVal = (float)sliderVal / 65535.0;
                
    stVal = (stVal * stVal) * 254.0 + 1.0;
                
    return (uint8_t)stVal;
}

void needsCanUpdate(int index, uint8 isTest, void* params)
{
    if(!isPro)
        return;
    
    machineParams *mParams = (machineParams *) params;
//	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
    
    machineEvent evt;
    
    int swVal = switchVal[index];
    int swSta = switchStatus[index];
    int strOn = onTime_buffer[index];
    int strOff = offTime_buffer[index];
    
    if(isTest)
    {
        switchVal[index] = 0xFF;
        switchStatus[index] = 1;
        
        onTime_buffer[index] = switchStrobeOn[index];
        offTime_buffer[index] = switchStrobeOff[index];
    }
    else
    {
        onTime_buffer[index] = 255;
        offTime_buffer[index] = 0;
//        switchStatus[index] = 0;
        
        
    }
    
    evt.asPtrs[0] = (int) &switchVal;
	evt.asPtrs[1] = (int) &switchStatus;
	evt.asPtrs[2] = (int) &switchIsDimmable;
	evt.asPtrs[3] = (int) &onTime_buffer;
	evt.asPtrs[4] = (int) &offTime_buffer;

	evt.asChars[20] = index;

	mParams->mEvt = &evt;

	publish(SWITCH_STATUS_FROM_TS_SIG, mParams);
    
    switchVal[index] = swVal;
    switchStatus[index] = swSta;
    onTime_buffer[index] = strOn;
    offTime_buffer[index] = strOff;
    
}


stnext onSwitchOptionsPage(int signal, void* params) {

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
	//FT800IMPL_SPI *FTImpl = (FT800IMPL_SPI *) mParams->machine->obj;

	static sTagXY touch;
	static tButtonLabel curSwText[1] = { { "", "", "" } };

    static uint16_t onSliderValue = 0;
    static uint16_t offSliderValue = 0;
    static uint8_t lastStrobeOn = 0;
    static uint8_t lastStrobeOff = 0;
    
	static sTrackTag track;
    
    static uint8_t strobeTest = 0;
    
	char pageId;

//	int16_t TextFont = 29, ButtonW = 30, ButtonH = 30, yBtnDst = 5, yOffset,
//			xOffset;

//	static bool textOn = false;
//	uint8_t touchedTag = 0;

    int swIndex = configureIndex + (sourceAdrMask * 8);
    
	switch (signal) {
	case SIG_INIT: {
		touch.tag = 0;
		touch.x = 0;
		touch.y = 0;

		curSwText[0] = buttonLabels[configureIndex + (sourceAdrMask * 8)];

		myPrintf("switch options page\n");
		currentPage = onSwitchOptionsPage;

//        tSliderValue = 65535 - (iSqrt(settings.sleepTimer / 236) * 256);
        
        onSliderValue = iSqrt((switchStrobeOn[swIndex] - 1) * 65535 / 254) * 256;
        offSliderValue = iSqrt((switchStrobeOff[swIndex] - 1) * 65535 / 254) * 256;
        
        lastStrobeOn = switchStrobeOn[swIndex];
        lastStrobeOff = switchStrobeOff[swIndex];
        
        if(onSliderValue < 0)
            onSliderValue = 0;
        
        if(offSliderValue < 0)
            offSliderValue = 0;
        
        strobeTest = 0;
        
//        drawSlider( 210, 126, 90, 12, bSliderValue, track, ON_SLIDER);
//            
//            drawSlider( 210, 159, 90, 12, tSliderValue, track, OFF_SLIDER);
        
        FT_GC_Cmd_Track(210, 126, 90, 12, ON_SLIDER);
			FT_GC_Finish();
            
        FT_GC_Cmd_Track(210, 159, 90, 12, OFF_SLIDER);
			FT_GC_Finish();
        
//        simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
		simplehsm_transition_state(mParams->machine->hsm, canComm, params);

		break;
	}
	case FRAME_TICK_SIG: {
//		int32_t tagoption;

        
        
		FT_GC_ClearColorRGB_int(settings.pageBgColor);
		FT_GC_Clear(1, 1, 1);
		FT_GC_TagMask(1);

		//FT_GC_Tag(5);
		//drawImage( sPod_Bitmap_Header, 0, 0, 0.5, 0.5, touch, SPODLOGO_TOUCH, 0);

		FT_GC_Tag(0);
		drawTriText( 90, -5, 0, 55, 26, "Switch Feature Options", "", "",
				false, 0, settings.color, true);
		drawSingleTriText( 180, 190, 0, 55, 26, curSwText[0].line1,
				curSwText[0].line2, curSwText[0].line3, false, FT_OPT_CENTER,
				settings.color);
		//drawTriText( TEXT_X,  TEXT_Y, 0, 55, 26, curSwText[0].line1, curSwText[0].line2, curSwText[0].line3, false, 0, settings.color, true);

		drawTriText( (OPTIONS_X - 160), OPTIONS_Y, 0, 55, 26, "Dimmable",
				"", "", false, 0, settings.color, true);
		drawTriText( (OPTIONS_X - 160), OPTIONS_Y + OPTIONS_Y_SPACE, 0,
				55, 26, "Momentary", "", "", false, 0, settings.color, true);
//		drawTriText( OPTIONS_X, OPTIONS_Y, 0, 55, 26, "Strobe", "", "",
//				false, 0, settings.color, true);
//		drawTriText( OPTIONS_X, OPTIONS_Y + OPTIONS_Y_SPACE, 0, 55, 26,
//				"Flash", "", "", false, 0, settings.color, true);

		drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "",
				touch, BACK, false);

		drawButton( (RADIO_X - 160), RADIO_Y, RADIO_W, RADIO_H, 26, "",
				"", "", touch, DIMABLE,
				switchIsDimmable[configureIndex + (sourceAdrMask * 8)]);

		drawButton( (RADIO_X - 160), RADIO_Y + RADIO_SPACE, RADIO_W,
				RADIO_H, 26, "", "", "", touch, MOMENTARY,
				switchIsMomentary[configureIndex + (sourceAdrMask * 8)]);

//		drawButton( RADIO_X, RADIO_Y, RADIO_W, RADIO_H, 26, "", "", "",
//				touch, STROBE,
//				switchIsStrobe[configureIndex + (sourceAdrMask * 8)]);

//        drawButton( RADIO_X, RADIO_Y + RADIO_SPACE, RADIO_W, RADIO_H, 26,
//				"", "", "", touch, FLASH,
//				switchIsFlash[configureIndex + (sourceAdrMask * 8)]);
        
        if(isPro)
        {
            char onOffVals[] = "0.02s/0.08s";
            
            int onVal = switchStrobeOn[swIndex] * 2;
            int offVal = switchStrobeOff[swIndex] * 2;
            
            if(lastStrobeOn != switchStrobeOn[swIndex] || lastStrobeOff != switchStrobeOff[swIndex])
            {
                onSliderValue = iSqrt((switchStrobeOn[swIndex] - 1) * 65535 / 254) * 256;
                offSliderValue = iSqrt((switchStrobeOff[swIndex] - 1) * 65535 / 254) * 256;
                
                lastStrobeOn = switchStrobeOn[swIndex];
                lastStrobeOff = switchStrobeOff[swIndex];
                
                if(onSliderValue < 0)
                    onSliderValue = 0;
                
                if(offSliderValue < 0)
                    offSliderValue = 0;
            }
            
            onOffVals[0] = onVal / 100 + '0';
            onOffVals[2] = (onVal / 10) % 10 + '0';
            onOffVals[3] = onVal % 10 + '0';
            
            onOffVals[6] = offVal / 100 + '0';
            onOffVals[8] = (offVal / 10) % 10 + '0';
            onOffVals[9] = offVal % 10 + '0';
            
            char stTest[] = "Strobe (testing)";
            
            if(strobeTest){
                stTest[6] = ' ';
            }else{
                stTest[6] = '\0';
            }
            
            drawTriText( OPTIONS_X-20, OPTIONS_Y, 0, 55, 26, stTest, "     ON/OFF", onOffVals,
				false, 0, settings.color, false);
            
            drawButton( RADIO_X-20, RADIO_Y, RADIO_W, RADIO_H, 26, "", "", "",
				touch, STROBE,
				switchIsStrobe[swIndex]);
            
            drawTriText( 165, 96, 0, 55, 26,
				"ON", "", "", false, 0, settings.color, false);
            drawTriText( 165, 129, 0, 55, 26,
				"OFF", "", "", false, 0, settings.color, false);
            
//            drawSlider( 283, 25, 20, 189, bSliderValue, track, ON_SLIDER);
            drawSlider( 210, 126, 75, 12, onSliderValue, track, ON_SLIDER);
            
            drawSlider( 210, 159, 75, 12, offSliderValue, track, OFF_SLIDER);
            
        }
        else
        {
            drawTriText( OPTIONS_X, OPTIONS_Y, 0, 55, 26, "Strobe", "", "",
				false, 0, settings.color, true);
            drawTriText( OPTIONS_X, OPTIONS_Y + OPTIONS_Y_SPACE, 0, 55, 26,
				"Flash", "", "", false, 0, settings.color, true);
            
            drawButton( RADIO_X, RADIO_Y, RADIO_W, RADIO_H, 26, "", "", "",
				touch, STROBE,
				switchIsStrobe[configureIndex + (sourceAdrMask * 8)]);
    		drawButton( RADIO_X, RADIO_Y + RADIO_SPACE, RADIO_W, RADIO_H, 26,
    				"", "", "", touch, FLASH,
    				switchIsFlash[swIndex]);
        }

		FT_GC_Begin(FT_LINE_STRIP);
		FT_GC_LineWidth(1 * 16);
		FT_GC_Vertex2f(10 * 16, 185 * 16);
		FT_GC_Vertex2f(310 * 16, 185 * 16);
		FT_GC_End();

		FT_GC_Begin(FT_LINE_STRIP);
		FT_GC_LineWidth(1 * 16);
		FT_GC_Vertex2f(10 * 16, 55 * 16);
		FT_GC_Vertex2f(310 * 16, 55 * 16);
		FT_GC_End();

		break;
	}

	case TOUCH_SIG: {

		touch = *((sTagXY *) mEvt->asInts[0]);
		machineEvent evt;

//        int index = configureIndex + (sourceAdrMask * 8);
        
		switch (touch.tag) {
		case (BACK):

        needsCanUpdate(swIndex, 0, mParams);
        
        uint8_t lastVal = eepromLocalArray[SW_OPTIONS_OFFSET + swIndex];
        
        lastVal = lastVal & 0xf0;
        
        eepromLocalArray[SW_OPTIONS_OFFSET + swIndex] = lastVal |
            (switchIsMomentary[swIndex] ? IS_MOMENTARY_MASK : 0) |
            (switchIsDimmable[swIndex] ? IS_DIMABLE_MASK : 0) |
            (switchIsStrobe[swIndex] ? IS_STROBE_MASK : 0) | 
            (switchIsFlash[swIndex] ? IS_FLASH_MASK : 0);
        
                    
        myPrintf("sv sw %d: %x\n", swIndex, eepromLocalArray[SW_OPTIONS_OFFSET + swIndex]);
        
        if(isPro)
        {
            eepromLocalArray[STROBE_OFF_OFFSET + swIndex] = switchStrobeOff[swIndex];
            eepromLocalArray[STROBE_ON_OFFSET + swIndex] = switchStrobeOn[swIndex];
            
            if(STROBE_OFF_OFFSET / CY_FLASH_SIZEOF_ROW != SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW)
            {
                saveSettingRow(STROBE_OFF_OFFSET / CY_FLASH_SIZEOF_ROW);
            }
        }
        
//        saveSettingRow(IS_DIMABLE_OFFSET / CY_FLASH_SIZEOF_ROW);
        saveSettingRow(SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW);
        
			pageId = 3;

			evt.asChars[0] = pageId;

			mParams->mEvt = &evt;

			publish(GOTO_PAGE_SIG, mParams);
			break;

		case (DIMABLE):{

//			int index = configureIndex + (sourceAdrMask * 8);
			switchIsDimmable[swIndex] = !switchIsDimmable[swIndex];
                    
			    switchVal[swIndex] = 0;

                eepromLocalArray[DIM_VALUE_OFFSET + swIndex] = (uint8_t) switchVal[swIndex] / 255;

			break;
		}
		case (MOMENTARY):{
//			int index = configureIndex + (sourceAdrMask * 8);

			switchIsMomentary[swIndex] = !switchIsMomentary[swIndex];

//            eepromLocalArray[IS_MOMENTARY_OFFSET + index] = (uint8_t) switchIsMomentary[index]; 

			break;
		}
		case (STROBE): {
//			int index = configureIndex + (sourceAdrMask * 8);

			switchIsStrobe[swIndex] = !switchIsStrobe[swIndex];

			if (switchIsFlash[swIndex]) {
				switchIsFlash[swIndex] = false;
//                eepromLocalArray[IS_FLASH_OFFSET + index] = (uint8_t) switchIsFlash[index];

			}
            
            if(switchIsStrobe[swIndex])
            {
                strobeTest = 1;
            }
            else
            {
                strobeTest = 0;
            }
            
            needsCanUpdate(swIndex, strobeTest, mParams);
//            eepromLocalArray[IS_STROBE_OFFSET + index] = (uint8_t) switchIsStrobe[index];

			break;
		}
		case (FLASH):{
//			int index = configureIndex + (sourceAdrMask * 8);

			switchIsFlash[swIndex] = !switchIsFlash[swIndex];

			if (switchIsStrobe[swIndex]) {
				switchIsStrobe[swIndex] = false;
//                eepromLocalArray[IS_STROBE_OFFSET + index] = (uint8_t) switchIsStrobe[index];
			}
            
//            eepromLocalArray[IS_FLASH_OFFSET + index] = (uint8_t) switchIsFlash[index];

			break;
		}
		}
            
//        uint8_t lastVal = eepromLocalArray[SW_OPTIONS_OFFSET + swIndex];
//        
//        lastVal = lastVal & 0xf0;
//        
//        eepromLocalArray[SW_OPTIONS_OFFSET + swIndex] = lastVal |
//            (switchIsMomentary[swIndex] ? IS_MOMENTARY_MASK : 0) |
//            (switchIsDimmable[swIndex] ? IS_DIMABLE_MASK : 0) |
//            (switchIsStrobe[swIndex] ? IS_STROBE_MASK : 0) | 
//            (switchIsFlash[swIndex] ? IS_FLASH_MASK : 0);
//        
//                    
//                    myPrintf("sv sw %d: %x\n", swIndex, eepromLocalArray[SW_OPTIONS_OFFSET + swIndex]);
//        
////        saveSettingRow(IS_DIMABLE_OFFSET / CY_FLASH_SIZEOF_ROW);
//        saveSettingRow(SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW);
        
		break; 
	}
    case TRACK_SIG:
		{
			track = *((sTrackTag *) mEvt->asInts[0]);

//                myPrintf("%d \n", track.tag);
            
			switch(track.tag)
			{
			case (ON_SLIDER): {
                
                onSliderValue = track.track;
                
//                float stVal = (float)onSliderValue / 65535.0;
//                
//                stVal = (stVal * stVal) * 254.0 + 1.0;
//                
//                switchStrobeOn[swIndex] = (int)stVal;
                switchStrobeOn[swIndex] = scaleSliderU8(onSliderValue);
                
                onTime_buffer[swIndex] = switchStrobeOn[swIndex];
                eepromLocalArray[STROBE_ON_OFFSET] = switchStrobeOn[swIndex];
                
                
                if(strobeTest)
                {
                    needsCanUpdate(swIndex, strobeTest, mParams);
                }
                
				break;
			}
            case (OFF_SLIDER): {
					
                offSliderValue = track.track;
                
//                float stVal = (float)offSliderValue / 65535.0;
//                
//                stVal = (stVal * stVal) * 254.0 + 1.0;
//                
//                switchStrobeOff[swIndex] = (int)stVal;
                switchStrobeOff[swIndex] = scaleSliderU8(offSliderValue);
                
                offTime_buffer[swIndex] = switchStrobeOff[swIndex];
                //eepromLocalArray[STROBE_OFF_OFFSET + swIndex] = switchStrobeOff[swIndex];
                
                if(strobeTest)
                {
                    needsCanUpdate(swIndex, strobeTest, mParams);
                }
                
				break;
			}
		}
        break;
        }
	}

	return (void *) mainIdle;
}

