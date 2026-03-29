#include "switch.h"
#include "switch_common.h"
#include <LP5024.h>


#define ENABLE_PIN 10
static LP5024 device(BRG, ENABLE_PIN);

static const char* TAG = "LED";


#define TESTING 0
#if TESTING
void led_test_setup()
{
  device.Begin(BROADCAST_ADDRESS);

  device.Configure(LED_GLOBAL_OFF | MAX_CURRENT_35mA | AUTO_INC_OFF | PWM_DITHERING_OFF | POWER_SAVE_OFF | LOG_SCALE_OFF);

  device.SetScaling(LOG_SCALE_ON);
  device.SetPowerSaving(POWER_SAVE_ON);
  device.SetAutoIncrement(AUTO_INC_ON);
  device.SetPWMDithering(PWM_DITHERING_ON);
  device.SetMaxCurrentOption(MAX_CURRENT_25mA);
  device.SetGlobalLedOff(LED_GLOBAL_ON);

  device.SetBankControl(LED_0 | LED_1 | LED_2 | LED_3);
  device.SetBankColorA(0x7f);
  device.SetBankColorB(0x20);

  device.SetOutputColor(11, 0x40);

  delay(1000);

  device.Reset();

  delay(10);

  device.SetBankControl(LED_0 | LED_1 | LED_2 | LED_3);
  device.SetBankColor(0x00, 0x40, 0xff);

  device.SetLEDColor(7, 0x40, 0x40, 0x40);
  device.SetLEDColor(6, 0x00, 0x00, 0xff);
  device.SetLEDColor(5, 0x00, 0xff, 0x00);
  device.SetLEDColor(4, 0xff, 0x00, 0x00);
}

void led_test_loop()
{
  LOGE(TAG, "%s()...\r\n", __FUNCTION__);
  for (int i = 0; i < 0xff; i+= 5) {
    device.SetBankBrightness(i);
    device.SetLEDBrightness(3, ~i);
    delay(10);
  }
  for (int i = 0xff; i > 0; i-= 5) {
    device.SetBankBrightness(i);
    device.SetLEDBrightness(3, ~i);
    delay(10);
  }
  LOGE(TAG, "...%s()\r\n", __FUNCTION__);
}
#endif


void led_init(void)
{
  device.Begin(BROADCAST_ADDRESS);  // set both sides of the switch

  device.Configure(LED_GLOBAL_OFF | MAX_CURRENT_35mA | AUTO_INC_OFF | PWM_DITHERING_OFF | POWER_SAVE_OFF | LOG_SCALE_OFF);
  device.SetScaling(LOG_SCALE_ON);
  device.SetPowerSaving(POWER_SAVE_ON);
  device.SetAutoIncrement(AUTO_INC_ON);
  device.SetPWMDithering(PWM_DITHERING_ON);
  device.SetMaxCurrentOption(MAX_CURRENT_25mA);
  device.SetGlobalLedOff(LED_GLOBAL_ON);

  delay(250);
  device.Reset();
  delay(10);
}


void led_set(uint8_t led, uint8_t red, uint8_t green, uint8_t blue)
{
  static uint8_t prev_rgb[8][3];

  led &= 7;
  bool changed = prev_rgb[led][0] != red || prev_rgb[led][1] != green || prev_rgb[led][2] != blue;
  
  if (changed) {
    prev_rgb[led][0] = red;
    prev_rgb[led][1] = green;
    prev_rgb[led][2] = blue;
    LOGE(TAG, "%d -- %s(led=%d, red=%x, green=%x, blue=%x)\r\n", millis(), __FUNCTION__, led, red, green, blue);
  }
  device.SetLEDColor(led, red, green, blue);
}


void led_setmask(uint8_t mask, uint8_t red, uint8_t green, uint8_t blue)
{
  for (uint8_t led = 0; led < 8; ++led) {
    if (mask & (1<<led)) {
      led_set(led, red, green, blue);
    }
  }
}


void led_alloff()
{
  led_setmask(0xFF, 0, 0, 0);
}


void led_tbird(bool left_to_right, uint8_t red, uint8_t green, uint8_t blue, uint32_t ms_sleep, void (*cb)())
{
  for (uint8_t index = 0; index < 8; ++index) {
    uint8_t led = left_to_right? index : 7 - index;
    led_set(led, red, green, blue);

    uint32_t delta = millis();
    cb();
    delta = millis() - delta;
  
    if (delta < ms_sleep) {
      delay(ms_sleep - delta);
    }
  }  
}


void led_post(void (*cb)())
{
  led_tbird(true,  0xFF, 0x00, 0x00, 50, cb);
  led_tbird(false, 0x00, 0xFF, 0x00, 50, cb);
  led_tbird(true,  0x00, 0x00, 0xFF, 50, cb);
  led_tbird(false, 0x00, 0x00, 0x00, 50, cb);
}


static Timer<>::Task blink_task;
static uint8_t blink_leds, blink_red, blink_green, blink_blue;
static uint32_t blink_period, blink_count, blink_total;

static bool led_blink_task(void* unused)
{
  if (blink_count < blink_total) {
    uint8_t r=0, g=0, b=0;
    if ((blink_count&1)==0) {
      r = blink_red; g = blink_green; b = blink_blue;
    } 
    led_setmask(blink_leds, r, g, b);
    
    if (++blink_count < blink_total) {
      blink_task = timer.in(blink_period, led_blink_task);
    }
  }

  return true;
}


void led_blink(uint8_t leds, uint8_t red, uint8_t green, uint8_t blue, uint32_t period, uint32_t count)
{
  blink_leds = leds;
  blink_count = 0;
  blink_total = count*2;
  blink_period = period;
  blink_red = red;
  blink_green = green;
  blink_blue = blue;
  blink_task = timer.in(period, led_blink_task);
}


void led_blink_cancel()
{
  if (blink_count < blink_total) {
    blink_count = blink_total = 0;
    timer.cancel(blink_task);
  }
}