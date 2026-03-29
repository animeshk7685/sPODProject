// 2_0_1_settings_page.cpp
//
// Settings Page, called by pressing "Settings Help" button from Home Page
//
// From this page the operator can select one of the buttons:
//   * Back
//   * Home
//   * Factory Reset
//   * More Info
//   * HOME
//

#include "touchscreen.h"
#include "image_data.h"
#include "config.h"

static const char* TAG = "SETTINGS";

#define FONT 26

#define LINE1_Y 8
#define LINE2_Y 45
#define LINE3_Y 70
#define LINE4_Y 100
#define LINE5_Y 123
#define LINE6_Y 147
#define LINE7_Y 177
#define LINE8_Y 217

#define ARROW_FONT 31
#define ARROW_W 70
#define ARROW_H 50

#define BL_SLIDER_X 160
#define BL_SLIDER_Y LINE3_Y+3
#define BL_SLIDER_W 160
#define BL_SLIDER_H 12

#define RED_SLIDER_X 355
#define RED_SLIDER_Y LINE2_Y+2
#define RED_SLIDER_W 100
#define RED_SLIDER_H 20

#define GRN_SLIDER_X RED_SLIDER_X
#define GRN_SLIDER_Y LINE3_Y+7
#define GRN_SLIDER_W RED_SLIDER_W
#define GRN_SLIDER_H RED_SLIDER_H

#define BLU_SLIDER_X RED_SLIDER_X
#define BLU_SLIDER_Y LINE4_Y+7
#define BLU_SLIDER_W RED_SLIDER_W
#define BLU_SLIDER_H RED_SLIDER_H


static sTagXY touch;
static sTrackTag track;

static uint16_t bSliderValue = 0xC000;
static uint16_t tSliderValue = 52572;
static int offset = -20;
static uint32_t resetHeld = 0;
static bool resetSent = false;
static bool screenTimerOn = 0;
static int lockoutHeld = 0;
static bool backlight_enabled = true;
static bool trigger_high = true;
static bool ignition_required = false;
static bool lockout = false;
static uint8_t password;
static uint32_t backlight_color;


