#ifndef _SPKT_H_
#define _SPKT_H_

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*
[STX][SEQ][TYPE][MAC1][MAC2][MAC3][LENGTH][~LENGTH][256 BYTE PAYLOAD][CRC1][CRC2][ETX]

STX -- 0x02
SEQ -- 8-bit serial number (incremented for every packet)
TYPE -- packet type, TBD
LENGTH -- number of bytes of data, does not include STX, SEQ, TYPE, CRC, or ETX
~LENGTH -- used to verify LENGTH byte, if not correct [STX][SEQ][TYPE][LENGTH][~LENGTH] bytes are dropped and scan for STX resumes
CRC16 -- 16 bit CRC with polynomial x^16+x^12+x^5+1 (CRC of the STX .. data bytes)
ETX -- 0x03
The receiver of the packet responds with either[ACK][SEQ][~SEQ][TYPE] or [NAK][SEQ][~SEQ][TYPE]

ACK -- 0x06 (data has been received okay)
NAK -- 0x15 (data is invalid, CRC does not match, or packet format corrupt)
SEQ -- this matches the SEQ provided in the transmitter's packet
~SEQ -- inverted SEQ to verify ACK/NAK packet correctness
*/

#define STX 0x02
#define ETX 0x03
#define ACK 0x06
#define NAK 0x15

#define SPKT_MAGIC 0xcafefade
#define SPKT_LENG  (256+11)

#define SPKT_TXQ_MAX 8

typedef enum {
/*0*/    STX_STATE,      // basically idle waiting for an input packet (starts with STX)
/*1*/    SEQ_STATE,      // receiving SEQ
/*2*/    TYPE_STATE,     // receiving data type
/*3*/    MAC1_STATE,
/*4*/    MAC2_STATE,
/*5*/    MAC3_STATE,
/*6*/    LEN_STATE,      // receiving length
/*7*/    NOTLEN_STATE,   // receiving ~length
/*8*/    DINP_STATE,     // data input state
/*9*/    CRC1_STATE,     // MSB of 16-bit CRC
/*10*/   CRC2_STATE,     // LSB of 16-bit CRC
/*11*/   ETX_STATE,      // finished receiving input packet, verify crc, all input() callback, etc
/*12*/   OSEQ_STATE,     // waiting for ACK/NAK SEQNO of an input packet
/*13*/   NSEQ_STATE,     // waiting for ACK/NAK ~SEQNO of an input packet
/*14*/   TYPE2_STATE     // waiting for ACK/NAK TYPE of an input packet
} SPKT_ISTATE;


typedef enum {
/*0*/    IDLE_STATE,     // idle
/*1*/    DOUT_STATE,     // outputing a packet
/*2*/    OACK_STATE,     // waiting for ACK/NAK of an output packet
} SPKT_OSTATE;


typedef struct spkt_data_t {
    struct spkt_data_t* next;
    uint32_t            magic;
    uint32_t            timestamp;
    int                 (*acknak)(uint8_t result, uint8_t seqno);
    uint8_t             data[SPKT_LENG];
} SPKT_DATA;


typedef struct {
    uint32_t    magic;
    uint8_t     devno;
    uint8_t     dstmac[3];
    uint8_t     srcmac[3];
    bool        send_our_mac;

    SPKT_DATA*  qhead;
    SPKT_DATA*  qtail;

    int         maxqcnt;
    int         queued;

    SPKT_OSTATE o_state;
    uint8_t     o_seqno;
    uint8_t     c_seqno;
    uint8_t     c_seqno2;
    int         o_retry;
    uint8_t     o_result;
    uint16_t    o_leng;
    uint8_t     o_data[SPKT_LENG];
    uint32_t    o_timestamp;

    SPKT_ISTATE i_state;
    uint16_t    i_index;
    uint16_t    i_leng;
    uint8_t     i_data[SPKT_LENG];
    uint16_t    i_crc;

    uint8_t     i_buffer[2048];
    int         i_total;
    int         i_offset;

    int         (*input)(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes);
} SPKT_DESC;


extern uint32_t ack_timeout;
extern uint32_t slow_timeout;


extern int        spkt_queued(SPKT_DESC* desc);
extern int        spkt_qwrite(SPKT_DESC* desc, uint8_t type, uint8_t* data, uint16_t nbytes, int (*acknak)(uint8_t result, uint8_t seqno));
extern int        spkt_process(SPKT_DESC* desc);
extern SPKT_DESC* spkt_create(uint8_t devno, int maxqcnt, int (*input)(uint8_t seqno, uint8_t type, uint8_t* data, uint16_t nbytes));
extern int        spkt_destroy(SPKT_DESC* desc);

extern void spkt_setsrcmac(SPKT_DESC* desc, uint8_t mac[3]);
extern void spkt_setdstmac(SPKT_DESC* desc, uint8_t mac[3]);

#ifdef __cplusplus
}
#endif

#endif
