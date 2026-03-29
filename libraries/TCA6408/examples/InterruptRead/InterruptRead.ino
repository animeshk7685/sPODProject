#include <TCA6408.h>

#if !defined(ARDUINO_ARCH_RP2040)
TwoWire & wire = Wire;
#else
TwoWire & wire = Wire1;
uint8_t SDA_PIN = 26;
uint8_t SCL_PIN = 27;
#endif

const TCA6408::DeviceAddress DEVICE_ADDRESS = TCA6408::DEVICE_ADDRESS_0;
uint8_t RESET_PIN = 0;
uint8_t INTERRUPT_PIN = 1;

const uint32_t SERIAL_BAUD_RATE = 115200;
const uint16_t LOOP_DELAY = 2000;

// Instantiate TCA6408
TCA6408 tca6408;

void interruptCallback()
{
  uint8_t input_register = tca6408.readInputRegister();
  Serial.print("input_register: 0b");
  Serial.print(input_register, BIN);
  Serial.print(", ");
  Serial.println(input_register);
  Serial.println("-------------------------------------");
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

#if defined(ARDUINO_ARCH_RP2040)
  wire.setSDA(SDA_PIN);
  wire.setSCL(SCL_PIN);
#endif

  tca6408.setup(wire, DEVICE_ADDRESS);
  tca6408.setResetPin(RESET_PIN);

  tca6408.attachInterrupt(INTERRUPT_PIN, interruptCallback);
}

void loop()
{
}
