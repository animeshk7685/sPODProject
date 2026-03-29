//-----------------------------------------------------------------------------
//  pcm_common.h
//
//-----------------------------------------------------------------------------

#ifndef __PCM_COMMON_H__
#define __PCM_COMMON_H__

#include "config.h"
#include "circuit.h"
#include "triggers.h"

#define PCM_MAGIC 0xCAFE
#define PCM_VERSION 1
#define PCM_1620    0x01

// commands for PCM_CONFIG_PKT_TYPE (if length < sizeof(pcm_config_t))
typedef enum {
    PCM_APPLY_CMD,
    PCM_RESTORE_CMD,
    PCM_UPDATE_CMD,
} pcm_config_command_t;


typedef struct __attribute__((__packed__)) {
    uint8_t which:3;
    uint8_t ignSense:1;
    uint8_t tempEdge:1;
    uint8_t is12v_not24:1;
    uint8_t overheated:1;
    uint8_t overheated2:1;
    uint8_t inputs;
    uint8_t changed;
    uint8_t triggers;
    adcVals_t batVolt;
    adcVals_t temp;
} pcm_status_t;


// ***IMPORTANT*** need to keep this structure smaller than 256 bytes + packet header so we can transfer on
// RS485 BUS!!! (currently 100 bytes)

typedef struct __attribute__((__packed__)) {
    uint16_t crc;           // CRC16 of remaining pcm_config_t structure, if 0 human generated file, assume good!
    uint16_t magic;         // PCM_MAGIC
    uint16_t version;       // PCM_VERSION
    uint16_t sizeof_this;    // sizeof(pcm_config[0])
    // don't add anything before this
    
    uint16_t avail;
    uint8_t pcm_index;      // 0..3
    uint8_t circuit_count;    // either CIRCUITS/2 or CIRCUITS
    circuit_config_t circuits[CIRCUITS];
    trigger_t        triggers[TRIGGERS];

    // add new stuff here...
} pcm_config_t;


extern uint8_t pcms;
extern uint8_t current_pcm;
extern uint8_t pcm_mac[PCMS][3];
extern uint8_t pcm_circuits[PCMS];
extern pcm_config_t pcm_config[PCMS];      // saved to flash
extern pcm_status_t pcm_status[PCMS];

extern void pcm_setmac(uint8_t addr, uint8_t mac[3]);
extern void pcm_init(uint8_t addr, uint8_t mac[3]);
extern bool pcm_local_circuit(uint8_t index);
extern void pcm_discover_type();
extern void pcm_update_configs();
extern void pcm_process_info_pkt(uint8_t* data);
extern bool pcm_dirty(uint8_t index, pcm_config_t* pcm, bool debug);
extern bool pcm_triggers(uint32_t triggers);
extern void pcm_activate_circuits(switch_status_t* s);
extern void zero_pcm_config(uint8_t pcm_index, pcm_config_t* p);
extern bool read_pcm_config(uint8_t pcm_index, pcm_config_t* p);
extern bool write_pcm_config(uint8_t pcm_index, pcm_config_t* p);
extern void send_pcm_status_pkt(uint8_t pcm_index);
extern void send_pcm_config_pkt(uint8_t command, uint8_t config_index);
extern void send_pcm_info_pkt();

#endif
