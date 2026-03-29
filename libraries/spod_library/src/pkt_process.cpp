//-----------------------------------------------------------------------------
//  pkt_process.cpp
//
//  Layer above packet protool, process the payload of the packets.
//  Includes OTA and SYS packets.
//
//-----------------------------------------------------------------------------

#include "spod_library.h"
#include "pkt_process.h"
#include "crc16.h"
#include "crc32.h"
#include "pkt.h"
#include "ota.h"


extern bool ota_active();


//#if defined(__cplusplus)
//extern "C" {
//#endif

static const char* TAG = "PKTPROC";

#define FPUT_TIMEOUT 25000 // timeout in msecs
#define TEMPFILE_NAME "/fput_temp.dat"


static uint8_t  data_cache[0x1000]; // download 32 128-byte packets consecutively without ACK
static uint16_t data_crc16[32];     // crc16 of each 128-byte packet
static uint32_t data_scoreboard;    // bitmap of received 128-byte packets
static uint32_t ota_length;
static uint8_t  cache_arbaddr;

static void (*ble_send_ota_ctrl_packet)(uint8_t dstmac[3], uint8_t* data, uint8_t leng);
static void (*packet_cb)(uint8_t ptype, uint8_t* data, uint8_t leng);
static const char* VERSION = NULL;


static void noop_packet_cb(uint8_t ptype, uint8_t* data, uint8_t leng) {}


void process_ota_data_packet(uint8_t* data, uint8_t leng)
{
    uint16_t offset;
    if (leng == 128+sizeof(offset)) {
        memcpy((uint8_t*)&offset, data, sizeof(offset));
        if ((offset & 0x7f) == 0 && offset <= (sizeof(data_cache)-128)) {
            static uint32_t last = 0;
            uint16_t index = offset/128;
            #if 0
            uint32_t now = millis();
            LOGI(TAG, "%d -- %s(): offset=%x, index=%d\r\n", now - last, __FUNCTION__, offset, index);
            last = now;
            #endif
            data_scoreboard |= (1<<(index));
            data_crc16[index] = crc16(data+sizeof(offset), 128);
            memcpy(data_cache+offset, data+sizeof(offset), 128);
        } else {
            LOGE(TAG, "%s(): OOPS -- incorrect offset=0x%x\r\n", __FUNCTION__, offset);
        }
    } else {
        LOGE(TAG, "%s(): OOPS -- incorrect leng=%d\r\n", __FUNCTION__, leng);
    }
}


static void serial_send_ota_ctrl_packet(uint8_t dstmac[3], uint8_t* data, uint8_t leng)
{
    pkt_put(dstmac, PKT_SRCMAC | OTA_CTRL_PKT_ACK_TYPE, data, leng, 2);
}


static void ack_ota_scoreboard_packet(uint8_t mac[3], const char* msg, 
    void (*send_ack_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng))
{
    LOGD(TAG, "%s('%s') -- ota_length=0x%x\r\n", __FUNCTION__, msg, ota_length);

    uint8_t data[256];
    uint8_t*  ptr = data;
    uint32_t crc;
    int leng = 2 + 16 + sizeof(data_scoreboard) + sizeof(data_crc16); // data without crc32
    *ptr++ = leng;
    *ptr++ = 0x50;
    memcpy(ptr, msg, 16); ptr += 16;
    memcpy(ptr, (uint8_t*)&data_scoreboard, sizeof(data_scoreboard)); ptr += sizeof(data_scoreboard);
    memcpy(ptr, (uint8_t*)data_crc16, sizeof(data_crc16)); ptr += sizeof(data_crc16);
    crc = crc32(0, data, leng);
    memcpy(data+leng, (uint8_t*)&crc, sizeof(crc));

    // send_ack_packet is a pointer to either ble_send_ota_ctrl_packet() or serial_send_ota_ctrl_packet()
    send_ack_packet(mac, data, leng+sizeof(crc));
}


