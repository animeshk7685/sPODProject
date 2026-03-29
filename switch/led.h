#ifndef __LED_H__
#define __LED_H__

extern void led_init();
extern void led_set(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);
extern void led_setmask(uint8_t mask, uint8_t red, uint8_t green, uint8_t blue);
extern void led_alloff();
extern void led_tbird(bool left_to_right, uint8_t red, uint8_t green, uint8_t blue, uint32_t ms_sleep, void (*cb)());
extern void led_post(void (*cb)());
extern void led_blink(uint8_t leds, uint8_t red, uint8_t green, uint8_t blue, uint32_t period, uint32_t count);
extern void led_blink_cancel();

#endif
