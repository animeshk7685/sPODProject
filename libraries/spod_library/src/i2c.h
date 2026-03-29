#ifndef __I2C_H__
#define __I2C_H__

#define I2C_LED_ADDR 0x15

//#define I2C_CLK_PIN 3
//#define I2C_DAT_PIN 4

#include <Wire.h>

extern void i2c_init(uint8_t dat_pin, uint8_t clk_pin);

#endif
