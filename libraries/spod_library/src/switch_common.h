//-----------------------------------------------------------------------------
//  switch.h
//
//-----------------------------------------------------------------------------

#ifndef __SWITCH_COMMON_H__
#define __SWITCH_COMMON_H__

#include "config.h"

#define SWITCH_MAGIC 0xFEED
#define SWITCH_VERSION 1

#define PRIMARY_ACTIVATION   0x01
#define SECONDARY_ACTIVATION 0x02
#define TRIGGER_LATCHED      0x40
#define TRIGGER_ACTIVE_LOW   0x80

#define TOGGLE_INPUT	0x01
#define MOMENTARY_INPUT	0x02
#define LONG_INPUT		0x04
#define DOUBLE_INPUT	0x08
#define LOCKED_INPUT    0x80
#define PRIMARY_INPUT   (TOGGLE_INPUT|MOMENTARY_INPUT)
#define SECONDARY_INPUT (LONG_INPUT|DOUBLE_INPUT)


// commands for SWITCH_CONFIG_PKT_TYPE (if length < sizeof(switch_config_t))
typedef enum {
    SWITCH_APPLY,
    SWITCH_RESTORE,
    SWITCH_UPDATE,
} switch_config_command_t;

typedef struct __attribute__((__packed__)) {
    uint32_t circuits[2];   // bitmap of primary (circuits[0]) and secondary (circuits[1]) circuits linked to a switch
    uint8_t  triggers[2];   // bitmap of primary (triggers[0]) and secondary (triggers[1]) triggers qualifying a switch
} switch_pcm_t;

// ***IMPORTANT*** need to keep this structure smaller than 256 bytes + packet header so we can transfer on
// RS485 BUS!!! (currently 52 bytes)

typedef struct __attribute__((__packed__)) {
    uint16_t crc;           // CRC16 of remaining switch_config_t structure, if 0 human generated file, assume good!
    uint16_t magic;         // SWITCH_MAGIC
    uint16_t version;       // SWITCH_VERSION
    uint16_t sizeof_this; // sizeof(switch_config[0])
    // don't add anything before this
    
    uint8_t config_index;   // 0..7 (auto trigger is 8)
    uint8_t switch_index;   // 0..7 (auto trigger is 8)
    uint8_t inputs;         // TOGGLE_INPUT .. DOUBLE_INPUT
    uint8_t unused;         // unused, available for future use
    switch_pcm_t pcm[PCMS];

    // add new stuff here...
} switch_config_t;


typedef struct {
    uint8_t index;          // 0 to CONFIGS*SWITCHES - 1
    uint8_t activation;     // TOGGLE_INPUT .. DOUBLE_INPUT | LOCKED_INPUT
    uint8_t owner;          // rs485 address of controller that activated the switch (locked if MOMENTARY_INPUT)
    uint8_t unused;         // available for future use
    uint32_t color;         // RGB color
} switch_status_t;


extern switch_status_t switch_status[CONFIGS*SWITCHES];  // ephemeral, not saved to flash
extern switch_config_t switch_config[CONFIGS+PCMS][SWITCHES];   // saved to flash

extern void switch_init();
extern bool switch_is_visible(uint8_t index);
extern uint32_t switch_triggers(uint8_t index, bool secondary);
extern switch_config_t* switch_config_ptr(uint8_t index);
extern bool switch_dirty(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p);
extern void zero_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p);
extern bool read_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p);
extern bool write_switch_config(uint8_t cfg_index, uint8_t switch_index, switch_config_t* p);
extern void send_switch_status_pkt(uint8_t index, uint8_t activation, uint32_t color);
extern void forward_switch_status_pkt(switch_status_t* s, uint8_t pi);
extern void send_switch_config_pkt(uint8_t command, uint8_t config_index, uint8_t switch_index);

#endif
