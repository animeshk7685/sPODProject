#include "spod_library.h"

static const char *TAG = "I2C";

void i2c_init(uint8_t dat_pin, uint8_t clk_pin)
{
    LOGD(TAG, "%s(): Wire.begin(data=%d, clk=%d)\r\n", __FUNCTION__, dat_pin, clk_pin);
    Wire.begin(dat_pin, clk_pin, 400000UL /*clock frequency*/); // TODO: up the frequency...
}
