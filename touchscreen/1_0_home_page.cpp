// 1_0_home_page.cpp: 
//
//Home Page, this is the initial page displayed. The operater
// can either select one of the buttons:
//  * Config Setup -- 2_0_0_config_setup_page.cpp
//  * Settings Help -- 2_0_1_settings_page.cpp
//  * Off (upper right hand corner)
//
//  Or, select one of the eight switches to activate circuit outputs.
//
#include "touchscreen.h"
#include "image_data.h"
#include "button_fsm.h"

#define FONT 26

#define SWITCH_X 98
#define SWITCH_Y 62

typedef struct {
	uint32_t count;
	uint32_t start;
	uint32_t color;
} BLINK_SWITCH_T;

static const char* TAG = "MAINPG";

static sTagXY touch;
static sTrackTag track;
static uint8_t setupHeld = 0;
static uint8_t sleepHeld = 0;
static uint32_t sPodLogoBMPSize;
static uint32_t iconBMPSize;
static uint32_t currentTimeout = 0;
static bool mainPgInit = true;
static bool logoInit = 1;

static BLINK_SWITCH_T blink[SWITCHES];


static void process_pkt_cb(uint8_t ptype, uint8_t* data, uint8_t leng)
{
  if (ptype == SWITCH_STATUS_PKT_TYPE) {
    switch_status_t status;
    memcpy(&status, data, sizeof(status));
    LOGD(TAG, "%d -- %s(): index=%x(%d), activation=%x, owner=%x, color=%x (visible=%d)\r\n", 
        millis(), __FUNCTION__, status.index, status.index, status.activation, status.owner, status.color, switch_is_visible(status.index));
    if (switch_is_visible(status.index)) {
        uint32_t color = switch_status[status.index].color;
        uint8_t index = status.index % SWITCHES;
        button_set_color(index, color);
    }
  }
}


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();
	
	logoInit = 1;
	if (currentPage == onHomePage) {
		LOGD(TAG, "%s(): currentPage == onHomePage\r\n", __FUNCTION__);
		logoInit = 0;
	}
	currentPage = onHomePage;
	LOGD(TAG, "%s(): logoInit=%d\r\n",__FUNCTION__, logoInit);

	current_switch = SWITCHES;
	pkt_register_callback(process_pkt_cb);
	button_fsm_init();
	memset((uint8_t*)&touch, 0, sizeof(touch));
	memset(blink, 0, sizeof(blink));

	if (logoInit) {
		static bool virgin = true;
		logoInit = 0;

		LOGD(TAG, "%s(): sPod_Bitmap_RawData copied to GD\r\n", __FUNCTION__);
		GC_Cmd_Memset(0, 0L, 1L * sPodLogoBMPSize + (iconBMPSize * 8));
		GC_Cmd_Inflate(0);
		GD.cmd_n((uint8_t*)sPod_Bitmap_RawData, 1L * SPOD_RAWDATA_LENGTH);
		if (virgin) { // TODO: very first time does not work
			virgin = false;
			GC_Cmd_Inflate(0);
			GD.cmd_n((uint8_t*)sPod_Bitmap_RawData, 1L * SPOD_RAWDATA_LENGTH);
		}
	}

	for (int i = 0 ; i < 8 ; i++) { 
		if (settings.iconIsOn[(8 * cfgindex) + i] != 0) {
			GC_Cmd_Memwrite((iconGpuBaseAddr + (i * iconBMPSize)), 1L * iconBMPSize);
			GD.cmd_n(button_Bitmap_RawData + 
				((8 * cfgindex) + i) * BUTTON_FLASH_RAWDATA_LENGTH, 1L * iconBMPSize);  
		}
	}

	GD.finish();

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
	LOGD(TAG, "main page init\r\n");
}


