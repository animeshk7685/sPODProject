#include "typedef.h"
#include "types.h"
#include "crc16.h"
#include "spkt.h"
#include "utils.h"
#include "serial.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"



#ifdef __cplusplus
extern "C"
{
#endif

extern board_info_t board_info;


#define ACK_TIMEOUT (5000*DATA_RATE_SCALE)       // milliseconds to wait for ACK/NAK before retrying
#define SLOW_ACK_TIMEOUT (1000*DATA_RATE_SCALE)  // when command is writing to EEPROM, etc
#define SPKT_STALE_DATA (ACK_TIMEOUT*5)    // pitch stale output data after this many milliseconds

//[STX][SEQ][TYPE][MAC1][MAC2][MAC3][LENGTH][~LENGTH][256 BYTE PAYLOAD][CRC1][CRC2][ETX]


uint32_t ack_timeout = ACK_TIMEOUT;
uint32_t slow_timeout = SLOW_ACK_TIMEOUT;


// return # of packets queued for transmission to DAB
int spkt_queued(SPKT_DESC* desc)
{
    return desc? desc->queued:0;
}


// free head packet in DAB TX packet queue
// return true if a packet was freed, false otherwise
static bool spkt_free_head(SPKT_DESC* desc)
{
    SPKT_DATA* done = desc->qhead;
    
    desc->o_state = IDLE_STATE;
    if (done != NULL && done->magic == SPKT_MAGIC) {
        desc->qhead = desc->qhead->next;
        if (desc->qhead == NULL)
            desc->qtail = NULL;
        done->magic = ~done->magic;
        if (desc->queued > 0)
            desc->queued -= 1;
        //dbg_printf(-1, "FREE: T/SEQ=%x/%d\n", done->data[2], done->data[1]);
        free(done);
        return true;
    }

    return false;
}


// if DAB TX packet state machine is idle, write the next packet to the DAB
// return 0 if a packet was written, -1 if not
static int spkt_out(SPKT_DESC* desc)
{
    if (desc->o_state == IDLE_STATE && desc->qhead != NULL) {
        desc->o_leng = 11 + desc->qhead->data[6];
        desc->o_retry = 0;
        desc->o_state = DOUT_STATE;
 
        memcpy(desc->o_data, desc->qhead->data, desc->o_leng);
    }
    if (desc->o_state == DOUT_STATE) {
        int rc;

        watchdog_kick();
        dbg_printf(3, "%d -- %s(nbytes=%d, type=%x): seqno=%d, retry=%d\n",
                   now(), __FUNCTION__, desc->o_leng, desc->o_data[2], desc->o_seqno, desc->o_retry);

        dbg_printf(1, "%d -- TX-T/SEQ=%x/%d (leng=%d)\n", now(), desc->o_data[2], desc->o_data[1], desc->o_leng);
        tcflush(desc->devno, TCOFLUSH); // flush pending output data
        if ((rc=write(desc->devno, desc->o_data, desc->o_leng)) != desc->o_leng) {
            fprintf(stderr, "serial output buffer full (rc=%d, o_leng=%d)!!!\n", rc, desc->o_leng);
            return -2;
        }
        tcdrain(desc->devno); // wait for pending output to be written
        desc->o_timestamp = now();
        desc->o_state = OACK_STATE;
        return 0;
    }

    return -1;
}


// retransmit a packet because of failure (either NAK or timeout)
// return 0 if packet was queued, -1 otherwise
static int spkt_retry(SPKT_DESC* desc) 
{
    dbg_printf(2, "%d -- %s(): retry=%d\n", now(), __FUNCTION__, desc->o_retry);
    if ((desc->o_retry += 1) <= 2) 
        desc->o_state = DOUT_STATE;
    else {
        dbg_printf(1, "%s(): DROP=%d\n", __FUNCTION__, desc->c_seqno);
        spkt_free_head(desc);
    } 
    spkt_out(desc);
    return 0;
}


// send an ACK or NAK to the DAB
static void acknak_out(SPKT_DESC* desc, uint8_t acknak)
{
    if (desc != NULL && desc->magic == SPKT_MAGIC) {
        u8 data[4] = {acknak, desc->i_data[1], (u8)~desc->i_data[1], desc->i_data[2]};
        int rc;
        bool broadcast = desc->i_data[3] == 0xFF && desc->i_data[4] == 0xFF && desc->i_data[5] == 0xFF;

        dbg_printf(5, "\nacknak_out(acknak=%x, seqno=%d) broadcast=%d\n", acknak, desc->i_data[1], broadcast);

        if (!broadcast) {
            if ((rc=write(desc->devno, data, sizeof(data))) != sizeof(data)) {
                fprintf(stderr, "%s(): serial output buffer full (rc=%d, leng=%ld)!!!\n", 
                        __FUNCTION__, rc, sizeof(data));
            }
            tcdrain(desc->devno); // wait for pending output to be written
        }
        desc->i_state = STX_STATE;
    }
}


// queue a packet for TX to DAB
// return 0 on success, -1 on failure
int spkt_qwrite(SPKT_DESC* desc, uint8_t type, uint8_t* data, uint16_t nbytes, 
                int (*acknak)(uint8_t result, uint8_t seqno))
{
    if (desc != NULL && desc->magic == SPKT_MAGIC && desc->queued < desc->maxqcnt) {
        uint16_t crc;
        uint8_t leng = nbytes;
        if (nbytes >= 256) {
            dbg_printf(-1, "%d -- %s %d >= 256!\n", now(), __FUNCTION__, nbytes);
            return -1;
        }
        SPKT_DATA* spkt = (SPKT_DATA*) calloc(1, sizeof(*spkt));
        if (spkt == NULL) {
            dbg_printf(-1, "%d -- %s().%d: OOPS OUT OF MEMORY!\n", now(), __FUNCTION__, __LINE__);
            return -1;
        }

        dbg_printf(3, "%d -- QTX-T/SEQ=%x/%d nbytes=%d\n", now(), type, desc->o_seqno+1, leng);

        spkt->next = NULL;
        spkt->magic = SPKT_MAGIC;
        spkt->acknak = acknak;
        spkt->timestamp = now();
//[STX][SEQ][TYPE][MAC1][MAC2][MAC3][LENGTH][~LENGTH][256 BYTE PAYLOAD][CRC1][CRC2][ETX]
        spkt->data[0] = STX;
        spkt->data[1] = ++(desc->o_seqno);
        spkt->data[2] = type;
        spkt->data[3] = desc->dstmac[0];
        spkt->data[4] = desc->dstmac[1];
        spkt->data[5] = desc->dstmac[2];
        spkt->data[6] = leng;
        spkt->data[7] = ~leng;
        if (nbytes > 0) memcpy(spkt->data+8, data, nbytes);
        crc = crc16(spkt->data, nbytes+8);
        spkt->data[8+nbytes] = crc>>8;
        spkt->data[9+nbytes] = crc;
        spkt->data[10+nbytes] = ETX;
        if (desc->qtail == NULL)
            desc->qhead = spkt;
        else
            desc->qtail->next = spkt;
        desc->qtail = spkt;
        desc->queued += 1;
        if (verbosity > 5)
            dump_memory((char*)"SPKT-OUT", 0, spkt->data, nbytes+8);
        spkt_out(desc);
        return 0;
    } else {
        if (desc == NULL) {
            dbg_printf(-1, "%d -- %s(): desc is NULL!\n", now(), __FUNCTION__);
        } else {
            if (desc->magic != SPKT_MAGIC) 
                dbg_printf(-1, "%d -- %s(): desc->magic=%x != %x\n", now(), __FUNCTION__, desc->magic, SPKT_MAGIC);
            if (desc->queued >= desc->maxqcnt)
                dbg_printf(-1, "%d -- %s(): queued=%d, maxqcnt=%d\n", now(), __FUNCTION__, desc->queued, desc->maxqcnt);
        }
    }

    return -1;
}



// called to output TX packets to DAB on idle or retry and to look for packets from DAB
// NOTE: as side effect, desc->input() routine will be called when a valid packet is
// received from the DAB.
//
// return 0 on success, -1 on error
int spkt_process(SPKT_DESC* desc)
{
    if (desc != NULL && desc->magic == SPKT_MAGIC) {
        uint8_t data;
        uint32_t timeout = ack_timeout;

        spkt_out(desc);
        watchdog_kick();
        if (desc->o_state == OACK_STATE && now() - desc->o_timestamp >= timeout) {
            dbg_printf(1, "%d -- ACK/NAK SEQ=%d timeout %d (o_state=%d), ack_timeout=%d\n",
                    now(), desc->o_seqno, now() - desc->o_timestamp, desc->o_state, timeout);
            spkt_retry(desc); 
        }
        if (desc->o_state == DOUT_STATE) {
            return 0;
        }

        // free stale data?
        if (desc->qhead != NULL && now() - desc->qhead->timestamp > SPKT_STALE_DATA) {
            dbg_printf(1, "%d -- %s(): TIMEOUT (STALE DATA) (T/SEQ=%x/%d)\n",
                       now(), __FUNCTION__, desc->qhead->data[2], desc->qhead->data[1]);
            spkt_free_head(desc);
        }

        // process input data
        while (1) {
            if (desc->i_offset >= desc->i_total) {
                desc->i_total = read(desc->devno, desc->i_buffer, sizeof(desc->i_buffer));
                desc->i_offset = 0;
                if (desc->i_total <= 0) 
                    break;
                if (verbosity > 5)
                    dump_memory((char*)"SPKT-IN", 0, desc->i_buffer, desc->i_total);
            }
            data = desc->i_buffer[desc->i_offset++];
            
            switch (desc->i_state) {
            case STX_STATE:
                switch (data) {
                case ACK:
                case NAK:
                    desc->i_state = OSEQ_STATE;
                    desc->o_result = data;
                    break;
                case STX:
                    desc->i_state = SEQ_STATE;
                    desc->i_data[0] = data;
                    break;
                default:
                    dbg_printf(1, "OOPS STX STATE data=%x\n", data);
                }
                break;

            case SEQ_STATE:
                desc->i_state = TYPE_STATE;
                desc->i_data[1] = data;
                break;

            case TYPE_STATE:
                desc->i_state = MAC1_STATE;
                desc->i_data[2] = data;
                break;

            case MAC1_STATE:
                desc->i_state = MAC2_STATE;
                desc->i_data[3] = data;
                break;

            case MAC2_STATE:
                desc->i_state = MAC3_STATE;
                desc->i_data[4] = data;
                break;

            case MAC3_STATE:
                desc->i_state = LEN_STATE;
                desc->i_data[5] = data;
                break;

            case LEN_STATE:
                desc->i_state = NOTLEN_STATE;
                desc->i_data[6] = data;
                break;

            case NOTLEN_STATE:
                if (((~data)&0xFF) != desc->i_data[6]) {
                    dbg_printf(-1, "OOPS NOTLEN STATE ~data=%x != %x (LEN)\n", ~data, desc->i_data[6]);
                    desc->i_state = STX_STATE;
                } else {
                    desc->i_data[7] = data;
                    desc->i_index = 0;
                    desc->i_leng = desc->i_data[6];
                    desc->i_state = desc->i_leng > 0? DINP_STATE : CRC1_STATE;
                }
                break;

            case DINP_STATE:
                desc->i_data[8+desc->i_index] = data;
                if ((desc->i_index += 1) == desc->i_leng)
                    desc->i_state = CRC1_STATE;
                break;

            case CRC1_STATE:
                desc->i_state = CRC2_STATE;
                desc->i_crc = data;
                break;

            case CRC2_STATE:
                desc->i_state = ETX_STATE;
                desc->i_crc = (desc->i_crc << 8) | data;
                break;

            case ETX_STATE:
                if (data != ETX) {
                    acknak_out(desc, NAK);
                } else {
                    uint16_t leng = desc->i_leng;
                    uint16_t crc;
                    int rc;

                    crc = crc16(desc->i_data, leng + 8);
                    if (crc != desc->i_crc) {
                        dbg_printf(-1, "\n OOPS T/SEQ=%x/%d leng=%d CCRC=%x != %x=RCRC\n",
                                   desc->i_data[2], desc->i_data[1], leng+7, crc, desc->i_crc);
                        acknak_out(desc, NAK);
                    } else {
                        watchdog_kick();
                        dbg_printf(7, "%d -- RX-T/SEQ=%x/%d (leng=%d)\n",
                                   now(), desc->i_data[2], desc->i_data[1], leng);
                        rc = desc->input(desc->i_data[1], desc->i_data[2], desc->i_data+8, leng);
                        acknak_out(desc, rc == 0? ACK : NAK);
                    }
                }
                break;

            case OSEQ_STATE:
                desc->i_state = NSEQ_STATE;
                desc->c_seqno = data;
                break;

            case NSEQ_STATE:
                desc->i_state = TYPE2_STATE;
                desc->c_seqno2 = data;
                break;

            case TYPE2_STATE:
                desc->i_state = STX_STATE;
                if (desc->o_state != OACK_STATE) break;

                if (((~desc->c_seqno2)&0xFF) != desc->c_seqno || desc->qhead == NULL ||
                    desc->c_seqno != desc->qhead->data[1]) {
                    tcflush(desc->devno, TCIFLUSH);
                    dbg_printf(-1, "%d -- %s(): ACK MISMATCH EXPECT SEQ=%d, GOT=%d (qhead=%p)\n",
                               now(), __FUNCTION__, desc->qhead? desc->qhead->data[1]:-1, desc->c_seqno, desc->qhead);
                    //desc->o_result = NAK; // TODO: testing the 'break' after this -- eat old ACK!
                    break;
                }
                if (desc->qhead != NULL && desc->qhead->magic == SPKT_MAGIC &&
                    (desc->qhead->acknak) && desc->qhead->acknak(desc->o_result, desc->o_data[1])) {
dbg_printf(-1, "THIS CASE -- retransmit?\n");
                    spkt_out(desc);
                    return -1;
                }
                if (desc->o_result != ACK || (desc->qhead && data != desc->qhead->data[2])) {
                    if (desc->o_result != ACK) {
                        dbg_printf(-1, "%d --  %s(%x)-T/SEQ=%x/%d\n", now(),
                                   desc->o_result == ACK? "ACK":"NAK", desc->o_result, data, desc->c_seqno);
                    } else {
                        dbg_printf(-1, "%d -- ACK-T/SEQ=%x/%d [EXPECT TYPE=%x]\n", now(),
                               data, desc->c_seqno, desc->qhead? desc->qhead->data[2]:-1);
                    }
                    spkt_retry(desc);
                } else {
                    dbg_printf(1, "%d -- AK-T/SEQ=%x/%d\n", now(),
                               data, desc->c_seqno);
                    spkt_free_head(desc);
                }
                break;

            default:
                desc->i_state = STX_STATE;
                break;
            }
        }
        return 0;
    }

    return -1;
}


void spkt_setsrcmac(SPKT_DESC* desc, uint8_t mac[3])
{
    memcpy(desc->srcmac, mac, sizeof(desc->srcmac));
}


void spkt_setdstmac(SPKT_DESC* desc, uint8_t mac[3])
{
    memcpy(desc->dstmac, mac, sizeof(desc->dstmac));
}


// create an SPKT descriptor
// return a pointer to the new desriptor
SPKT_DESC* spkt_create(uint8_t devno, int maxqcnt, 
                       int (*input)(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes))
{
    SPKT_DESC* desc = (SPKT_DESC*)calloc(1, sizeof(*desc));
    if (desc != NULL) {
        desc->magic   = SPKT_MAGIC;
        desc->devno   = devno;
        desc->input   = input;
        desc->maxqcnt = maxqcnt;
        desc->o_seqno = random();
        dbg_printf(-1, "%s(): o_seqno=%d\n", __FUNCTION__, desc->o_seqno);
    }
    return desc;
}


// destroy an SPKT descriptor
// return 0 on success, -1 on failure
int spkt_destroy(SPKT_DESC* desc)
{
    if (desc != NULL && desc->magic == SPKT_MAGIC) {
        desc->magic = ~desc->magic;
        desc->input(0, 0xFF, NULL, 0);

        while (spkt_free_head(desc))
            continue;
        free(desc);
        return 0;
    }

    return -1;
}

#ifdef __cplusplus
}
#endif
