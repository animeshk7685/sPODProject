#include "spod_library.h"
#include "pcm_pins.h"
#include "xi.h"
#include <PCA95x5.h>


static const char* TAG = "XI";

// PCA9535 is the port expander used to provide the triggers, including IGN and KEY_ON
// Refer to the spod/libraries/PCA95x5 for specifics, also look in the xi.h header file
// for port mapping


#define INPUTS ((1<<PCM_INT1) | (1<<BT_PAIR) | (1<<IGN_SENSE) | (1<<KEY_ON) | (1<<INPUT1) | (1<<INPUT2) | (1<<INPUT3) | (1<<INPUT4) | (1<<INPUT5))


static PCA9535 xi;


void xi_init()
{
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    xi.attach(Wire, I2C_PCA9535_ADDR); // assumes i2c_init() was already called to initize Wire to correct SDA/SCl
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    xi.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    xi.direction(INPUTS);
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    xi.write(PCA95x5::Level::L_ALL);
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
}


void xi_write(uint8_t port, bool on)
{
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    xi.write(static_cast<PCA95x5::Port::Port>(port), on? PCA95x5::Level::L : PCA95x5::Level::H);
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
}


bool xi_read(uint8_t port)
{
    PCA95x5::Level::Level level;
    level = xi.read(static_cast<PCA95x5::Port::Port>(port));
    return level == PCA95x5::Level::H? false : true;
}


uint16_t get_inputs()
{
    uint16_t raw = xi.read();
    return ((raw>>INPUT1)&0x1F) | (((raw>>KEY_ON)&1)<<5);
}


void bypass_ctrl(bool on) {xi_write(BYPASS_CTRL, on);}
void pcm_int2(bool on)    {xi_write(PCM_INT2, on);}
bool pcm_int1()           {return xi_read(PCM_INT1);}
bool key_on()             {return xi_read(KEY_ON);}
bool ign_sense()          {return xi_read(IGN_SENSE);}
bool bt_pair()            {return xi_read(BT_PAIR);}

bool get_input(uint8_t which) {return xi_read(which);}
void set_input_ctrl(uint8_t which, bool on) {xi_write(which, on);}