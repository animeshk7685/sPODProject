#include "touchscreen.h"
#include "image_data.h"
#include "button_fsm.h"

static const char* TAG = "TSTPG";

#define FONT 26
#define ARROW_FONT 31

#define LINE1_Y 13
#define LINE2_Y 30
#define LINE3_Y 111
#define LINE4_Y 141
#define LINE5_Y 170
#define LINE6_Y 217

#define ARROW_W 35
#define ARROW_H 31

#define SWITCH_X 98
#define SWITCH_Y 62
#define SWITCH_W 62
#define SWITCH_H 70


static sTagXY touch;
static uint32_t sPodLogoBMPSize;
static uint32_t iconBMPSize;
static uint32_t currentTimeout = 0;
static uint8_t switch_index;
static bool show_activation;
static char input_string[256];
static char triggers_string[256];
static char output_string[PCMS][256];



static void restore()
{
	read_switch_config(cfgindex, current_switch, &switch_config[cfgindex][current_switch]);
	test_switch.config_index = CONFIGS+1;
	test_switch.switch_index = SWITCHES+1;
	send_switch_config_pkt(SWITCH_RESTORE, cfgindex, current_switch);
}


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	memset((uint8_t*)&touch, 0, sizeof(touch));
	button_fsm_init();
	pkt_register_callback(NULL);

	show_activation = false;
	memset(input_string, 0, sizeof(input_string));
	memset(triggers_string, 0, sizeof(triggers_string));
	memset(output_string, 0, sizeof(output_string));

	switch_index = cfgindex*CONFIGS + current_switch;
	currentPage = onAutoTriggerTestPage;
	
	for (int i = 0 ; i < 8 ; i++) { 
		if (iconIsOn[(8 * cfgindex) + i] != 0) {
			GC_Cmd_Memwrite((iconGpuBaseAddr + (i * iconBMPSize)), 1L * iconBMPSize);
			GD.cmd_n(button_Bitmap_RawData + ((8 * cfgindex) + i) * BUTTON_FLASH_RAWDATA_LENGTH, 1L * iconBMPSize);  
		}
	}

	GD.finish();

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static void switch_on(uint8_t switchID, int16_t switchX, int16_t switchY)
{
	uint8_t switchIndex = switchID - 1;
	int bIndex = switchIndex + (cfgindex * 8);

	GC_Begin(FT_RECTS);
	GC_Tag(0);
	GC_ColorRGB_int(settings.fgColor);
	GC_LineWidth(5 * 16);
	GC_Vertex2ii(switchX + 5, switchY + 5, 0, 0);
	GC_Vertex2ii(switchX + SWITCH_W - 2, switchY + SWITCH_H - 2, 0, 0);
	GC_End();

	if (iconIsOn[bIndex] == 0) {   // switch on, icon off
		drawButtonSmall(switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, SWITCH_FONT,
			buttonLabels[bIndex].line1,
			buttonLabels[bIndex].line2,
			buttonLabels[bIndex].line3,
			touch, switchID, switch_status[bIndex].color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
	} else {     // switch on, icon on
		float buttonIconScale = 0.8;
		float buttonIconScaleText = 0.55;

		drawButtonSmall( switchX + 1, switchY + 1, SWITCH_W, SWITCH_H, SWITCH_FONT, 0, 0, 0,
			touch, switchID, switch_status[bIndex].color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );

		if (buttonLabels[bIndex].line1[0] == 0) {
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
				buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
		}
	}
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

	if (iconIsOn[bIndex] == 0) {   // switch off, icon off
		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT,
			buttonLabels[bIndex].line1, buttonLabels[bIndex].line2, buttonLabels[bIndex].line3,
			touch, switchID, switch_status[bIndex].color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );
	} else {     // switch off, icon on
		float buttonIconScale = 0.8;
		float buttonIconScaleText = 0.55;

		drawButtonSmall(switchX, switchY, SWITCH_W, SWITCH_H, SWITCH_FONT, 0, 0, 0,
			touch, switchID, switch_status[bIndex].color,
			(currentTimeout == 0 ? -1.0 : switchCurrent[bIndex]) );

		if (buttonLabels[bIndex].line1[0] == 0) {
			drawImage(button_Bitmap_Header, switchX+5, switchY+3, buttonIconScale, buttonIconScale, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));
		} else {
			drawImage(button_Bitmap_Header, switchX+14, switchY+2, buttonIconScaleText, buttonIconScaleText, touch,
				TEST_LOGO_TOUCH, iconGpuBaseAddr + (switchIndex * iconBMPSize));

			drawTriText(switchX, switchY + 34, SWITCH_W, 20, SWITCH_FONT,
				buttonLabels[bIndex].line1, 0, 0, false, FT_OPT_CENTER, settings.color, false);
		}
	}
}


