#ifndef __PORTX_H__
#define __PORTX_H__

#define PORTX_GREEN_LED 0
#define PORTX_FT81X_GC  1
#define PORTX_FT81X_PD  2
#define PORTX_BLUE_LED  3
#define PORTX_RS485_CTL 5

extern void portx_setmode(uint8_t pin, uint8_t mode);
extern void portx_write(uint8_t pin, uint8_t value);
extern void portx_set_blue_led(bool on);
extern void portx_set_green_led(bool on);
extern void portx_init();

#endif
