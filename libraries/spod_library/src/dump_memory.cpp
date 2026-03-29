#include "spod_library.h"


void dump_memory(char* banner, uint32_t addr, uint8_t* data, int count)
{
    int i, offset = 0;

    if (data == NULL || count <= 0) {
        Serial.printf("%s(data=%p, count=%d)\r\n", __FUNCTION__, data, count);
        Serial.flush();
        return;
    }
  
    for (; count >= 16; count -= 16, offset += 16) {
        Serial.printf("%s_%08X: ", banner, addr + offset);
        
        for (i=0; i<16; ++i) {
            Serial.printf("%02x ", data[offset+i]);
        }
        for (i=0; i<16; ++i) {
            char ch = data[offset+i];
            Serial.printf("%c", ch >= ' ' && ch <= '~'? ch : '?');
        }
        Serial.printf("\r\n");
    }

    if (count) {
        Serial.printf("%s_%08X: ", banner, addr + offset);

        for (i=0; i<count; ++i) {
            Serial.printf("%02x ", data[offset+i]);
        }
        for (; i<16; ++i) {
            Serial.printf("   ");
        }
        for (i=0; i<count; ++i) {
            char ch = data[offset+i];
            Serial.printf("%c", ch >= ' ' && ch <= '~'? ch : '?');
        }
        Serial.printf("\r\n");
    }
}
