#include "I2C_coms.h"

//#define I2C_DEBUG

int8_t i2c1_init() {
    Wire.begin(4, 3);
    return 0;
}

int8_t i2c1_write_multi(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *pdata, uint32_t count) {
    Wire.beginTransmission(deviceAddress);
    Wire.write(registerAddress);
#ifdef I2C_DEBUG
    //Serial.print("\tWriting "); Serial.print(count); Serial.print(" to addr 0x"); Serial.print(registerAddress, HEX); Serial.print(": ");
#endif
    while(count--) {
        Wire.write((uint8_t)pdata[0]);
#ifdef I2C_DEBUG
        //Serial.print("0x"); Serial.print(pdata[0], HEX); Serial.print(", ");
#endif
        pdata++;
    }
#ifdef I2C_DEBUG
    //Serial.println();
#endif
    return Wire.endTransmission();
}

int8_t i2c1_read_multi(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *pdata, uint32_t count){
    Wire.beginTransmission(deviceAddress);
    Wire.write(registerAddress);
    Wire.endTransmission(false); // Dont send a stop bit
    Wire.requestFrom(deviceAddress, (byte)count);
#ifdef I2C_DEBUG
    Serial.print("\tReading "); Serial.print(count); Serial.print(" from addr 0x"); Serial.print(registerAddress, HEX); Serial.print(": ");
#endif

    while (count--) {
        pdata[0] = Wire.read();
#ifdef I2C_DEBUG
        Serial.print("0x"); Serial.print(pdata[0], HEX); Serial.print(", ");
#endif
        pdata++;
    }
#ifdef I2C_DEBUG
    Serial.println();
#endif
    return 0;
}

int8_t i2c1_write_byte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t data) {
    return i2c1_write_multi(deviceAddress, registerAddress, &data, 1);
}

int8_t i2c1_read_byte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data) {
    return i2c1_read_multi(deviceAddress, registerAddress, data, 1);
}