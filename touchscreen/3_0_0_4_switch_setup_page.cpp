// 3_0_0_4_switch_setup_page.cpp
//
// Parent: 2_0_0_config_setup_page.cpp

#include "touchscreen.h"
#include "image_data.h"
#include "config.h"

static const char* TAG = "CSPG";

#define FONT 26
#define STATUS_FONT 30
#define ARROW_FONT 31

#define LINE1_Y 13
#define LINE2_Y 60
#define LINE3_Y 81
#define LINE4_Y 132
#define LINE5_Y 170
#define LINE6_Y 217

#define ARROW_W 35
#define ARROW_H 31

#define SWITCH_X 98
#define SWITCH_Y 62
#define SWITCH_W 62
#define SWITCH_H 70


static sTagXY touch;
static bool updated;
static bool second8 = false;
static uint8_t circuit_activation[PCMS][CIRCUITS]; // 0 -- off, 1 -- primary, 2 -- secondary, 3 -- both 


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


static void init_circuit_activation()
{
	uint8_t index = 0;
	for (uint8_t pcm = 0; pcm < PCMS; ++pcm) {
		for (uint8_t circuit = 0; circuit < CIRCUITS; ++circuit, ++index) {
			circuit_activation[pcm][circuit] = 0;
			if (switch_config[cfgindex][current_switch].pcm[pcm].circuits[0]&(1<<circuit)) {
				circuit_activation[pcm][circuit] |= PRIMARY_ACTIVATION;
			}
			if (switch_config[cfgindex][current_switch].pcm[pcm].circuits[1]&(1<<circuit)) {
				circuit_activation[pcm][circuit] |= SECONDARY_ACTIVATION;
			}
			if (pcm == 0 && circuit == 2) {
				LOGD(TAG, "%d -- %s(): switch_config[0][2].pcm[0].circuits[0]=%x, circuits[1]=%x\r\n",
					millis(), __FUNCTION__, switch_config[0][2].pcm[0].circuits[0], switch_config[0][2].pcm[0].circuits[1]);
			}
		}
	}
}


static void set_output_activation_bits()
{
	for (uint8_t pcm = 0; pcm < PCMS/*pcms*/; ++pcm) {
		for (uint8_t circuit = 0; circuit < CIRCUITS; ++circuit) {
			uint8_t output = pcm_config[pcm].circuits[circuit].output;
			uint8_t activation = circuit_activation[pcm][circuit];
			if ((activation & PRIMARY_ACTIVATION) == 0) {
				output &= ~OUTPUT_PRIMARY_MASK;
			} else {
				if ((output & OUTPUT_PRIMARY_MASK) == 0) output |= OUTPUT_TOGGLE;
				if (switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) output |= PRIMARY_MOMENTARY; else output &= ~PRIMARY_MOMENTARY;
			}
			if ((activation & SECONDARY_ACTIVATION) == 0) {
				output &= ~OUTPUT_SECONDARY_MASK;
			} else {
				if ((output & OUTPUT_SECONDARY_MASK) == 0) output |= (OUTPUT_TOGGLE<<4);
				if (switch_config[cfgindex][current_switch].inputs & DOUBLE_INPUT) output |= SECONDARY_DOUBLE; else output &= ~DOUBLE_INPUT;
			}
			if (output != pcm_config[pcm].circuits[circuit].output) {
				LOGD(TAG, "%d -- %s(): pcm_config[%d].circuit[%d].output %x --> %x\r\n", millis(), __FUNCTION__, pcm, circuit, pcm_config[pcm].circuits[circuit].output, output);
			}
			pcm_config[pcm].circuits[circuit].output = output;
		}
		if (pcm_dirty(pcm, &saved_pcm_config[pcm], false)) {
			send_pcm_config_pkt(PCM_UPDATE_CMD, pcm);			
		}
	}
}


static void clear()
{
	memset(circuit_activation, 0, sizeof(circuit_activation));
	memset(&switch_config[cfgindex][current_switch], 0, sizeof(switch_config[cfgindex][current_switch]));
	switch_config[cfgindex][current_switch].config_index = cfgindex;
	switch_config[cfgindex][current_switch].switch_index = current_switch;
	switch_config[cfgindex][current_switch].inputs = TOGGLE_INPUT;
	second8 = false;
	LOGD(TAG, "%s()\r\n", __FUNCTION__);
}


