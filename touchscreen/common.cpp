#include "touchscreen.h"
#include <stdio.h>


tCalibrate calibrate;
tConfig settings;
char ScratchArray[100];
char ScratchArray1[20];
char ScratchArray2[20];
uint8_t configureIndex;

uint8_t sourceAdrMask = 0x00;

double voltage = 0;
uint8_t temperature = 0;


const tButtonLabel buttonDefaultLabels[64] = {
			{"Switch","1",""},
			{"Switch","2",""},
			{"Switch","3",""},
			{"Switch","4",""},
			{"Switch","5",""},
			{"Switch","6",""},
			{"Switch","7",""},
			{"Switch","8",""},

			{"Switch","9",""},
			{"Switch","10",""},
			{"Switch","11",""},
			{"Switch","12",""},
			{"Switch","13",""},
			{"Switch","14",""},
			{"Switch","15",""},
			{"Switch","16",""},

			{"Switch","17",""},
			{"Switch","18",""},
			{"Switch","19",""},
			{"Switch","20",""},
			{"Switch","21",""},
			{"Switch","22",""},
			{"Switch","23",""},
			{"Switch","24",""},

			{"Switch","25",""},
			{"Switch","26",""},
			{"Switch","27",""},
			{"Switch","28",""},
			{"Switch","29",""},
			{"Switch","30",""},
			{"Switch","31",""},
			{"Switch","32",""},

			{"Switch","1",""}, // TODO: 33 .. 64
			{"Switch","2",""},
			{"Switch","3",""},
			{"Switch","4",""},
			{"Switch","5",""},
			{"Switch","6",""},
			{"Switch","7",""},
			{"Switch","8",""},

			{"Switch","9",""},
			{"Switch","10",""},
			{"Switch","11",""},
			{"Switch","12",""},
			{"Switch","13",""},
			{"Switch","14",""},
			{"Switch","15",""},
			{"Switch","16",""},

			{"Switch","17",""},
			{"Switch","18",""},
			{"Switch","19",""},
			{"Switch","20",""},
			{"Switch","21",""},
			{"Switch","22",""},
			{"Switch","23",""},
			{"Switch","24",""},

			{"Switch","25",""},
			{"Switch","26",""},
			{"Switch","27",""},
			{"Switch","28",""},
			{"Switch","29",""},
			{"Switch","30",""},
			{"Switch","31",""},
			{"Switch","32",""},
};

float switchCurrent[64] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	

bool globalNeedsSleepReset = true;
bool globalNeedsWake = false;

bool isDeepSleepDisabled = IS_DEEP_SLEEP_DISABLED;//false;

uint32_t iconGpuBaseAddr = 0;

bool isConnected;
bool authGood;

machineEvent newEvt;
stfunc currentPage;

uint8_t  GInit;
uint8_t PDNPin;
uint8_t TrnsFlag;//Global flag to indicate that initialization is done
int32_t  GError;//Global error flag
uint16_t IntPin;//interrupt pin	
uint8_t  DispGpioPin;
uint8_t AudioGpioPin;//default GC pin assignments for diaplay and audio control	

void crc8(unsigned char *crc, unsigned char m)
{
	static const uint8_t crc8_table[256] = {
		0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
		0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
		0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
		0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
		0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
		0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
		0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
		0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
		0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
		0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
		0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
		0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
		0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
		0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
		0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
		0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
		0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
		0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
		0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
		0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
		0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
		0xfa, 0xfd, 0xf4, 0xf3
	};

	*crc = crc8_table[(*crc) ^ m];
}


unsigned int rand_interval(unsigned int min, unsigned int max)
{
	unsigned int r;
	const unsigned int range = 1 + max - min;
	const unsigned int buckets = RAND_MAX/range;
	const unsigned int limit = buckets * range;

	do {
		r = rand();
	} while (r >= limit);

	return min + (r/buckets);
}


