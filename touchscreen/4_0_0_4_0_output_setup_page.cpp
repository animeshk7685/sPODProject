#include "touchscreen.h"
#include "image_data.h"
#include "config.h"

static const char* TAG = "OUTPG";

#define FONT 26
#define STATUS_FONT 30
#define ARROW_FONT 31

#define LINE1_Y 13
#define LINE2_Y 47
#define LINE3_Y 87
#define LINE4_Y 136
#define LINE5_Y 167
#define LINE6_Y 217

#define ARROW_W 35
#define ARROW_H 31

#define DL_SLIDER_X 82
#define DL_SLIDER_Y LINE4_Y
#define DL_SLIDER_W 100
#define DL_SLIDER_H 8

#define SF_SLIDER_X 300
#define SF_SLIDER_Y LINE4_Y
#define SF_SLIDER_W 100
#define SF_SLIDER_H 8


static sTagXY touch;
static sTrackTag track;
static bool updated = false;
static bool second8 = false;
static bool primary = true;
static circuit_output_type_t output_style = OUTPUT_UNUSED;
static uint8_t mask = 0x0F;
static uint8_t shift = 0;
static uint16_t dim_slider = 52572;  // TODO: should these be uint32_t?
static uint16_t freq_slider = 52572;  // TODO: should these be uint32_t?


static bool circuit_dirty()
{
	for (uint8_t pi = 0; pi < PCMS/*pcms*/; ++pi) {
		if (pcm_dirty(pi, &saved_pcm_config[pi], false)) return true;
	}
	return false;
}


static bool dirty()
{
	return switch_dirty(cfgindex, current_switch, &saved_switch_config) || circuit_dirty();
}


static void clear()
{
	for (uint8_t pcm = 0; pcm < PCMS/*pcms*/; ++pcm) {
		for (uint8_t ci = 0; ci < CIRCUITS; ++ci) {
			bool primary = (switch_config[cfgindex][current_switch].pcm[pcm].circuits[0]&(1<<ci)) != 0;
			bool secondary = (switch_config[cfgindex][current_switch].pcm[pcm].circuits[1]&(1<<ci)) != 0;
			uint8_t output = pcm_config[pcm].circuits[ci].output;

			if (primary || secondary) {
				if (primary) output &= ~OUTPUT_PRIMARY_MASK;
				if (secondary) output &= ~OUTPUT_SECONDARY_MASK;
				pcm_config[pcm].circuits[ci].output = output;
				send_pcm_config_pkt(PCM_UPDATE_CMD, pcm);	
			}
		}
	}
}


static void restore()
{
	if (dirty()) {
		for (uint8_t pi = 0; pi < PCMS/*pcms*/; ++pi) {
			if (pcm_dirty(pi, &saved_pcm_config[pi], false)) {
				read_pcm_config(pi, &pcm_config[pi]);
				send_pcm_config_pkt(PCM_RESTORE_CMD, pi);
			}			
		}
		memcpy(saved_pcm_config, pcm_config, sizeof(saved_pcm_config));

		read_switch_config(cfgindex, current_switch, &switch_config[cfgindex][current_switch]);
		saved_switch_config = switch_config[cfgindex][current_switch];
		send_switch_config_pkt(SWITCH_RESTORE, cfgindex, current_switch);
	}
}


static void apply()
{
	for (uint8_t pi = 0; pi < PCMS/*pcms*/; ++pi) {
		if (pcm_dirty(pi, &saved_pcm_config[pi], false)) {
			write_pcm_config(pi, &pcm_config[pi]);
			send_pcm_config_pkt(PCM_UPDATE_CMD, pi);
			send_pcm_config_pkt(PCM_APPLY_CMD, pi);
		}
	}
	memcpy(saved_pcm_config, pcm_config, sizeof(saved_pcm_config));

	write_switch_config(cfgindex, current_switch, &switch_config[cfgindex][current_switch]);
	send_switch_config_pkt(SWITCH_UPDATE, cfgindex, current_switch);
	send_switch_config_pkt(SWITCH_APPLY, cfgindex, current_switch);
	saved_switch_config = switch_config[cfgindex][current_switch];
	updated = true;
}


