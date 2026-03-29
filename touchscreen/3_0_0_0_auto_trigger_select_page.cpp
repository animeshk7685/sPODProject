// 3_0_0_0_auto_trigger_select_page.cpp
//
// Parent: 2_0_0_config_setup_page.cpp

#include "touchscreen.h"
#include "image_data.h"
#include "config.h"

static const char* TAG = "ATPG";

#define FONT 26
#define ARROW_FONT 31

#define LINE1_Y 13
#define LINE2_Y 60
#define LINE3_Y 81
#define LINE4_Y 112
#define LINE5_Y 170
#define LINE6_Y 217

#define ARROW_W 35
#define ARROW_H 31


static sTagXY touch;


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	pkt_register_callback(NULL);
	currentPage = onAutoTriggerSelectPage;
	memset((uint8_t*)&touch, 0, sizeof(touch));

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static const char* trigger_string(uint8_t trigger)
{
	switch (trigger) {
	case 0: return "1"; case 1: return "2"; case 2: return "3"; case 3: return "4";
	case 4: return "5"; case 5: return "6"; case 6: return "IGN"; case 7: return "LoV";
	}

	return "??";
}


static void draw_triggers()
{
	drawTextColor(22, LINE4_Y+3, FONT, "Trigger:", settings.greyColor);

	for (uint16_t trigger = 0; trigger < TRIGGERS; ++trigger) {
		uint32_t on_color = 0x000000;
		drawButton2(82+(trigger*33), LINE4_Y, 33, 31, 26, trigger_string(trigger), TRIGGER_TAG + trigger, on_color != 0, on_color);
	}
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);
	GC_TagMask(1);

	// LINE1: < PCM: # >
	drawTextColor(187, LINE1_Y, FONT, "PCM:", settings.greyColor);
	/*if (current_pcm > 0)*/ drawButton2(138, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, "<", CONFIG_LEFT, touch.tag == CONFIG_LEFT, settings.highlightColor);
	/*if (current_pcm < pcms-1)*/ drawButton2(237, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, ">", CONFIG_RIGHT, touch.tag == CONFIG_RIGHT, settings.highlightColor);
	drawInt(217, LINE1_Y, FONT, 0xFF0000, current_pcm+1);

	// LINE3:
	drawTextColor(80, LINE3_Y, FONT, "Select Auto Trigger:", settings.greyColor);

	// LINE3:
	draw_triggers();	

	// LINE6: button line -- Back Home Advanced Clear Test Apply
	drawButton2(0,   LINE6_Y, 80, 50, FONT, "Back", BACK_TAG, false, 0);
	drawButton2(80,  LINE6_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
	drawButton2(160, LINE6_Y, 80, 50, FONT, "", ADVANCED_TAG, false, 0);
	drawButton2(240, LINE6_Y, 80, 50, FONT, "", CLEAR_TAG, false, 0);
	drawButton2(320, LINE6_Y, 80, 50, FONT, "", TEST_TAG, false, 0);
	drawButton2(400, LINE6_Y, 80, 50, FONT, "", APPLY_TAG, false, 0);
	drawButton2(430, 0,       50, 40, FONT, "Help", HELP_TAG, false, 0);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	static machineEvent evt;
	
	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag >= TRIGGER_TAG && touch.tag <= TRIGGER_TAG8) {
		evt.asChars[0] = TRIGGERS_OUTPUT_PAGE;
		evt.asChars[1] = touch.tag - TRIGGER_TAG;
		evt.asChars[2] = current_pcm;
		mParams->mEvt = &evt;
		publish(GOTO_PAGE_SIG, mParams);			
	} else {
		switch (touch.tag) {
		case CONFIG_LEFT:  current_pcm = (current_pcm-1)&(PCMS-1); break;
		case CONFIG_RIGHT: current_pcm = (current_pcm+1)&(PCMS-1); break;
	
		case BACK_TAG:
			evt.asChars[0] = CONFIG_SETUP_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
	
		case HOME_TAG:
			evt.asChars[0] = HOME_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
	
		default:
			break;
		}
	}
}


stnext onAutoTriggerSelectPage(int signal, void* params) 
{
	if (signal != SIG_NULL) {
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;

		switch (signal) {
		case SIG_INIT:       sig_init(mParams, mEvt); break;
		case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}