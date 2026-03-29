/** PCA9956B LED driver operation sample
 *  
 *  This sample code is showing PCA9956B LED driver operation with direct register access.
 *  "write_r8()"" and "read_r8()"" can be used to write/read PCA9956B internal register. 
 *
 *  @author  Tedd OKANO
 *
 *  Released under the MIT license License
 *
 *  About PCA9956B:
 *    https://www.nxp.com/products/power-management/lighting-driver-and-controller-ics/led-drivers/24-channel-fm-plus-ic-bus-57-ma-20-v-constant-current-led-driver:PCA9956BTW
 */

#include <PCA9956B.h>

#define I2C_ADDR 0x01
#define I2C_CLK_PIN 5
#define I2C_DAT_PIN 6
#define I2C_FREQ 100000

PCA9956B ledd(Wire, I2C_ADDR);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Wire.begin(I2C_DAT_PIN, I2C_CLK_PIN);
  ledd.begin(1.0, PCA9956B::NONE);

  Serial.println("\n***** Hello, PCA9956B *****");
}

void loop() {
  for (int i = 0; i < 256; i += 8) {
    ledd.write_r8(PCA9956B::PWM12, i);

    Serial.print("PCA9956B PWM12 register value = ");
    Serial.println(ledd.read_r8(PCA9956B::PWM12), HEX);

    delay(100);
  }
}