static void process_ota_scoreboard_packet(uint8_t mac[3], uint8_t* data, uint8_t leng, 
    void (*send_ack_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng))
{
    //LOGD(TAG, "%s(): leng=%d, ota_length=%x\r\n", __FUNCTION__, leng, ota_length);
    //LOG_HEXDUMP(TAG, data, 32, ESP_LOG_INFO);
    
    if (leng != 10 || !ota_active()) {
        ack_ota_scoreboard_packet(mac, "NAK-LENG", send_ack_packet);
    } else {
        //LOGI(TAG, "OTA-%x\r\n", ota_length);
        if (data_scoreboard != 0xFFFFFFFF) {
            ack_ota_scoreboard_packet(mac, "NAK-SCORE", send_ack_packet);
            return;
        }
        uint32_t expect_crc;
        memcpy((uint8_t*)&expect_crc, data+2, sizeof(expect_crc));
        uint32_t calc_crc = crc32(0, data_cache, sizeof(data_cache));
        if (calc_crc != expect_crc) {
            ack_ota_scoreboard_packet(mac, "NAK-CRC", send_ack_packet);
            return;
        }

        if (ota_write(data_cache, sizeof(data_cache)) != ESP_OK) {
            ack_ota_scoreboard_packet(mac, "NAK-WRITE", send_ack_packet);
            return;           
        }
        if (ota_length == 0) {
            ms_delay(50);
        }
        ota_length += sizeof(data_cache);
        data_scoreboard = 0;
        ack_ota_scoreboard_packet(mac, "ACK", send_ack_packet);
    }
}


static void ack_ota_packet(uint8_t mac[3], const char* msg, 
        void (*send_ack_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng))
{
    uint8_t data[64];
    int leng = strlen(msg)+1;
    if (leng > sizeof(data)-2) leng = sizeof(data)-2;

    LOGD(TAG, "%s('%s'): leng=%d\r\n", __FUNCTION__, msg, leng);

    data[0] = leng+2;
    data[1] = 0x80;
    memcpy(&data[2], msg, leng);

    // points to either ble_send_ota_ctrl_packet() or serial_send_ota_ctrl_packet()
    send_ack_packet(mac, data, leng+2);

    ota_length = 0;
}


static void ota_setup(size_t total_length)
{
    memset(data_cache, 0, sizeof(data_cache));
    memset(data_crc16, 0, sizeof(data_crc16));
    data_scoreboard = 0;
    ota_start(total_length);
}


//          0123456789AB
// INPUT = "PDB-01.00.00"
static uint32_t version_number(const char* a)
{
    uint32_t accum = 0;
    for (int i = 4; a[i] != '\0'; ++i) {
        if (a[i] != '.') {
            accum = (accum*10) + (a[i] - '0');
        }
    }
    return accum;
}


void process_ota_ctrl_packet(uint8_t dstmac[3], uint8_t* data, uint8_t pleng, 
            void (*send_ack_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng))
{
    uint8_t leng = data[0];
    uint8_t addr = data[1] & 0x0F;
    uint8_t mode = data[1] & 0xF0;
    uint8_t* version = data+2;
    uint8_t* totleng = data+18;

    LOGI(TAG, "%s(): pleng=%d, leng=%d, addr=%d, mode=0x%x\r\n", __FUNCTION__, pleng, leng, addr, mode);
    //LOG_HEXDUMP(TAG, data, 32, ESP_LOG_INFO);

    if (mode == 0x50) {
        process_ota_scoreboard_packet(dstmac, data, leng, send_ack_packet);
        return;
    } 
    
    if (mode != 0x00 && mode != 0xF0) {
        ack_ota_packet(dstmac, VERSION, send_ack_packet);
        return;
    }
     
    // VERSION = "PDB-01.00.00"
    uint32_t this_board, ota_board;
    memcpy(&this_board, VERSION, sizeof(this_board));
    memcpy(&ota_board, version, sizeof(ota_board));

    if (ota_board != this_board) { // first 4 bytes are the board type -- must match (e.g. "PDB-")
        LOGE(TAG, "Will Not Update: Incompatible hardware (%x != %x)\r\n", ota_board, this_board);
        ack_ota_packet(dstmac, "ER:INCOMPATIBLE", send_ack_packet);
        return;
    }

    size_t total_length;
    memcpy(&total_length, totleng, sizeof(total_length));

    if (mode == 0xF0) {
        LOGI(TAG, "Forcing Update: %s --> %s\r\n", VERSION, version);
        ota_setup(total_length);
        ack_ota_packet(dstmac, "OK:FORCE UPDATE", send_ack_packet);
    } else {
        uint32_t this_version = version_number(VERSION);
        uint32_t ota_version  = version_number((const char*)version);

        if (ota_version < this_version) {
            LOGI(TAG, "Will Not Update: Newer app firmware installed\r\n");
            LOGI(TAG, "Current App Version: %s\r\n", VERSION);
            LOGI(TAG, "OTA App Version:     %s\r\n", version);
            ack_ota_packet(dstmac, "ER:TOO OLD", send_ack_packet);
        } else
        if (ota_version == this_version) {
            LOGI(TAG, "Will Not Update: VERSIONS MATCH! (%s)\r\n", VERSION);
            ack_ota_packet(dstmac, "ER:ALREADY MATCH", send_ack_packet);
        } else
        if (ota_version > this_version) {
            LOGI(TAG, "Updating: %s --> %s\r\n", VERSION, version);
            ota_setup(total_length);
            ack_ota_packet(dstmac, "OK:UPDATING", send_ack_packet);
        }
    }
}