void drawBatteryMeter(float range)
{
	// Battery indicator outline
	GC_ColorRGB(196,196,196);
	GC_Begin(FT_LINE_STRIP);
	GC_LineWidth(1 *16);
	GC_Vertex2f((BATT_X-3)*16,(BATT_Y-3)*16);
	GC_Vertex2f((BATT_X-3)*16,(BATT_Y + BATT_H + 3)*16);
	GC_Vertex2f((BATT_X + BATT_W + (BATT_SPACE*8) + 3)*16,(BATT_Y + BATT_H + 3)*16);
	GC_Vertex2f((BATT_X + BATT_W + (BATT_SPACE*8) + 3)*16,(BATT_Y-3)*16);
	GC_Vertex2f((BATT_X-3)*16,(BATT_Y-3)*16);
	GC_End();

	GC_Begin(FT_LINE_STRIP);
	GC_LineWidth(1 *16);
	GC_Vertex2f((BATT_X-3)*16,(BATT_Y+3)*16);
	GC_Vertex2f((BATT_X-10)*16,(BATT_Y+3)*16);
	GC_Vertex2f((BATT_X-10)*16,(BATT_Y +13)*16);
	GC_Vertex2f((BATT_X-3)*16,(BATT_Y + 13)*16);
	GC_End();

	if (range > 12.8) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(0,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X, BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W, BATT_Y + BATT_H, 0, 0);
		GC_End();
	}
	if (range > 12.49) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(0,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + BATT_SPACE, BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + BATT_SPACE, BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 12.18) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(0,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*2), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*2), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 11.87) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*3), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*3), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 11.56) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*4), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*4), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 11.25) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,255,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*5), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*5), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 10.94) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,0,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*6), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*6), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 10.63) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,0,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*7), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*7), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}

	if (range > 10.32) {
		GC_Begin(FT_RECTS);
		GC_ColorRGB(255,0,0);
		GC_LineWidth(1 *16);
		GC_Vertex2ii(BATT_X + (BATT_SPACE*8), BATT_Y, 0, 0);
		GC_Vertex2ii(BATT_X + BATT_W + (BATT_SPACE*8), BATT_Y + BATT_H, 0, 0);
		GC_End();
	}
}


void drawTemperatureMeter(float range, bool fah)
{
	GC_ColorRGB(196,196,196);
	GC_Begin(FT_LINE_STRIP);
	GC_LineWidth(1 *16);
	GC_Vertex2f((THERM_X-3)*16,(THERM_Y-3)*16);
	GC_Vertex2f((THERM_X-3)*16,(THERM_Y + THERM_H + 3)*16);
	GC_Vertex2f((THERM_X + BATT_W + (THERM_SPACE*3) + 3)*16,(THERM_Y + THERM_H + 3)*16);

	GC_Vertex2f((THERM_X + (BATT_W/2) + (THERM_SPACE*6))*16,(THERM_Y + THERM_H + 11)*16);
	GC_Vertex2f((THERM_X + (BATT_W/2) + (THERM_SPACE*9) + 3)*16,(THERM_Y + (THERM_H/2))*16);
	GC_Vertex2f((THERM_X + (BATT_W/2) + (THERM_SPACE*6))*16,(THERM_Y - 11)*16);

	GC_Vertex2f((THERM_X + BATT_W + (THERM_SPACE*3) + 3)*16,(THERM_Y-3)*16);
	GC_Vertex2f((THERM_X-3)*16,(THERM_Y-3)*16);
	GC_End();

	if (fah) {
		range = celsiusToFahrenheit(range);

		if (range > celsiusToFahrenheit(0)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X, THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W, THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(14)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + THERM_SPACE, THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + THERM_SPACE, THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(33)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*2), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*2), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(48)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*3), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*3), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if(range > celsiusToFahrenheit(65)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*4), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*4), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(82)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*5), THERM_Y-3, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*5), THERM_Y + THERM_H+3, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(99)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*6), THERM_Y-6, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*6), THERM_Y + THERM_H+6, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(116)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*7), THERM_Y-3, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*7), THERM_Y + THERM_H+3, 0, 0);
			GC_End();
		}

		if (range > celsiusToFahrenheit(133)){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*8), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*8), THERM_Y + THERM_H, 0, 0);
			GC_End();
	    }
	} else {
        if (range > 200)
            return;
        
		if (range > 0){
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X, THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W, THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > 14) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + THERM_SPACE, THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + THERM_SPACE, THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > 33) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(0,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*2), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*2), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > 48) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*3), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*3), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > 65) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*4), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*4), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}

		if (range > 82) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,255,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*5), THERM_Y-3, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*5), THERM_Y + THERM_H+3, 0, 0);
			GC_End();
		}

		if (range > 99) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*6), THERM_Y-6, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*6), THERM_Y + THERM_H+6, 0, 0);
			GC_End();
		}

		if (range > 116) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*7), THERM_Y-3, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*7), THERM_Y + THERM_H+3, 0, 0);
			GC_End();
		}

		if (range > 133) {
			GC_Begin(FT_RECTS);
			GC_ColorRGB(255,0,0);
			GC_LineWidth(1 *16);
			GC_Vertex2ii(THERM_X + (THERM_SPACE*8), THERM_Y, 0, 0);
			GC_Vertex2ii(THERM_X + THERM_W + (THERM_SPACE*8), THERM_Y + THERM_H, 0, 0);
			GC_End();
		}
	}
}


