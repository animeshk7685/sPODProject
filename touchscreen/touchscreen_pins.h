#ifndef __TOUCHSCREEN_PINS_H__
#define __TOUCHSCREEN_PINS_H__

#define I2C_CLK_PIN         3   // the I2C clock
#define I2C_DAT_PIN         4   // the I2C data
#define RS485_DE_PIN        5
#define RS485_CTRL_PIN      0xFF // OUTPUT -- after boot set high to enable RS485_INTR_PIN functionality -- this is in the port expander, so not set up in rs485 module
#define RS485_INTR_PIN      8   // INPUT/OUTPUT -- RS485 attention line

#endif