static bool process_pkt_delay(uint32_t msecs)
{
    while (pkt_get(NULL)) {
       uint8_t ptype = pkt_type(NULL) & MASK_PKT_TYPE;
       pkt_release();
       if (ptype == ABORT_PKT_TYPE) { // TODO: need to fix this, ABORT_PKT_TYPE is a SYS_PKT_TYPE subtype...
        return true;
       }
    }
    pet_watchdog(nullptr);
    ms_delay(msecs);
    return false;
}


void process_abort_packet(uint8_t* pdata, uint8_t pleng)
{
    if (pleng > 128) pleng = 128;
    pdata[pleng] = '\0';
    LOGW(TAG, "ABORT(%s)\r\n", pdata);
}


void process_fget_packet(uint8_t* name, uint8_t leng)
{
    File file;
    char msg[128];
    uint8_t smac[3];
    uint32_t crc = 0;

    memcpy(smac, pkt_srcmac(NULL), sizeof(smac));
    if (!fs_open_for_reading(LittleFS, (const char*) name, file)) {
        snprintf(msg+sizeof(crc), sizeof(msg)-sizeof(crc), "OOPS, unable to open file '%s'\r\n", name);
    } else {
        uint8_t data[128];
        size_t total = 0;

        while (true) {
            LOGD(TAG, "%s(): reading %s total=%d...\r\n", __FUNCTION__, name, total);
            size_t nread = fs_read(file, data, sizeof(data));
            if (nread == 0) {
                break;
            }
            crc = crc32(crc, data, nread);
            pkt_put(smac, PKT_SRCMAC|FDAT_PKT_TYPE, data, (uint8_t)nread, 2);
            total += nread;
            if (process_pkt_delay(2)) {
                memcpy(msg, (uint8_t*)&crc, sizeof(crc));
                snprintf(msg+sizeof(crc), sizeof(msg)-sizeof(crc), "OOPS, file '%s' GET ABORTED!\r\n", name);
                LOGD(TAG, "%s\r\n", msg+sizeof(crc));
                pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, sizeof(crc) + strlen(msg+sizeof(crc)), 2);
                return;
            }
        }
        memcpy(msg, (uint8_t*)&crc, sizeof(crc));
        snprintf(msg+sizeof(crc), sizeof(msg)-sizeof(crc), "OKAY, sent file '%s' (bytes %d, crc 0x%x)!\r\n", name, total, crc);
    }
    LOGD(TAG, "%s\r\n", msg+sizeof(crc));
    pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, sizeof(crc)+strlen(msg+sizeof(crc)), 2);
}


