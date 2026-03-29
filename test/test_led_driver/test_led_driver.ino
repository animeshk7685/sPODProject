/** PCA9956B LED driver operation sample
 *  
 *  This sample code is showing PCA9956B LED driver operation with PWM value setting change.
 *  The ledd.pwm() takes channel number and PWM ratio. 
 *
 *  @author  Tedd OKANO
 *
 *  Released under the MIT license License
 *
 *  About PCA9956B:
 *    https://www.nxp.com/products/power-management/lighting-driver-and-controller-ics/led-drivers/24-channel-fm-plus-ic-bus-57-ma-20-v-constant-current-led-driver:PCA9956BTW
 */

#include <PCA9956B.h>

#if 1
#define I2C_ADDR 0x01
#define I2C_CLK_PIN 5
#define I2C_DAT_PIN 6
#else
#define I2C_ADDR 0x15
#define I2C_CLK_PIN 3
#define I2C_DAT_PIN 4
#endif


PCA9956B ledd(Wire, I2C_ADDR);


void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(I2C_DAT_PIN, I2C_CLK_PIN);
  ledd.begin(1.0, PCA9956B::NONE);
  ledd.pwm(0, 1.0);
  Serial.println("\n***** Hello, PCA9956B! *****");
}


static void set_leds(bool on)
{
  if (on) Serial.println("ON"); else Serial.println("OFF");
  for (int led = 0; led < 24; ++led) {
    ledd.pwm(led, on? 1.0 : 0.0);
    delay(250);
  }
}


void loop() {
  // set_leds(true);
  // set_leds(false);
}
