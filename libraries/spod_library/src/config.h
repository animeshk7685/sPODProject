//-----------------------------------------------------------------------------
//  config.h
//
//-----------------------------------------------------------------------------

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define CONFIGS 8
#define TRIGGERS 8
#define SWITCHES 8
#define CIRCUITS 20
#define PCMS 4

#define CONFIG_FILENAME         "config.bin"
#define PCM_FILENAME_FORMAT     "pcm%d.bin"         // pcm0.bin .. pcm3.bin (4)
#define SWITCH_FILENAME_FORMAT  "switch%x%d.bin"    // switch00.bin .. switch77.bin, switch80.bin .. switchb7.bin (96)

#define CONFIG_MAGIC 0xCAFE
#define CONFIG_VERSION 1

#define CONFIG_UPDATE_CMD 1
#define CONFIG_FACTORY_DEFAULTS_CMD 2
#define CONFIG_GET_CRCS_CMD 3
#define CONFIG_PUT_CRCS_CMD 4

#define CONFIG_CRC_IDX      0
#define PCM1_CRC_IDX        (CONFIG_CRC_IDX+1)
#define SWITCH_00_CRC_IDX   (PCM1_CRC_IDX+PCMS)
#define MAX_CRC_IDX         (SWITCH_00_CRC_IDX + ((CONFIGS+PCMS)*SWITCHES)) // 1 + 4 + 96 = 101

#define MIN_INACTIVITY_TIMEOUT (5*1000)
#define MAX_INACTIVITY_TIMEOUT (60*1000)
#define DEFAULT_INACTIVITY_TIMEOUT (20*1000)
#define MAX_DEEPSLEEP_TIME (60*1000)
#define DEFAULT_DEEPSLEEP_TIME (10*1000)


typedef struct {
    uint16_t crc;                       // CRC16 of remaining config_t structure, if 0 human generated file, assume good!
    uint16_t magic;                     // CONFIG_MAGIC
    uint16_t version;                   // CONFIG_VERSION
    uint16_t sizeof_this;               // sizeof(config_t)
    // don't add anything before this

    uint32_t backlight_color;
    uint8_t backlight_brightness;
    uint8_t cfgindex;
    bool backlight;
    bool ignition;
    bool lockout;
    bool security;
    uint8_t password[32];
    uint32_t inactivity_timeout;
    uint32_t deepsleep_time;

    // add new stuff here
} config_t;


typedef struct __attribute__((__packed__)) {
    uint16_t crcs[MAX_CRC_IDX];
    uint8_t  configs;
    uint8_t  pcms;
    uint8_t  pcm_circuits[PCMS];
} config_crcs_t;


extern uint8_t cfgindex;  // current config index 0..configs-1
extern uint8_t configs;   // the number of configurations (based on PCMs)
extern config_t config;

/*
** The configuration is stored in the file system with the following filename encoding schemes:
**   config.bin   -- config
**   pcmP.bin     -- pcm_config[P] where P is 0-3
**   switchXY.bin -- switch_config[X][Y] where X is 0-7 and Y is 0-7
**   switchXY.bin -- switch_config[X][Y] where X is 8,9,A,B representing PCM0-3, and Y is 0-7 trigger
*/

extern void zero_config(config_t* c);
extern bool read_config(config_t* c);
extern bool write_config(config_t* c);
extern bool config_sanity(config_t* c);
extern uint16_t config_incremental_factory_defaults();
extern void config_factory_defaults(bool incremental);
extern uint8_t config_init(uint8_t step);
extern void send_config_pkt(uint8_t command);

#if defined(__cplusplus)
}
#endif

#endif
