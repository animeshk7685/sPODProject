#include "touchscreen.h"


static bool led_state[2];


void led_init(void)
{
  led_alloff();
}


void led_set(uint8_t led, bool on)
{
  led &= 1;
  led_state[led] = on;
  switch (led) {
  case BLUE_LED: portx_set_blue_led(on); break;
  case GREEN_LED: portx_set_green_led(on); break;
  default: break;
  }
}


void led_toggle(uint8_t led)
{
  led &= 1;
  led_set(led, !led_state[led]);
}


void led_alloff()
{
  led_set(GREEN_LED, false);
  led_set(BLUE_LED, false);
}