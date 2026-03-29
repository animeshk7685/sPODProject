// ----------------------------------------------------------------------------
// TCA6408.h
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------

#ifndef TCA6408_H
#define TCA6408_H
#include <Arduino.h>
#include <Wire.h>


class TCA6408
{
public:
  TCA6408();

  enum DeviceAddress
  {
    DEVICE_ADDRESS_0=0x20,
    DEVICE_ADDRESS_1=0x21,
  };

  void setup(TwoWire & wire,
    DeviceAddress device_address);

  enum RegisterAddress
  {
    INPUT_PORT=0,
    OUTPUT_PORT=1,
    POLARITY_INVERSION=2,
    CONFIGURATION=3
  };
  uint8_t readRegister(RegisterAddress register_address);
  void writeRegister(RegisterAddress register_address, uint8_t data);

  uint8_t readInputRegister();

  // If a bit in the polarity inversion register is set to 1 the corresponding
  // port pin is inverted polarity
  uint8_t readPolarityInversionRegister();
  void writePolarityInversionRegister(uint8_t data);
  void setAllPinsPolarityOriginal();
  void setAllPinsPolarityInverted();

  // If a bit in the configuration register is set to 1 the corresponding port
  // pin is enabled as an input
  uint8_t readConfigurationRegister();
  void writeConfigurationRegister(uint8_t data);
  void setAllPinsInput();
  void setAllPinsOutput();

  void writeOutputRegister(uint8_t data);

  void setResetPin(uint8_t reset_pin);
  typedef void (*voidFuncPtr)(void);
  void attachInterrupt(uint8_t interrupt_pin, voidFuncPtr callback);

private:
  TwoWire * wire_ptr_;
  DeviceAddress device_address_;
  uint8_t reset_pin_;
  const static uint8_t NO_PIN = 255;
};

#endif
