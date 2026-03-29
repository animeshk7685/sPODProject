#include "switch.h"
#include "config.h"
#include "sleep.h"
#include "button_fsm.h"
#include <TCA6408.h>


static const char* TAG = "SBTN";

#define BUTTONS_TEST_ENABLED 0

#define ADDRESS_CHANGE_COMMIT_TIME (20*1000) // if it has been 20 seconds since an address change, commit it to flash

#define BUTTONS_TEST      0x06    // BUTTONS 2&3 (1-based) -- for Baja testing...
#define BUTTONS_NEXTADDR  0x88    // BUTTONS 4&8 (1-based) -- cycle through source address
#define BUTTONS_SLEEP     0x60    // BUTTONS 6&7 (1-based) -- sleep
#define BUTTONS_LOCKOUT   0xC0    // BUTTONS 7&8 (1-based) -- lockout keypad (press again to unlock)
#define BUTTONS_RESET     0x81    // BUTTONS 1&8 (1-based) -- request reset to default configuration
#define BUTTONS_CONFIRM   0x18    // BUTTONS 4&5 (1-based) -- confirm reset to default configuration
#define BUTTONS_CLEAR     0x00


static TCA6408 tca6408;
static uint32_t initial_time = 0;
static uint32_t last_change_time = 0;
static uint8_t switch_index = 0xFF;


uint8_t button_read()
{
  static uint8_t previous = 0;
  uint8_t buttons = ~tca6408.readInputRegister();
  if (buttons != previous) {
    LOGE(TAG, "%d -- %s(): %x --> %x\r\n", millis(), __FUNCTION__, previous, buttons);
    previous = buttons;
    last_change_time = millis();
  }
  return buttons;
}


static void button_test()
{
  led_setmask(0xFF, 0x80, 0x80, 0x80);
  
  for (uint8_t rgb = 0x80;;) {
    uint8_t pressed = button_read();
    if (pressed) {
      if (pressed & 0x0F) rgb += 1;
      if (pressed & 0xF0) rgb -= 1;
      LOGD(TAG, "button_test(): rgb=%x\r\n", rgb);
      led_setmask(0xFF, rgb, rgb, rgb);
      delay(5);
    }
  }
}