void drawImage(const tBitmap_header header[], int16_t x, int16_t y, float sX, float sY,sTagXY touch, uint8_t tagVal, uint32_t imageId)
{
    uint8_t f = header[0].Format;
	uint16_t w = header[0].Width;
	uint16_t h = header[0].Height;
	uint16_t s = header[0].Stride;

	GC_Cmd_LoadIdentity();
	GC_Cmd_Scale( sX*65536,sY*65536);//scale by 2x2
	GC_Cmd_SetMatrix();

	GC_Begin(FT_BITMAPS);
	GC_BitmapSource(imageId);
	GC_BitmapLayout(f, s, h);
    GC_BitmapSize(FT_BILINEAR,FT_BORDER,FT_BORDER,w * sX,h * sY);
	GC_Vertex2ii(x,y,0,0);//display Lena at (xoffset,yoffset) pixel coordinates
	GC_End();

	GC_Cmd_LoadIdentity();
	GC_Cmd_SetMatrix();
}

void drawText(int16_t x, int16_t y, uint8_t font, const char *line)
{
	GC_Cmd_Text(x, y, font, 0, line);
}

void drawTextColor(int16_t x, int16_t y, uint8_t font, const char *line, uint32_t color)
{
	GC_ColorRGB_int(color);
	drawText(x, y, font, line);
}


void drawTriText(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, 
				const char *line1, const char *line2, const char *line3, 
				bool isOn, uint32_t options, uint32_t color, bool offset)
{
	uint16_t x2  =  x + (w / 2);
	uint16_t x2m1 = x2 - 1;
	uint16_t y4 = (h / 4);
	uint16_t y3 = ( h / 3);

	if (line2[0] == '\0') {
		if (!isOn) {
			GC_ColorRGB_int(0x000000);
			GC_Cmd_Text( x2m1, y + (h / 2) - 1, font, options, line1);
		}
		GC_ColorRGB_int(color);
		GC_Cmd_Text( x2, y + (h / 2), font, options, line1);
	} else 
	if(line3[0] == '\0') {
		if (!isOn) {
			GC_ColorRGB_int(0x000000);
			GC_Cmd_Text( x2m1, y + y3 - 1, font, options, line1);
			GC_Cmd_Text( offset ? x2m1 + 15 : x2m1, y + (2 * y3) - 1, font, options, line2);
		}
		GC_ColorRGB_int(color);
		GC_Cmd_Text( x2, y + y3, font, options, line1);
		GC_Cmd_Text( offset ? x2 + 15 : x2, y + (2 * y3), font, options, line2);
	} else {
		if (!isOn) {                                 
			GC_ColorRGB_int(0x000000);
			GC_Cmd_Text(x2m1, y + y4 - 1, font, options, line1);
			GC_Cmd_Text( offset ? x2m1 + 15 : x2m1, y + (2 * y4) - 1, font, options, line2);
			GC_Cmd_Text( offset ? x2m1 + 30 : x2m1, y + (3 * y4) - 1, font, options, line3);
		}
		GC_ColorRGB_int(color);
		GC_Cmd_Text( x2, y + y4, font, options, line1);
		GC_Cmd_Text( offset ? x2 + 15 : x2, y + (2 * y4), font, options, line2);
		GC_Cmd_Text( offset ? x2 + 30 : x2, y + (3 * y4), font, options, line3);
	}

	GC_ColorRGB_int(settings.color);
}


