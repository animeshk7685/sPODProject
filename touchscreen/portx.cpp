#include "touchscreen.h"
#include "TCA9554.h"
#include "portx.h"


static const char* TAG = "TCA9554";


static TCA9554 device(0x20);
static bool initialized = false;


void portx_setmode(uint8_t pin, uint8_t mode)
{
    if (device.pinMode1(pin, mode) == 0) {
        LOGE(TAG, "%s(pin=%d, mode=%x): FAILED! error=%d\r\n", __FUNCTION__, pin, mode, device.lastError());
        for (;;);
    }
}


void portx_write(uint8_t pin, uint8_t value)
{
    if (initialized && device.write1(pin, value) == 0) {
        LOGE(TAG, "%s(pin=%d, value=%d): FAILED! error=%d\r\n", __FUNCTION__, pin, value, device.lastError());
        for (;;);
    }
}


void portx_set_blue_led(bool on)
{
   portx_write(PORTX_BLUE_LED, on? HIGH:LOW);
}


void portx_set_green_led(bool on)
{
    portx_write(PORTX_GREEN_LED, on? HIGH:LOW);
}


void portx_init()
{
    LOGD(TAG, "%s()\r\n", __FUNCTION__);
    portx_setmode(PORTX_GREEN_LED, OUTPUT);
    portx_setmode(PORTX_BLUE_LED, OUTPUT);
    portx_setmode(PORTX_FT81X_GC, OUTPUT);
    portx_setmode(PORTX_FT81X_PD, OUTPUT);
    portx_setmode(PORTX_RS485_CTL, OUTPUT);
    initialized = true;

    portx_write(PORTX_GREEN_LED, LOW);
    portx_write(PORTX_BLUE_LED, LOW);
    portx_write(PORTX_FT81X_GC, HIGH);
    portx_write(PORTX_FT81X_PD, HIGH);
    portx_write(PORTX_RS485_CTL, HIGH);
}