void process_fput_packet(uint8_t* pdata, uint8_t pleng)
{
    File file;
    uint32_t size;
    uint32_t expected_crc;
    uint8_t smac[3];
    char name[128];
    char msg[128];

    memcpy(smac, pkt_srcmac(NULL), sizeof(smac));
    memcpy((uint8_t*)&size, pdata, sizeof(size));
    memcpy((uint8_t*)&expected_crc, pdata + sizeof(size), sizeof(expected_crc));
    memcpy((uint8_t*)name, pdata + sizeof(size) + sizeof(expected_crc), pleng - (sizeof(size)+sizeof(expected_crc)));
    LOGD(TAG, "%d -- %s(pleng=%d): size=%d, crc=0x%x, name='%s'\r\n", millis(), __FUNCTION__, pleng, size, expected_crc, name);
    Serial.flush();

    if (!fs_open_for_writing(LittleFS, (const char*) TEMPFILE_NAME, file)) {
        snprintf(msg, sizeof(msg), "OOPS, Unable to create file '%s'", name);
        LOGE(TAG, "%d -- %s(): %s\r\n", millis(), __FUNCTION__, msg);
        pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
    } else {
        size_t total = 0;
        uint32_t crc = 0;

        LOGD(TAG, "%d -- %s(): WAITING FOR PKTs...\r\n", millis(), __FUNCTION__); Serial.flush();
        for (uint32_t last = millis(); millis() - last < FPUT_TIMEOUT; ) {
            pet_watchdog(nullptr);
            while (pkt_get(NULL)) {
                size_t nwrote;
                uint8_t ptype = pkt_type(NULL) & MASK_PKT_TYPE;
                switch (ptype) {
                case ABORT_PKT_TYPE:
                    process_abort_packet(pkt_data(NULL), pkt_leng(NULL));
                    pkt_release();
                    snprintf(msg, sizeof(msg), "OOPS, file '%s' PUT -- ABORTED!", name);
                    LOGW(TAG, "%d -- %s(): %s\r\n", millis(), __FUNCTION__, msg);
                    pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
                                    
                    fs_close(file);
                    fs_remove(TEMPFILE_NAME);
                    return;

                case FDAT_PKT_TYPE:
                    last = millis();

                    if ((total & 0xFFF) == 0)
                        LOGD(TAG, "%d -- %s(): received %d bytes of file '%s'\r\n", millis(), __FUNCTION__, total, name);

                    nwrote = fs_write(file, pkt_data(NULL), pkt_leng(NULL));
                    if (nwrote != pkt_leng(NULL)) {
                        snprintf(msg, sizeof(msg), "OOPS, file '%s' PUT -- WRITE FAILED!", name);
                        LOGW(TAG, "%s(): %s\r\n", __FUNCTION__, msg);

                        pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
                        fs_close(file);
                        fs_remove(TEMPFILE_NAME);
                        pkt_release();
                        return;
                    }
                    crc = crc32(crc, pkt_data(NULL), pkt_leng(NULL));
                    pkt_release();

                    total += nwrote;
                    if (total >= size) {
                        fs_close(file);
                        if (crc != expected_crc) {
                            snprintf(msg, sizeof(msg), "OOPS, file '%s' PUT -- crc mismatch 0x%x != 0x%x!", 
                                name, crc, expected_crc);
                            LOGW(TAG, "%s(): %s\r\n", __FUNCTION__, msg);
                            pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
                            fs_remove("/fput_temp.dat");
                        } else {
                            snprintf(msg, sizeof(msg), "OKAY, created file '%s' (%d bytes)!", name, total);
                            LOGW(TAG, "%s(): %s\r\n", __FUNCTION__, msg);
                            pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
                            fs_remove(name);
                            fs_rename(TEMPFILE_NAME, name);
                        }
                        return;
                    }
                    break;

                default:
                    pkt_release();
                    break;
                }
            }
        }

        snprintf(msg, sizeof(msg), "OOPS, file '%s' PUT -- TIMED OUT!", name);
        LOGW(TAG, "%s(): %s\r\n", __FUNCTION__, msg);

        pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
        fs_close(file);
        fs_remove(TEMPFILE_NAME);
    }
}