void drawSingleTriText( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, 
					const char *line1, const char *line2, const char *line3, bool isOn, uint32_t options, uint32_t color)
{
	GC_ColorRGB_int(color);

	uint16_t x2  =  x + (w / 2);
	uint16_t x2m1 = x2 - 1;

	strcpy(ScratchArray,line1);
	strcat(ScratchArray," ");
	strcat(ScratchArray,line2);
	strcat(ScratchArray," ");
	strcat(ScratchArray,line3);

	if (!isOn) {
		GC_ColorRGB_int(0x000000);
		GC_Cmd_Text( x2m1, y + (h / 2) - 1, font, options, ScratchArray);
		GC_ColorRGB_int(color);
	}

	GC_Cmd_Text( x2, y + (h / 2), font, options, ScratchArray);
}


void drawButtonSmall(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, uint32_t isOn, float current)
{
	uint32_t tagoption = 0;  //no touch is default 3d effect and touch is flat effect
    uint8_t offset = 2;
    
    if(isOn){
		tagoption = FT_OPT_FLAT;
	}

	GC_Tag(0);
	if (tagoption) {
		GC_Cmd_FGColor(isOn /*settigns.highlightColor*/); // overload isOn -- non-zero means on and also RGB color (RED/BLUE/GREEN)
		GC_Cmd_Button(x, y, w, h, font, tagoption, "");
		GC_Cmd_FGColor(settings.fgColor);
	} else {
		GC_Cmd_Button(x, y, w, h, font, 0, "");
	}

    drawTriText(x, (y+offset), w, (h-(offset*2)), font, line1, line2, line3, false, FT_OPT_CENTER, settings.color, false);
    drawSwitchCurrent(x, y, w, h, 20, current);
    
	// Invisible Temp button
	GC_Begin(FT_POINTS);
	GC_Tag(tagVal);
	GC_PointSize(300);
	GC_ColorMask(0,0,0,0);
	GC_Vertex2ii(x+(w/2),y+(h/2),0,0);

	GC_End();
	GC_ColorMask(1,1,1,1);
}


void drawSwitchCurrent( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, float current)
{
    if (current >= 0) {
		int16_t curr_x = x + 29;
		int16_t curr_y = y + 19;        
		char xstring[5] = {0};
		
		dtostrf(current,4,1,xstring);
		GC_Tag(0);   
		drawTriText( curr_x, curr_y, w, h, font, xstring, "", "", false, FT_OPT_RIGHTX, settings.greyColor, true);    
	}
}


void drawButton2(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line, uint8_t tagVal, bool on, uint32_t on_color)
{
	GC_Tag(tagVal);
	if (!on) {
		GC_Cmd_Button(x, y, w, h, font, FT_OPT_CENTER, line);
	} else {
		GC_Cmd_FGColor(on_color);
		GC_ColorRGB(0,0,0);
		GC_Cmd_Button(x, y, w, h, font, FT_OPT_FLAT|FT_OPT_CENTER, line);
		GC_ColorRGB(196,196,196);
		GC_Cmd_FGColor(settings.fgColor);
	}
}


void drawButton(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn)
{
	uint32_t tagoption = 0;  //no touch is default 3d effect and touch is flat effect

	if (tagVal == touch.tag || isOn) {
		tagoption = FT_OPT_FLAT;
	}

	//assign tag value
	GC_Tag(tagVal);

	if (tagoption) {
		GC_Cmd_FGColor(settings.highlightColor);
		GC_Cmd_Button(x, y, w, h, font, tagoption, "");
		GC_Cmd_FGColor(settings.fgColor);
	} else {
		GC_Cmd_Button(x, y, w, h, font, 0, "");
	}

	drawTriText( x, y, w, h, font, line1, line2, line3, false, FT_OPT_CENTER, settings.color, false);
}


void drawInvisibleButton( int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool isOn)
{
	GC_LineWidth(4);
	
	GC_ColorMask(0, 0, 0, 0);

	GC_Begin(FT_RECTS);
	GC_Tag(tagVal);
	
	GC_Vertex2f(x*16, y*16);
	GC_Vertex2f((x + w)*16, (y + h)*16);
}