static void sync_pcm_outputs_to_switch()
{
	for (uint8_t pcm = 0; pcm < PCMS/*pcms*/; ++pcm) {
		for (uint8_t ci = 0; ci < CIRCUITS; ++ci) {
			bool primary = (switch_config[cfgindex][current_switch].pcm[pcm].circuits[0]&(1<<ci)) != 0;
			bool secondary = (switch_config[cfgindex][current_switch].pcm[pcm].circuits[1]&(1<<ci)) != 0;
			uint8_t output = pcm_config[pcm].circuits[ci].output;

			if (primary || secondary) {
				if (primary && (output & OUTPUT_PRIMARY_MASK) == OUTPUT_UNUSED) output |= OUTPUT_TOGGLE;
				if (secondary && (output & OUTPUT_SECONDARY_MASK) == OUTPUT_UNUSED) output |= (OUTPUT_TOGGLE<<4);
				pcm_config[pcm].circuits[ci].output = output;
			}
		}
		if (pcm_dirty(pcm, &saved_pcm_config[pcm], false)) {
			send_pcm_config_pkt(PCM_UPDATE_CMD, pcm);			
		}
	}
}


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	updated = false;
	currentPage = onOutputSetupPage;
	pkt_register_callback(NULL);
	memset(&touch, 0, sizeof(touch));
	sync_pcm_outputs_to_switch();

    GC_Cmd_Track(DL_SLIDER_X, DL_SLIDER_Y, DL_SLIDER_W, DL_SLIDER_H, BRIGHT_SLIDER);
    GC_Cmd_Track(SF_SLIDER_X, SF_SLIDER_Y, SF_SLIDER_W, SF_SLIDER_H, FREQUENCY_SLIDER);
    GD.finish();

	simple_hsm_transition_state(mParams->machine->hsm, canComm, mParams);
}


static const char* circuit_string(uint8_t circuit)
{
	switch (circuit) {
	case 0: return "1"; case 1: return "2"; case 2: return "3"; case 3: return "4";
	case 4: return "5"; case 5: return "6"; case 6: return "7"; case 7: return "8";
	case 8: return "X"; case 9: return "Y";

	case 10: return "9";  case 11: return "10"; case 12: return "11"; case 13: return "12";
	case 14: return "13"; case 15: return "14"; case 16: return "15"; case 17: return "16";
	case 18: return "V"; case 19: return "W";
	}

	return "??";
}


static void draw_circuits(int16_t y, bool second8)
{
	uint8_t offset = second8? CIRCUITS/2 : 0;

	for (uint16_t circuit = 0; circuit < CIRCUITS/2; ++circuit) {
		uint8_t index = offset + circuit;
		uint32_t on_color = 0x000000;
		uint8_t shift = primary? 0:4;
		uint8_t outputs = pcm_config[current_pcm].circuits[index].output;

		outputs = (outputs >> shift) & OUTPUT_PRIMARY_MASK;
		switch (outputs) {
		case OUTPUT_TOGGLE: on_color = 0x00FF00; break;
		case OUTPUT_STROBE: on_color = 0x0000FF; break;
		case OUTPUT_WIG:    on_color = 0x808000; break;
		case OUTPUT_WAG:    on_color = 0x008080; break;
		}
		drawButton2(82+(circuit*33), y, 33, 31, 26, circuit_string(index), CIRCUIT_TAG + index, on_color != 0, on_color);
	}
}


