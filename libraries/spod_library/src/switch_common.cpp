#include "spod_library.h"
#include "crc16.h"


static const char* TAG = "SWITCH";

switch_status_t switch_status[CONFIGS*SWITCHES];
switch_config_t switch_config[CONFIGS+PCMS][SWITCHES];


void switch_init()
{
    memset(switch_status, 0, sizeof(switch_status));
}


bool switch_is_visible(uint8_t index)
{
    return cfgindex == (index/SWITCHES);
}


bool switch_dirty(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p)
{
    return memcmp(p, &switch_config[cfg_index][switch_index], sizeof(*p)) != 0;
}


switch_config_t* switch_config_ptr(uint8_t index)
{
    return &switch_config[index/SWITCHES][index%SWITCHES];
}


// Given the switch index, return the triggers needed to qualify switch activation
uint32_t switch_triggers(uint8_t index, bool secondary)
{
    return 0xFFFFFFFFUL; // TODO
}


void zero_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* s)
{
    cfg_index %= (CONFIGS+PCMS);
    switch_index &= (SWITCHES-1);

    LOGD(TAG, "%d -- %s(ci=%d, si=%d()\r\n", millis(), __FUNCTION__, cfg_index, switch_index);
    memset((uint8_t*)s, 0, sizeof(*s));
    s->config_index = cfg_index;
    s->switch_index = switch_index;

    if (cfg_index < CONFIGS) {
        // associate one switch with one circuit and make it a primary input TOGGLE
        // TODO: actually need to know the configuration of all the PCMs to do this
        // right (i.e., 8 or 16) For now we assume they are all 8...
        s->inputs = TOGGLE_INPUT;
        s->pcm[cfg_index&3].circuits[0] |= (1<<switch_index);
    }

    #if 0
    if (cfg_index < CONFIGS && cfg_index >= configs) {
        char filename[128];
        snprintf(filename, sizeof(filename), SWITCH_FILENAME_FORMAT, cfg_index, switch_index); 
        fs_remove(filename);     
    }
    #endif
}


bool read_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p)
{
    uint16_t crc;
    char filename[128];
    size_t nbytes;

    cfg_index %= (CONFIGS+PCMS);
    switch_index &= (SWITCHES-1);

    snprintf(filename, sizeof(filename), SWITCH_FILENAME_FORMAT, cfg_index, switch_index);
    memset(p, 0, sizeof(*p));
    nbytes = fs_read_file(filename, (uint8_t*)p, sizeof(*p));
    if (nbytes > 8) {
        crc = crc16(((uint8_t*)p)+sizeof(crc), p->sizeof_this-sizeof(crc));
    }
    if (nbytes <= 8 || crc != p->crc) {
        LOGE(TAG, "%s(ci=%d, si=%d) OOPS!!! -- nbytes=%d, crc=0x%x, p->crc=0x%x\r\n", __FUNCTION__, cfg_index, switch_index, nbytes, crc, p->crc);
        zero_switch_config(cfg_index, switch_index, p);
        fs_remove(filename);
    }

    LOGD(TAG, "%s(ci=%d, si=%d) nbytes=%d, crc=0x%x, p->crc=0x%x\r\n", __FUNCTION__, cfg_index, switch_index, nbytes, crc, p->crc);
    
    return nbytes == sizeof(*p) && crc == p->crc;
}


bool write_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p)
{
    size_t nbytes;
    char filename[128];

    cfg_index %= (CONFIGS+PCMS);
    switch_index &= (SWITCHES-1);

    p->magic = SWITCH_MAGIC;
    p->version = SWITCH_VERSION;
    p->sizeof_this = sizeof(*p);
    p->config_index = cfg_index;
    p->switch_index = switch_index;
    p->crc = crc16(((uint8_t*)p)+sizeof(p->crc), sizeof(*p)-sizeof(p->crc));

    switch_config[cfg_index][switch_index] = *p;

    snprintf(filename, sizeof(filename), SWITCH_FILENAME_FORMAT, cfg_index, switch_index);
    bool okay = fs_write_file(filename, (uint8_t*)p, sizeof(*p));
    
    LOGD(TAG, "%s(ci=%d, si=%d) okay=%d, sizeof(p)=%d\r\n", __FUNCTION__, cfg_index, switch_index, okay, sizeof(*p));

    return okay;
}


void send_switch_config_pkt(uint8_t command, uint8_t config_index, uint8_t switch_index)
{
    if (pcm1_known) {
        config_index %= (CONFIGS+PCMS);
        switch_index &= (SWITCHES-1);
    
        if (command == SWITCH_UPDATE) {
            LOGD(TAG, "%d -- %s(command=%d, ci=%d, si=%d): sizeof(switch_config_t)=%d\r\n", 
                millis(), __FUNCTION__, command, config_index, switch_index, sizeof(switch_config_t));
            pkt_put(pcm1_mac, SWITCH_CONFIG_PKT_TYPE, (uint8_t*)&switch_config[config_index][switch_index], sizeof(switch_config_t), 2);
        } else {
            uint8_t msg[3] = {command, config_index, switch_index};
            LOGD(TAG, "%d -- %s(command=%d, ci=%d, si=%d)\r\n", millis(), __FUNCTION__, command, config_index, switch_index);
            pkt_put(pcm1_mac, SWITCH_CONFIG_PKT_TYPE, msg, sizeof(msg), 2);
        }
    } else {
        LOGE(TAG, "%d -- %s(command=%d, ci=%d, si=%d): pcm1_known=%d\r\n", millis(), __FUNCTION__, config_index, switch_index, pcm1_known);
    }
}


void send_switch_status_pkt(uint8_t index, uint8_t activation, uint32_t color)
{
    switch_status_t status = {.index = index, .activation = activation, .owner = rs485_getaddr(), .color = color};
    switch_status[index] = status;
    if (pcm1_known) {
        LOGE(TAG, "%d -- %s(index=%d, activation=%d, color=%x)\r\n", millis(), __FUNCTION__, index, activation, color);
        pkt_put(pcm1_mac, SWITCH_STATUS_PKT_TYPE, (uint8_t*)&status, sizeof(status), 2);
    } else {
        LOGE(TAG, "%d -- %s(index=%d, activation=%d, color=%x): PCM1 UNKNOWN!11\r\n", millis(), __FUNCTION__, index, activation, color);
    }
}


void forward_switch_status_pkt(switch_status_t* s, uint8_t pi)
{
    if (!pkt_mac_zero(pcm_mac[pi])) {
        pkt_put(pcm_mac[pi], SWITCH_STATUS_PKT_TYPE, (uint8_t*)s, sizeof(*s), 2);
    }
}