void process_fack_packet(uint8_t* pdata, uint8_t pleng)
{
}


void process_frm_packet(uint8_t* name, uint8_t pleng)
{
    char msg[128];

    if (fs_remove((const char*)name)) {
        snprintf(msg, sizeof(msg), "OKAY, file '%s' removed", name);
    } else {
        snprintf(msg, sizeof(msg), "OOPS, unable to remove file '%s'", name);
    }
    LOGD(TAG, "%s\r\n", msg);
    
    uint8_t smac[3];
    memcpy(smac, pkt_srcmac(NULL), sizeof(smac));
    pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
}


static int file_count;

static bool fls_cb(uint8_t* smac, const char* name, size_t size)
{
    if (name != NULL) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s %d", name, fs_filesize(name));
        pkt_put(smac, PKT_SRCMAC|FDAT_PKT_TYPE, (uint8_t*)msg, strlen(msg)+1, 2);
        file_count += 1;
    }
    process_pkt_delay(10);
    return false;
}


void process_fls_packet(uint8_t* pdata, uint8_t pleng)
{
    char msg[128];
    uint8_t smac[3];
    memcpy(smac, pkt_srcmac(NULL), sizeof(smac));

    file_count = 0;
    bool success = fs_list(LittleFS, smac, fls_cb);
    if (success) {
        snprintf(msg, sizeof(msg), "OKAY, list complete (%d files)", file_count);
    } else {
        snprintf(msg, sizeof(msg), "OOPS, unable to list files");
    }
    LOGD(TAG, "%s\r\n", msg);
    pkt_put(smac, PKT_SRCMAC|FACK_PKT_TYPE, (uint8_t*)msg, strlen(msg), 2);
}


static void process_sys_packet(uint8_t sys_type, uint8_t* data, uint8_t leng)
{
    switch ((sys_pkt_type_t)sys_type) {
    case ABORT_PKT_TYPE: process_abort_packet(data, leng); break;
    case FGET_PKT_TYPE:  process_fget_packet(data, leng); break;
    case FPUT_PKT_TYPE:  process_fput_packet(data, leng); break;
    case FACK_PKT_TYPE:  process_fack_packet(data, leng); break;
    case FRM_PKT_TYPE:   process_frm_packet(data, leng); break;
    case FLS_PKT_TYPE:   process_fls_packet(data, leng); break;
    default: break;
    }
}


void process_rs485_packet(uint8_t mac[3], uint8_t pkt_type, uint8_t* data, uint8_t leng)
{
    uint8_t addr;

    switch ((rs485_pkt_type_t)pkt_type) {
    case ARBITER_REBOOTED_PKT_TYPE:
        if (rs485_getaddr() == RS485_ARBITER) {
            // TODO: what do we do here? The RS485 bus is not wired correctly
            rs485_arbiter_duplicate();
        }
        rs485_arbiter_lost();
        break;

    case PCM_INFO_PKT_TYPE:
        pcm_process_info_pkt(data);
        break;

    default:
        break;
    }
}


static void process_config_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
    if (leng >= sizeof(config_t)) {
        // CONFIG_UPDATE_CMD
        config_t cfg;
        memcpy(&cfg, data, sizeof(cfg));
        if (write_config(&cfg)) config = cfg;
        forward_packet(owner, true, true, CONFIG_PKT_TYPE, data, leng);
        LOGD(TAG, "%d -- %s(): CONFIG_UPDATE_CMD\r\n", millis(), __FUNCTION__);
    } else
    if (leng >= 1) {
        uint8_t command = data[0];
        switch (command) {
            case CONFIG_FACTORY_DEFAULTS_CMD:
            LOGD(TAG, "%d -- %s(): CONFIG_FACTORY_DEFAULTS_CMD\r\n", millis(), __FUNCTION__);
            config_factory_defaults(false);
            break;
        }
        forward_packet(owner, true, true, CONFIG_PKT_TYPE, data, leng);
    }
}


