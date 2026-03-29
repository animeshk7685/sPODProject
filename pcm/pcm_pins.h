#ifndef __PCM_PINS_H__
#define __PCM_PINS_H__

#define TEMP_DETECT_PIN     A0  // ADCIN -- temperature near MCU
#define C_SENSE_IN_PIN      A1  // ADCIN -- current for enabled SE (muxed output pins)
#define LV_DETECT_PIN       A2  // ADCIN -- battery voltage, 1.2 or 2.4
#define IO_INT_PIN          3   // INPUT -- interrupt for PCA9535 port expander, to catch pulses (edges or changes)
#define TEMP_TRIGGER_PIN    4   // ADCIN -- 4 termistors on corners of board, measures temperature of the board
#define I2C_CLK_PIN         5   // the I2C clock
#define I2C_DAT_PIN         6   // the I2C data
#define RS485_CTRL_PIN      7   // OUTPUT -- after boot set high to enable RS485_INTR_PIN functionality
#define RS485_INTR_PIN      8   // INPUT/OUTPUT -- RS485 attention line
#define GPIO9_PIN           9   // unused
#define RS485_DE_PIN        10

#define I2C_PCA9535_ADDR 0x21   // i2c address of PCA9535 which controls PCM inputs (triggers, IGN, etc)
#define I2C_PCA9956_ADDR 0x01   // i2c address of PCA9956 which controls PCM outputs

#endif