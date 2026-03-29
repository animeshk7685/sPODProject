/**
 * @file  I2C_coms.h
 * @brief Contains i2c implementation of the platform
 */
 
#ifndef _I2C_COMS_H_
#define _I2C_COMS_H_

#include "Arduino.h"
#include "Wire.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief i2c1_init() definition.\n
 * 
 */
int8_t i2c1_init();

/** @brief i2c1_write_multi() definition.\n
 * To be implemented by the developer
 */
int8_t i2c1_write_multi(
        uint8_t       deviceAddress,
        uint8_t       registerAddress,
        uint8_t      *pdata,
        uint32_t      count);
/** @brief i2c1_read_multi() definition.\n
 * To be implemented by the developer
 */
int8_t i2c1_read_multi(
        uint8_t       deviceAddress,
        uint8_t       registerAddress,
        uint8_t      *pdata,
        uint32_t      count);
/** @brief i2c1_write_byte() definition.\n
 * To be implemented by the developer
 */
int8_t i2c1_write_byte(
        uint8_t       deviceAddress,
        uint8_t       registerAddress,
        uint8_t       data);

/** @brief i2c1_read_byte() definition.\n
 * To be implemented by the developer
 */
int8_t i2c1_read_byte(
        uint8_t       deviceAddress,
        uint8_t       registerAddress,
        uint8_t      *pdata);

#ifdef __cplusplus
}
#endif

#endif