static void process_pcm_config_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
    if (leng >= sizeof(pcm_config_t)) {
        // PCM_UPDATE_CMD
        pcm_config_t pcm;
        memcpy(&pcm, data, sizeof(pcm));
        if (pcm.pcm_index < PCMS) {
            LOGI(TAG, "%d -- %s(leng=%d): pi=%d, PCM_UPDATE!\r\n", millis(), __FUNCTION__, leng, pcm.pcm_index);
            pcm_config[pcm.pcm_index] = pcm;
            forward_packet(owner, true, true, PCM_CONFIG_PKT_TYPE, data, leng);
            packet_cb(PCM_CONFIG_PKT_TYPE, data, leng);
        } else {
            LOGE(TAG, "%d -- %s(leng=%d): pi=%d\r\n", millis(), __FUNCTION__, leng, pcm.pcm_index);
        }
    } else
    if (leng >= 3) {
        uint8_t command = data[0];
        uint8_t pcm_index = data[1];
        if (pcm_index < PCMS) {
            switch (command) {
            case PCM_APPLY_CMD: // write to flash
                LOGI(TAG, "%d -- %s(leng=%d): pi=%d, PCM_APPLY!\r\n", millis(), __FUNCTION__, leng, pcm_index);
                write_pcm_config(pcm_index, &pcm_config[pcm_index]);
                break;
            case SWITCH_RESTORE: // restore from flash
                LOGI(TAG, "%d -- %s(leng=%d): ci=%d, si=%d, PCM_RESTORE!\r\n", millis(), __FUNCTION__, leng, pcm_index);
                read_pcm_config(pcm_index, &pcm_config[pcm_index]);
                break;
            }
            forward_packet(owner, true, true, PCM_CONFIG_PKT_TYPE, data, leng);
            packet_cb(PCM_CONFIG_PKT_TYPE, data, leng);
        }
    } else {
            LOGE(TAG, "%d -- %s(leng=%d): sizeof(pcm_config_t)=%d\r\n", millis(), __FUNCTION__, leng, sizeof(pcm_config_t));
    }
}


static void process_pcm_status_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
     if (leng >= sizeof(pcm_status_t)) {
        pcm_status_t status;
        memcpy(&status, data, sizeof(status));
        LOGD(TAG, "%d -- %s(): which=%d, ignSense=%d, overheated=%d, inputs=%x, changed=%x, triggers=%x\r\n", 
            millis(), __FUNCTION__, status.which, status.ignSense, status.overheated, status.inputs, status.changed, status.triggers);
        pcm_status[status.which] = status;
        forward_packet(owner, true, true, PCM_STATUS_PKT_TYPE, data, leng);
        packet_cb(PCM_STATUS_PKT_TYPE, data, leng);
    }
}


static void process_switch_config_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
    if (leng >= sizeof(switch_config_t)) {
        // SWITCH_UPDATE
        switch_config_t sw;
        memcpy(&sw, data, sizeof(sw));
        if (sw.config_index < CONFIGS+PCMS && sw.switch_index < SWITCHES) {
            LOGI(TAG, "%d -- %s(leng=%d): ci=%d, si=%d, SWITCH_UPDATE!\r\n", millis(), __FUNCTION__, leng, sw.config_index, sw.switch_index);
            switch_config[sw.config_index][sw.switch_index] = sw;
            forward_packet(owner, true, true, SWITCH_CONFIG_PKT_TYPE, data, leng);
            packet_cb(SWITCH_CONFIG_PKT_TYPE, data, leng);
        } else {
            LOGE(TAG, "%d -- %s(leng=%d): ci=%d, si=%d\r\n", millis(), __FUNCTION__, leng, sw.config_index, sw.switch_index);
        }
    } else
    if (leng >= 3) {
        uint8_t command = data[0];
        uint8_t config_index = data[1];
        uint8_t switch_index = data[2];
        if (config_index < CONFIGS+PCMS && switch_index < SWITCHES) {
            switch (command) {
                case SWITCH_APPLY: // write to flash
                LOGI(TAG, "%d -- %s(leng=%d): ci=%d, si=%d, SWITCH_APPLY!\r\n", millis(), __FUNCTION__, leng, config_index, switch_index);
                write_switch_config(config_index, switch_index, &switch_config[config_index][switch_index]);
                break;
                case SWITCH_RESTORE: // restore from flash
                LOGI(TAG, "%d -- %s(leng=%d): ci=%d, si=%d, SWITCH_RESTORE!\r\n", millis(), __FUNCTION__, leng, config_index, switch_index);
                read_switch_config(config_index, switch_index, &switch_config[config_index][switch_index]);
                break;
            }
            forward_packet(owner, true, true, SWITCH_CONFIG_PKT_TYPE, data, leng);
            packet_cb(SWITCH_CONFIG_PKT_TYPE, data, leng);
        }
    } else {
        LOGE(TAG, "%d -- %s(leng=%d): sizeof(switch_config_t)=%d\r\n", millis(), __FUNCTION__, leng, sizeof(switch_config_t));
    }
}


