#include "spod_library.h"
#include "crc16.h"
#include "config.h"

static const char* TAG = "CIRCUITS";


circuit_status_t circuit_status[PCMS*CIRCUITS];
circuit_local_status_t circuit_local[CIRCUITS];


void circuit_off(circuit_status_t* c)
{
    LOGD(TAG, "%d -- %s(): index=%d\r\n", millis(), __FUNCTION__, c->index);
    c->status = CIRCUIT_OFF;
    c->outCmd = 0x00;
    c->output = 0x00;
    c->blinkOn = 0xFF;
    c->blinkOff = 0;
}


void circuit_toggle(circuit_status_t* c)
{
    LOGD(TAG, "%d -- %s(): index=%d\r\n", millis(), __FUNCTION__, c->index);
    c->status = CIRCUIT_NOM;
    c->outCmd = 0xFF;
    c->output = 0xFF;
    c->blinkOn = 0;
    c->blinkOff = 0;
}


void circuit_strobe(circuit_status_t* c)
{
    circuit_config_t* cc = &pcm_config[current_pcm].circuits[c->index%CIRCUITS];
    uint32_t freq = cc->frequency;
    uint32_t msecs;

    if (freq == 0) freq = 10;
    msecs = 1000/freq;

    c->status = CIRCUIT_STROBE;
    c->outCmd = 0xFF;
    c->output = 0xFF;
    c->blinkOn = msecs/2;
    c->blinkOff = msecs - c->blinkOn;
    LOGD(TAG, "%d -- %s(): index=%d, freq=%d, msecs=%d, blinkOn=%d, blinkOff=%d\r\n", 
        millis(), __FUNCTION__, c->index, freq, msecs, c->blinkOn, c->blinkOff);

    // TODO: need to add dim_level...
}


void circuit_wig(circuit_status_t* c)
{
    LOGD(TAG, "%d -- %s(): index=%d\r\n", millis(), __FUNCTION__, c->index);
    circuit_strobe(c);
    c->status = CIRCUIT_FLASH;
}


void circuit_wag(circuit_status_t* c)
{
    LOGD(TAG, "%d -- %s(): index=%d\r\n", millis(), __FUNCTION__, c->index);
    circuit_strobe(c);
    c->status = CIRCUIT_FLASH;
}


void send_circuit_status_pkt(uint8_t index)
{
    if (pcm1) {
        forward_packet(RS485_ARBITER, true, true, CIRCUIT_STATUS_PKT_TYPE, (uint8_t*)&circuit_status[index], sizeof(circuit_status[index]));
    } else {
        if (pcm1_known) pkt_put(pcm1_mac, CIRCUIT_STATUS_PKT_TYPE, (uint8_t*)&circuit_status[index], sizeof(circuit_status[index]), 2);
    }
}
