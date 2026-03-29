/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "switch.h"

static const char* TAG = "485RX";

#define DEBUG_ENABLED 0

static uint8_t rxBuffer[255];

bool needsWakeupVavgReset = false;
static uint8_t voltAvgBuf[VOLT_AVG_SAMPLE_S] = {0};
static uint16_t voltAvg = 0;
static uint16_t voltAvgOlder = 0;
static uint8_t voltAvgSecCnt = 0;
static uint16_t voltAvgSecSum = 0;
static uint32_t voltAvgSecLastTime = 0;
static bool voltAvgNeedsInit = true;


#if 0 // TODO
static void led_packet()
{
    switch (bleRxBuffer[3]) {
    case RED:   settings->backlightIntensity[RED] = bleRxBuffer[4];   break;
    case GREEN: settings->backlightIntensity[GREEN] = bleRxBuffer[4]; break;
    case BLUE:  settings->backlightIntensity[BLUE] = bleRxBuffer[4];  break;
    default:    settings->indicatorIntensity = bleRxBuffer[4];        break;
    }
}
#endif


#if 0 // TODO
static void settings_packet()
{
    settings->appSourceAddress = bleRxBuffer[3];

    int length = bleRxBuffer[1] - 2;

    if(lastAddrChng != bleRxBuffer[4])
    {
        if(settings->sourceAddress != bleRxBuffer[4])
        {
            changeAddr = bleRxBuffer[4] + 1;
        }
        
        settings->sourceAddress = bleRxBuffer[4];
        
        lastAddrChng = bleRxBuffer[4];
    }

    settings->backlightIntensity[RED] = bleRxBuffer[5];
    settings->backlightIntensity[GREEN] = bleRxBuffer[6];
    settings->backlightIntensity[BLUE] = bleRxBuffer[7];
    settings->indicatorIntensity = bleRxBuffer[8];
    settings->sleepTimer = bleRxBuffer[9];

    if(length >= 15 && (bleRxBuffer[14] & 0x04))
    {
        settings->isWakeFromIgn = (bleRxBuffer[14] & 0x02) ? 1 : 0;

    for(i = 0; i < 8; i++){
        
        uint8_t i2 = settings->sourceAddress * 8 + i;
        
        settings->switches[i2].type = ((bleRxBuffer[10] >> i) & 0x01) ? MOMENTARY : TOGGLE;
        settings->switches[i2].isDimmable = ((bleRxBuffer[11] >> i) & 0x01) ? 1 : 0;
        settings->switches[i2].isStrobeOrFlash = ((bleRxBuffer[12] >> i) & 0x01) ? (((bleRxBuffer[13] >> i) & 0x01) ? 1 : 2) : 0;
    }
}
#endif


#if 0 // TODO
static void link_packet()
{
    myPrintf("Links Recieved... %d \n", bleRxBuffer[3]);

    uint8 offset = bleRxBuffer[3];

    if(offset > 3)
        return;

    updateLinks2(offset * 8 + 0, bleRxBuffer[4]);
    updateLinks2(offset * 8 + 1, bleRxBuffer[5]);
    updateLinks2(offset * 8 + 2, bleRxBuffer[6]);
    updateLinks2(offset * 8 + 3, bleRxBuffer[7]);
    updateLinks2(offset * 8 + 4, bleRxBuffer[8]);
    updateLinks2(offset * 8 + 5, bleRxBuffer[9]);
    updateLinks2(offset * 8 + 6, bleRxBuffer[10]);
    updateLinks2(offset * 8 + 7, bleRxBuffer[11]);

    if(offset == 3)
        saveSettings();
}
#endif


#if 0 // TODO
static void settings_packet()
{
    uint8_t swIndex = bleRxBuffer[3];
    if(swIndex > 32) return;

    if(bleRxBuffer[4] != 0)     // for versioning
        return;

    if(bleRxBuffer[5] > 0 && bleRxBuffer[5] < 0xFE)
    {
        dimValue[swIndex] = bleRxBuffer[5];
    }

    settings->switches[swIndex].type = (bleRxBuffer[6] & IS_MOMENTARY_MASK) > 0 ? MOMENTARY : TOGGLE;
    settings->switches[swIndex].isDimmable = (bleRxBuffer[6] & IS_DIMABLE_MASK) > 0 ? 1 : 0;

    switch(bleRxBuffer[6] & (IS_FLASH_MASK | IS_STROBE_MASK))
    {
        case 0:
            settings->switches[swIndex].isStrobeOrFlash = 0; 
        break;
        case IS_FLASH_MASK:
            settings->switches[swIndex].isStrobeOrFlash = 1;
        break;
        case IS_STROBE_MASK:
            settings->switches[swIndex].isStrobeOrFlash = 2;
        break;
        default:
            myPrintf("invalid stobe/flash: %d)%x\n", swIndex, bleRxBuffer[6]);
        break;
    }

    if(bleRxBuffer[7] != 0 && bleRxBuffer[8] != 0)
    {
        settings->switches[swIndex].strobeOn = bleRxBuffer[7];
        settings->switches[swIndex].strobeOff = bleRxBuffer[8];
    }

    if(swIndex >= 31)
        isSyncing = false;

    if(!isSyncing)
        saveSettings();
}
#endif


#if 0 // TODO
void processProPacket(void)
{
    bool updateFlash = 0;
    bool isPermWrite = 0;
    
    if(((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) // check that the app sent the correct type of packet
    {
        if((bleRxBuffer[1] & 0x01) == 1)    // read
        {
            sendProPacket = NUM_PRO_SYNC_PACKETS;
            return;
        }
        
        if((bleRxBuffer[1] & 0x02) > 0)    // isn't locked
        {
            isPermWrite = true;
        }
        else
        {
            isPermWrite = false;
        }
        
        if((bleRxBuffer[1] & 0x04) > 0)    // is writable until ble disconnect
        {
            isTempWritable = true;
        }
        else
        {
            isTempWritable = false;
        }
        
        
        if((bleRxBuffer[2] & 0x02) != 0 && isTempWritable) // update deep sleep flag
        {
            uint8_t nds = bleRxBuffer[2] & 0x01;
            if(settings->noDeepSleep != nds)
            {
                myPrintf("Deep Sleep disabled %d \n", nds);
                settings->noDeepSleep = nds;
                updateFlash = 1;
            }
        }
        
        isSyncing = (bleRxBuffer[1] & 0x08) ? 1 : 0;
        
        if(updateFlash)
        {
            saveSettings();
        }
    }
}
#endif


static void switch_packet()
{
#if 0
    uint8_t index = indexPos(rxBuffer[1]);
    if (index != 255) {
        uint8_t srcAdd = rxBuffer[0] & 0x03;
        uint8_t swAdd = index + (8 * srcAdd);
        
        if (rxBuffer[2]) {
            switchStatus[index] = 1;
            if (switchShort[index]) {
                switchShort[index] = 0;
                buttonStatus[index] = 0;
            }
        } else {
            switchStatus[index] = 0;
        }
        
        if (!switchShort[index]) updateSwitchStatusLeds(__LINE__);
        
        if (rxBuffer[3] != 0 && rxBuffer[4] != 0) {
            if (settings->switches[swAdd].strobeOn != rxBuffer[3] || settings->switches[swAdd].strobeOff != rxBuffer[4]) {
                settings->switches[swAdd].strobeOn = rxBuffer[3];
                settings->switches[swAdd].strobeOff = rxBuffer[4];
                saveSettings();
            }
        }
    }
#endif
}


static void system_packet()
{
    uint8_t localVolt = rxBuffer[1];
    
    if (voltAvgNeedsInit || needsWakeupVavgReset) {
        voltAvgNeedsInit = false;
        needsWakeupVavgReset = false;
        memset(voltAvgBuf, localVolt, sizeof(voltAvgBuf));
        voltAvgOlder = localVolt;
        voltAvg = localVolt;
    }
    
    #if 0
    // GRH-TODO: do we need something like this?
    if ((voltAvgOlder + VOLT_WAKE_THRESH) < localVolt) {
        if(settings->isWakeFromIgn) resetSleepTimer = true;
    }
    #endif

    voltAvgSecSum += localVolt;
    voltAvgSecCnt++;
    
    if ((millis() - voltAvgSecLastTime) > 1000) {
        voltAvgSecSum /= voltAvgSecCnt;
        voltAvgOlder = 0;
        voltAvg = 0;
        
        for (int i = 0; i < VOLT_AVG_SAMPLE_S; i++) {
            if (i < VOLT_AVG_SAMPLE_S - 1) {
                voltAvgBuf[i] = voltAvgBuf[i + 1];
            } else {
                voltAvgBuf[i] = voltAvgSecSum;
            }
            
            if (i < VOLT_AVG_SAMPLE_S1) {
                voltAvgOlder += voltAvgBuf[i];
            }
            
            if (i >= (VOLT_AVG_SAMPLE_S - VOLT_AVG_SAMPLE_S2)) {
                voltAvg += voltAvgBuf[i];
            }
        }
        
        voltAvgOlder /= VOLT_AVG_SAMPLE_S1;
        voltAvg /= VOLT_AVG_SAMPLE_S2;
        
        voltAvgSecCnt = 0;
        voltAvgSecSum = 0;
        voltAvgSecLastTime = millis();
    }
    
    rxBuffer[1] = voltAvg;      // for ble comm
}


static void debug_packet()
{
}


static void status_packet()
{
#if 0
    if (rxBuffer[2] == 8) {
        uint8_t index = indexPos(rxBuffer[1]);
        if (index != 255) switchShort[index] = 1;
    }
#endif
}


static void pro_packet()
{
#if 0
    myPrintf("CAN: pro rec -> %x %x %x %x %x\n", rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3], rxBuffer[4]);
    uint8_t index = indexPos(rxBuffer[1]);
    if (index != 255) {
        uint8_t srcAdd = rxBuffer[0] & 0x03;
        uint8_t swAdd = index + 8 * srcAdd;
        
        if (rxBuffer[2] & 0x20) {
            uint8_t linkPacketIndex = (rxBuffer[2] & 0xC0) == 0x40 ? 1 : 0;
            
            if (linkPacketIndex == 0) {
                updateLinks(swAdd, 1, rxBuffer[3]);
                updateLinks(swAdd, 0, rxBuffer[4]);
            } else {
                updateLinks(swAdd, 3, rxBuffer[3]);
                updateLinks(swAdd, 2, rxBuffer[4]);
            }
        }
    }
#endif
}


void rs485_receive()
{
    uint8_t owner;
    if (pkt_get(&owner)) {
        uint8_t rtype = pkt_type(NULL);
        pkt_type_t ptype = (pkt_type_t) (rtype & MASK_PKT_TYPE);
        board_type_t btype = (board_type_t) (rtype & MASK_BOARD_TYPE);
       
        #if DEBUG_ENABLED
        LOGD(TAG, "%d -- %s(): type=%x, leng=%d\n", millis(), __FUNCTION__, rtype, pkt_leng(NULL));
        #endif

        if (ptype != CAN_PKT_TYPE) {
            if (deepsleep_wakeup && ptype == TRIGGER_PKT_TYPE) {
                trigger_status_t trigger;
                memcpy(&trigger, pkt_data(NULL), sizeof(trigger));
                if (trigger.trigger_mask & TRIGGER_IGN) {
                    deepsleep_wakeup = false;
                    ignition_on = true;
                }
            }

            pkt_process(owner);
            if (!pcm1_known) {
                board_info_t bi;
                pcm1_known = board_tad_search(PDB_BOARD_TYPE, RS485_PCM1, &bi);
                if (pcm1_known) {
                    memcpy(pcm1_mac, bi.mac, sizeof(pcm1_mac));
                    LOGI(TAG, "%d -- %s(): pcm1_known!!!, mac=(%02x:%02x:%02x)\r\n", millis(), __FUNCTION__, pcm1_mac[0], pcm1_mac[1], pcm1_mac[2]);
                }
            }
        } else {
            LOG_HEXDUMP("PKTRX", pkt_data(NULL), pkt_leng(NULL), ESP_LOG_DEBUG);
            uint8_t bytesRead = pkt_leng(NULL);
            memcpy(rxBuffer, pkt_data(NULL), bytesRead);
            if (rxBuffer[0] == (SWITCH_PACKET | settings->sourceAddress))
                switch_packet();
            else if (rxBuffer[0] == SYSTEM_PACKET)
                system_packet();
            else if (((rxBuffer[0] & 0xF0) == DEBUG_PACKET) && (rxBuffer[1] == 0x0A))
                debug_packet();
            else if (rxBuffer[0] == (STATUS_PACKET | settings->sourceAddress))
                status_packet();
            else if(rxBuffer[0] == PRO_PACKET)
                pro_packet();
        }
        pkt_release();
    }
}