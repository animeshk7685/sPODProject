#include "stdFlash.h"
#include "status.h"

static const char* TAG = "FLASH";


#define STATUS_FILENAME "status.bin"


status_t status;
uint32_t passkey = 123456;


uint32_t readPasskey(void)
{
    passkey = status.passkey;
    passkey = 123456; // KLUDGE
    LOGI(TAG, "Read passkey: %d\r\n", passkey);
    return passkey;
}


bool readFlash()
{
    uint16_t crc;
    size_t nbytes;
    
    memset(&status, 0, sizeof(status));
    nbytes = fs_read_file(STATUS_FILENAME, (uint8_t*)&status, sizeof(status));
    if (nbytes > 8) {
        crc = crc16(((uint8_t*)&status)+sizeof(crc), status.sizeof_this-sizeof(crc));
    }
    if (nbytes <= 8 || crc != status.crc) {
        fs_remove(STATUS_FILENAME);
        loadFlashDefaults();
        nbytes = sizeof(status);
        crc = 0;
    }
    
    return crc == status.crc && status.magic == STATUS_MAGIC;
}


bool writeFlash(void)
{
    status.magic = STATUS_MAGIC;
    status.version = STATUS_VERSION;
    status.sizeof_this = sizeof(status);
    status.passkey = passkey;
    status.crc = crc16(((uint8_t*)&status)+sizeof(status.crc), sizeof(status)-sizeof(status.crc));
    return fs_write_file(STATUS_FILENAME, (uint8_t*)&status, sizeof(status));
}


void loadFlashDefaults(void)
{
    memset(&status, 0, sizeof(status));

    for (int i = 0; i < 8; i++) {
        /*status.pro[i].isInputLatch = true;*/
        status.pro[i].currentLimit = 30;
        //status.pro[i].isInputEnabled = true;
    }
    
    for (int i = 0; i < 64; i++) {
        lastRecVals[i].dimVal = 0xff;
        lastRecVals[i].onVal = 0xff;
        lastRecVals[i].offVal = 0;
    }

    status.address = 0xFF;
}