void drawButtonColor(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, const char *line1, const char *line2, const char *line3, sTagXY touch, uint8_t tagVal, bool on, uint32_t onColor, uint32_t offColor)
{
	GC_Tag(tagVal);
    if (on) {
		GC_Cmd_FGColor(onColor);
		GC_Cmd_Button(x, y, w, h, font, FT_OPT_FLAT, "");
	} else {
		GC_Cmd_FGColor(offColor);
		GC_Cmd_Button(x, y, w, h, font, 0, "");
	}
	GC_Cmd_FGColor(settings.fgColor);

	drawTriText( x, y, w, h, font, line1, line2, line3, false, FT_OPT_CENTER, settings.color, false);
}


uint16_t normalizeSlider(uint16_t val)
{
	uint32_t temp = 65535;
	temp = 65535 - val;
	return temp;
}


void drawSlider( int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t val, sTrackTag touch, uint8_t tagVal)
{
	uint32_t tagoption = 0;  //no touch is default 3d effect and touch is flat effect
	if (tagVal == touch.tag) {
		//tagoption = FT_OPT_FLAT;
	}

	//assign tag value
	GC_ColorRGB_int(settings.sliderColor);
	GC_Cmd_BGColor(settings.sliderBgColor);
	GC_Cmd_FGColor(settings.sliderFgColor);

	GC_Tag(tagVal);
	GC_Cmd_Slider(x, y, w, h, tagoption, val, 65535);

	GC_ColorRGB_int(settings.color);
	GC_Cmd_BGColor(settings.bgColor);
	GC_Cmd_FGColor(settings.fgColor);
}


void drawPercentGauge( int16_t x, int16_t y, const char * label, uint8_t font, uint32_t val)
{
	strcpy(ScratchArray, label);
	strcat(ScratchArray," ");
	Dec2Ascii(ScratchArray, (val * 100) / 65535);
	strcat(ScratchArray,"%");
	GC_Cmd_Text(x,y,font, 0, ScratchArray);
}


void drawBatteryVoltage( int16_t x, int16_t y, const char * label, uint8_t font, double val)
{
	char xstring[10] = {0};
   
	strcpy(ScratchArray, label);
	strcat(ScratchArray," ");
	dtostrf(val,4,1,xstring);
	strcat(ScratchArray, xstring);
	strcat(ScratchArray," V");
       
    GC_ColorRGB_int(settings.greyColor);
	GC_Cmd_Text(x,y,font, 0, ScratchArray);
    GC_ColorRGB_int(settings.color);
}


void drawNum( int16_t x, int16_t y, uint8_t font, uint32_t color, double val)
{
	char xstring[10] = {0};
   
	dtostrf(val,4,1,xstring);
	strcpy(ScratchArray, xstring);
	GC_ColorRGB_int(color);
	GC_Cmd_Text(x, y, font, 0, ScratchArray);
}


void drawInt( int16_t x, int16_t y, uint8_t font, uint32_t color, uint32_t val)
{
	char xstring[10] = {0};
   
	dtostrf(val,2,0,xstring);
	strcpy(ScratchArray, xstring);
	GC_ColorRGB_int(color);
	GC_Cmd_Text(x, y, font, 0, ScratchArray);
}


void drawSourceNum( int16_t x, int16_t y, const char * label, uint8_t font, double val)
{
	char xstring[10] = {0};
   
	strcpy(ScratchArray, label);
	dtostrf(val,1,0,xstring);
	strcat(ScratchArray, xstring);
	GC_Cmd_Text(x,y,font, 0, ScratchArray);
}


float celsiusToFahrenheit(uint8_t cels)
{
    int32_t tempC = cels;
    float tempF;
    tempC = tempC > 200 ? tempC - 256 : tempC;
    tempF = (1.8 * (float)tempC) + 32.0;
	return tempF;
}


void drawTempVal( int16_t x, int16_t y, const char * label, uint8_t font, uint8_t val, bool fah)
{
    int tempVal = val;
    bool isNeg = tempVal > 200;
    
	char xstring[10];
	strcpy(ScratchArray, label);
	strcat(ScratchArray," ");
    
	if (fah) {
        tempVal = (int)celsiusToFahrenheit(tempVal);
        if (tempVal < 0) {
            tempVal = tempVal * -1;
            strcat(ScratchArray,"-");
        }
		dtostrf((uint16_t)tempVal,2,0,xstring);
	} else {
        if (isNeg) {
            strcat(ScratchArray,"-");
            tempVal = 256 - tempVal;
        }
		dtostrf(tempVal,2,0,xstring);
	}
	strcat(ScratchArray, xstring);
	strcat(ScratchArray, fah? " F" : " C");

    GC_ColorRGB_int(settings.greyColor);
	GC_Cmd_Text(x,y,font, 0, ScratchArray);
    GC_ColorRGB_int(settings.color);
}


