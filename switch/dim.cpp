#include "switch.h"


bool dim(void* unused)
{
#if 0
    for (int i = 0; i < 8; i++) {
        uint8_t i2 = i + 8 * settings->sourceAddress;
        
        if (switchIsDimming[i]) {
            if (dimDirection[i]) {
                if (dimValue[i2] < 254) dimValue[i2]++;
            } else {
                if (dimValue[i2] > 0) dimValue[i2]--;
            }
            
            //uint8_t buffer[5] = {SWITCH_PACKET | settings->sourceAddress, oneHot(i), dimValue[i2], 0xFF, 0x00};

            // TODO: uncomment once serial OTA (RS485) is working
            //pkt_put(pkt_broadcast, CAN_PKT_TYPE, buffer, sizeof(buffer), 2); // TODO: PDB board or potentially another switch?
        }
    }
#endif
    return true;
}