static void set_switch_color(uint8_t idx, uint32_t color)
{
  switch_status[idx].color = color;
  if (switch_index == (idx%SWITCHES)) {
    led_setmask(1<<switch_index, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
    if (color == 0x000000 ) {
      switch_index = 0xFF;
      initial_time = 0;
    }
  }
}


static void button_tick(uint8_t idx)
{
  uint32_t color;
  if (switch_index != 0xFF && button_fsm_tick(switch_index, &switch_config[cfgindex][switch_index], &color)) {
    LOGD(TAG, "%d -- %s(idx=%d): switch_index=%d, color=%x\r\n", millis(), __FUNCTION__, idx, switch_index, color);
    set_switch_color(idx, color);
    send_switch_status_pkt(idx, button_get_activation_method(switch_index), color);
  }
}


static void button_pressed(uint8_t idx)
{
  if (switch_index != 0xFF) {
    static uint32_t last_time = 0;
    uint32_t now = millis();
    if (last_time == 0 || now - last_time >= 48) {
      uint32_t color = switch_status[idx].color;
      bool action = button_press(switch_index, &switch_config[cfgindex][switch_index], &color);
      LOGD(TAG, "%d -- %s(idx=%d): switch_index=%d, color=%x, action=%d\r\n", millis(), __FUNCTION__, idx, switch_index, color, action);
      set_switch_color(idx, color);
      last_time = now;
      if (action) {
        uint8_t activation = button_get_activation_method(switch_index);
        uint32_t triggers = switch_triggers(idx, (activation&SECONDARY_INPUT) != 0);
        if (pcm_triggers(triggers)) {
          send_switch_status_pkt(idx, button_get_activation_method(switch_index), color);
        } else {
          LOGD(TAG, "%d -- %s(idx=%d): cannot activate switch, triggers\r\n", millis(), __FUNCTION__, idx);
          led_blink(1<<cfgindex, 0xFF, 0x00, 0x00, 100, 3); // flash red 
          button_fsm_clear(switch_index, 0);
        }
      }
    }
  }
}


static void button_released(uint8_t index)
{
  uint8_t idx = cfgindex*CONFIGS + index;
  uint32_t color = switch_status[idx].color;
  bool action = button_release(index, &switch_config[cfgindex][index], &color);
  LOGD(TAG, "%d -- %s(idx=%d): switch_index=%d, color=%x, action=%d\r\n", millis(), __FUNCTION__, idx, index, color, action);
  if (index == switch_index) {
    set_switch_color(idx, color);
    if (action) {
      send_switch_status_pkt(idx, button_get_activation_method(switch_index), color);
      switch_index = 0xFF;
      initial_time = 0;
    }
  }
}


// given the bimap of pressed buttons (1b is pressed, 0b is released)
// return the button index of pressed button if there is only one pressed
static uint8_t single_button(uint8_t buttons)
{
  switch (buttons) {
    case 0x01: return 0;
    case 0x02: return 1;
    case 0x04: return 2;
    case 0x08: return 3;
    case 0x10: return 4;
    case 0x20: return 5;
    case 0x40: return 6;
    case 0x80: return 7;
  }
  return 0xFF;
}


static void reset_config()
{
  for (uint8_t sw = 0; sw < SWITCHES; ++sw) {
    zero_switch_config(cfgindex, sw, &switch_config[cfgindex][sw]);
    write_switch_config(cfgindex, sw, &switch_config[cfgindex][sw]);
    send_switch_config_pkt(SWITCH_UPDATE, cfgindex, sw);
  }
}


static void save_cfgindex()
{
    if (cfgindex != config.cfgindex) {
      config.cfgindex = cfgindex;
      write_config(&config);
    }
}


void button_loop()
{
  static bool locked = false;
  static bool reset_confirm = false;
  static uint32_t address_change_time = 0;
  static uint32_t multi_start;
  static uint8_t multi = 0x00;
  static uint8_t last = 0xFF;
  uint8_t buttons  = button_read();
  uint8_t changed  = last ^ buttons;
  uint8_t released = (~buttons) & changed;
  uint8_t pressed  = buttons & changed;
  uint8_t single = single_button(buttons);
  uint32_t now = millis();
  last = buttons;
  
  if (changed == 0x00) {
    if (now - address_change_time > ADDRESS_CHANGE_COMMIT_TIME) {
      address_change_time = 0;
      save_cfgindex();
    }

    if (config.deepsleep_time != 0 && buttons == 0 && 
      (config.inactivity_timeout && now - last_change_time > config.inactivity_timeout)) {
      last_change_time = now;
      save_cfgindex();
      goto_sleep(); // does not return from this
    }

    if (switch_index != 0xFF) {
      uint32_t idx = cfgindex*SWITCHES + switch_index;
      if (buttons & (1<<switch_index)) button_pressed(idx);
      button_tick(idx);
    } else
    if (single != 0xFF) {
      if (initial_time != 0 && now - initial_time > 100) {
        initial_time = 0;
        if ((switch_status[cfgindex*SWITCHES + single].activation & LOCKED_INPUT) == 0) {
          switch_index = single;
          button_pressed(cfgindex*SWITCHES + switch_index);
        }
      }
    }
    return;
  }

  LOGD(TAG, "%d -- %s(): buttons=%x, changed=%x, released=%x, pressed=%x, locked=%d, multi=%x, single=%x, si=%d\r\n", 
    millis(), __FUNCTION__, buttons, changed, released, pressed, locked, multi, single, switch_index);

  if (reset_confirm && now - multi_start > 5000) {
    reset_confirm = false;
    multi_start = 0;
  }

  if (single == 0xFF) {
    multi |= buttons;
    if (buttons) led_setmask(0xFF, 0x00, 0x00, 0x00);
    if (multi) {
      LOGD(TAG, "%d -- %s(): multi=0x%x\r\n", millis(), __FUNCTION__, multi);
      led_setmask(multi, 0xFF, 0x00, 0x00);
      if (multi_start == 0) multi_start = now;
    }
  }
  
  if (buttons == 0x00 && multi) {
    led_setmask(multi, 0x00, 0x00, 0x00);
    switch (multi) {
    case BUTTONS_LOCKOUT:
      locked = !locked;
      LOGD(TAG, "%d -- %s(): LOCKOUT %d\r\n", millis(), __FUNCTION__, locked);
      led_blink(0xFF, locked? 0xFF:0x00, locked?0x00:0xFF, 0x00, 100, 3); // flash red if locked, green if unlocked
      break;
    case BUTTONS_NEXTADDR:
      LOGD(TAG, "%d -- %s(): NEXTADDR cfgindex=%d, configs=%d, delta=%d\r\n", millis(), __FUNCTION__, cfgindex, configs, now-multi_start);
      if (now - multi_start < 5000) {
        #if 1
        cfgindex = (cfgindex + 1) & (CONFIGS-1);
        #else
        if (cfgindex < configs-1) cfgindex += 1;
        #endif
        address_change_time = now;
      }
      led_blink(1<<cfgindex, 0x00, 0xFF, 0x00, 100, 3); // flash green
      break;
    case BUTTONS_SLEEP:
      LOGD(TAG, "%d -- %s(): SLEEP\r\n", millis(), __FUNCTION__);
      save_cfgindex();
      goto_sleep(); // does not return from this
      break;
    case BUTTONS_RESET:
      LOGD(TAG, "%d -- %s(): RESET\r\n", millis(), __FUNCTION__);
      reset_confirm = true;
      led_blink(BUTTONS_RESET, 0x00, 0x00, 0xFF, 100, 3); // flash blue
      switch_index = 0xFF;
      multi = 0;
      return; // want to leave multi_start set
    case BUTTONS_CONFIRM:
      LOGD(TAG, "%d -- %s(): CONFIRM reset_confirm=%d\r\n", millis(), __FUNCTION__, reset_confirm);
      if (reset_confirm) {
        reset_confirm = false;
        led_blink(1<<cfgindex, 0x00, 0xFF, 0x00, 100, 3); // flash green
        reset_config();
      }
      break;
    #if BUTTONS_TEST_ENABLED
    case BUTTONS_TEST:
      button_test();
      break;
    #endif
    default:
      break;
    }

    switch_index = 0xFF;
    multi_start = 0;
    multi = 0;
    return;
  }

  if (locked) {
    if (pressed) led_setmask(pressed, 0xFF, 0x00, 0x00);
    if (released) led_setmask(released, 0x00, 0x00, 0x00); 
    return;
  }

  if (multi) {
    switch_index = 0xFF;
    return;
  }

  if (single == 0xFF) {
    if (buttons == 0x00) {
      for (uint8_t b = 0; b < SWITCHES; ++b) {
        button_released(b);
      }
    }
  } else {
    if (initial_time == 0) {
      initial_time = millis();
      LOGD(TAG, "%d -- %s(): initial_time=%d, single=%d\r\n", millis(), __FUNCTION__, initial_time, single);
    }
  }
}


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
      led_setmask(1<<index, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);       
    }
  }
}


void button_init()
{
  static TwoWire & wire = Wire;
  static const uint8_t RESET_PIN = 0;

  tca6408.setup(wire, TCA6408::DEVICE_ADDRESS_0);
  tca6408.setResetPin(RESET_PIN);
  tca6408.setAllPinsInput();
  pkt_register_callback(process_pkt_cb);
  button_fsm_init();
}