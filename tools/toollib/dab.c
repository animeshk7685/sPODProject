#include "lib.h"
#include "common.h"
#include "pkt.h"
#include "pkt_type.h"
#include "board_type.h"


#ifdef __cplusplus
extern "C"
{
#endif

static SPKT_DESC* desc = NULL;
static int (*input_cb)(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes) = NULL;

static volatile bool dab_ack_received;
static volatile bool dab_input_received;
static uint8_t       dab_type_expected;
static uint8_t*      dab_data;
static uint8_t       dab_leng;


int dab_acknak_cb(uint8_t result, uint8_t seqno)
{
    switch (result) {
    case ACK: dbg_printf(7, "\nACK-%d\n", seqno); dab_ack_received = true; break;
    case NAK: dbg_printf(1, "\nNAK-%d\n", seqno); break;
    case ETX: dbg_printf(1, "\nETX-%d\n", seqno); break;
    default:  dbg_printf(1, "\nUNKNOWN CODE %02x\n", result); break;
    }

    return 0;
}


static int dab_input_cb(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes)
{
    if (type == dab_type_expected) {
        if (nbytes > dab_leng) nbytes = dab_leng;
        dab_leng = nbytes;
        dab_input_received = true;
        if (nbytes) memcpy(dab_data, data, nbytes);
    }
    return 0;
}


int dab_output(uint8_t ptype, uint8_t* data, uint8_t nbytes, 
               int (*acknak_cb)(uint8_t result, uint8_t seqno))
{
    if (desc->send_our_mac) {
        static uint8_t buffer[PKT_MAXLENG];
        memcpy(buffer, desc->srcmac, sizeof(desc->srcmac));
        memcpy(buffer + sizeof(desc->srcmac), data, nbytes);

        ptype |= PKT_SRCMAC;
        return spkt_qwrite(desc, ptype, buffer, nbytes + sizeof(desc->srcmac),
                        acknak_cb == NULL? dab_acknak_cb : acknak_cb);
    } else {
        return spkt_qwrite(desc, ptype, data, nbytes,
                        acknak_cb == NULL? dab_acknak_cb : acknak_cb);
    }
}


static int dab_input(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes)
{
    switch (type & MASK_PKT_TYPE) {
    case CAN_PKT_TYPE:
        if (verbosity > 7) dump_can_packet(data);
        break;
    case DEBUG_PKT_TYPE:
        if (verbosity > 7) dump_packet((char*)"DBUG", data, nbytes);
        break;
    case HEARTBEAT_PKT_TYPE:
        if (verbosity > 3) dump_heartbeat_packet(data);
        break;
    default:
        if (verbosity > 4) {
            fprintf(stdout, "dab_input seqno=%d -- %d bytes, type=0x%x\n", seqno, nbytes, type);
            dump_memory((char*)"???", 0, data, nbytes);
        }
    }

    if (input_cb != NULL) {
        input_cb(seqno, type, data, nbytes);
    }

    return 0;
}


static int download_acknak(uint8_t result, uint8_t seqno)
{
    switch (result) {
    case ACK: if (verbosity > 1) fprintf(stderr, "ACK-%d\n", seqno);     break;
    case NAK: fprintf(stderr, "\nNAK-%d\n", seqno);                      break;
    case ETX: fprintf(stderr, "\nETX-%d -- DOWNLOAD ABORTED!\n", seqno); break;
    default:  fprintf(stderr, "\nUNKNOWN CODE %02x\n", result);          break;
    }

    return 0;
}


#if 0
int dab_firmware_download(char* filename, 
                          int major_version, 
                          int minor_version, 
                          int build_number,
                          bool is64k,
                          board_type_t board_type)
{
    static uint8_t app_image[FLASH64_APP_SIZE]; // application binary image
    uint32_t app_image_base, app_image_end, app_addr;
    uint16_t crc;
    uint8_t ptype;
 
    int rc = ihex_load_image(filename,
                         app_image, sizeof(app_image),
                         is64k? FLASH64_APP_START:FLASH_APP_START, 
                         is64k? FLASH64_APP_END:FLASH_APP_END,
                         &app_image_base, &app_image_end, &app_addr,
                         board_type, major_version, minor_version, build_number, &crc);
    if (rc) {
        fprintf(stderr, "Unable to load '%s'\n", filename);
        return -1;
    }

    uint32_t length = app_image_end - app_image_base;
    dump_memory((char*)"IMAGE-", 0, app_image, 32);
    dump_memory((char*)"IMAGE-", length - 16, app_image + length - 16, 16);

    ptype = (uint8_t)board_type | (uint8_t)FIRMWARE_PKT_TYPE;

    if (major_version != 0) {
        fprintf(stderr, "Loading file '%s' v%d.%d.%d-%04x (%d bytes)...\n",
                filename, major_version, minor_version, build_number, crc, length);
    } else {
        fprintf(stderr, "Loading file '%s' (%d bytes)...\n", filename, length);
    }
    fflush(stderr);

    for (app_addr = app_image_base;;) {
        spkt_process(desc);

        rc = poll(fds, ARRAY_SIZE(fds), spkt_queued(desc)? 2 : 250);
        if (rc < 0) {
            fprintf(stderr, "poll()=%d error '%m'\n", rc);
            continue;
        }

        if (rc == 0) {
            if (!spkt_queued(desc)) {
                bool last_packet = FALSE;
                uint8_t data[sizeof(app_addr)+64];

                if (app_addr >= app_image_end) {
                    memset(data, 0, sizeof(data));
                    last_packet = TRUE;
                } else {
                    memcpy(data, &app_addr, sizeof(app_addr));
                    memcpy(data+sizeof(app_addr), app_image + (app_addr-app_image_base), 64);
                    app_addr += 64;

                    /*if (verbosity == 0)*/ {
                        uint32_t percent = ((app_addr - app_image_base) * 100) / (app_image_end - app_image_base);
                        fprintf(stdout, "\b\b\b\b    \b\b\b\b%d%%", percent); fflush(stdout);
                    }
                }

                spkt_qwrite(desc, ptype, data, sizeof(data), download_acknak);

                if (last_packet) {
                    fprintf(stdout, "\nSUCCESS '%s' LOADED!\n", filename);
                    break;  
                }
            }
        } else {
            if (fds[1].fd >= 0 && fds[1].revents) {
                spkt_process(desc);
            }
        }
    }

    return 0;
}
#endif


bool dab_queued(void)
{
    return spkt_queued(desc);
}


bool dab_txq_full()
{
    return spkt_queued(desc) >= SPKT_TXQ_MAX;
}


void dab_process(void)
{
    spkt_process(desc);
}


void dab_show_status(void)
{
    if (desc == NULL || desc->magic != SPKT_MAGIC) {
        fprintf(stderr, "desc BAD!\n");
        return;
    }

    fprintf(stderr, "qhead=%p, qtail=%p, maxqcnt=%d, queued=%d\n", 
          desc->qhead, desc->qtail, desc->maxqcnt, desc->queued);
    fprintf(stderr, "o_state=%d, o_seqno=%d, o_retry=%d, o_result=%d, o_leng=%d, c_seqno=%d\n",
          desc->o_state, desc->o_seqno, desc->o_retry, desc->o_result, desc->o_leng, desc->c_seqno);
    fprintf(stderr, "o_timestamp=%d, qhead->timestamp=%d\n",
           now() - desc->o_timestamp,
          desc->qhead? now() - desc->qhead->timestamp : 0);
    fprintf(stderr, "i_state=%d, i_index=%d, i_leng=%d\n",
          desc->i_state, desc->i_index, desc->i_leng);
}


void dab_register_input(int (*cb)(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes))
{
    input_cb = cb;
}


void dab_unregister_input(void)
{
    input_cb = NULL;
}


void dab_cb_init(uint8_t expected_input_type, uint8_t* data, uint8_t leng)
{
    dab_ack_received = false;
    dab_input_received = false;

    dab_data = data;
    dab_leng = leng;
    dab_type_expected = expected_input_type;

    dab_register_input(dab_input_cb);
}


// return true if have not received input yet and waiting for a valid input packet
// return true if not waiting for a valid input packet and have not received an ACK
static bool dab_incomplete(void)
{
    if ((dab_type_expected & MASK_PKT_TYPE) != MASK_PKT_TYPE) return !dab_input_received;
    return !dab_ack_received;
}


void dab_cb_wait(uint32_t timeout, bool* ack, bool* input)
{
    dbg_printf(2, "Waiting for DAB ACK... (timeout=%d)\n", timeout);
    uint32_t start = now();
    while (dab_incomplete() && now() - start < timeout) {
        dab_process();
    }
    dbg_printf(2, "...wait completed (timeout=%d, now()-start=%d, dab_type_expected=%x)\n", timeout, now()-start, dab_type_expected);

    *ack = dab_ack_received;
    *input = dab_input_received;
}


void dab_setmac(uint8_t mac[3])
{
   spkt_setdstmac(desc, mac);
}


void dab_init(int dab_fd, uint8_t mac[3], bool include_mac)
{
    desc = spkt_create(dab_fd, SPKT_TXQ_MAX, dab_input);
    if (desc == NULL) {
        fprintf(stderr, "%s(): out of memory, error %m\n", __FUNCTION__);
        die(__LINE__);
    }

    spkt_setsrcmac(desc, mac);
    desc->send_our_mac = include_mac;
}

#ifdef __cplusplus
}
#endif
