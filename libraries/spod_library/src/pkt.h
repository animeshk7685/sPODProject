//-----------------------------------------------------------------------------
//  pkt.h
//
//  Packet protocol, send packets reliably over serial.
//
//  Currently don't implement "reliable" feature, just ACK or NAK packets and
//  throws away duplicates (i.e., if it receives a packet of expected-seqno-minus-1 
//  it throws it away  because this is assumed to be a duplicate packet that has already
//  been processed). The sender retransmits packets if it receives a NAK or times out
//  waiting for an ACK.  The sender will retry 3 times and then give up.
//
//-----------------------------------------------------------------------------

#ifndef __PKT_H__
#define __PKT_H__

#include "pkt_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define STX 0x02
#define ETX 0x03
#define ACK 0x06
#define NAK 0x15

#define PKT_MACLENG 3
#define PKT_OVERHEAD 11
#define PKT_MAXLENG (256+PKT_OVERHEAD)

/*
[STX][SEQ][TYPE][MAC1][MAC2][MAC3][LENGTH][~LENGTH][256 BYTE PAYLOAD][2 BYTE CRC][ETX]

STX     -- 0x02
SEQ     -- 8-bit serial number (incremented for every packet sent)
TYPE    -- top 4 bits are board_type_t bottom 4 are pkt_type_t
MAC     -- 3-byte destination MAC address (bottom 24-bits of BLE MAC), all 1s for broadcast
LENGTH  -- number of bytes of data, does not include STX, SEQ, TYPE, CRC, or ETX
~LENGTH -- used to verify LENGTH byte, if not correct [STX] through [~LENGTH] bytes are 
           dropped and scan for STX resumes
CRC16   -- 16 bit CRC with polynomial x^16+x^12+x^5+1 (crc of STX .. through data bytes only)
ETX     -- 0x03

The receiver of the packet responds with either [ACK][SEQ][~SEQ][TYPE] or [NAK][SEQ][~SEQ][TYPE]

ACK     -- 0x06 (data has been received okay)
NAK     -- 0x15 (data is invalid, CRC does not match, or packet format corrupt)
SEQ     -- this matches the SEQ provided in the transmitter's packet
~SEQ    -- inverted SEQ to verify ACK/NAK packet correctness
*/

// following two bits of type field
#define PKT_SRCMAC 0x80 // this bit means the source MAC address is prepended to the packet payload
#define PKT_NOACK  0x40 // this bit means targetted recipient should not respond with ACK/NAK


enum state_enum
{
    STX_STATE    = 0,   // idle waiting for packet (starts with STX)
    SEQ_STATE    = 1,   // receiving SEQ
    TYPE_STATE   = 2,   // receiving data type
    MAC1_STATE   = 3,   // bottom 24-bits of destination MAC address (all 1s for broadcast)
    MAC2_STATE   = 4,
    MAC3_STATE   = 5,
    LEN_STATE    = 6,   // receiving length
    NOTLEN_STATE = 7,  // receiving ~length
    DINP_STATE   = 8,  // data input state
    CRC1_STATE   = 9,  // MSB of 16-bit CRC
    CRC2_STATE   = 10,  // LSB of 16-bit CRC
    ETX_STATE    = 11,   // finished receiving input packet
};
typedef enum state_enum state_t;


typedef struct {
    uint8_t   sender; // RS485 address of device that sent the packet
    state_t   istate;
    uint8_t   iseqno; // seqno of last packet processed
    uint16_t  iindex;
    uint16_t  ileng;
    uint16_t  icrc;
    uint8_t*  ipayload;
    uint8_t   idata[PKT_MAXLENG];   // for building input packet
} pkt_t;


// note: the board type and source MAC address are sent as part of the packet meta (control) data
// (PKT_SRMAC has to be specified and the board type is the top 4 bits of the TYPE byte)
typedef struct {
    uint8_t  addr;      // rs485 addr used for arbitration (PCM1=0, PCM2=1, PCM3=2, PCM4=3)
    uint8_t  major;     // major version
    uint8_t  minor;     // minor version
    uint8_t  revision;  // version revision
    uint32_t beat;      // number of beats since boot
} pkt_heartbeat_t;


typedef struct {
    uint8_t pcm;
    uint8_t available;
    uint16_t trigger_mask;
} trigger_status_t;


extern uint8_t pkt_broadcast[3];


extern uint32_t     pkt_mac_integer(uint8_t mac[3]);
extern bool         pkt_mac_match(uint8_t* a, uint8_t* b);
extern bool         pkt_mac_ours(uint8_t* mac);
extern bool         pkt_mac_broadcast(uint8_t* mac);
extern bool         pkt_mac_zero(uint8_t* mac);
extern void         pkt_set_promiscuous(bool onoff);
extern void         pkt_set_ourmac(uint8_t* mac);
extern void         pkt_get_ourmac(uint8_t* mac);
extern uint8_t*     pkt_ourmac();
extern uint8_t*     pkt_dstmac(pkt_t* p);
extern uint8_t*     pkt_srcmac(pkt_t* p);

extern const char*  pkt_type_string(pkt_type_t pkt_type);
extern const char*  pkt_board_type_string(board_type_t board_type);
extern void         pkt_init(uint8_t* mac, board_type_t board_type);
extern uint8_t*     pkt_data(pkt_t* p);
extern uint8_t      pkt_type(pkt_t* p);
extern uint8_t      pkt_leng(pkt_t* p);
extern board_type_t pkt_get_boardtype(void);
extern void         pkt_set_boardtype(board_type_t btype);
extern bool         pkt_ota_in_progress(void);
extern bool         pkt_get(uint8_t* arbaddr);
extern int          pkt_put(uint8_t* dstmac, uint8_t type, const uint8_t* data, size_t leng, int retries);
extern void         pkt_notify(void);
extern bool         pkt_pending();
extern void         pkt_release();
extern void         pkt_random_backoff();

#if defined(__cplusplus)
}
#endif

#endif
