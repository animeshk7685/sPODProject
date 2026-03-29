#include <GradationControl.h>
#include <LED.h>
#include <LEDDriver.h>
#include <PCA9955B.h>
#include <PCA9956B.h>
#include <PCA9957.h>

#if 0
/*
 * settingPage.h
 *
 *  Created on: Oct 22, 2014
 *      Author: drewrawlings
 */

#include "touchscreen.h"
#include "image_data.h"

static const char* TAG = "SETTINGS";

#define SLIDER_X 160
#define SLIDER_Y 250
#define SLIDER_W 140
#define SLIDER_H 12

#define SWITCH_X 98
#define SWITCH_Y 62

#define SET_SWITCH_X 45
#define SET_SWITCH_Y 45
#define SET_SWITCH_W 49
#define SET_SWITCH_H 49


static sTagXY touch;
static sTagXY nullTag;
static uint16_t bSliderValue = 0;
static uint16_t tSliderValue = 52572;
static sTrackTag track;
static int offset = -20;
static uint32_t resetHeld = 0;
static bool resetSent = false;
static bool screenTimerOn = 0;
static int lockoutHeld = 0;
static char strSelectedSw[] = "Linking to Source 0 Switch 0";
static char strLinkedSw[] = "01234567 89012345 67890123 45678901";

static int linkStatus = 0;
static int linkIndex = -1;
static bool isLinked[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};


int iSqrt(int x)
{
    int i;
    
    for (i = 0; i <= 256 ; i++) {
        if ((i * i) > x) {
            return (i - 1); 
        }
    }
    
    return i;
}


void endLinking(bool isClearStat)
{
    if (isClearStat) linkStatus = 0;
    
    for (int i = 0; i < 32; i++) {
    	isLinked[i] = 0;
    }
    
    linkIndex = -1;
}


static void sig_init(machineParams* mParams, machineEvent* mEvt)
{
    LOGE(TAG, "%s()...\n", __FUNCTION__); Serial.flush();
    touch.tag = 0;
    touch.x = 0;
    touch.y = 0;

    bSliderValue = 65535 - (settings.dayBrightness * 511);
    if(bSliderValue < 0)
        bSliderValue = 0;
    
    
    tSliderValue = 65535 - (iSqrt(settings.sleepTimer / 236) * 256);
    if(tSliderValue < 0)
        tSliderValue = 0;
    
    nullTag.tag = 0;
    
    myPrintf("settings page\n");
    currentPage = onSettingsPage;

    
    GC_Cmd_Track(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, BRIGHT_SLIDER);
    GD.finish();

    simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
    linkStatus = 0;
    endLinking(1);
    
    resetHeld = 0;
}


