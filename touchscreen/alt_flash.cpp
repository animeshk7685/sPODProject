#include "touchscreen.h"
#include "image_data.h"
#include "main.h"

static const char* TAG = "ALTFLASH";


#define CALIBR8_FILENAME "calibr8.bin"
#define SETTINGS_FILENAME "settings.bin"
#define ICON_FILENAME_FORMAT "icon%d.bin"


static void write_settings()
{
    fs_write_file(SETTINGS_FILENAME, (uint8_t*)&settings, sizeof(settings));
}


// set "sourceAdrMask" to the new value, then call this function to update other variables and store in flash
void updateSourceAddress(void)     
{
    settings.sourceAdrMask = sourceAdrMask;
    write_settings();
}


void setWakeFromHib(bool set)
{
    settings.wakeFromHib = set;
    write_settings();
}


static bool read_icon(uint8_t index, uint8_t* icon)
{
    char filename[128];
    snprintf(filename, sizeof(filename), ICON_FILENAME_FORMAT, index);
    memset(icon, 0, BUTTON_FLASH_RAWDATA_LENGTH);
    size_t nbytes = fs_read_file(filename, (uint8_t*)icon, BUTTON_FLASH_RAWDATA_LENGTH);
    return nbytes == BUTTON_FLASH_RAWDATA_LENGTH;
}


static bool write_icon(uint8_t index, uint8_t* icon)
{
    char filename[128];
    snprintf(filename, sizeof(filename), ICON_FILENAME_FORMAT, index);
    return fs_write_file(filename, (uint8_t*)icon, BUTTON_FLASH_RAWDATA_LENGTH);
}


// api to load new icon into flash
// cindex = switch# (0-31)
// data is the icon in L1 format (57p x 57p) in array
void loadIcon(uint8_t cindex, uint8_t* data)
{
    uint32_t offset = cindex * BUTTON_FLASH_RAWDATA_LENGTH;
    read_icon(cindex, &button_Bitmap_RawData[offset]);
}


void loadDefaults(void)
{
    extern void loadSettings();
    memset(&settings, 0, sizeof(settings));
    memcpy(settings.buttonLabels, buttonDefaultLabels, sizeof(settings.buttonLabels));
    loadSettings();
    write_settings();
}


void loadSavedValues(void)
{
    memset(&settings, 0, sizeof(settings));
    if (fs_read_file(SETTINGS_FILENAME, (uint8_t*)&settings, sizeof(settings)) != sizeof(settings)) {
        fs_remove(SETTINGS_FILENAME);
        loadDefaults();
    }
}


void loadCalibration(void)
{
    if (fs_read_file(CALIBR8_FILENAME, (uint8_t*)&calibrate, sizeof(calibrate)) != sizeof(calibrate)) {
        memset(&calibrate, 0, sizeof(calibrate));
    }
}


void saveCalibration(void) 
{
	calibrate.crc = getConfigCrc();
    fs_write_file(CALIBR8_FILENAME, (uint8_t*)&calibrate, sizeof(calibrate));
}