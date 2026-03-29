/**
 * @file LP5024.h
 * @author rneurink (ruben.neurink@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2021-07-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __LP5024_H
#define __LP5024_H

#include <Arduino.h>
#include <Wire.h>

#define DEFAULT_ADDRESS 0x28
#define BROADCAST_ADDRESS 0x3C
//#define DEFAULT_ADDRESS 0x14
//#define BROADCAST_ADDRESS 0x1E

enum LED_Configuration {
    RGB,
    GRB,
    BGR,
    RBG,
    GBR,
    BRG,
};

enum LP5024_LEDS {
    LED_0 = 0x01,
    LED_1 = 0x02,
    LED_2 = 0x04,
    LED_3 = 0x08,
    LED_4 = 0x10,
    LED_5 = 0x20,
    LED_6 = 0x40,
    LED_7 = 0x80
};

enum LP5024_Configuration {
    LED_GLOBAL_ON = 0 << 0,
    LED_GLOBAL_OFF = 1 << 0,
    MAX_CURRENT_25mA = 0 << 1,
    MAX_CURRENT_35mA = 1 << 1,
    PWM_DITHERING_OFF = 0 << 2,
    PWM_DITHERING_ON = 1 << 2,
    AUTO_INC_OFF = 0 << 3,
    AUTO_INC_ON = 1 << 3,
    POWER_SAVE_OFF = 0 << 4,
    POWER_SAVE_ON = 1 << 4,
    LOG_SCALE_OFF = 0 << 5,
    LOG_SCALE_ON = 1 << 5
};

enum EAddressType {
    Normal,
    Broadcast
};

// Register definitions
#define DEVICE_CONFIG0 0x00     // Chip_EN
#define DEVICE_CONFIG1 0x01     // Configurations for Log_scale, Power_save, Auto_inc, PWM_dithering, Max_current_option and LED_Global_off
#define LED_CONFIG0 0x02        // Contains the BANK configurations
#define BANK_BRIGHTNESS 0x03    // Contains the BANK brightness level 
#define BANK_A_COLOR 0x04       // Contains the BANK A color value
#define BANK_B_COLOR 0x05       // Contains the BANK B color value
#define BANK_C_COLOR 0x06       // Contains the BANK C color value
#define LEDx_BRIGHTNESS 0x07
#define LED0_BRIGHTNESS 0x07    // Contains the brightness level for LED 0
#define LED1_BRIGHTNESS 0x08    // Contains the brightness level for LED 1
#define LED2_BRIGHTNESS 0x09    // Contains the brightness level for LED 2
#define LED3_BRIGHTNESS 0x0A    // Contains the brightness level for LED 3
#define LED3_BRIGHTNESS 0x0B    // Contains the brightness level for LED 4
#define LED3_BRIGHTNESS 0x0C    // Contains the brightness level for LED 5
#define LED3_BRIGHTNESS 0x0D    // Contains the brightness level for LED 6
#define LED3_BRIGHTNESS 0x0E    // Contains the brightness level for LED 7
#define OUTx_BASE  0x0F
#define OUT0_COLOR 0x0F         // Contains the color value for output 0
#define OUT1_COLOR 0x10         // Contains the color value for output 1
#define OUT2_COLOR 0x11         // Contains the color value for output 2
#define OUT3_COLOR 0x12         // Contains the color value for output 3
#define OUT4_COLOR 0x13         // Contains the color value for output 4
#define OUT5_COLOR 0x14         // Contains the color value for output 5
#define OUT6_COLOR 0x15         // Contains the color value for output 6
#define OUT7_COLOR 0x16         // Contains the color value for output 7
#define OUT8_COLOR 0x17         // Contains the color value for output 8
#define OUT9_COLOR 0x18         // Contains the color value for output 9 
#define OUT10_COLOR 0x19        // Contains the color value for output 10
#define OUT11_COLOR 0x1A        // Contains the color value for output 11
#define OUT12_COLOR 0x1B        // Contains the color value for output 12
#define OUT13_COLOR 0x1C        // Contains the color value for output 13
#define OUT14_COLOR 0x1D        // Contains the color value for output 14
#define OUT15_COLOR 0x1E        // Contains the color value for output 15
#define OUT16_COLOR 0x1F        // Contains the color value for output 16
#define OUT17_COLOR 0x20        // Contains the color value for output 17
#define OUT18_COLOR 0x21        // Contains the color value for output 18
#define OUT19_COLOR 0x22        // Contains the color value for output 19
#define OUT20_COLOR 0x23        // Contains the color value for output 20
#define OUT21_COLOR 0x24        // Contains the color value for output 21
#define OUT22_COLOR 0x25        // Contains the color value for output 22
#define OUT23_COLOR 0x26        // Contains the color value for output 23
#define RESET_REGISTERS 0x27    // Resets all the registers to their default values

/**
 * @brief Class to communicate with the LP5024
 */
class LP5024 
{
    public:
        LP5024(void); // Constructor
        LP5024(LED_Configuration ledConfiguration); // Constructor with a specific led configuration
        LP5024(uint8_t enablePin); // Constructor with enable pin
        LP5024(LED_Configuration ledConfiguration, uint8_t enablePin); // Constructor with a specific led configuration and an enable pin

        /**
         * Initialisation functions
         */
        void Begin(uint8_t i2c_address = DEFAULT_ADDRESS); // Initialize the driver
        void Reset();
        void ResetRegisters(EAddressType addressType = EAddressType::Normal);

        /**
         * Configuration functions
         */
        void Configure(uint8_t configuration, EAddressType addressType = EAddressType::Normal);
        void SetScaling(uint8_t scaling);
        void SetPowerSaving(uint8_t powerSave);
        void SetAutoIncrement(uint8_t autoInc);
        void SetPWMDithering(uint8_t dithering);
        void SetMaxCurrentOption(uint8_t option);
        void SetGlobalLedOff(uint8_t value);

        void SetEnablePin(uint8_t enablePin);
        void SetLEDConfiguration(LED_Configuration ledConfiguration);
        void SetI2CAddress(uint8_t address);

        /**
         * Bank control functions
         */
        void SetBankControl(uint8_t leds, EAddressType addressType = EAddressType::Normal);
        void SetBankBrightness(uint8_t brightness, EAddressType addressType = EAddressType::Normal);
        void SetBankColorA(uint8_t value, EAddressType addressType = EAddressType::Normal);
        void SetBankColorB(uint8_t value, EAddressType addressType = EAddressType::Normal);
        void SetBankColorC(uint8_t value, EAddressType addressType = EAddressType::Normal);
        void SetBankColor(uint8_t red, uint8_t green, uint8_t blue, EAddressType addressType = EAddressType::Normal);

        /**
         * Output control functions
         */
        void SetLEDBrightness(uint8_t led, uint8_t brighness, EAddressType addressType = EAddressType::Normal);
        void SetOutputColor(uint8_t output, uint8_t value, EAddressType addressType = EAddressType::Normal);
        void SetLEDColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue, EAddressType addressType = EAddressType::Normal);


        /**
         * Low level functions
         */
        void WriteRegister(uint8_t reg, uint8_t value, EAddressType addressType = EAddressType::Normal);
        void ReadRegister(uint8_t reg, uint8_t *value);

    protected:

    private:
        uint8_t     _i2c_address;
        uint8_t     _i2c_address_broadcast = BROADCAST_ADDRESS;
        uint8_t     _enable_pin = 0xFF;
        LED_Configuration     _led_configuration = BRG;

        uint8_t getAddress(EAddressType addressType);
};

#endif