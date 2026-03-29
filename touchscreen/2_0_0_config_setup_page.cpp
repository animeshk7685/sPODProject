// 2_0_0_config_setup_page.cpp
//
// Config Setup Page, called by pressing "Config Setup" button from Home Page
//
// From this page the operator can select one of the buttons:
//   * Automatic Triggers -- 3_0_0_0_auto_trigger_select_page.cpp
//   * Config Triggers    -- 3_0_0_1_config_triggers_page.cpp
//   * Config Circuit     -- 3_0_0_2_config_circuit_page.cpp
//   * Change Icon/Name   -- TODO
//   * HOME
//
//  Or, select one of the eight switches to configure triggers, circuits and circuit outputs.
//

#include "touchscreen.h"
#include "image_data.h"

static const char* TAG = "CSPG";


#define SWITCH_X 98
#define SWITCH_Y 62
#define SWITCH_W 62
#define SWITCH_H 70


static sTagXY touch;
static uint32_t sPodLogoBMPSize;
static uint32_t iconBMPSize;
static uint32_t currentTimeout = 0;
static uint32_t switchStatus[64];


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGV(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	touch.tag = 0;
	touch.x = 0;
	touch.y = 0;
	currentPage = onConfigSetupPage;
	pkt_register_callback(NULL);

	memset((uint8_t*)switchStatus, 0, sizeof(switchStatus));
	if (current_switch < SWITCHES) {
		switchStatus[cfgindex*CONFIGS + current_switch] = 0x00FFFF;
	} else {
		current_switch = 0;
	}

	for (int i = 0 ; i < 8 ; i++) { 
		if (settings.iconIsOn[(8 * cfgindex) + i] != 0) {
			GC_Cmd_Memwrite((iconGpuBaseAddr + (i * iconBMPSize)), 1L * iconBMPSize);
			GD.cmd_n(button_Bitmap_RawData + ((8 * cfgindex) + i) * BUTTON_FLASH_RAWDATA_LENGTH, 1L * iconBMPSize);  
		}
	}

	GD.finish();

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void switch_off(uint8_t switchID, int16_t switchX, int16_t switchY)
{
	uint8_t switchIndex = switchID - 1;
	int bIndex = switchIndex + (cfgindex * 8);

	GC_Begin(FT_RECTS);
	GC_Tag(0);
	GC_ColorRGB_int(settings.highlightColor);
	GC_LineWidth(5 * 16);
	GC_Vertex2ii(switchX + 6, switchY + 6, 0, 0);
	GC_Vertex2ii(switchX + SWITCH_W - 3, switchY + SWITCH_H - 3, 0, 0);
	GC_End();

	if (settings.iconIsOn[bIndex] == 0) {   // switch off, icon off
		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT,
			settings.buttonLabels[bIndex].line1, settings.buttonLabels[bIndex].line2, settings.buttonLabels[bIndex].line3,
			touch, switchID, switchStatus[bIndex],
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
	} else {     // switch off, icon on
		float buttonIconScale = 0.8;
		float buttonIconScaleText = 0.55;

		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT, 0, 0, 0,
			touch, switchID, switchStatus[bIndex],
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

	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);

	GC_TagMask(1);

	uint8_t switchID = SWITCH_0;
	for (int row = 0; row < 2; row++) {
		int16_t switchX = SWITCH_X;
		int16_t switchY = SWITCH_Y + ((SWITCH_H + SWITCH_SPACE) * row);

		for (int column = 0; column < 4; column++, switchX += (SWITCH_W + SWITCH_SPACE)) {
			switch_off(switchID++, switchX, switchY);
		}
	}

	/*if (cfgindex > 0)*/ drawButton(SWITCH_X, 14, 35, 31, 31, "<", "", "", touch, CONFIG_LEFT, false);
	drawTriText(155,2, 0, 22, 26, "Controller Configuration", "", "", false, 0, settings.greyColor, true);
	drawTriText(SWITCH_X+52, 28, 0, 15, 26, "Select switch to configure", "", "", false, 0, settings.greyColor, true);
	drawInt(313, 13, 26, 0xFF0000, cfgindex+1);
	/*if (cfgindex < configs-1)*/ drawButton(340, 14, 35, 31, 31, ">", "", "", touch, CONFIG_RIGHT, false);

	drawButton(0, 217, 120, 50, 26, "Automatic", "Triggers", "", touch, TRIGGERS_TAG, false);
	drawButton(120, 217, 120, 50, 26, "Config", "Triggers", "", touch, CONFIG_TRIGGERS_TAG, false);
	drawButton(240, 217, 120, 50, 26, "Change", "Icon/Name", "", touch, SWITCH_LABEL_TAG, false);
	drawButton(360, 217, 120, 50, 26, "HOME", "", "", touch, HOME_TAG, false);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	static machineEvent evt;
	
	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGV(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag > NO_TAG && touch.tag <= SWITCH_7) {
		current_switch = touch.tag - 1;
		evt.asChars[0] = SWITCH_SETUP_PAGE;
		mParams->mEvt = &evt;
		publish(GOTO_PAGE_SIG, mParams);
	} else {
		LOGV(TAG, "%s().%d: switch(touch.tag=%d)...\r\n", __FUNCTION__, __LINE__, touch.tag); Serial.flush();
		switch (touch.tag) {
		#if 1
		case CONFIG_LEFT:  cfgindex = (cfgindex-1)&(CONFIGS-1); break;
		case CONFIG_RIGHT: cfgindex = (cfgindex+1)&(CONFIGS-1); break;
		#else
		case CONFIG_LEFT:  if (cfgindex > 0) cfgindex -= 1; break;
		case CONFIG_RIGHT: if (cfgindex < configs-1) cfgindex += 1; break;
		#endif

		case TRIGGERS_TAG:
			evt.asChars[0] = TRIGGERS_SELECT_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case CONFIG_TRIGGERS_TAG:
			evt.asChars[0] = CONFIG_TRIGGERS_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case SWITCH_LABEL_TAG:
			evt.asChars[0] = SWITCH_LABEL_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case HOME_TAG:
			evt.asChars[0] = HOME_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
		}
	}
}


static void switch_status2_from_can_sig(machineParams *mParams, machineEvent *mEvt)
{
	uint8_t *receive_buffer = (uint8_t *) mEvt->asPtrs[0];
	uint8_t canIndex = indexPos(receive_buffer[1]);
	switchCurrent[canIndex + (cfgindex * 8)] = ((float) ((receive_buffer[3] << 8) | receive_buffer[4])) * CURRENT_FACTOR;
	currentTimeout = millis();
}


stnext onConfigSetupPage(int signal, void* params) 
{
	if (signal != SIG_NULL) {
		sPodLogoBMPSize = sPod_Bitmap_Header->Stride * sPod_Bitmap_Header->Height;
		iconBMPSize = button_Bitmap_Header->Stride * button_Bitmap_Header->Height;
		iconGpuBaseAddr = sPodLogoBMPSize;
		
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;

		switch (signal) {
		case SIG_INIT:                    sig_init(mParams, mEvt); break;
		case TOUCH_SIG:                   touch_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG:              frame_tick_sig(mParams, mEvt); break;
		case SWITCH_STATUS2_FROM_CAN_SIG: switch_status2_from_can_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}