static void process_switch_status_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
    if (leng >= sizeof(switch_status_t)) {
        switch_status_t status;
        memcpy(&status, data, sizeof(status));
        LOGD(TAG, "%d -- %s(): index=%x(%d), activation=%x, owner=%x, color=%x\r\n", 
            millis(), __FUNCTION__, status.index, status.index, status.activation, status.owner, status.color);
        if (status.index < CONFIGS*SWITCHES) {
            if (switch_status[status.index].activation != MOMENTARY_INPUT || switch_status[status.index].owner != rs485_getaddr()) { // TODO: correct? Don't understand first clause, advise removing
                switch_status[status.index] = status;
                if (status.activation == MOMENTARY_INPUT && status.owner != rs485_getaddr()) {
                    switch_status[status.index].activation |= LOCKED_INPUT;
                }
                forward_packet(owner, true, true, SWITCH_STATUS_PKT_TYPE, data, leng);
                LOGD(TAG, "%d -- %s().%d\r\n", millis(), __FUNCTION__, __LINE__); Serial.flush();
                packet_cb(SWITCH_STATUS_PKT_TYPE, data, leng);
                LOGD(TAG, "%d -- %s().%d\r\n", millis(), __FUNCTION__, __LINE__); Serial.flush();
            }
        }
    }
}


static void process_circuit_status_packet(uint8_t* data, uint8_t leng)
{
    if (leng >= sizeof(circuit_status_t)) {
        circuit_status_t cs;
        memcpy(&cs, data, sizeof(cs));
        if (cs.index < PCMS*CIRCUITS) {
            LOGI(TAG, "%d -- %s(leng=%d): ci=%d, status=%x, current=%d, dim_level=0x%x\r\n", millis(), 
            __FUNCTION__, leng, cs.index, cs.status, cs.current, cs.outCmd);
            circuit_status[cs.index] = cs;
            packet_cb(CIRCUIT_STATUS_PKT_TYPE, data, leng);
        } else {
            LOGE(TAG, "%d -- %s(leng=%d): ci=%d\r\n", millis(), __FUNCTION__, leng, cs.index);
        }
    } else {
        LOGE(TAG, "%d -- %s(leng=%d): sizeof(circuit_status_t)=%d\r\n", millis(), __FUNCTION__, leng, sizeof(circuit_status_t));
    }
}


static void process_trigger_packet(uint8_t owner, uint8_t* data, uint8_t leng)
{
    /*TODO*/
    packet_cb(TRIGGER_PKT_TYPE, data, leng);
}


void pkt_register_callback(void (*packet_callback)(uint8_t ptype, uint8_t* data, uint8_t leng))
{
    packet_cb = packet_callback == NULL? noop_packet_cb : packet_callback;    
}


