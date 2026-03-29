#include <GD23Z.h>

static const unsigned char PROGMEM image[] = {   
  #include "image.h"
};

void setup()
{
  GD.begin();
  GD.cmd_inflate(0);
  GD.copy(image, sizeof(image));
  GD.BitmapHandle(0);
  GD.BitmapSource(0);
  GD.BitmapLayout(ARGB1555, 102, 173);
  GD.BitmapSize(NEAREST, BORDER, BORDER, 102, 173);
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
