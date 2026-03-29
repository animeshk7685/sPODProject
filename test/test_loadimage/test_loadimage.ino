#include <GD23Z.h>

// verified and compressed a JPEG image, then converted to a C file
// stripped the uint8_t declaration and converted to an H file that
// is included here -- this makes it easy to do sizeof(image), etc.

// takes 12ms to render
static uint8_t image[] = {
  #include "image.h"
};

uint32_t setup_time;

void setup() {
  setup_time = millis();
  GD.begin();
  GD.BitmapHandle(0);
  GD.cmd_loadimage(0, 0);
  GD.cmd_n(image, sizeof(image));
  setup_time = millis() - setup_time;
}

void loop() {
  uint32_t start = millis();

  GD.ClearColorRGB(0x80,0x80,0x80);
  GD.Clear();
  GD.Begin(BITMAPS);  
  GD.cmd32(BLEND_FUNC(SRC_ALPHA, ONE_MINUS_SRC_ALPHA));
  //GD.cmd_bgcolor(0xFF0000);
  //GD.cmd_fgcolor(0x000000);
  //GD.ColorRGB(0,255,0);
  GD.ColorA(128);
  GD.Vertex2ii(0, 0, 0); 
  GD.ColorRGB(255,0,0);  
  GD.cmd_number(30,260,21,OPT_CENTER|OPT_SIGNED,millis() - start);
  GD.cmd_number(60,260,21,OPT_CENTER|OPT_SIGNED,setup_time);
  GD.swap();
}