static const char* trigger_string(uint8_t trigger)
{
	switch (trigger) {
	case 0: return "Trig1"; case 1: return "Trig2"; case 2: return "Trig3"; case 3: return "Trig4";
	case 4: return "Trig5"; case 5: return "Trig6"; case 6: return "IGN"; case 7: return "LoV";
	}

	return "??";
}


static const char* circuit_output_string(uint8_t output)
{
	switch (output & 7) {
	case OUTPUT_STROBE: return "STROBE";
	case OUTPUT_WIG:    return "WIG";
	case OUTPUT_WAG:    return "WAG";
	case OUTPUT_TOGGLE: 
	default:            return "TOGGLE";
	}
}


static const char* activation_to_string(uint8_t method) 
{
	switch (method) {
	case TOGGLE_INPUT:    return "TOGGLE";
	case MOMENTARY_INPUT: return "MOMENTARY";
	case DOUBLE_INPUT:    return "DOUBLE";
	case LONG_INPUT:      return "LONG";
	default:              return "";
	}
}


static void activation_input_string(uint8_t method, char* msg, uint32_t sizeof_msg) 
{
	msg[0] ='\0';
	if (method) {
		char* p = msg + snprintf(msg, sizeof_msg, "INPUT -- %s", activation_to_string(method));
	}
}


static void activation_triggers_string(uint8_t method, char* msg, uint32_t sizeof_msg) 
{
	msg[0] ='\0';
	if (method) {
		bool primary = method == TOGGLE_INPUT || method == MOMENTARY_INPUT;
		uint8_t mask = primary? PRIMARY_ACTIVATION : SECONDARY_ACTIVATION;
		bool havesome = false;
		for (uint8_t pcm = 0; pcm < PCMS && !havesome; ++pcm) {
			for (uint8_t trigger = 0; trigger < TRIGGERS && !havesome; ++trigger) {
				uint8_t triggers = (test_switch.pcm[pcm].triggers[trigger])&mask;
				havesome = triggers != 0;
			}
		}

		if (havesome) {
			char* p = msg + snprintf(msg, sizeof_msg, "TRIGS --");
			for (uint8_t pcm = 0; pcm < PCMS; ++pcm) {
				bool first = true;
				for (uint8_t trigger = 0; trigger < TRIGGERS; ++trigger) {
					uint8_t triggers = (test_switch.pcm[pcm].triggers[trigger])&mask;
					if (triggers) {
						if (first) {
							first = false;
							p += snprintf(p, sizeof_msg - (p-msg), " PCM%d", pcm+1);
						}
						p += snprintf(p, sizeof_msg - (p-msg), "&%s", trigger_string(trigger));
					}
				}
			}
		}
	}
}


