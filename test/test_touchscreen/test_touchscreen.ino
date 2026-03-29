#include <GD23Z.h>

#define GC_AT_INT_PIN  0   // graphics controller interrupt
#define GC_AT_PD_PIN   3   // graphics controller power down

#define GC_SCLK_PIN    6   // graphics controller SPI clock
#define GC_MISO_PIN    2   // graphics controller SPI MISO
#define GC_MOSI_PIN    7   // graphics controller SPI MOSI
#define GC_CS_PIN     10   // graphics controller CS

void slow_boot()
{
// TODO: for development only -- we do this so we can see all of the output on the serial monitor
  for (int i = 0; i < 10; ++i) {
    Serial.printf("%s(): %d\r\n", __FUNCTION__, i);
    delay(1000);
  }
  Serial.flush();
}


void setup()
{
  Serial.begin(115200);
  pinMode(GC_AT_INT_PIN, INPUT_PULLUP);
  pinMode(GC_AT_PD_PIN, OUTPUT);
  //slow_boot();
  Serial.printf("%s(): setting GC_AT_PD_PIN high\r\n", __FUNCTION__);
  digitalWrite(GC_AT_PD_PIN, HIGH);

//  EEPROM.begin(512); Use only for ESP8266 and ESP32 calibration screen
  GD.begin();
  Serial.printf("%s(): GD.w=%d, GW.h=%d\r\n", __FUNCTION__, GD.w, GD.h);
}

void loop()
{
  GD.ClearColorRGB(0x000015);
  GD.Clear();
  GD.cmd_text(GD.w / 2, GD.h / 2, 25, OPT_CENTER, "Hello Shreel!!!");
  GD.swap();
}