void drawVersion( int16_t x, int16_t y, const char * label, uint8_t font, char* version)
{
	strcpy(ScratchArray, label);
    strcat(ScratchArray, version);
    
    GC_ColorRGB_int(settings.greyColor);
	GC_Cmd_Text(x,y,font, FT_OPT_RIGHTX, ScratchArray);
    GC_ColorRGB_int(settings.color);
}


void drawBacklightVal( int16_t x, int16_t y, const char * label, uint8_t font, uint8_t val, bool timer)
{
	char xstring[10];
	strcpy(ScratchArray, label);
	strcat(ScratchArray," ");
    
	if (timer) {
		if (val == 0) {
            strcat(ScratchArray,"OFF");
            GC_Cmd_Text(x,y,font, 0, ScratchArray);
            return;
        }
	} else {
		val = val * 2 * 100 / 255;
	}
    
    dtostrf(val,2,0,xstring);
    
	strcat(ScratchArray, xstring);
	strcat(ScratchArray, timer? " min" : " %");

	GC_Cmd_Text(x,y,font, 0, ScratchArray);
}


uint8_t movingAvg(uint8_t tvalue)
{
	static uint8_t prev_temp = 0;

	if (prev_temp == 0){
		prev_temp = tvalue;
		return prev_temp;
	} else {
		if (abs(prev_temp - tvalue) > 10){
			return prev_temp;
		} else {
			prev_temp = tvalue;
			return prev_temp;
		}
	}
}


/* Helper API to convert decimal to ascii - pSrc shall contain NULL terminated string */
int32_t Dec2Ascii(char *pSrc, int32_t value)
{
	int16_t Length;
	char *pdst, charval;
	int32_t CurrVal = value, tmpval, i;
	char tmparray[16], idx = 0; //assumed that output string will not exceed 16 characters including null terminated character

	//get the length of the string
	Length = strlen(pSrc);
	pdst = pSrc + Length;

	//cross check whether 0 is sent
	if (0 == value) {
		*pdst++ = '0';
		*pdst++ = '\0';
		return 0;
	}

	//handling of -ve number
	if (CurrVal < 0) {
		*pdst++ = '-';
		CurrVal = -CurrVal;
	}
	/* insert the digits */
	while (CurrVal > 0 && idx < sizeof(tmparray)) {
		tmpval = CurrVal;
		CurrVal /= 10;
		tmpval = tmpval - CurrVal * 10;
		charval = '0' + tmpval;
		tmparray[(int)idx++] = charval;
	}

	//flip the digits for the normal order
	for (i = 0; i < idx; i++) {
		*pdst++ = tmparray[idx - i - 1];
	}
	*pdst++ = '\0';

	return 0;
}


uint32_t getTextIndex(char* text)
{
	char* vline = strtok(text,"|");
	return vline != NULL? strlen(vline) : 0;
}


uint8_t oneHot(uint8_t sw)
{
	switch(sw) {
	case 0:return SW0;
	case 1:return SW1;
	case 2:return SW2;
	case 3:return SW3;
	case 4:return SW4;
	case 5:return SW5;
	case 6:return SW6;
	case 7:return SW7;
	default:return 0;
	}
}


uint8_t indexPos(uint8_t pos)
{
	switch(pos) {
	case SW0:return 0;
	case SW1:return 1;
	case SW2:return 2;
	case SW3:return 3;
	case SW4:return 4;
	case SW5:return 5;
	case SW6:return 6;
	case SW7:return 7;
	default: return 0xff;
	}
}


char *dtostrf (double val, signed char width, unsigned char prec, char *sout)
{
    int intVal = (int) val;
    int intValPrec;
    
    intValPrec = (int) (val * (10 * prec));        
    intValPrec = intValPrec % (10 * prec);
        
    if (prec == 0) {
        sprintf(sout, "%d", intVal);
    } else {
        sprintf(sout, "%d.%d", intVal, intValPrec);
    }
  
    return sout;
}