static void switch_on(uint8_t switchID, int16_t switchX, int16_t switchY)
{
	uint8_t switchIndex = switchID - 1;
	int bIndex = switchIndex + (cfgindex * 8);
	uint32_t color = switch_status[bIndex].color;

	GC_Begin(FT_RECTS);
	GC_Tag(0);
	GC_ColorRGB_int(settings.fgColor);
	GC_LineWidth(5 * 16);
	GC_Vertex2ii(switchX + 5, switchY + 5, 0, 0);
	GC_Vertex2ii(switchX + SWITCH_W - 2, switchY + SWITCH_H - 2, 0, 0);
	GC_End();

	if (settings.iconIsOn[bIndex] == 0) {   // switch on, icon off
		drawButtonSmall(switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, SWITCH_FONT,
			settings.buttonLabels[bIndex].line1,
			settings.buttonLabels[bIndex].line2,
			settings.buttonLabels[bIndex].line3,
			touch, switchID, color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
	} else {     // switch on, icon on
		float buttonIconScale = 0.8;
		float buttonIconScaleText = 0.55;

		drawButtonSmall( switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, SWITCH_FONT, 0, 0, 0,
			touch, switchID, color, (currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );

		if (settings.buttonLabels[bIndex].line1[0] == 0) {
			GC_ColorRGB_int(0x000000);

			drawImage(button_Bitmap_Header, switchX+5, switchY+3, buttonIconScale, buttonIconScale, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));

			GC_ColorRGB_int(settings.color);

			drawImage(button_Bitmap_Header, switchX+6, switchY+4, buttonIconScale, buttonIconScale, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
		} else {
			GC_ColorRGB_int(0x000000);

			drawImage(button_Bitmap_Header, switchX+14, switchY+2, buttonIconScaleText, buttonIconScaleText, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));

			GC_ColorRGB_int(settings.color);

			drawImage(button_Bitmap_Header, switchX+15, switchY+3, buttonIconScaleText, buttonIconScaleText, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));

			drawTriText(switchX+1, switchY+35, SWITCH_W, 20, SWITCH_FONT,
				settings.buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
		}
	}
}


static void switch_off(uint8_t switchID, int16_t switchX, int16_t switchY)
{
	uint8_t switchIndex = switchID - 1;
	int bIndex = switchIndex + (cfgindex * SWITCHES);
	uint32_t color = switch_status[bIndex].color;

	GC_Begin(FT_RECTS);
	GC_Tag(0);
	GC_ColorRGB_int(settings.highlightColor);
	GC_LineWidth(5 * 16);
	GC_Vertex2ii(switchX + 6, switchY + 6, 0, 0);
	GC_Vertex2ii(switchX + SWITCH_W - 3, switchY + SWITCH_H - 3, 0, 0);
	GC_End();

	#if 1
	if (blink[switchIndex].count > 0 && millis() - blink[switchIndex].start > 1000) {
		blink[switchIndex].count -= 1;
		blink[switchIndex].start = millis();
		color = blink[switchIndex].count&1? blink[switchIndex].color : 0x000000;
	}
	#endif
	if (settings.iconIsOn[bIndex] == 0) {   // switch off, icon off
		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT,
			settings.buttonLabels[bIndex].line1, settings.buttonLabels[bIndex].line2, settings.buttonLabels[bIndex].line3,
			touch, switchID, color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
	} else {     // switch off, icon on
		float buttonIconScale = 0.8;
		float buttonIconScaleText = 0.55;

		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT, 0, 0, 0,
			touch, switchID, color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );

		if (settings.buttonLabels[bIndex].line1[0] == 0) {
			drawImage(button_Bitmap_Header, switchX+5, switchY+3, buttonIconScale, buttonIconScale, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
		} else {
			drawImage(button_Bitmap_Header, switchX+14, switchY+2, buttonIconScaleText, buttonIconScaleText, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));

			drawTriText(switchX, switchY + 34, SWITCH_W, 20, SWITCH_FONT,
				settings.buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
		}
	}
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	if ((millis() - currentTimeout) > 2000) currentTimeout = 0;

	for (uint8_t button = 0; button < SWITCHES; ++button) {
		uint8_t idx = cfgindex*SWITCHES + button;
		if (button_fsm_tick(button, &switch_config[cfgindex][button], &switch_status[idx].color)) {
			send_switch_status_pkt(idx, button_get_activation_method(button), switch_status[idx].color);
		}
	}

	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);

	GC_TagMask(1);
	drawImage(sPod_Bitmap_Header, 6, 10, 0.7, 0.7, touch, SPOD_LOGO_TOUCH, 0);

	GC_Tag(0);
	drawTriText(275, 0, 0, 22, 20, "Battery Voltage (V)", "", "", false, 0, settings.greyColor, true);
	drawNum(370, 12, 20, 0xFF0000, 13.5f);

	GC_Tag(0);
	drawTriText(275, 20, 0, 22, 20, "Current Power (A)", "", "", false, 0, settings.greyColor, true);
	drawNum(370, 32, 20, 0xFF0000, 32.5f);

	drawButton(410, 7, SWITCH_W, 35, FONT, "Off", "", "", touch, SLEEP, false);

	uint8_t switchID = SWITCH_0;
	for (int row = 0; row < 2; row++) {
		int16_t switchX = SWITCH_X;
		int16_t switchY = SWITCH_Y + ((SWITCH_H + SWITCH_SPACE) * row);

		for (int column = 0; column < 4; column++, switchX += (SWITCH_W + SWITCH_SPACE)) {
			int index = (switchID-SWITCH_0) + (cfgindex * SWITCHES);
			if (switch_status[index].color) {
				switch_on(switchID++, switchX, switchY);
			} else {
				switch_off(switchID++, switchX, switchY);
			}
		}
	}

	GC_ColorRGB(196,196,196);
	GC_Begin(FT_LINE_STRIP);
	GC_LineWidth(1 *16);
	GC_Vertex2f(5*16,217*16);
	GC_Vertex2f(475*16,217*16);
	GC_End();

	drawButton(2, 226, 62, 42/*h*/, FONT, "Config", settings.isLockoutOn? "Locked" : "Setup", "", touch, CONFIG_SETUP, false);
	drawButton(410, 226, 62, 42/*h*/, FONT, "Settings", "Help", "", touch, SETTINGS, false);

	/*if (cfgindex > 0)*/ drawButton(98, 226, 35, 31, 31, "<", "", "", touch, CONFIG_LEFT, false);
	drawTriText(138, 212, 0, 42, 27, "Controller Configuration", "", "", false, 0, settings.greyColor, true);
	drawInt(320, 234, 27, 0xFF0000, cfgindex+1);
	/*if (cfgindex < configs-1)*/ drawButton(340, 226, 35, 31, 31, ">", "", "", touch, CONFIG_RIGHT, false);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	machineEvent evt;
	uint8_t idx;

	touch = *((sTagXY *) mEvt->asInts[0]);

	if (settings.isLockoutOn) {
		if (touch.tag == SETTINGS) setupHeld = 0;
		touch.tag = NO_TAG;
		LOGD(TAG, "%s(): isLockoutOn!\r\n", __FUNCTION__);
		return;
	}

	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag == NO_TAG) {
		if (current_switch < SWITCHES) {
			idx = cfgindex * SWITCHES + current_switch;
			if (button_release(current_switch, &switch_config[cfgindex][current_switch], &switch_status[idx].color)) {
				send_switch_status_pkt(idx, button_get_activation_method(current_switch), switch_status[idx].color);
			}
			current_switch = SWITCHES;
		}
	} else
	if (touch.tag > NO_TAG && touch.tag <= SWITCH_7) {
		current_switch = touch.tag - 1;
		idx = current_switch + (cfgindex * SWITCHES);
		if (button_press(current_switch, &switch_config[cfgindex][current_switch], &switch_status[idx].color)) {
			uint8_t activation = button_get_activation_method(current_switch);
			bool locked = (switch_status[idx].activation & LOCKED_INPUT) != 0;
			if (!locked && pcm_triggers(switch_triggers(idx, (activation&SECONDARY_INPUT) != 0))) {
				send_switch_status_pkt(idx, button_get_activation_method(current_switch), switch_status[idx].color);			
			} else {
				LOGD(TAG, "%d -- %s(): locked(%d) or NEED TRIGGERS!\r\n", millis(), __FUNCTION__, locked);
				if (locked) {
					switch_status[idx].color = 0x00FF00;
				} else {
					blink[current_switch].count = 6;
					blink[current_switch].start = millis();
					blink[current_switch].color = 0xFF0000; // red
				}
				button_fsm_clear(current_switch, locked? 0x00FF00 : 0x000000);
			}
		}
	} else  {
		LOGD(TAG, "%s().%d: switch(touch.tag=%d)...\r\n", __FUNCTION__, __LINE__, touch.tag); Serial.flush();
		switch (touch.tag) {
		case CONFIG_LEFT:  cfgindex = (cfgindex-1)&(CONFIGS-1); break;
		case CONFIG_RIGHT: cfgindex = (cfgindex+1)&(CONFIGS-1); break;

		case SETTINGS:
			LOGE(TAG, "%s(): goto SETTINGS page\r\n", __FUNCTION__); Serial.flush();
			evt.asChars[0] = SETTINGS_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
	
		case CONFIG_SETUP:
			LOGE(TAG, "%s(): goto CONFIG_SETUP page\r\n", __FUNCTION__); Serial.flush();
			evt.asChars[0] = CONFIG_SETUP_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
		
		case SLEEP:
			sleepHeld = 1;
			break;
		}
	}
	
	if (touch.tag == NO_TAG && sleepHeld > 0) {
		LOGD(TAG, "%s(): touch.tag == 0 and sleepHeld > 0 --> DsiplaySleep(true)...\r\n", __FUNCTION__);
		sleepHeld = 0;
		DisplaySleep(true);
		CyDelay(1000);
	}
}