static void restore()
{
	if (dirty()) {
		for (uint8_t pi = 0; pi < PCMS/*pcms*/; ++pi) {
			if (pcm_dirty(pi, &saved_pcm_config[pi], true)) {
				read_pcm_config(pi, &pcm_config[pi]);
				send_pcm_config_pkt(PCM_RESTORE_CMD, pi);
				LOGD(TAG, "%d -- %s(): pcm_config[%d].circuits[%d].output=%x\r\n", 
					millis(), __FUNCTION__, pi, 2, pcm_config[pi].circuits[2].output);
			}			
		}
		memcpy(saved_pcm_config, pcm_config, sizeof(saved_pcm_config));

		read_switch_config(cfgindex, current_switch, &switch_config[cfgindex][current_switch]);
		saved_switch_config = switch_config[cfgindex][current_switch];
		send_switch_config_pkt(SWITCH_RESTORE, cfgindex, current_switch);
						
		LOGD(TAG, "%d -- %s(): switch_config[%d][%d].inputs = %x\r\n", 
				millis(), __FUNCTION__, cfgindex, current_switch, switch_config[cfgindex][current_switch].inputs);
	}
}


static void apply()
{
	for (uint8_t pi = 0; pi < PCMS/*pcms*/; ++pi) {
		if (pcm_dirty(pi, &saved_pcm_config[pi], true)) {
			write_pcm_config(pi, &pcm_config[pi]);
			send_pcm_config_pkt(PCM_UPDATE_CMD, pi);			
			send_pcm_config_pkt(PCM_APPLY_CMD, pi);
			LOGD(TAG, "%d -- %s(): pcm_config[%d].circuits[%d].output=%x\r\n", 
				millis(), __FUNCTION__, pi, 2, pcm_config[pi].circuits[2].output);
		}
	}
	memcpy(saved_pcm_config, pcm_config, sizeof(saved_pcm_config));

	write_switch_config(cfgindex, current_switch, &switch_config[cfgindex][current_switch]);
	send_switch_config_pkt(SWITCH_UPDATE, cfgindex, current_switch);
	send_switch_config_pkt(SWITCH_APPLY, cfgindex, current_switch);
	saved_switch_config = switch_config[cfgindex][current_switch];
	updated = true;

	LOGD(TAG, "%d -- %s(): switch_config[%d][%d].inputs = %x\r\n", 
		millis(), __FUNCTION__, cfgindex, current_switch, switch_config[cfgindex][current_switch].inputs);
}


static void init_switch()
{
	if (saved_switch_config.config_index != cfgindex || saved_switch_config.switch_index != current_switch) {
		LOGD(TAG, "%s(): saved_switch.ci=%d, ci=%d, saved_switch.si=%d, si=%d\r\n", __FUNCTION__, saved_switch_config.config_index, cfgindex, saved_switch_config.switch_index, current_switch);
		saved_switch_config = switch_config[cfgindex][current_switch];
	}
	updated = false;
	memcpy(saved_pcm_config, pcm_config, sizeof(pcm_config));

	LOGD(TAG, "%d -- %s(): TESTING pcm_dirty()...\r\n", millis(), __FUNCTION__);
	for (uint8_t pi = 0; pi < PCMS; ++pi) {
		if (pcm_dirty(pi, &pcm_config[pi], true)) {
			LOGD(TAG, "%d -- %s(): pcm_config[%d] SHOULD NOT BE DIRTY!\r\n", millis(), __FUNCTION__, pi);
		}
	}
	init_circuit_activation();

	LOGD(TAG, "%d -- %s(): pcm_config[%d].circuits[%d].output=%x, dirty=%d\r\n", 
		millis(), __FUNCTION__, 0, 2, pcm_config[0].circuits[2].output);
}


