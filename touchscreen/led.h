#ifndef __LED_H__
#define __LED_H__

#define GREEN_LED   0
#define BLUE_LED    1

extern void led_init();
extern void led_set(uint8_t led, bool on);
extern void led_alloff();
extern void led_toggle(uint8_t led);

#endif
