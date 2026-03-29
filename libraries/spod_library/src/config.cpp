#include "spod_library.h"
#include "crc16.h"
#include "config.h"

static const char* TAG = "CFG";


uint8_t cfgindex;
uint8_t configs = 0;
config_t config;


void zero_config(config_t* c)
{
    memset((uint8_t*)c, 0, sizeof(*c));
}


bool config_sanity(config_t* c)
{
    bool dirty = false;
    if (c->cfgindex > CONFIGS-1) {
        c->cfgindex = 0;
        dirty = true;
    }
    if (c->inactivity_timeout < MIN_INACTIVITY_TIMEOUT) {
        c->inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT;
        dirty = true;
    }
    if (c->inactivity_timeout > MAX_INACTIVITY_TIMEOUT) {
        c->inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT;
        dirty = true;
    }
    if (c->deepsleep_time > MAX_DEEPSLEEP_TIME) {
        c->deepsleep_time = DEFAULT_DEEPSLEEP_TIME;
        dirty = true;
    }
    return !dirty;
}


bool read_config(config_t* c)
{
    uint16_t crc;
    size_t nbytes;
    
    memset(&config, 0, sizeof(config));
    nbytes = fs_read_file(CONFIG_FILENAME, (uint8_t*)&config, sizeof(config));
    if (nbytes > 8) {
        crc = crc16(((uint8_t*)c)+sizeof(crc), config.sizeof_this-sizeof(crc));
    }
    if (nbytes <= 8 || crc != c->crc) {
        fs_remove(CONFIG_FILENAME);
        if (c != &config) {
            zero_config(c);
        } else {
            config_factory_defaults(false);
            nbytes = sizeof(config);
            crc = 0;
        }
    }
    
    bool okay = c->magic == CONFIG_MAGIC && crc == c->crc;
    if (okay) {
        config_sanity(c);
        cfgindex = config.cfgindex;
    }
    return okay;
}


bool write_config(config_t* c)
{
    c->magic = CONFIG_MAGIC;
    c->version = CONFIG_VERSION;
    c->sizeof_this = sizeof(*c);
    c->crc = crc16(((uint8_t*)c)+sizeof(c->crc), sizeof(*c)-sizeof(c->crc));
    
    bool okay = fs_write_file(CONFIG_FILENAME, (uint8_t*)c, sizeof(*c));
    return okay;
}


static uint16_t fd_index;

uint16_t config_incremental_factory_defaults()
{
    uint8_t ci = fd_index/SWITCHES;
    uint8_t si = fd_index%SWITCHES;
    
    zero_switch_config(ci, si, &switch_config[ci][si]); 
    /*if (ci < configs)*/ write_switch_config(ci, si, &switch_config[ci][si]);
    if (ci < PCMS) {
        zero_switch_config(CONFIGS+ci, si, &switch_config[CONFIGS+ci][si]);
        /*if (ci < pcms)*/ write_switch_config(CONFIGS+ci, si, &switch_config[CONFIGS+ci][si]);
        zero_pcm_config(ci, &pcm_config[ci]);
        /*if (ci < pcms)*/ write_pcm_config(ci, &pcm_config[ci]);       
    }
    fd_index += 1;
    uint16_t percent = (fd_index*100)/(CONFIGS*SWITCHES);
    LOGD(TAG, "%d -- %s(): ci=%d,si=%d, fd_index=%d, percent=%d\r\n", millis(), __FUNCTION__, ci, si, fd_index, percent);
    return percent;
}


void config_factory_defaults(bool incremental)
{
    char filename[128];

    LOGD(TAG, "%d -- %s(incremental=%d): pcms=%d, configs=%d\r\n", millis(), __FUNCTION__, incremental, pcms, configs);
    zero_config(&config);
    config.backlight_color = 0;
    config.backlight_brightness = 0;
    config.inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT;
    config.deepsleep_time = DEFAULT_DEEPSLEEP_TIME;
    write_config(&config);
    
    if (incremental) {
        fd_index = 0;
    } else {
        for (uint8_t pi = 0; pi < PCMS; ++pi) {
            zero_pcm_config(pi, &pcm_config[pi]);
            /*if (pi < pcms)*/ {
                write_pcm_config(pi, &pcm_config[pi]);
            }
        }

        for (uint8_t ci = 0; ci < CONFIGS; ++ci) {
            for (uint8_t si = 0; si < SWITCHES; ++si) {
                zero_switch_config(ci, si, &switch_config[ci][si]);
                /*if (ci < configs)*/ {
                    write_switch_config(ci, si, &switch_config[ci][si]);
                }
                if (ci < PCMS) {
                    zero_switch_config(CONFIGS+ci, si, &switch_config[CONFIGS+ci][si]);
                    /*if (ci < pcms)*/ {
                        write_switch_config(CONFIGS+ci, si, &switch_config[CONFIGS+ci][si]);
                    }
                }
            }
        }
    }
}


// break config init into steps because the LittleFS takes some time and there are 101 files
// to read... the first files are the most important, the other initialization takes place
// in the first 5 seconds of after boot while waiting for the sytem to do something
//
// the later files are for upper PCMs which in general probably do not exist!

uint8_t config_init(uint8_t step)
{
    static uint8_t ci, si;

    LOGD(TAG, "%d -- %s(step=%d): pcms=%d, configs=%d\r\n", millis(), __FUNCTION__, step, pcms, configs);

    switch (step) {
    case 0:
        ci = si = 0;
        if (pcms == 0) pcms = 1;
        if (configs < pcms) configs = pcms;
        read_config(&config);
        return 1;

    case 1: read_pcm_config(0, &pcm_config[0]); return 2;
    case 2: read_pcm_config(1, &pcm_config[1]); return 3;
    case 3: read_pcm_config(2, &pcm_config[2]); return 4;
    case 4: read_pcm_config(3, &pcm_config[3]); return 5;
    
    case 0xFF: return 0xFF;

    default:
        read_switch_config(ci, si, &switch_config[ci][si]);
        if (ci < PCMS) {
            read_switch_config(CONFIGS+ci, si, &switch_config[CONFIGS+ci][si]);
        }
        if ((si += 1) == SWITCHES) {
            if ((ci += 1) == CONFIGS) return 0xFF;
            si = 0;
        } 
        return 5 + (ci*SWITCHES) + si;
    }

    return 0xFF;
}


void send_config_pkt(uint8_t command)
{
    if (pcm1_known) {
        if (command == CONFIG_UPDATE_CMD) {
            pkt_put(pcm1_mac, CONFIG_PKT_TYPE, (uint8_t*)&config, sizeof(config), 2);
        } else {
            pkt_put(pcm1_mac, CONFIG_PKT_TYPE, &command, sizeof(command), 2);
        }
    } else {
        LOGE(TAG, "%d -- %s(command=%d): pcm1_known=0\r\n", millis(), __FUNCTION__, command);
    }
}