static void switch_status2_from_can_sig(machineParams *mParams, machineEvent *mEvt)
{
	uint8_t *receive_buffer = (uint8_t *) mEvt->asPtrs[0];
	uint8_t canIndex = indexPos(receive_buffer[1]);
	switchCurrent[canIndex + (cfgindex * 8)] = ((float) ((receive_buffer[3] << 8) | receive_buffer[4])) * CURRENT_FACTOR;
	currentTimeout = millis();
}


static void track_sig(machineParams *mParams, machineEvent *mEvt)
{
	track = *((sTrackTag *) mEvt->asInts[0]);

	if (settings.isLockoutOn) {
		if (track.tag == SETTINGS) {
			if (++setupHeld > 40) {
				machineEvent evt;
				evt.asChars[0] = SETTINGS_PAGE;
				mParams->mEvt = &evt;
				publish(GOTO_PAGE_SIG, mParams);
			}
		}
	}
}


stnext onHomePage(int signal, void* params) 
{
	LOGV(TAG, "%s(signal=%d, params=%p)\r\n", __FUNCTION__, signal, params); Serial.flush();

	if (signal != SIG_NULL) {
		sPodLogoBMPSize = sPod_Bitmap_Header->Stride * sPod_Bitmap_Header->Height;
		iconBMPSize = button_Bitmap_Header->Stride * button_Bitmap_Header->Height;
		iconGpuBaseAddr = sPodLogoBMPSize;
		
		if (mainPgInit && (!settings.wakeFromHib)) {
			mainPgInit = false;
			LOGD(TAG, "woke from rst\r\n");
		} else 
		if (settings.wakeFromHib) {
			mainPgInit = false;
			LOGD(TAG, "woke from ds\r\n");
			setWakeFromHib(false);  // set flag in flash
		}

		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;
		switch (signal) {
		case SIG_INIT:                    sig_init(mParams, mEvt); break;
		case TRACK_SIG:                   track_sig(mParams, mEvt); break;
		case TOUCH_SIG:                   touch_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG:              frame_tick_sig(mParams, mEvt); break;
		case SWITCH_STATUS2_FROM_CAN_SIG: switch_status2_from_can_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}