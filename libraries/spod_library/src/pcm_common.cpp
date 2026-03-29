#include "spod_library.h"
#include "crc16.h"


static const char* TAG = "PCM";

uint8_t pcms = 0;
uint8_t current_pcm;
uint8_t pcm_mac[PCMS][3];
uint8_t pcm_circuits[PCMS];
pcm_config_t pcm_config[PCMS];
pcm_status_t pcm_status[PCMS];


void pcm_setmac(uint8_t addr, uint8_t mac[3])
{
    if (addr >= RS485_PCM1 && addr <= RS485_PCM4) {
        memcpy(pcm_mac[addr-RS485_PCM1], mac, sizeof(pcm_mac[0]));
    }
}


void pcm_init(uint8_t addr, uint8_t mac[3])
{
    pcm_setmac(addr, mac);
    memset(pcm_circuits, 0, sizeof(pcm_circuits));
    memset(pcm_status, 0, sizeof(pcm_status));
    for (uint8_t pi = 0; pi < PCMS; ++pi) pcm_status[pi].which = pi;
}


void pcm_update_configs()
{
    configs = 0;
    for (uint8_t i = 0; i < PCMS; ++i) {
        switch (pcm_circuits[i]) {
        case CIRCUITS/2: configs += 1; if (pcms <= i) pcms = i+1; break;
        case CIRCUITS:   configs += 2; if (pcms <= i) pcms = i+1; break;
        default: break;
        }
    }

    LOGD(TAG, "%d -- %s(): configs=%d, pcms[]=(%d,%d,%d,%d)\r\n", 
        millis(), __FUNCTION__, configs, pcm_circuits[0], pcm_circuits[1], pcm_circuits[2], pcm_circuits[3]);
}

    
// TODO: determine if PCM810 or PCM1620 by seeing which i2c address responds and set pcm_circuits[addr] to either 8 or 16
void pcm_discover_type()
{
    uint8_t index = rs485_getaddr() - RS485_PCM1;
    pcm_circuits[index] = CIRCUITS/2;  // TODO: KLUDGE!!! replace with real code...
    pcm_update_configs();

    LOGD(TAG, "%d -- %s(): pcm_circuits[%d]=%d\r\n", millis(), __FUNCTION__, index, pcm_circuits[index]);
}


void pcm_process_info_pkt(uint8_t* data)
{ 
    uint8_t their_addr = data[0];
    data += 1;
    if (memcmp(data, pcm_circuits, sizeof(pcm_circuits)) != 0) {
        uint8_t our_addr = rs485_getaddr();
        bool is_pcm = our_addr >= RS485_PCM1 && our_addr <= RS485_PCM4;
        if (is_pcm) {
            our_addr -= RS485_BASE;
            data[our_addr] = pcm_circuits[our_addr];
        }
        for (uint8_t i = 0; i < PCMS; ++i) {
            if (data[i] != 0) {
                pcm_circuits[i] = data[i];
            }
        }
        if (is_pcm) send_pcm_info_pkt();

        pcm_update_configs();
    } else {
        LOGD(TAG, "%d -- %s(): UNCHANGED -- configs=%d, pcms[]=(%d,%d,%d,%d)\r\n", 
            millis(), __FUNCTION__, configs, pcm_circuits[0], pcm_circuits[1], pcm_circuits[2], pcm_circuits[3]);       
    }
}


bool pcm_dirty(uint8_t index, pcm_config_t* pcm, bool debug)
{
    bool dirty = memcmp(pcm, &pcm_config[index], sizeof(*pcm)) != 0;
    if (debug) LOGD(TAG, "%d -- %s(index=%d): dirty=%d\r\n", millis(), __FUNCTION__, index, dirty);
    return dirty;
}


bool pcm_local_circuit(uint8_t index)
{
    return (index/SWITCHES) == current_pcm;
}


// Return true if the bitmap of desired triggers are all set
bool pcm_triggers(uint32_t triggers)
{
    return true; // TODO
}


void pcm_activate_circuits(switch_status_t* s)
{
    uint8_t index = s->index;
    uint8_t secondary = (s->activation & SECONDARY_INPUT) != 0;
    uint32_t circuits;
    switch_config_t* c = switch_config_ptr(index);

    LOGD(TAG, "%d -- %s(): index=%d, secondary=%d\r\n", millis(), __FUNCTION__, index, secondary);
    circuits = c->pcm[current_pcm].circuits[secondary];
    if (circuits) {
        uint8_t shift = secondary? 4 : 0;
        bool on = s->activation != 0;
        for (uint8_t ci = 0; ci < pcm_config[current_pcm].circuit_count; ++ci) {
            if (circuits & (1<<ci)) {
                uint8_t index = (current_pcm*CIRCUITS)+ci;
                circuit_status_t* c = &circuit_status[index];
                uint8_t output = on? (pcm_config[current_pcm].circuits[ci].output >> shift) & 0xF : OUTPUT_UNUSED;

                LOGD(TAG, "%d -- %s(): index=%d, pi=%d, ci=%d, output=%d\r\n", millis(), __FUNCTION__, index, current_pcm, ci, output);
                switch (output) {
                case OUTPUT_UNUSED: circuit_off(c); break;
                case OUTPUT_TOGGLE: circuit_toggle(c); break; 
                case OUTPUT_STROBE: circuit_strobe(c); break;
                case OUTPUT_WIG:    circuit_wig(c); break;
                case OUTPUT_WAG:    circuit_wag(c); break;
                }
            }
        }
    }
}