static void frame_tick_sig(machineParams* mParams, machineEvent* mEvt)
{
    int16_t switchX = SET_SWITCH_X;
    int16_t switchY = SET_SWITCH_Y;

    GC_ClearColorRGB_int(settings.pageBgColor);
    GC_Clear(/*1, 1, 1*/);

    GC_TagMask(1);
    drawTriText( switchX + offset, -25, 0, 55, 26, "Select", "", "", false, 0, settings.color, true);
    drawTriText( switchX + offset, -10, 0, 55, 26, "Switch", "", "", false, 0, settings.color, true);
    drawTriText( 147 + offset, 161, 0, 55, 26, "Select Source", "", "", false, 0, settings.color, true);
    
    GC_ColorRGB(196,196,196);
    GC_Begin(FT_LINE_STRIP);
    GC_LineWidth(1 *16);
    GC_Vertex2f((offset + 98)*16,205*16);
    GC_Vertex2f((offset + 272)*16,205*16);
    GC_End();
    
    drawButton2( 100 + offset, 210, 22, 22, 26, "1", SOURCE1, sourceAddr[0] || touch.tag==SOURCE1, settings.highlightColor);
    drawButton2( 130 + offset, 210, 22, 22, 26, "2", SOURCE2, sourceAddr[1] || touch.tag==SOURCE2, settings.highlightColor);
    drawButton2( 160 + offset, 210, 22, 22, 26, "3", SOURCE3, sourceAddr[2] || touch.tag==SOURCE3, settings.highlightColor);
    drawButton2( 190 + offset, 210, 22, 22, 26, "4", SOURCE4, sourceAddr[3] || touch.tag==SOURCE4, settings.highlightColor);
    
    drawButton2( 220 + offset, 210, SET_SWITCH_W, 22, 20, "Swipe on", SWIPE_TOGGLE, isSwipeOn, settings.highlightColor); 
    
    if(isPro)
    {
        drawButton2(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W), 4, SET_SWITCH_W, 26, 26, "Link", LINK, linkStatus || touch.tag == LINK, settings.highlightColor);
        drawButton2(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W) * 2, 4, SET_SWITCH_W, 26, 26, "Inputs", INPUTS, touch.tag == INPUTS, settings.highlightColor);
    }
    else
    {
        drawButton2(SET_SWITCH_X + offset + (SWITCH_SPACE + SET_SWITCH_W), 4, SET_SWITCH_W *2 + SWITCH_SPACE, 26, 26, 
            "Switch Linking", LINK, linkStatus || touch.tag == LINK, settings.highlightColor);
    }
    
    if(0 && linkStatus)
    {
        if(linkIndex < 0)// || switchIndex < 0)
        {
            drawTriText( 50 + offset, 125, 0, 55, 21, "Select switch to link", "", "", false, 0, settings.color, true);
        }
        else
        {
            uint8_t normSrc = linkIndex / 8 + 1;
            uint8_t normSw = linkIndex % 8 + 1;
            uint8_t swPos = sizeof(strSelectedSw) - 2;
            uint8_t srcPos = sizeof(strSelectedSw) - 11;
            
            strSelectedSw[srcPos] = '0' + normSrc;
            strSelectedSw[swPos] = '0' + normSw;
            
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
            drawButton2(SET_SWITCH_X + offset, 163, SET_SWITCH_W, 22, 20, "Locked", LOCKOUT_TOGGLE, true, settings.highlightColor); 
        }
        else
        {
            drawButton2(SET_SWITCH_X + offset, 163, SET_SWITCH_W, 22, 20, "Unlocked", LOCKOUT_TOGGLE, false, settings.highlightColor); 
        }
    
        drawButton2( SET_SWITCH_X + offset + 1*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
            "Pair", BLE_PAIR, pairOn || touch.tag == BLE_PAIR, settings.highlightColor);
        
        if(resetSent)
        {
            drawButton2( SET_SWITCH_X + offset + 2*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                "Reset", BLE_RESET, false, settings.highlightColor); 
        }
        else
        {
            drawButton2( SET_SWITCH_X + offset + 2*(SET_SWITCH_W + SWITCH_SPACE), 163, SET_SWITCH_W, 22, 20, 
                "Reset", BLE_RESET, false, settings.highlightColor); 
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
    
    LOGE(TAG, "%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
    GC_Tag(0);
    drawVersion( 315, 226, "", 20, (char*)VERSION);
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            switchX = SET_SWITCH_X + ((SET_SWITCH_W + SWITCH_SPACE) * j) + offset;
            switchY = SET_SWITCH_Y + ((SET_SWITCH_H + SWITCH_SPACE) * i) - 10;

            uint8_t switchID = SWITCH_0 + (i * 4) + j;
            uint8_t switchIndex = switchID -1;


                GC_Begin(FT_RECTS);
                GC_ColorRGB_int(settings.highlightColor);
                GC_LineWidth(5 *16);
                GC_Vertex2ii(switchX + 6, switchY + 6, 0 , 0);
                GC_Vertex2ii(switchX + SET_SWITCH_W - 3, switchY + SET_SWITCH_H - 3, 0 , 0);
                GC_End();
                drawButton( switchX, switchY, SET_SWITCH_W, SET_SWITCH_H, 20, 
                    buttonLabels[switchIndex + (sourceAdrMask * 8)].line1, 
                    buttonLabels[switchIndex + (sourceAdrMask * 8)].line2 , 
                    buttonLabels[switchIndex + (sourceAdrMask * 8)].line3, 
                    touch, switchID, isLinked[switchIndex + (sourceAdrMask * 8)]);
        }
    }
    drawButton( 3, 260, SWITCH_W, 35, 26, "Back", "", "", touch, BACK, false);
    if(screenTimerOn)
    {
        drawSlider( SLIDER_X, SLIDER_Y+5, SLIDER_W, SLIDER_H/*-15*/, tSliderValue, track, BRIGHT_SLIDER);
        GC_Tag(0);
        drawBacklightVal( 210 + offset, 4, "", 28, settings.sleepTimer / 60000, screenTimerOn);
    }
    else
    {
        drawSlider( SLIDER_X, SLIDER_Y+5, SLIDER_W, SLIDER_H/*-15*/, bSliderValue, track, BRIGHT_SLIDER);
        GC_Tag(0);
        drawBacklightVal( 230 + offset, 4, "", 28, settings.dayBrightness, screenTimerOn);
    }
    LOGE(TAG, "%s().%d\n", __FUNCTION__, __LINE__); Serial.flush();
}