static void sig_init(machineParams *mParams, machineEvent *mEvt)
{
	LOGD(TAG, "%s(mParams=%p, mEvt=%p)\r\n", __FUNCTION__, mParams, mEvt); Serial.flush();

	pkt_register_callback(NULL);
	currentPage = onSwitchSetupPage;
	memset((uint8_t*)&touch, 0, sizeof(touch));
	init_switch();

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


static void draw_circuits(bool second8)
{
	uint8_t offset = second8? CIRCUITS/2 : 0;

	for (uint16_t circuit = 0; circuit < CIRCUITS/2; ++circuit) {
		uint8_t index = offset + circuit;
		uint32_t on_color = 0x000000;

		if (circuit_activation[current_pcm][index] & PRIMARY_ACTIVATION) on_color |= 0x00FF00;
		if (circuit_activation[current_pcm][index] & SECONDARY_ACTIVATION) on_color |= 0x0000FF;
		drawButton2(77+(circuit*33), LINE4_Y, 33, 31, 26, circuit_string(index), CIRCUIT_TAG + index, on_color != 0, on_color);
	}
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
	drawTextColor(22, LINE5_Y+3, FONT, "Trigger:", settings.greyColor);

	for (uint16_t trigger = 0; trigger < TRIGGERS; ++trigger) {
		uint32_t on_color = 0;
		if (switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[0]&(1<<trigger)) on_color |= 0x00FF00;
		if (switch_config[cfgindex][current_switch].pcm[current_pcm].triggers[1]&(1<<trigger)) on_color |= 0x0000FF;
		drawButton2(82+(trigger*33), LINE5_Y, 33, 31, 26, trigger_string(trigger), TRIGGER_TAG + trigger, on_color != 0, on_color);
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
		bool toggle       = (switch_config[cfgindex][current_switch].inputs & TOGGLE_INPUT) != 0;
		bool momentary    = (switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) != 0;
		bool double_press = (switch_config[cfgindex][current_switch].inputs & DOUBLE_INPUT) != 0;
		bool long_press   = (switch_config[cfgindex][current_switch].inputs & LONG_INPUT) != 0;
	
		// LINE1: < Configuration: #  Switch: #  PCM: # >
		drawTextColor(120, LINE1_Y, FONT, "Configuration:", settings.greyColor);
		drawText(226, LINE1_Y, FONT, "Switch:");
		drawText(292, LINE1_Y, FONT, "PCM:");
		/*if (current_pcm > 0)*/ drawButton2(80, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, "<", CONFIG_LEFT, touch.tag == CONFIG_LEFT, settings.highlightColor);
		/*if (current_pcm < pcms-1)*/ drawButton2(342, LINE1_Y-11, ARROW_W, ARROW_H, ARROW_FONT, ">", CONFIG_RIGHT, touch.tag == CONFIG_RIGHT, settings.highlightColor);
	
		drawInt(208, LINE1_Y, FONT, 0x00FF00, cfgindex+1);
		drawInt(272, LINE1_Y, FONT, 0x00FF00, current_switch+1);
		drawInt(322, LINE1_Y, FONT, 0xFF0000, current_pcm+1);
	
		// LINE2: Pimary Activation   Secondary Activation
		drawTextColor(27, LINE2_Y, FONT, "Primary Activation", settings.greyColor);
		drawText(320, LINE2_Y, FONT, "Secondary Activation");
		
		// LINE3: Toggle|Momentary   Long Press|Double Press
		drawButton2(27, LINE3_Y, 50, 31, FONT, "Toggle", TOGGLE_TAG, toggle, 0x00FF00);
		drawButton2(78, LINE3_Y, 73, 31, FONT, "Momentary", MOMENTARY_TAG, momentary, 0x00FF00);
	
		uint32_t on_color = momentary? 0xd0d0d0 : 0x0000FF;
		drawButton2(310, LINE3_Y, 80, 31, FONT, "Long Press", LONG_PRESS_TAG, momentary || long_press, on_color);
		drawButton2(391, LINE3_Y, 87, 31, FONT, "Double Press", DOUBLE_PRESS_TAG, momentary || double_press, on_color);
	
		// LINE4: Circuit: 1 2 3 4 ..     X Y  ..or.. 9 10 11 12 ..  V W
		drawTextColor(27, LINE4_Y+3, FONT, "Circuit:", settings.greyColor);
		if (pcm_circuits[current_pcm] > CIRCUITS/2) drawButton2(413, LINE4_Y, ARROW_W-3, ARROW_H-1, ARROW_FONT, ">", CIRCUIT_RIGHT, touch.tag == CIRCUIT_RIGHT, settings.highlightColor);
		draw_circuits(second8);
	
		// LINE5:
		draw_triggers();
	
		// LINE6: button line -- Back Home Advanced Clear Test Apply
		drawButton2(0,   LINE6_Y, 80, 50, FONT, dirty()? "Abort" : "Back", BACK_TAG, false, 0);
		drawButton2(80,  LINE6_Y, 80, 50, FONT, "Home", HOME_TAG, false, 0);
		drawButton(160,  LINE6_Y, 80, 50, FONT, "Output", "Style", "", touch, ADVANCED_TAG, false);
		drawButton2(240, LINE6_Y, 80, 50, FONT, "Clear", CLEAR_TAG, false, 0);
		drawButton2(320, LINE6_Y, 80, 50, FONT, "Test", TEST_TAG, false, 0);
		drawButton2(400, LINE6_Y, 80, 50, FONT, "Apply", APPLY_TAG, false, 0);
		drawButton2(430, 0,   50, 40, FONT, "Help", HELP_TAG, false, 0);
	}
}


static uint8_t next_activation(uint8_t activation)
{
	uint8_t inputs = switch_config[cfgindex][current_switch].inputs;
	if (inputs == 0) {
		activation = 0;
	} else
	if ((inputs & (TOGGLE_INPUT|MOMENTARY_INPUT)) == 0) {
		activation ^= SECONDARY_ACTIVATION;
	} else
	if ((inputs & (DOUBLE_INPUT|LONG_INPUT)) == 0) {
		activation ^= PRIMARY_ACTIVATION;
	} else {
		activation = (activation + 1) & (PRIMARY_ACTIVATION|SECONDARY_ACTIVATION);
	}
	return activation;
}


static void touch_sig(machineParams *mParams, machineEvent *mEvt)
{
	static machineEvent evt;
	
	touch = *((sTagXY *) mEvt->asInts[0]);
	LOGD(TAG, "%s(): touch.tag=%d\r\n", __FUNCTION__, touch.tag);
	if (touch.tag >= CIRCUIT_TAG && touch.tag <= CIRCUIT_TAG20) {
		switch_config_t* c = &switch_config[cfgindex][current_switch];
		switch_pcm_t* p = &c->pcm[current_pcm];
		uint8_t index = touch.tag - CIRCUIT_TAG;
		uint8_t activation = next_activation(circuit_activation[current_pcm][index]);
		uint8_t outputs = pcm_config[current_pcm].circuits[index].output;
		LOGD(TAG, "%s(): switch_config.inputs=0x%x, index=%d, activation %d --> %d\r\n", __FUNCTION__, c->inputs, index, circuit_activation[current_pcm][index], activation);
		circuit_activation[current_pcm][index] = activation;
		if (activation & PRIMARY_ACTIVATION) {
			if ((outputs & OUTPUT_PRIMARY_MASK) == 0) outputs |= OUTPUT_TOGGLE;
			if (c->inputs & MOMENTARY_INPUT) outputs |= PRIMARY_MOMENTARY; else outputs &= ~PRIMARY_MOMENTARY;
		} else {
			outputs &= ~OUTPUT_PRIMARY_MASK;
		}
		if (activation & SECONDARY_ACTIVATION) {
			if ((outputs & OUTPUT_SECONDARY_MASK) == 0) outputs |= (OUTPUT_TOGGLE<<4);
			if (c->inputs & DOUBLE_INPUT) outputs |= SECONDARY_DOUBLE; else outputs &= ~SECONDARY_DOUBLE;
		} else {
			outputs &= ~OUTPUT_SECONDARY_MASK;
		}
		if (outputs & OUTPUT_PRIMARY_MASK) p->circuits[0] |= (1<<index); else p->circuits[0] &= ~(1<<index);
		if (outputs & OUTPUT_SECONDARY_MASK) p->circuits[1] |= (1<<index); else p->circuits[1] &= ~(1<<index);
		LOGD(TAG, "%d -- %s(): switch_config[%d][%d].pcm[%d].circuits[0]=%x, circuits[1]=%x\r\n", 
			millis(), __FUNCTION__, cfgindex, current_switch, current_pcm, 
			p->circuits[0], p->circuits[1]);
		pcm_config[current_pcm].circuits[index].output = outputs;
		LOGD(TAG, "%d -- %s(): pcm_config[%d].circuits[%d].output=%x, dirty=%d\r\n", 
			millis(), __FUNCTION__, current_pcm, index, outputs, pcm_dirty(current_pcm, &saved_pcm_config[current_pcm], true));
	} else
	if (touch.tag >= TRIGGER_TAG && touch.tag <= TRIGGER_TAG8) {
		switch_pcm_t* p = &switch_config[cfgindex][current_switch].pcm[current_pcm];
		uint8_t index = touch.tag - TRIGGER_TAG;
		uint8_t activation = 0;
		if (p->triggers[0]&(1<<index)) activation |= PRIMARY_ACTIVATION;
		if (p->triggers[1]&(1<<index)) activation |= SECONDARY_ACTIVATION;
		activation = next_activation(activation);
		if (activation & PRIMARY_ACTIVATION) p->triggers[0] |= (1<<index); else p->triggers[0] &= ~(1<<index);
		if (activation & SECONDARY_ACTIVATION) p->triggers[1] |= (1<<index); else p->triggers[1] &= ~(1<<index);
	} else {
		LOGD(TAG, "%s().%d: switch(touch.tag=%d)...\r\n", __FUNCTION__, __LINE__, touch.tag); Serial.flush();
		switch (touch.tag) {
		#if 1
		case CONFIG_LEFT:  current_pcm = (current_pcm-1)&(PCMS-1); break;
		case CONFIG_RIGHT: current_pcm = (current_pcm+1)&(PCMS-1); break;
		#else
		case CONFIG_LEFT: if (current_pcm > 0) current_pcm -= 1; break;
		case CONFIG_RIGHT: if (current_pcm < pcms-1) current_pcm += 1; break;
		#endif
		case CIRCUIT_RIGHT: second8 = !second8; break;
		case CLEAR_TAG: if (!updated) clear(); break;

		case TOGGLE_TAG:
			switch_config[cfgindex][current_switch].inputs ^= TOGGLE_INPUT;
			if (switch_config[cfgindex][current_switch].inputs & TOGGLE_INPUT) switch_config[cfgindex][current_switch].inputs &= ~MOMENTARY_INPUT;
			break;

		case MOMENTARY_TAG:
			switch_config[cfgindex][current_switch].inputs ^= MOMENTARY_INPUT;
			if (switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) switch_config[cfgindex][current_switch].inputs = MOMENTARY_INPUT;
			break;

		case LONG_PRESS_TAG:
			if ((switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) == 0) {
				switch_config[cfgindex][current_switch].inputs ^= LONG_INPUT;
				switch_config[cfgindex][current_switch].inputs &= ~DOUBLE_INPUT;
				LOGD(TAG, "%d -- %s(): switch_config[%d][%d].inputs = %x\r\n", 
					millis(), __FUNCTION__, cfgindex, current_switch, switch_config[cfgindex][current_switch].inputs);
			}
			break;

		case DOUBLE_PRESS_TAG:
			if ((switch_config[cfgindex][current_switch].inputs & MOMENTARY_INPUT) == 0) {
				switch_config[cfgindex][current_switch].inputs ^= DOUBLE_INPUT;
				switch_config[cfgindex][current_switch].inputs &= ~LONG_INPUT;
			}
			break;

		case BACK_TAG:  // label can be BACK or ABORT (depending on whether changes have been made)
			if (!updated) restore();
			evt.asChars[0] = CONFIG_SETUP_PAGE;
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
			if (!updated) {
				evt.asChars[0] = ADVANCED_PAGE;
				mParams->mEvt = &evt;
				publish(GOTO_PAGE_SIG, mParams);
			}
			break;
			
		case TEST_TAG:
			if (!updated) {
				set_output_activation_bits();
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


stnext onSwitchSetupPage(int signal, void* params) 
{
	if (signal != SIG_NULL) {
		machineParams *mParams = (machineParams *) params;
		machineEvent *mEvt = (machineEvent *) mParams->mEvt;

		switch (signal) {
		case SIG_INIT:        sig_init(mParams, mEvt); break;
		case TOUCH_SIG:       touch_sig(mParams, mEvt); break;
		case FRAME_TICK_SIG:  frame_tick_sig(mParams, mEvt); break;
		}
	}
	return (void *) mainIdle;
}