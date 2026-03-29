#ifndef __XO_H__
#define __XO_H__


#define XO_LEDOUT_OFF 0x1
#define XO_LEDOUT_ON  0x0
#define XO_LEDOUT_PWM 0x2


extern void xo_init();
extern void xo_set_se(uint8_t port, uint8_t mode /* XO_LEDOUT_ON or XO_LEDOUT_OFF */);
extern void xo_set_ledout(uint8_t port, uint8_t mode /* XO_LEDOUT_xx */);
extern void xo_set_iref(uint8_t port, uint8_t iref);
extern void xo_set_pwm(uint8_t port, uint8_t duty_cycle);
extern void xo_set_alloff();
extern void xo_dump_regs();

#endif