static void activation_output_string(uint8_t method, uint8_t pcm, char* msg, uint32_t sizeof_msg) 
{
	msg[0] ='\0';
	if (method) {
		bool primary = method == TOGGLE_INPUT || method == MOMENTARY_INPUT;
		uint8_t shift = primary? 0 : 4;

		char* p = msg;
		uint8_t circuits = 0;
		for (uint8_t circuit = 0; circuit < CIRCUITS; ++circuit) {
			uint8_t outputs = (test_switch.pcm[pcm].outputs[circuit] >> shift) & 0x0F;
			if (outputs) circuits += 1;
		}
		if (circuits) {
			bool first = true;
			p += snprintf(p, sizeof_msg - (p-msg), "PCM%d  -- ", pcm+1);
			for (uint8_t circuit = 0; circuit < CIRCUITS; ++circuit) {
				uint8_t outputs = (test_switch.pcm[pcm].outputs[circuit] >> shift) & 0xF;
				if (outputs) {
					p += snprintf(p, sizeof_msg - (p-msg), first? "Circuit%d(%s)" : "&Circuit%d(%s)", circuit+1, circuit_output_string(outputs));
					first = false;
				}
			}
		}
	}
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	uint8_t index = cfgindex*8 + current_switch;
	uint32_t now = millis();
	bool activate = button_fsm_tick(current_switch, &test_switch, &switch_status[switch_index].color);
	if (activate) {
		send_switch_status_pkt(switch_index, button_get_activation_method(current_switch), switch_status[switch_index].color);
	}
	show_activation = show_activation || activate;

	if ((now - currentTimeout) > 2000) currentTimeout = 0;

	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);

	GC_TagMask(1);
	
	// LINE1: < Configuration: #  Switch: #  PCM: # >
	drawInt(208, LINE1_Y, FONT, 0x00FF00, cfgindex+1);
	drawInt(272, LINE1_Y, FONT, 0x00FF00, current_switch+1);
	drawTextColor(120, LINE1_Y, FONT, "Configuration:", settings.greyColor);
	drawText(226, LINE1_Y, FONT, "Switch:");
	int16_t x = 180;
	if (switch_status[index].color) {
		switch_on(SWITCH_0+current_switch, x, LINE2_Y);
	} else {
		switch_off(SWITCH_0+current_switch, x, LINE2_Y);
	}

	if (show_activation) {
		show_activation = false;
		uint8_t activation_method = button_get_activation_method(current_switch);
		activation_input_string(activation_method, input_string, sizeof(input_string));
		activation_triggers_string(activation_method, triggers_string, sizeof(triggers_string));
		for (uint8_t pcm = 0; pcm < PCMS; ++pcm) {
			activation_output_string(activation_method, pcm, output_string[pcm], sizeof(output_string[pcm]));
		}
	}

	drawTextColor(10, LINE3_Y-20, FONT, input_string, settings.greyColor);
	drawTextColor(10, LINE3_Y, FONT, triggers_string, settings.greyColor);
	for (uint8_t pcm = 0; pcm < PCMS; ++pcm) {
		drawTextColor(15, LINE3_Y+((pcm+1)*20), FONT, output_string[pcm], settings.greyColor);
	}

	// LINE6: button line -- Back Home Advanced Clear Test Apply
	drawButton2(0,   LINE6_Y, 80, 50, FONT, "Back", BACK_TAG, false, 0);
	drawButton2(80,  LINE6_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
	drawButton2(160, LINE6_Y, 80, 50, FONT, "", ADVANCED_TAG, false, 0);
	drawButton2(240, LINE6_Y, 80, 50, FONT, "", CLEAR_TAG, false, 0);
	drawButton2(320, LINE6_Y, 80, 50, FONT, "", TEST_TAG, false, 0);
	drawButton2(400, LINE6_Y, 80, 50, FONT, "Apply", APPLY_TAG, false, 0);
	drawButton2(430, 0,   50, 40, FONT, "Help", HELP_TAG, false, 0);
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	static machineEvent evt;
	bool activate;

	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag == NO_TAG) {
		LOGD(TAG, "%d -- %s(): NO_TAG --  current_switch=%d\r\n", millis(), __FUNCTION__, current_switch);
		activate = button_release(current_switch, &test_switch, &switch_status[switch_index].color);
		show_activation = show_activation || activate;
		if (activate) {
			send_switch_status_pkt(switch_index, button_get_activation_method(current_switch), switch_status[switch_index].color);			
		}
	} else
	if (touch.tag > NO_TAG && touch.tag <= SWITCH_7) {
		LOGD(TAG, "%d -- %s(): tag=%d, current_switch=%d\r\n", millis(), __FUNCTION__, touch.tag-1, current_switch);
		if (touch.tag-1 == current_switch) {
			activate = button_press(current_switch, &test_switch, &switch_status[switch_index].color);
			show_activation = show_activation || activate;
			if (activate) {
				send_switch_status_pkt(switch_index, button_get_activation_method(current_switch), switch_status[switch_index].color);			
			}
		}
	} else {
		LOGD(TAG, "%d -- %s().%d: switch(touch.tag=%d)...\r\n", millis(), __FUNCTION__, __LINE__, touch.tag); Serial.flush();
		switch (touch.tag) {
		case BACK_TAG:
			evt.asChars[0] = SWITCH_SETUP_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case HOME_TAG:
			restore();
			evt.asChars[0] = HOME_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case APPLY_TAG:
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
	switchMode[canIndex + (cfgindex * 8)] = receive_buffer[2];
	switchCurrent[canIndex + (cfgindex * 8)] = ((float) ((receive_buffer[3] << 8) | receive_buffer[4])) * CURRENT_FACTOR;
	currentTimeout = millis();
}


stnext onAutoTriggerTestPage(int signal, void* params) 
{
	LOGV(TAG, "%s(signal=%d, params=%p)\r\n", __FUNCTION__, signal, params); Serial.flush();

	if (signal != SIG_NULL) {
		sPodLogoBMPSize = sPod_Bitmap_Header->Stride * sPod_Bitmap_Header->Height;
		iconBMPSize = button_Bitmap_Header->Stride * button_Bitmap_Header->Height;
		iconGpuBaseAddr = sPodLogoBMPSize;
		
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;
		LOGV(TAG, "%s(): switch(%d)...\r\n", __FUNCTION__, signal); Serial.flush();
		switch (signal) {
		case SIG_INIT:                    sig_init(mParams, mEvt); break;
		case TOUCH_SIG:                   touch_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG:              frame_tick_sig(mParams, mEvt); break;
		case SWITCH_STATUS2_FROM_CAN_SIG: switch_status2_from_can_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}