void pkt_process(uint8_t owner)
{
    if (VERSION) {
        pkt_type_t ptype = (pkt_type_t) (pkt_type(NULL) & MASK_PKT_TYPE);
        uint8_t* data = pkt_data(NULL);
        uint8_t leng = pkt_leng(NULL);
        
        //LOGD(TAG, "%d -- %s(): type=%x, leng=%d\r\n", millis(), __FUNCTION__, pkt_type(NULL), leng);
        //LOG_HEXDUMP("PKTRX", data, leng, ESP_LOG_DEBUG);

        if (pkt_mac_broadcast(pkt_dstmac(NULL)) && pkt_mac_ours(pkt_srcmac(NULL))) {
            LOGD(TAG, "%d -- %s(): DROPPING BROADCAST FROM US\r\n", millis(), __FUNCTION__);
            return;
        }
    
        switch (ptype) {
        case HEARTBEAT_PKT_TYPE:      board_process_heartbeat_packet(pkt_srcmac(NULL), data, pkt_type(NULL), leng); break;
        case SYS_PKT_TYPE:            process_sys_packet(*data, data+1, leng-1); break;
        case RS485_PKT_TYPE:          process_rs485_packet(pkt_srcmac(NULL), *data, data+1, leng-1); break;

        case CONFIG_PKT_TYPE:         process_config_packet(owner, data, leng); break;
        case PCM_CONFIG_PKT_TYPE:     process_pcm_config_packet(owner, data, leng); break;
        case PCM_STATUS_PKT_TYPE:     process_pcm_status_packet(owner, data, leng); break;
        case SWITCH_CONFIG_PKT_TYPE:  process_switch_config_packet(owner, data, leng); break;
        case SWITCH_STATUS_PKT_TYPE:  process_switch_status_packet(owner, data, leng); break;
        case CIRCUIT_STATUS_PKT_TYPE: process_circuit_status_packet(data, leng); break;
        case TRIGGER_PKT_TYPE:        process_trigger_packet(owner, data, leng); break;

        case OTA_CTRL_PKT_TYPE:       process_ota_ctrl_packet(pkt_srcmac(NULL), data, leng, serial_send_ota_ctrl_packet); break;
        case OTA_DATA_PKT_TYPE:       process_ota_data_packet(data, leng); break;
        case OTA_CTRL_PKT_ACK_TYPE:   if (ble_send_ota_ctrl_packet) ble_send_ota_ctrl_packet(pkt_srcmac(NULL), data, leng); break;
        default: break;
        }
    }
}


void pkt_process_init(const char version[32],
    void (*send_ota_ctrl_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng),
    void (*packet_callback)(uint8_t ptype, uint8_t* data, uint8_t leng))
{
    LOGD(TAG, "%s(): version='%s'\r\n", __FUNCTION__, version);
    VERSION = version;
    ble_send_ota_ctrl_packet = send_ota_ctrl_packet;
    pkt_register_callback(packet_callback);
}


void send_heartbeat_pkt(uint8_t rs485_addr)
{
    if (VERSION) {
        static pkt_heartbeat_t heartbeat;
        if (heartbeat.beat == 0) {
            uint32_t accum = 0;
            uint8_t number[3];
            int j = 0;
            
            for (int i = 4; j < 3 && VERSION[i] != '\0'; ++i) {
                if (VERSION[i] == '.') {
                    number[j++] = accum;
                    accum = 0;
                } else {
                    accum = (accum*10) + (VERSION[i] - '0');
                }
            }
            if (j < 3) {
                number[j] = accum;
            }
            heartbeat.major = number[0];
            heartbeat.minor = number[1];
            heartbeat.revision = number[2];
        }
        heartbeat.addr = rs485_addr;
        heartbeat.beat += 1;

        #if 0
        LOGD(TAG, "%d -- %s(rs485_addr=0x%x): v%d.%d.%d beats=0x%x\r\n", 
            millis(), __FUNCTION__, rs485_addr, heartbeat.major, heartbeat.minor, heartbeat.revision, heartbeat.beat);
        #endif
        pkt_put(pkt_broadcast, PKT_SRCMAC|HEARTBEAT_PKT_TYPE, (uint8_t*)&heartbeat, sizeof(heartbeat), 0);
    }
}


void send_arbiter_rebooted_pkt()
{
    uint8_t data = ARBITER_REBOOTED_PKT_TYPE;
    pkt_put(pkt_broadcast, PKT_SRCMAC|RS485_PKT_TYPE, &data, sizeof(data), 0);
}


//#if defined(__cplusplus)
//}
//#endif