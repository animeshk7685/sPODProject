// ----------------------------------------------------------------------------
// TCA6408.cpp
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "TCA6408.h"


TCA6408::TCA6408()
{
  wire_ptr_ = nullptr;
  reset_pin_ = NO_PIN;
}

void TCA6408::setup(TwoWire & wire,
  DeviceAddress device_address)
{
  wire_ptr_ = &wire;
  device_address_ = device_address;

  wire_ptr_->begin();
#if defined(WIRE_HAS_TIMEOUT)
  wire_ptr_->setWireTimeout(3000 /* us */, true /* reset_on_timeout */);
#endif
}

uint8_t TCA6408::readRegister(RegisterAddress register_address)
{
  if (wire_ptr_ == nullptr)
  {
    return 0;
  }

  wire_ptr_->beginTransmission(device_address_);
  wire_ptr_->write((uint8_t)register_address);
  wire_ptr_->endTransmission();

#if defined(WIRE_HAS_TIMEOUT)
  wire_ptr_->clearWireTimeoutFlag();
#endif
  wire_ptr_->requestFrom(device_address_, 1);

  uint8_t read_byte = wire_ptr_->read();
  return read_byte;
}

void TCA6408::writeRegister(RegisterAddress register_address, uint8_t data)
{
  if (wire_ptr_ == nullptr)
  {
    return;
  }

  wire_ptr_->beginTransmission(device_address_);
  wire_ptr_->write((uint8_t)register_address);
  wire_ptr_->write(data);
  wire_ptr_->endTransmission();
}

uint8_t TCA6408::readInputRegister()
{
  return readRegister(INPUT_PORT);
}

uint8_t TCA6408::readPolarityInversionRegister()
{
  return readRegister(POLARITY_INVERSION);
}

void TCA6408::writePolarityInversionRegister(uint8_t data)
{
  writeRegister(POLARITY_INVERSION, data);
}

void TCA6408::setAllPinsPolarityOriginal()
{
  writeRegister(POLARITY_INVERSION, 0x00);
}

void TCA6408::setAllPinsPolarityInverted()
{
  writeRegister(POLARITY_INVERSION, 0xFF);
}

uint8_t TCA6408::readConfigurationRegister()
{
  return readRegister(CONFIGURATION);
}

void TCA6408::writeConfigurationRegister(uint8_t data)
{
  writeRegister(CONFIGURATION, data);
}

void TCA6408::setAllPinsInput()
{
  writeRegister(CONFIGURATION, 0xFF);
}

void TCA6408::setAllPinsOutput()
{
  writeRegister(CONFIGURATION, 0x00);
}

void TCA6408::writeOutputRegister(uint8_t data)
{
  writeRegister(OUTPUT_PORT, data);
}

void TCA6408::setResetPin(uint8_t reset_pin)
{
  reset_pin_ = reset_pin;
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);
}

void TCA6408::attachInterrupt(uint8_t interrupt_pin, voidFuncPtr callback)
{
  ::attachInterrupt(digitalPinToInterrupt(interrupt_pin), callback, FALLING);
}
