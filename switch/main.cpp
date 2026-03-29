#include "switch.h"
#include "timer.h"
#include "crc32.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static const char* TAG = "MAIN";


static tSettings __settings;
tSettings* settings = &__settings;


void saveSettings()
{
    LOGD(TAG, "%s(): Settings saved sizeof=0x%x sourceAddress=0x%x...\r\n", __FUNCTION__, sizeof(tSettings), settings->sourceAddress);
    settings->crc = crc32(0, settings, sizeof(*settings) - sizeof(settings->crc));
    fs_write_file("settings", (uint8_t*) settings, sizeof(*settings));
}


void factorySettings()
{
    LOGE(TAG, "%s(): DEFAULTS\r\n", __FUNCTION__);
    memset(settings, 0, sizeof(*settings));
    settings->backlightIntensity[RED] = 0;
    settings->backlightIntensity[GREEN] = 0;
    settings->backlightIntensity[BLUE] = 30;
    settings->indicatorIntensity = 100;
    settings->appSourceAddress = 0;
    settings->sourceAddress = 0;
    settings->sleepTimer = SLEEP_MIN;
    settings->isSecChng = 0;
    settings->isWakeFromIgn = 1;
    settings->isProMode = IS_DEFAULT_PRO;
        
    settings->noDeepSleep = 0;
    settings->isWritable = 0;
    settings->isWakeFromDS = 0;
    for (int i = 0; i < 64; i++) {
        settings->switches[i].type = 0;
        
        settings->switches[i].links = 0;
        settings->switches[i].isDimmable = 0;
        
        settings->switches[i].strobeOn = 0xFF;
        settings->switches[i].strobeOff = 0;
        settings->switches[i].isStrobeOrFlash = 0;
    }
        
    saveSettings();
}


void readSettings()
{
    int nbytes = fs_read_file("settings", (uint8_t*) settings, sizeof(*settings));
    uint32_t crc = crc32(0, (uint8_t*)settings, sizeof(*settings) - sizeof(crc));
    if (crc != settings->crc) {
        factorySettings();
    }
}