void zero_pcm_config(uint8_t pcm_index, pcm_config_t* p)
{
    LOGD(TAG, "%d -- %s(pcm_index=%d)\r\n", millis(), __FUNCTION__, pcm_index);
    memset((uint8_t*)p, 0, sizeof(*p));
    p->pcm_index = pcm_index;
    p->circuit_count = pcm_circuits[pcm_index];

    /*if (pcm_index < pcms)*/ {
        for (uint8_t ci = 0; ci < CIRCUITS; ++ci) {
            p->circuits[ci].index = (pcm_index*CIRCUITS) + ci;
            p->circuits[ci].output = OUTPUT_TOGGLE;
        }
    } 
#if 0
    else {
        char filename[128];
        snprintf(filename, sizeof(filename), PCM_FILENAME_FORMAT, pcm_index);  
        fs_remove(filename);    
    }
#endif
}


bool read_pcm_config(uint8_t pcm_index, pcm_config_t* p)
{
    uint16_t crc;
    char filename[128];
    size_t nbytes;

    snprintf(filename, sizeof(filename), PCM_FILENAME_FORMAT, pcm_index);
    memset(p, 0, sizeof(*p));
    nbytes = fs_read_file(filename, (uint8_t*)p, sizeof(*p));
    if (nbytes > 8) {
        crc = crc16(((uint8_t*)p)+sizeof(crc), p->sizeof_this-sizeof(crc));
    }
    if (nbytes <= 8 || crc != p->crc) {
        LOGE(TAG, "%s(pi=%d) OOPS!!! -- nbytes=%d, crc=0x%x, p->crc=0x%x\r\n", __FUNCTION__, pcm_index, nbytes, crc, p->crc);
        zero_pcm_config(pcm_index, p);
        fs_remove(filename);
    }

    LOGE(TAG, "%s(pi=%d) nbytes=%d, crc=0x%x, p->crc=0x%x\r\n", __FUNCTION__, pcm_index, nbytes, crc, p->crc);
    
    return nbytes == sizeof(*p) && crc == p->crc;
}


bool write_pcm_config(uint8_t pcm_index, pcm_config_t* p)
{
    pcm_index &= (PCMS-1);
    p->magic = PCM_MAGIC;
    p->version = PCM_VERSION;
    p->sizeof_this = sizeof(*p);
    p->pcm_index = pcm_index;
    p->crc = crc16(((uint8_t*)p)+sizeof(p->crc), sizeof(*p)-sizeof(p->crc));
    
    pcm_config[pcm_index] = *p;
    
    char filename[128];
    snprintf(filename, sizeof(filename), PCM_FILENAME_FORMAT, pcm_index);
    bool okay = fs_write_file(filename, (uint8_t*)p, sizeof(*p));
    
    LOGE(TAG, "%s(pi=%d) okay=%d, sizeof(p)=%d\r\n", __FUNCTION__, pcm_index, okay, sizeof(*p));

    return okay;
}


void send_pcm_status_pkt(uint8_t index)
{
    if (pcm1) {
        forward_packet(RS485_ARBITER, true, true, CIRCUIT_STATUS_PKT_TYPE, (uint8_t*)&circuit_status[index], sizeof(circuit_status[index]));
    } else {
        if (pcm1_known) pkt_put(pcm1_mac, CIRCUIT_STATUS_PKT_TYPE, (uint8_t*)&circuit_status[index], sizeof(circuit_status[index]), 2);
    }
}


void send_pcm_config_pkt(uint8_t command, uint8_t pcm_index)
{
    if (pcm1_known) {
        if (command == PCM_UPDATE_CMD) {
            pkt_put(pcm1_mac, PCM_CONFIG_PKT_TYPE, (uint8_t*)&pcm_config[pcm_index], sizeof(pcm_config_t), 2);
        } else {
            uint8_t msg[2] = {command, pcm_index};
            pkt_put(pcm1_mac, PCM_CONFIG_PKT_TYPE, msg, sizeof(msg), 2);
        }
    } else {
        LOGE(TAG, "%d -- %s(command=%d, pi=%d): pcm1_known=%d!!!\r\n", millis(), __FUNCTION__, command, pcm_index, pcm1_known);
    }
}


void send_pcm_info_pkt()
{
    uint8_t data[2+sizeof(pcm_circuits)] = {PCM_INFO_PKT_TYPE, rs485_getaddr()};
    memcpy(data+2, pcm_circuits, sizeof(pcm_circuits));
    pkt_put(pkt_broadcast, PKT_SRCMAC|RS485_PKT_TYPE, data, sizeof(data), 0);
}