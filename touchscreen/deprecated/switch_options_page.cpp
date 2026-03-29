/*
 * switchOptionsPage.h
 *
 *  Created on: Mar 11, 2015
 *      Author: drewrawlings
 */

#include "touchscreen.h"

enum tSwOptionsPageButtonIds {
	SW_OPTIONS_PAGE_NONE, DIMABLE, MOMENTARY, STROBE, FLASH, SPODLOGO_TOUCH,
};

#define SWITCH_X 98
#define SWITCH_Y 62

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


static sTagXY touch;
static tButtonLabel curSwText[1] = { { "", "", "" } };
static uint16_t onSliderValue = 0;
static uint16_t offSliderValue = 0;
static uint8_t lastStrobeOn = 0;
static uint8_t lastStrobeOff = 0;
static sTrackTag track;
static uint8_t strobeTest = 0;
static int swIndex;


uint8_t scaleSliderU8(uint16_t sliderVal)
{
    float stVal = (float)sliderVal / 65535.0;
                
    stVal = (stVal * stVal) * 254.0 + 1.0;
                
    return (uint8_t)stVal;
}


static void needsCanUpdate(int index, uint8_t isTest, void* params)
{
    if (!isPro) return;
    
    machineParams *mParams = (machineParams *) params;
    machineEvent evt;
    
    int swVal = switchVal[index];
    int swSta = switchStatus[index];
    int strOn = onTime_buffer[index];
    int strOff = offTime_buffer[index];
    
    if (isTest) {
        switchVal[index] = 0xFF;
        switchStatus[index] = 1;
        
        onTime_buffer[index] = switchStrobeOn[index];
        offTime_buffer[index] = switchStrobeOff[index];
    } else {
        onTime_buffer[index] = 255;
        offTime_buffer[index] = 0;
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


static void sig_init(machineParams* mParams, machineEvent* mEvt)
{
    touch.tag = 0;
    touch.x = 0;
    touch.y = 0;

    curSwText[0] = buttonLabels[configureIndex + (sourceAdrMask * 8)];

    myPrintf("switch options page\n");
    currentPage = onSwitchOptionsPage;
    onSliderValue = iSqrt((switchStrobeOn[swIndex] - 1) * 65535 / 254) * 256;
    offSliderValue = iSqrt((switchStrobeOff[swIndex] - 1) * 65535 / 254) * 256;
    
    lastStrobeOn = switchStrobeOn[swIndex];
    lastStrobeOff = switchStrobeOff[swIndex];
    
    if (onSliderValue < 0) onSliderValue = 0;
    if (offSliderValue < 0) offSliderValue = 0;
    
    strobeTest = 0;
    
    GC_Cmd_Track(210, 126, 90, 12, ON_SLIDER);
    GD.finish();
        
    GC_Cmd_Track(210, 159, 90, 12, OFF_SLIDER);
    GD.finish();
    
    simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void frame_tick_sig(machineParams* mParams, machineEvent* mEvt)
{
    GC_ClearColorRGB_int(settings.pageBgColor);
    GC_Clear(/*1, 1, 1*/);
    GC_TagMask(1);

    GC_Tag(0);
    drawTriText( 90, -5, 0, 55, 26, "Switch Feature Options", "", "",
            false, 0, settings.color, true);
    drawSingleTriText( 180, 190, 0, 55, 26, curSwText[0].line1,
            curSwText[0].line2, curSwText[0].line3, false, FT_OPT_CENTER,
            settings.color);

    drawTriText( (OPTIONS_X - 160), OPTIONS_Y, 0, 55, 26, "Dimmable",
            "", "", false, 0, settings.color, true);
    drawTriText( (OPTIONS_X - 160), OPTIONS_Y + OPTIONS_Y_SPACE, 0,
            55, 26, "Momentary", "", "", false, 0, settings.color, true);

    drawButton( SWITCH_X, 200, SWITCH_W, 35, 26, "Back", "", "",
            touch, BACK, false);

    drawButton( (RADIO_X - 160), RADIO_Y, RADIO_W, RADIO_H, 26, "",
            "", "", touch, DIMABLE,
            switchIsDimmable[configureIndex + (sourceAdrMask * 8)]);

    drawButton( (RADIO_X - 160), RADIO_Y + RADIO_SPACE, RADIO_W,
            RADIO_H, 26, "", "", "", touch, MOMENTARY,
            switchIsMomentary[configureIndex + (sourceAdrMask * 8)]);

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

    GC_Begin(FT_LINE_STRIP);
    GC_LineWidth(1 * 16);
    GC_Vertex2f(10 * 16, 185 * 16);
    GC_Vertex2f(310 * 16, 185 * 16);
    GC_End();

    GC_Begin(FT_LINE_STRIP);
    GC_LineWidth(1 * 16);
    GC_Vertex2f(10 * 16, 55 * 16);
    GC_Vertex2f(310 * 16, 55 * 16);
    GC_End();
}


static void touch_sig(machineParams* mParams, machineEvent* mEvt)
{
    machineEvent evt;
    uint8_t lastVal;

    touch = *((sTagXY *) mEvt->asInts[0]);

    switch (touch.tag) {
    case BACK:
        needsCanUpdate(swIndex, 0, mParams);
        
        lastVal = eepromLocalArray[SW_OPTIONS_OFFSET + swIndex] & 0xF0;
        eepromLocalArray[SW_OPTIONS_OFFSET + swIndex] = lastVal |
            (switchIsMomentary[swIndex] ? IS_MOMENTARY_MASK : 0) |
            (switchIsDimmable[swIndex] ? IS_DIMABLE_MASK : 0) |
            (switchIsStrobe[swIndex] ? IS_STROBE_MASK : 0) | 
            (switchIsFlash[swIndex] ? IS_FLASH_MASK : 0);
                    
        myPrintf("sv sw %d: %x\n", swIndex, eepromLocalArray[SW_OPTIONS_OFFSET + swIndex]);
        
        if (isPro) {
            eepromLocalArray[STROBE_OFF_OFFSET + swIndex] = switchStrobeOff[swIndex];
            eepromLocalArray[STROBE_ON_OFFSET + swIndex] = switchStrobeOn[swIndex];
            
            if (STROBE_OFF_OFFSET / CY_FLASH_SIZEOF_ROW != SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW) {
                saveSettingRow(STROBE_OFF_OFFSET / CY_FLASH_SIZEOF_ROW);
            }
        }
        
        saveSettingRow(SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW);
        evt.asChars[0] = 3;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;

    case DIMABLE:
        switchIsDimmable[swIndex] = !switchIsDimmable[swIndex];
        switchVal[swIndex] = 0;
        eepromLocalArray[DIM_VALUE_OFFSET + swIndex] = (uint8_t) switchVal[swIndex] / 255;
        break;

    case MOMENTARY:
        switchIsMomentary[swIndex] = !switchIsMomentary[swIndex];
        break;

    case STROBE:
        switchIsStrobe[swIndex] = !switchIsStrobe[swIndex];

        if (switchIsFlash[swIndex]) {
            switchIsFlash[swIndex] = false;
        }
        
        strobeTest = switchIsStrobe[swIndex] != 0;
        needsCanUpdate(swIndex, strobeTest, mParams);
        break;

    case FLASH:
        switchIsFlash[swIndex] = !switchIsFlash[swIndex];
        if (switchIsStrobe[swIndex]) {
            switchIsStrobe[swIndex] = false;
        }
        break;
    }
}


static void track_sig(machineParams* mParams, machineEvent* mEvt)
{
    track = *((sTrackTag *) mEvt->asInts[0]);
    switch(track.tag)
    {
    case (ON_SLIDER):
        onSliderValue = track.track;
        switchStrobeOn[swIndex] = scaleSliderU8(onSliderValue);
        
        onTime_buffer[swIndex] = switchStrobeOn[swIndex];
        eepromLocalArray[STROBE_ON_OFFSET] = switchStrobeOn[swIndex];
        
        if(strobeTest)
        {
            needsCanUpdate(swIndex, strobeTest, mParams);
        }
        break;

    case (OFF_SLIDER):
        offSliderValue = track.track;
        switchStrobeOff[swIndex] = scaleSliderU8(offSliderValue);
        
        offTime_buffer[swIndex] = switchStrobeOff[swIndex];
        
        if(strobeTest)
        {
            needsCanUpdate(swIndex, strobeTest, mParams);
        }
        break;
    }
}


stnext onSwitchOptionsPage(int signal, void* params)
{
	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;

    int swIndex = configureIndex + (sourceAdrMask * 8);
    
	switch (signal) {
	case SIG_INIT:       sig_init(mParams, mEvt); break;
	case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
	case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
    case TRACK_SIG:      track_sig(mParams, mEvt); break;
	}

	return (void *) mainIdle;
}