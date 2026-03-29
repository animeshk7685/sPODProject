#include <PCA95x5.h>

#define I2C_ADDR 0x32
#define I2C_CLK_PIN 5
#define I2C_DAT_PIN 6

PCA9535 ioex;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Wire.begin(I2C_DAT_PIN, I2C_CLK_PIN);
    ioex.attach(Wire, I2C_ADDR);
    ioex.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    ioex.direction(PCA95x5::Direction::OUT_ALL);
    ioex.write(PCA95x5::Level::L_ALL);
}

void loop() {
    for (size_t i = 0; i < 16; ++i) {
        Serial.print("set port high: ");
        Serial.println(i);

        ioex.write(static_cast<PCA95x5::Port::Port>(i), PCA95x5::Level::L);
        Serial.println(ioex.read(), BIN);
        delay(500);
    }

    for (size_t i = 0; i < 16; ++i) {
        Serial.print("set port low: ");
        Serial.println(i);

        ioex.write(static_cast<PCA95x5::Port::Port>(i), PCA95x5::Level::H);
        Serial.println(ioex.read(), BIN);
        delay(500);
    }
}