static void frame_tick_sig(machineParams *mParams, machineEvent *mEvt)
{
	GC_ClearColorRGB_int(settings.pageBgColor);
	GC_ClearCST(1, 1, 1);
	GC_TagMask(1);
	
	if (updated) {
		drawButton2(0,   LINE6_Y, 80, 50, FONT, "Back", BACK_TAG, false, 0);
		drawButton2(80,  LINE6_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
		drawButton2(160, LINE6_Y, 80, 50, FONT, "", ADVANCED_TAG, false, 0);
		drawButton2(240, LINE6_Y, 80, 50, FONT, "", CLEAR_TAG, false, 0);
		drawButton2(320, LINE6_Y, 80, 50, FONT, "", TEST_TAG, false, 0);
		drawButton2(400, LINE6_Y, 80, 50, FONT, "", APPLY_TAG, false, 0);
		drawTextColor(80, LINE3_Y, STATUS_FONT,  "UPDATE COMPLETE", 0x00FF00);
	} else {
		// LINE1: < Configuration: #  Switch: #  PCM: # >
		drawTextColor(120, LINE1_Y, FONT, "Configuration:", settings.greyColor);
		drawText(226, LINE1_Y, FONT, "Switch:");
		drawText(292, LINE1_Y, FONT, "PCM:");
		/*if (current_pcm > 0)*/ drawButton2(80, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, "<", CONFIG_LEFT, touch.tag == CONFIG_LEFT, settings.highlightColor);
		/*if (current_pcm < pcms-1)*/ drawButton2(342, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, ">", CONFIG_RIGHT, touch.tag == CONFIG_RIGHT, settings.highlightColor);

		drawInt(208, LINE1_Y, FONT, 0x00FF00, cfgindex+1);
		drawInt(272, LINE1_Y, FONT, 0x00FF00, current_switch+1);
		drawInt(322, LINE1_Y, FONT, 0xFF0000, current_pcm+1);

		// LINE2: Activation: Primary|Secondary
		drawTextColor(5, LINE2_Y+5, FONT, "Activation:", settings.greyColor);
		drawButton2(82, LINE2_Y, 60, 31, FONT, "Primary", PRIMARY_TAG, primary, 0x00FF00);
		drawButton2(142, LINE2_Y, 75, 31, FONT, "Secondary", SECONDARY_TAG, !primary, 0x000FF);
		
		// LINE3: Output Style: On/OFF|Strobe|WIG|WAG
		drawTextColor(27, LINE3_Y, FONT, "Output", settings.greyColor);
		drawText(36, LINE3_Y+15, FONT, "Style:");
		drawButton2(82, LINE3_Y, 50, 31, FONT, "On/Off", ONOFF_TAG, output_style == OUTPUT_TOGGLE, 0x00FF00);
		drawButton2(132, LINE3_Y, 50, 31, FONT, "Strobe", STROBE_TAG, output_style == OUTPUT_STROBE, 0x0000FF);
		drawButton2(182, LINE3_Y, 50, 31, FONT, "WIG", WIG_TAG, output_style == OUTPUT_WIG, 0x808000);
		drawButton2(232, LINE3_Y, 50, 31, FONT, "WAG", WAG_TAG, output_style == OUTPUT_WAG, 0x008080);
		
		// LINE4: Dim Level: slider
		drawTextColor(10, LINE4_Y-3, FONT, "Dim Level:", settings.greyColor);
		drawSlider(DL_SLIDER_X, DL_SLIDER_Y, DL_SLIDER_W, DL_SLIDER_H, dim_slider, track, BRIGHT_SLIDER);
		
		if (output_style == OUTPUT_STROBE) {
			drawTextColor(300, LINE3_Y-3, FONT, "Strobe Frequency:", settings.greyColor);
			drawSlider(SF_SLIDER_X, SF_SLIDER_Y, SF_SLIDER_W, SF_SLIDER_H, dim_slider, track, BRIGHT_SLIDER);
		}

		if (output_style == OUTPUT_WIG || output_style == OUTPUT_WAG) {
			drawTextColor(300, LINE3_Y-3, FONT, "WIG/WAG Frequency:", settings.greyColor);
			drawSlider(SF_SLIDER_X, SF_SLIDER_Y, SF_SLIDER_W, SF_SLIDER_H, dim_slider, track, BRIGHT_SLIDER);
		}

		// LINE4: Circuit: 1 2 3 4 ..     X Y  ..or.. 9 10 11 12 ..  V W
		drawTextColor(29, LINE5_Y+5, FONT, "Circuit:", settings.greyColor);
		drawButton2(418, LINE5_Y, ARROW_W-3, ARROW_H-1, ARROW_FONT, ">", CIRCUIT_RIGHT, touch.tag == CIRCUIT_RIGHT, settings.highlightColor);
		draw_circuits(LINE5_Y, second8);

		// LINE6: button line -- Back Home Advanced Clear Test Apply
		drawButton2(0,   LINE6_Y, 80, 50, FONT, "Back", BACK_TAG, false, 0);
		drawButton2(80,  LINE6_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
		drawButton2(160, LINE6_Y, 80, 50, FONT, "", ADVANCED_TAG, false, 0);
		drawButton2(240, LINE6_Y, 80, 50, FONT, "Clear", CLEAR_TAG, false, 0);
		drawButton2(320, LINE6_Y, 80, 50, FONT, "Test", TEST_TAG, false, 0);
		drawButton2(400, LINE6_Y, 80, 50, FONT, "Apply", APPLY_TAG, false, 0);
		drawButton2(430, 0,   50, 40, FONT, "Help", HELP_TAG, false, 0);		
	}
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	machineEvent evt;
	
	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag >= CIRCUIT_TAG && touch.tag <= CIRCUIT_TAG20) {
		uint8_t index = touch.tag - CIRCUIT_TAG;
		uint32_t on_color = 0x000000;
		uint8_t mask = primary? 0x0F : 0xF0;
		uint8_t shift = primary? 0:4;
		uint8_t outputs = pcm_config[current_pcm].circuits[index].output;
		uint8_t other_nibble = outputs & ~mask;
		uint8_t active_nibble = (output_style) << shift;
		
		// LOGD(TAG, "%s(): CIRCUIT index=%d, outputs=%x, mask=%x, shift=%d\r\n", __FUNCTION__, index, outputs, mask, shift);
		if (primary) {
			if (switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) active_nibble |= PRIMARY_MOMENTARY;
		} else {
			if (switch_config[cfgindex][current_switch].inputs & DOUBLE_INPUT) active_nibble |= SECONDARY_DOUBLE;
		}
		outputs = other_nibble | active_nibble;
		pcm_config[current_pcm].circuits[index].output = outputs;

		if (outputs & OUTPUT_PRIMARY_MASK) {
			switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[0] |= (1<<index);
		} else {
			switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[0] &= ~(1<<index);
		}
		if (outputs & OUTPUT_SECONDARY_MASK) {
			switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[0] |= (1<<index);
		} else {
			switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[1] &= ~(1<<index);
		}
		LOGD(TAG, "%s(): index=%d, outputs=%x\r\n", __FUNCTION__, index, outputs);
	} else {	
		LOGD(TAG, "%s().%d: switch(touch.tag=%d)...\r\n", __FUNCTION__, __LINE__, touch.tag); Serial.flush();
		switch (touch.tag) {
		case CONFIG_LEFT:  current_pcm = (current_pcm-1)&(PCMS-1); break;
		case CONFIG_RIGHT: current_pcm = (current_pcm+1)&(PCMS-1); break;
		case CIRCUIT_RIGHT: second8 = !second8; shift = second8? 4 : 0; mask = second8? 0xF0:0x0F; break;
		case CLEAR_TAG: if (!updated) clear(); break;
		case ONOFF_TAG: output_style = output_style == OUTPUT_TOGGLE? OUTPUT_UNUSED : OUTPUT_TOGGLE; break;
		case STROBE_TAG: output_style = output_style == OUTPUT_STROBE? OUTPUT_UNUSED : OUTPUT_STROBE; break;
		case WIG_TAG: output_style = output_style == OUTPUT_WIG? OUTPUT_UNUSED : OUTPUT_WIG; break;
		case WAG_TAG: output_style = output_style == OUTPUT_WAG? OUTPUT_UNUSED : OUTPUT_WAG; break;
		case PRIMARY_TAG: primary = true; break;
		case SECONDARY_TAG: primary = false; break;

		case BACK_TAG:
			evt.asChars[0] = updated? CONFIG_SETUP_PAGE : SWITCH_SETUP_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;

		case HOME_TAG:
			if (!updated) restore();
			evt.asChars[0] = HOME_PAGE;
			mParams->mEvt = &evt;
			publish(GOTO_PAGE_SIG, mParams);
			break;
			
		case ADVANCED_TAG:
			break;
			
		case TEST_TAG:
			if (!updated) {
				sync_pcm_outputs_to_switch();
				send_switch_config_pkt(SWITCH_UPDATE, cfgindex, current_switch);
				evt.asChars[0] = TEST_PAGE;
				mParams->mEvt = &evt;
				publish(GOTO_PAGE_SIG, mParams);
			}
			break;
			
		case APPLY_TAG:
			if (!updated) apply();
			break;
		}
	}
}


static void track_sig(machineParams *mParams, machineEvent *mEvt)
{
    track = *((sTrackTag *) mEvt->asInts[0]);
    switch (track.tag) {
	case FREQUENCY_SLIDER: freq_slider = (100 * track.track) / 65536; break;
    case BRIGHT_SLIDER: dim_slider = (100 * track.track) / 65536; break;
    }
}


stnext onOutputSetupPage(int signal, void* params) 
{
	LOGV(TAG, "%s(signal=%d, params=%p)\r\n", __FUNCTION__, signal, params); Serial.flush();

	if (signal != SIG_NULL) {
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;
		LOGV(TAG, "%s(): switch(%d)...\r\n", __FUNCTION__, signal); Serial.flush();
		switch (signal) {
		case SIG_INIT:       sig_init(mParams, mEvt); break;
		case TOUCH_SIG:      touch_sig(mParams, mEvt); break;
		case TRACK_SIG:      track_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG: frame_tick_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}
#if 0
   drawTextColor(150, LINE6_Y, FONT, "Strobe Freq (Hz):", settings.greyColor);
    GD.cmd_number(273, LINE6_Y, FONT, 0, strobe_freq);
    GD.Tag(STROBE_FREQ_TAG);
    GD.cmd_scrollbar(SF_SLIDER_X, SF_SLIDER_Y, SF_SLIDER_W, SF_SLIDER_H, 0, strobe_freq, 10, 999);

    drawTextColor(150, LINE7_Y, FONT, "Wig/Wag Freq (Hz):", settings.greyColor);
    GD.cmd_number(273, LINE7_Y, FONT, 0, wigwag_freq);
    GD.Tag(WIGWAG_FREQ_TAG);
    GD.cmd_scrollbar(WW_SLIDER_X, WW_SLIDER_Y, WW_SLIDER_W, WW_SLIDER_H, 0, wigwag_freq, 10, 999);
#endif