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

const uint32_t SERIAL_BAUD_RATE = 115200;
const uint16_t LOOP_DELAY = 2000;

// Instantiate TCA6408
TCA6408 tca6408;
bool pins_inverted;

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

#if defined(ARDUINO_ARCH_RP2040)
  wire.setSDA(SDA_PIN);
  wire.setSCL(SCL_PIN);
#endif

  tca6408.setup(wire, DEVICE_ADDRESS);
  tca6408.setResetPin(RESET_PIN);

  pins_inverted = false;
}

void loop()
{
  uint8_t input_register = tca6408.readInputRegister();
  Serial.print("input_register: 0b");
  Serial.print(input_register, BIN);
  Serial.print(", ");
  Serial.println(input_register);

  uint8_t polarity_inversion_register = tca6408.readPolarityInversionRegister();
  Serial.print("polarity_inversion_register: 0b");
  Serial.println(polarity_inversion_register, BIN);

  uint8_t configuration_register = tca6408.readConfigurationRegister();
  Serial.print("configuration_register: 0b");
  Serial.println(configuration_register, BIN);

  if (pins_inverted)
  {
    Serial.println("all pins polarity were inverted now switching");
    pins_inverted = false;
    tca6408.setAllPinsPolarityOriginal();
  }
  else
  {
    Serial.println("all pins polarity were original now switching");
    pins_inverted = true;
    tca6408.setAllPinsPolarityInverted();
  }

  Serial.println("-------------------------------------");
  delay(LOOP_DELAY);
}
