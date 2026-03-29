#include "touchscreen.h"
#include "image_data.h"

static const char* TAG = "FACTORY";

#define FONT 26
#define STATUS_FONT 30

#define LINE1_Y 13
#define LINE2_Y 60
#define LINE3_Y 81
#define LINE4_Y 112
#define LINE5_Y 170
#define LINE6_Y 217


static sTagXY touch;
static uint32_t last_time;
static uint16_t progress = 0;
static bool updating;


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	last_time = millis();
	touch.tag = 0;
	touch.x = 0;
	touch.y = 0;
	progress = 0;
	updating = false;
	currentPage = onFactoryResetPage;

	pkt_register_callback(NULL);
	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	static uint32_t color = 0xFF0000;

	if ((millis() - last_time) > 1000) {
		last_time = millis();
		color = color == 0xFF0000? 0x0000FF : 0xFF0000;
	}

	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);
	GC_TagMask(1);

	if (!updating) {
		drawTextColor(90, LINE3_Y, STATUS_FONT, "Press", settings.greyColor);
		drawTextColor(190, LINE3_Y, STATUS_FONT,  "Confirm", color);
		drawTextColor(40, LINE4_Y, STATUS_FONT,  "To restore factory defaults", settings.greyColor);
	} else {
		if (progress < 100) {
			progress = config_incremental_factory_defaults();
			GD.cmd_progress(110, LINE3_Y, 100, 20, 0, progress, 100);
			drawTextColor(100, LINE4_Y, STATUS_FONT, "  PLEASE WAIT", color);
		} else {
			static machineEvent evt;

			drawTextColor(80, LINE3_Y, STATUS_FONT,  "UPDATE  COMPLETE", 0x00FF00);
			send_config_pkt(CONFIG_FACTORY_DEFAULTS_CMD);

			evt.asChars[0] = SETTINGS_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
		}
	}

	// LINE6: button line -- Back Home Advanced Clear Test Apply
	drawButton2(0,   LINE6_Y, 80, 50, FONT, "Abort", ABORT_TAG, false, 0);
	drawButton2(80,  LINE6_Y, 80, 50, FONT, "", HOME_TAG, false, 0);
	drawButton2(160, LINE6_Y, 80, 50, FONT, "", ADVANCED_TAG, false, 0);
	drawButton2(240, LINE6_Y, 80, 50, FONT, "", CLEAR_TAG, false, 0);
	drawButton2(320, LINE6_Y, 80, 50, FONT, "", TEST_TAG, false, 0);
	drawButton2(400, LINE6_Y, 80, 50, FONT, "Confirm", CONFIRM_TAG, false, 0);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	static machineEvent evt;
	
	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	switch (touch.tag) {
	case CONFIRM_TAG:
		if (!updating) {
			updating = true;
			config_factory_defaults(true);
		}
		break;

	case ABORT_TAG:
		if (!updating) {
			evt.asChars[0] = SETTINGS_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
		}
		break;
	}
}


stnext onFactoryResetPage(int signal, void* params) 
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