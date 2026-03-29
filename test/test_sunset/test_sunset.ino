#include <GD23Z.h>

extern uint8_t sunrise_bitmap[];
extern uint32_t sizeof_sunrise_bitmap;

void setup()
{
  GD.begin();
  GD.cmd_inflate(0);
  GD.copy(sunrise_bitmap, sizeof_sunrise_bitmap);
  GD.BitmapHandle(0);
  GD.BitmapSource(0);
  GD.BitmapLayout(ARGB1555, 220, 147);
  GD.BitmapSize(NEAREST, BORDER, BORDER, 220, 147);
}

void loop()
{
  GD.SaveContext();
  GD.Begin(BITMAPS);
  GD.Vertex2ii(0, 0, 0, 0);
  GD.End();
  GD.RestoreContext();
  GD.swap();
}