static void set_touchscreen_brightness(uint32_t slider_value)
{
    settings.dayBrightness = (slider_value * 128) / 65536;
    if (settings.dayBrightness < 9)
        settings.dayBrightness = 9;
    else if(settings.dayBrightness > 120)
        settings.dayBrightness = 128;
    GD.wr32(REG_PWM_DUTY, settings.dayBrightness);
}


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	pkt_register_callback(NULL);
    currentPage = onSettingsPage;
	memset((uint8_t*)&touch, 0, sizeof(touch));

    set_touchscreen_brightness(bSliderValue);
    GC_Cmd_Track(BL_SLIDER_X, BL_SLIDER_Y, BL_SLIDER_W, BL_SLIDER_H, BRIGHT_SLIDER);
    GC_Cmd_Track(RED_SLIDER_X, RED_SLIDER_Y, RED_SLIDER_W, RED_SLIDER_H, BL_RED_TAG);
    GC_Cmd_Track(GRN_SLIDER_X, GRN_SLIDER_Y, GRN_SLIDER_W, GRN_SLIDER_H, BL_GRN_TAG);
    GC_Cmd_Track(BLU_SLIDER_X, BLU_SLIDER_Y, BLU_SLIDER_W, BLU_SLIDER_H, BL_BLU_TAG);
    GD.finish();

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);
	GC_TagMask(1);

	// LINE1: SETTINGS
	drawTextColor(200, LINE1_Y, FONT+2, "SETTINGS", settings.greyColor);
	//drawText(140, LINE1_Y, FONT, VERSION);
    //drawText(260, LINE1_Y, FONT, __DATE__);
    //drawText(350, LINE1_Y, FONT, __TIME__);

	// LINE2: Backlight Options:  Backlight Color:
	drawTextColor(15, LINE2_Y, FONT, "Backlight Options: ", settings.greyColor);
    GD.Tag(BACKLIGHT_TAG);
    GD.cmd_toggle(143, LINE2_Y+2, 30, FONT, 0, backlight_enabled, backlight_enabled? "On" : "Off");

	drawTextColor(360, LINE1_Y+10, FONT, "Backlight Color:", settings.greyColor);
    GD.Tag(BL_RED_TAG);
    GD.cmd_bgcolor(0x800000);
    GD.cmd_scrollbar(RED_SLIDER_X, RED_SLIDER_Y, RED_SLIDER_W, RED_SLIDER_H, 0, (backlight_color>>16)&0xFF, 10, 256);
    GD.Tag(BL_GRN_TAG);
    GD.cmd_bgcolor(0x008000);
    GD.cmd_scrollbar(GRN_SLIDER_X, GRN_SLIDER_Y, GRN_SLIDER_W, GRN_SLIDER_H, 0, (backlight_color>>8)&0xFF, 10, 256);
    GD.Tag(BL_BLU_TAG);
    GD.cmd_bgcolor(0x000080);
    GD.cmd_scrollbar(BLU_SLIDER_X, BLU_SLIDER_Y, BLU_SLIDER_W, BLU_SLIDER_H, 0, backlight_color&0xFF, 10, 256);
    GD.cmd_bgcolor(settings.bgColor);
	drawButton2(403, LINE5_Y+15, ARROW_W, ARROW_H, ARROW_FONT, "", CIRCUIT_RIGHT, true, backlight_color);

	// LINE3: Backlight Brightness
	drawTextColor(15, LINE3_Y, FONT, "Backlight Brightness:", settings.greyColor);
	drawSlider(BL_SLIDER_X, BL_SLIDER_Y, BL_SLIDER_W, BL_SLIDER_H, bSliderValue, track, BRIGHT_SLIDER);

    drawTextColor(15, LINE4_Y, FONT, "Trigger Activation Style: ", settings.greyColor);
    GD.Tag(TRIGGER_SIDE_TAG);
    GD.cmd_toggle(190, LINE4_Y, 75, FONT, 0, trigger_high, trigger_high? "High Side" : "Low Side");

    drawTextColor(15, LINE5_Y, FONT, "Ignition Control: ", settings.greyColor);
    GD.Tag(IGNITION_TAG);
    GD.cmd_toggle(130, LINE5_Y, 30, FONT, 0, ignition_required, ignition_required? "On" : "Off");

    drawTextColor(15, LINE6_Y, FONT, "Lock Out:", settings.greyColor);
    GD.Tag(LOCKOUT_TAG);
    GD.cmd_toggle(93, LINE6_Y, 30, FONT, 0, lockout, lockout? "On" : "Off");

    drawTextColor(15, LINE7_Y, FONT, "Password:", settings.greyColor);
    GD.Tag(PASSWORD_TAG);
    GD.cmd_toggle(93, LINE7_Y, 30, FONT, 0, password, password==0? "Off" : password==1? "On" : "Set");

	// LINE8: button line -- Back Home Advanced Clear Test Apply
	drawButton2(0,   LINE8_Y, 80, 50, FONT, "Back", BACK_TAG, false, 0);
	drawButton2(80,  LINE8_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
	drawButton(160,  LINE8_Y, 80, 50, FONT, "Factory", "Reset", "", touch, FACTORY_RESET_TAG, false);
	drawButton(320,  LINE8_Y, 80, 50, FONT, "More", "Info", "", touch, MORE_INFO_TAG, false);
	drawButton2(400, LINE8_Y, 80, 50, FONT, "Help", HELP_TAG, false, 0);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
    static machineEvent evt;
    
    touch = *((sTagXY *) mEvt->asInts[0]);
    LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
    switch (touch.tag) {
    case BACKLIGHT_TAG:    backlight_enabled = !backlight_enabled; break;
    case TRIGGER_SIDE_TAG: trigger_high = !trigger_high; break;
    case IGNITION_TAG:     ignition_required = !ignition_required; break;
    case LOCKOUT_TAG:      lockout = !lockout; break;
    case PASSWORD_TAG:     password = (password+1)%3; break;

    case BACK_TAG:
        evt.asChars[0] = HOME_PAGE;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;

    case HOME_TAG:
        evt.asChars[0] = HOME_PAGE;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;

    case FACTORY_RESET_TAG:
        evt.asChars[0] = FACTORY_RESET_PAGE;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;
        
    case MORE_INFO_TAG:
        evt.asChars[0] = MORE_INFO_PAGE;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;
        
    case HELP_TAG:
        evt.asChars[0] = HELP_PAGE;
        mParams->mEvt = &evt;
        publish(GOTO_PAGE_SIG, mParams);
        break;
    }
}


static void track_sig(machineParams* mParams, machineEvent* mEvt)
{
    uint32_t accum;

    track = *((sTrackTag *) mEvt->asInts[0]);
    switch(track.tag) {
    case BL_RED_TAG:
        accum = (256 * track.track) / 65536; 
        backlight_color = (backlight_color & 0x00FFFF) | (accum << 16);
        break;
    case BL_GRN_TAG:
        accum = (256 * track.track) / 65536; 
        backlight_color = (backlight_color & 0xFF00FF) | (accum << 8);
        break;
    case BL_BLU_TAG:
        accum = (256 * track.track) / 65536; 
        backlight_color = (backlight_color & 0xFFFF00) | accum;
        break;

    case BRIGHT_SLIDER:
        if(screenTimerOn) {
            tSliderValue = track.track;
            uint32_t slTime = (65535 - tSliderValue) / 256;
            settings.sleepTimer = (slTime * slTime) * 393 / 100000 * 60000;
        } else {
            bSliderValue = track.track;
            set_touchscreen_brightness(bSliderValue);
        }
        break;
    case LOCKOUT_TOGGLE:
        lockoutHeld++;
        if(!settings.isLockoutOn && lockoutHeld > 15) {
            myPrintf("Lock\n");
            settings.isLockoutOn = true;
        }
        break;
    }
}


stnext onSettingsPage(int signal, void* params)
{
	if (signal != SIG_NULL) {
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;

        switch (signal) {
		case SIG_INIT:       sig_init(mParams, mEvt); break;
		case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
		case TRACK_SIG:      track_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}