static void touch_sig(machineParams* mParams, machineEvent* mEvt)
{
    touch = *((sTagXY *) mEvt->asInts[0]);

    if (touch.tag <= SWITCH_7 && touch.tag > 0)
    {
        uint8_t index = touch.tag - 1 + (sourceAdrMask * 8);

        if(index >= 32)
        {
            myPrintf("index out of range %d\n", index);
            return;
        }

        if(linkStatus)
        {
            if(linkIndex == -1)
            {
                linkIndex = index;
                isLinked[index] = 1;
                
                uint32_t link = switchIsLinked[linkIndex];
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
                isLinked[index] = !isLinked[index];
                if(isLinked[index]) {
                    switchIsLinked[linkIndex] = switchIsLinked[linkIndex] | (0x00000001 << index);
                } else {
                    switchIsLinked[linkIndex] = switchIsLinked[linkIndex] & ~(0x00000001 << index);
                }
                eepromLocalArray32[IS_LINKED_OFFSET/4 + linkIndex] = switchIsLinked[linkIndex];
            }
        }
        else
        {
            configureIndex = index % 8;
            newEvt.asChars[0] = 3;
            mParams->mEvt = &newEvt;
            publish(GOTO_PAGE_SIG, mParams);
        }
    } else {
        switch (touch.tag) {
        case (LINK):
            linkStatus = !linkStatus;
            if(!linkStatus && linkIndex > 0) {
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
            
            newEvt.asChars[0] = 1;
            mParams->mEvt = &newEvt;
            publish(GOTO_PAGE_SIG, mParams);
            
            break;
        case (INPUTS):
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

            newEvt.asChars[0] = 5;
            mParams->mEvt = &newEvt;
            publish(GOTO_PAGE_SIG, mParams);
            break;
        case (SOURCE1):
            sourceAdrMask = 0x00;
            updateSourceAddress();
            break;
        case (SOURCE2):
            sourceAdrMask = 0x01;
            updateSourceAddress();
            break;
        case (SOURCE3):
            sourceAdrMask = 0x02;
            updateSourceAddress();
            break;
        case (SOURCE4):
            sourceAdrMask = 0x03;
            updateSourceAddress();
            break;

        }
    }
}


static void track_sig(machineParams* mParams, machineEvent* mEvt)
{
    track = *((sTrackTag *) mEvt->asInts[0]);
    switch(track.tag)
    {
    case (BRIGHT_SLIDER):
        if(screenTimerOn)
        {
            tSliderValue = track.track;
            
            uint32_t slTime;
            
            slTime = (65535 - tSliderValue) / 256;
            
            settings.sleepTimer = (slTime * slTime) * 393 / 100000 * 60000;
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

            GD.wr32(REG_PWM_DUTY, settings.dayBrightness);
            
            eepromLocalArray[BRIGHTNESS_OFFSET] = settings.dayBrightness;

        }

        break;
    case (BLE_RESET):
        resetHeld++;
        if(resetHeld > 40)
            {
                resetHeld = 0;
                resetSent = true;
                
                newEvt.asChars[0] = BLE_RESET;
                mParams->mEvt = &newEvt;
                publish(BLE_PAIR_SIG, mParams);
            }
        break;
    case (LOCKOUT_TOGGLE):
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


stnext onSettingsPage(int signal, void* params)
{
    
    if (signal != SIG_NULL) {
        machineParams *mParams = (machineParams *) params;
        machineEvent *mEvt = (machineEvent *) mParams->mEvt;
        
        LOGE(TAG, "%s().%d: signal=%d\n", __FUNCTION__, __LINE__, signal); Serial.flush();
        switch (signal) {
        case SIG_INIT:       sig_init(mParams, mEvt); break;
        case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
        case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
        case TRACK_SIG:      track_sig(mParams, mEvt); break;
        }
    }
	return (void *) mainIdle;
}
#endif