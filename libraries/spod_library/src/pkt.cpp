//-----------------------------------------------------------------------------
//  pkt.c
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

#include "spod_library.h"
#include "crc16.h"
#include "pkt.h"
#include "ota.h"
#include "rs485.h"

#define RS485_ENABLED 1
#define PKT_PRIORITY 2
#define TASK_STACK_SIZE 8192

#if defined(__cplusplus)
extern "C" {
#endif

static const char* TAG = "PKT";

//#define STX 0x02
///#define ETX 0x03
//#define ACK 0x06
//#define NAK 0x15

//#define PKT_MAXLENG (256+11)

#define ACK_TIMEOUT_US (2500*DATA_RATE_SCALE)
#define BACKOFF_MAX    0xFFF // *MUST* be a power of 2
#define QMAX 8

/*
[STX][SEQ][TYPE][MAC1][MAC2][MAC3][LENGTH][~LENGTH][256 BYTE PAYLOAD][2 BYTE CRC][ETX]

STX     -- 0x02
SEQ     -- 8-bit serial number (incremented for every packet sent)
TYPE    -- top 4 bits are board_type_t bottom 4 are pkt_type_t
MACx    -- 3-byte destination MAC address (bottom 24-bits of BLE MAC), all 1s for broadcast
LENGTH  -- number of bytes of data, does not include STX, SEQ, TYPE, CRC, or ETX
~LENGTH -- used to verify LENGTH byte, if not correct [STX] through [~LENGTH] bytes are 
           dropped and scan for STX resumes
CRC16   -- 16 bit CRC with polynomial x^16+x^12+x^5+1 (crc of STX .. through data bytes only)
ETX     -- 0x03

The receiver of the packet responds with either[ACK][SEQ][~SEQ][TYPE] or [NAK][SEQ][~SEQ][TYPE]

ACK     -- 0x06 (data has been received okay)
NAK     -- 0x15 (data is invalid, CRC does not match, or packet format corrupt)
SEQ     -- this matches the SEQ provided in the transmitter's packet
~SEQ    -- inverted SEQ to verify ACK/NAK packet correctness
*/


static uint8_t qhead;  // &pktq[qhead] is the next packet to process
static uint8_t qtail;  // &pktq[qtail] is the next packet under construction (qtail != qhead unless pktq is empty)
static pkt_t pktq[QMAX];
static pkt_t pktrx;
static TaskHandle_t task_handle;


// currently we don't expect an ACK/NAK and don't retransmit
static uint8_t   oseqno;
static uint8_t   odata[PKT_MAXLENG];

static board_type_t boardtype = UNSPECIFIED_BOARD_TYPE;
static uint8_t   our_mac[3] = {0};
uint8_t pkt_broadcast[3] = {0xFF, 0xFF, 0xFF};

static uint64_t saw_stx;
static uint32_t last_ota_pkt_time;

static bool promiscuous;


const char* pkt_type_string(pkt_type_t pkt_type)
{
    switch (pkt_type) {
    case HEARTBEAT_PKT_TYPE:      return "HEARTBEAT_PKT_TYPE";
    case SYS_PKT_TYPE:            return "SYS_PKT_TYPE";
    case CONFIG_PKT_TYPE:         return "CONFIG_PKT_TYPE";
    case PCM_CONFIG_PKT_TYPE:     return "PCM_CONFIG_PKT_TYPE";
    case PCM_STATUS_PKT_TYPE:     return "PCM_STATUS_PKT_TYPE";
    case SWITCH_CONFIG_PKT_TYPE:  return "SWITCH_CONFIG_PKT_TYPE";
    case SWITCH_STATUS_PKT_TYPE:  return "SWITC_STATUS_PKT_TYPE";
    case AVAIL07_PKT_TYPE:        return "AVAIL07_PKT_TYPE";       
    case OTA_CTRL_PKT_TYPE:       return "OTA_CtRL_PKT_TYPE";
    case OTA_CTRL_PKT_ACK_TYPE:   return "OTA_CTRL_PKT_ACK_TYPE";  
    case OTA_DATA_PKT_TYPE:       return "OTA_DATA_PKT_TYPE";   
    case RS485_PKT_TYPE:          return "RS485_PKT_TYPE";
    case CAN_PKT_TYPE:            return "CAN_PKT_TYPE";
    case DEBUG_PKT_TYPE:          return "DEBUG_PKT_TYPE";         
    case CIRCUIT_STATUS_PKT_TYPE: return "CIRCUIT_STATUS_PKT_TYPE";
    case TRIGGER_PKT_TYPE:        return "TRIGGER_PKT_TYPE";      
    }
    return "UNKNOWN_PKT_TYPE";
}


const char* pkt_board_type_string(board_type_t board_type)
{
    switch (board_type) {
    case UNSPECIFIED_BOARD_TYPE:  return "UNSPECIFIED";
    case PDB_BOARD_TYPE:          return "PDB";
    case SDB_BOARD_TYPE:          return "SDB";
    case TSB_BOARD_TYPE:          return "TSB";
    }
    return "UNKNOWN BOARD TYPE";
}


void pkt_set_promiscuous(bool onoff)
{
    promiscuous = onoff;
}


board_type_t pkt_get_boardtype(void)
{
    return boardtype;
}


void pkt_set_boardtype(board_type_t btype)
{
    boardtype = (board_type_t) (((uint8_t)btype) & MASK_BOARD_TYPE);
}


uint32_t pkt_mac_integer(uint8_t mac[3])
{
    uint32_t accum = 0;
    for (int i = 0; i < PKT_MACLENG; ++i) {
        accum = (accum<<8) | mac[i];
    }
    return accum;
}


bool pkt_mac_match(uint8_t a[3], uint8_t b[3])
{
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2];
}


bool pkt_mac_ours(uint8_t* mac)
{
    return pkt_mac_match(mac, our_mac);
}


bool pkt_mac_broadcast(uint8_t* mac)
{
    return pkt_mac_match(mac, pkt_broadcast);
}


bool pkt_mac_zero(uint8_t* mac)
{
    return mac[0] == 0 && mac[1] == 0 && mac[2] == 0;   
}


void pkt_set_ourmac(uint8_t* mac)
{
    memcpy(our_mac, mac, PKT_MACLENG);
}


void pkt_get_ourmac(uint8_t* mac)
{
    memcpy(mac, our_mac, PKT_MACLENG);
}


uint8_t* pkt_ourmac()
{
    return our_mac;
}


uint8_t* pkt_dstmac(pkt_t* p)
{
    if (p == NULL) p = &pktq[qhead];
    return p->idata+3;
}


uint8_t* pkt_srcmac(pkt_t* p)
{
    if (p == NULL) p = &pktq[qhead];
    return (p->idata[2] & PKT_SRCMAC)? p->ipayload : pkt_broadcast;
}


static bool our_packet(void)
{
    return pkt_mac_broadcast(pktrx.idata+3) || pkt_mac_ours(pktrx.idata+3);
}


static bool filtered_packet(void)
{
    return !our_packet();
}


static void acknak_put(uint8_t acknak)
{
    if (pkt_mac_ours(pktrx.idata+3) && (pktrx.idata[2] & PKT_NOACK)==0) { // only ACK/NAK our data (shared bus)
        uint8_t not_seq = pktrx.idata[1];
        not_seq = ~not_seq;
        uint8_t data[4] = {acknak, pktrx.idata[1], not_seq, pktrx.idata[2]};
        bool okay;
        int retry = 0;
        //delayMicroseconds(16000);
        do {
            //LOGD(TAG, "%d -- %02x-%d (%d)\r\n", millis(), acknak, pktrx.idata[1], retry++); Serial.flush();
            okay = serial_write(data, sizeof(data));
        } while (!okay);
    } else {
        //LOGD(TAG, "%02x-%d (%02x.%02x.%02x -- NOT OURS)\r\n", acknak, pktrx.idata[1], pktrx.idata[3], pktrx.idata[4], pktrx.idata[5]); Serial.flush();
    }

    pktrx.istate = STX_STATE;
}


uint8_t* pkt_data(pkt_t* p)
{
    if (p == NULL) p = &pktq[qhead];
    return (p->idata[2] & PKT_SRCMAC)? p->ipayload + PKT_MACLENG : p->ipayload;
}


uint8_t pkt_leng(pkt_t* p)
{
    if (p == NULL) p = &pktq[qhead];
    return (p->idata[2] & PKT_SRCMAC)? p->ileng - PKT_MACLENG : p->ileng;
}


uint8_t pkt_type(pkt_t* p)
{
    if (p == NULL) p = &pktq[qhead];
    return p->idata[2];
}


bool pkt_ota_in_progress(void)
{
    return last_ota_pkt_time != 0 && millis() - last_ota_pkt_time < 15000;
}


#if RS485_ENABLED
static int wait_for_acknak(uint8_t type, uint8_t seqno)
{
    uint64_t start = get_microseconds();
    uint8_t seq;
    int state = 0;

    do {
        uint8_t data;
        if (!serial_get_singleton(&data)) continue;

        //LOGD(TAG, "ACK-%d, data=%02x\r\n", state, data);
        switch (state) {
        case 0:
            if (data == ACK) state = 1; else
            if (data == NAK) state = 4; else {
                LOGD(TAG, "%d -- ACK-%d, data=%02x\r\n", millis(), state, data);         
            }
            break;
        case 1:
        case 4:
            ++state; 
            seq = data;
            break;
        case 2:
        case 5:
            ++state;
            if (seq != ((~data)&0xFF)) {
                LOGD(TAG, "%d -- ACK-%d, data=%02x\r\n", millis(), state, data);
                state = 0;
            }
            break;
        case 3:
            if (data == type && seq == seqno) {
                LOGD(TAG, "%d -- %s(type=%x, seqno=%d): ACK!\r\n", millis(), __FUNCTION__, type, seq);
                return 0;
            }
            LOGD(TAG, "%d -- ACK-%d, data=%02x (seq=%d, seqno=%d)\r\n", millis(), state, data, seq, seqno);  
            state = 0;
            break;
        case 6:
        default:
            LOGD(TAG, "%d -- ACK-%d, data=%02x\r\n", millis(), state, data);  
            state = 0;
        }
    } while (get_microseconds() - start < ACK_TIMEOUT_US);

    LOGE(TAG, "%d -- %s(type=%x): TIMEOUT!\r\n", millis(), __FUNCTION__, type);
    return -1;
}
#endif

void pkt_random_backoff()
{
    delayMicroseconds(rand()&BACKOFF_MAX);
}


// Transmit a packet over RS485. Note, anything transmitted over RS485 is
// also received. This "feature" is used to verify the packet was sent
// without collision. If the same data that was sent is received it is
// assumed there was no collision. Packets that are sent are either ACKed
// or NAKed. (If board type is UNSPECIFIED_BOARD_TYPE, packet is NOT ACK/NAKed)
//
// Arguments:
//   dstmac  -- destination MAC address (can be pkt_broadcast)
//   type    -- packet type (upper 4-bits is board type, lower 4-bits is packet type)
//   data    -- data to transmit over RS485
//   leng    -- # of bytes of data to transmit
//   retries -- # of retries:
//                -1 -- no retries and don't wait for ACK/NAK
//                 0 -- no retries, wait for ACK/NAK
//                >0 -- number of retries, wait for ACK/NAK
//
// Returns: 
//   0       -- success
//  -1       -- error 
// 
int pkt_put(uint8_t* dstmac, uint8_t type, const uint8_t* data, size_t leng, int retries)
{
    #if RS485_ENABLED
    if (rs485_lock()) {
        int rc, retry = 0;
        size_t  dleng = leng;
        uint8_t doff = 8;
        
        odata[0] = STX;
        odata[1] = oseqno++;
        odata[2] = type | boardtype;
        memcpy(odata+3, dstmac, PKT_MACLENG);
        if (type & PKT_SRCMAC) {
            memcpy(odata+8, our_mac, PKT_MACLENG);
            doff += PKT_MACLENG;
            leng += PKT_MACLENG;
        }
        memcpy(odata+doff, data, dleng);
        odata[6] = leng;
        odata[7] = ~leng;
        
        uint16_t crc = crc16(odata, leng+8);
        leng += 8;
        odata[leng++] = crc>>8;
        odata[leng++] = crc;
        odata[leng++] = ETX;
        //LOG_HEXDUMP("PKTTX", odata, leng, ESP_LOG_DEBUG);
        
        if (pkt_mac_broadcast(dstmac)) retries = 0;
        bool expect_ack = (retries >= 0) && (!pkt_mac_broadcast(dstmac));
        do {
            if (retry > 0) {
                pkt_random_backoff(); // random backoff for retry
            }
            rc = serial_write(odata, leng)? 0 : -1;
            if (rc == 0 && expect_ack) rc = wait_for_acknak(odata[2], odata[1]);
        } while (rc != 0 && ++retry <= retries);
        
        if (expect_ack) serial_acked();
        //LOGD(TAG, "%s(type=%x, leng=%d, retries=%d(%d)): rc=%d\r\n", __FUNCTION__, type, leng, retries, retry, rc);
        rs485_unlock();
        return rc;
    }
    #endif
    return -1;
}


bool pkt_get(uint8_t* arbaddr)
{
    if (qhead != qtail) {
        if (arbaddr != NULL) *arbaddr = pktq[qhead].sender;
        return true;
    }

    return false;
}


bool pkt_pending()
{
    return qhead != qtail;
}


void pkt_release()
{
    if (qhead != qtail) {
        uint8_t next = (qhead + 1) & (QMAX-1);
        qhead = next;
    }
}


void pkt_notify(void)
{
    BaseType_t yield = pdFALSE;
    uint32_t status = serial_available();
    xTaskNotifyFromISR(task_handle, status, eSetBits, &yield);
    if (yield) portYIELD_FROM_ISR();
}


static bool pkt_receive()
{
    #if RS485_ENABLED
    uint8_t data;

    while (serial_get(&data)) {
//if (pktrx.istate > TYPE_STATE) LOGD(TAG, "%s(): istate=%d, data=0x%02x\n", __FUNCTION__, pktrx.istate, data); Serial.flush();
        switch (pktrx.istate) {
        case STX_STATE:
            if (data == STX) {
                pktrx.idata[0] = STX;
                pktrx.istate = SEQ_STATE;
            }
            break;

        case SEQ_STATE:
            pktrx.idata[1] = data;
            pktrx.istate = TYPE_STATE;
            break;

        case TYPE_STATE:
            pktrx.idata[2] = data;
            pktrx.istate = MAC1_STATE;
            break;

        case MAC1_STATE:
            pktrx.idata[3] = data;
            pktrx.istate = MAC2_STATE;
            break;

        case MAC2_STATE:
            pktrx.idata[4] = data;
            pktrx.istate = MAC3_STATE;
            break;

        case MAC3_STATE:
            pktrx.idata[5] = data;
            pktrx.istate = LEN_STATE;
            break;

        case LEN_STATE:
            pktrx.idata[6] = data;
            pktrx.istate = NOTLEN_STATE;
            break;

        case NOTLEN_STATE:
            if (((~data)&0xFF) != pktrx.idata[6]) {
                pktrx.istate = STX_STATE;
            } else {
                pktrx.idata[7] = data;
                pktrx.iindex = 0;
                pktrx.ileng  = pktrx.idata[6];
                pktrx.istate = pktrx.ileng > 0? DINP_STATE : CRC1_STATE;
            }
            break;

        case DINP_STATE:
            pktrx.idata[8+pktrx.iindex] = data;
            pktrx.iindex += 1;
            if (pktrx.iindex == pktrx.ileng) {
                pktrx.istate = CRC1_STATE;
            }
            break;

        case CRC1_STATE:
            pktrx.icrc = data;
            pktrx.istate = CRC2_STATE;
            break;

        case CRC2_STATE:
            pktrx.icrc = (pktrx.icrc << 8) | data;
            pktrx.istate = ETX_STATE;
            break;

        case ETX_STATE:
            pktrx.idata[8+pktrx.iindex] = 0;
            if (data != ETX) {
                LOGD(TAG, "%d -- NAK -- NOT ETX (%02x)\r\n", millis(), data);
                acknak_put(NAK);
            } else {
                uint16_t crc = crc16(pktrx.idata, pktrx.ileng+8);
                if (crc != pktrx.icrc) {
                    LOGD(TAG, "%d -- NAK -- BAD CRC\r\n", millis());
                    acknak_put(NAK);
                } else {
                    bool duplicate = (pktrx.iseqno == pktrx.idata[1]) && pkt_mac_ours(pkt_dstmac(&pktrx)); // don't process packet if duplicate
                    bool process;

                    acknak_put(ACK);
    //LOGD(TAG, "IDATA: %02x %02x %02x %02x %02x %02x %02x %02x\r\n", 
    //    pktrx.idata[0], pktrx.idata[1], pktrx.idata[2], pktrx.idata[3], pktrx.idata[4], pktrx.idata[5], pktrx.idata[6], pktrx.idata[7]);
    //LOGD(TAG, "PKT %d OK D/F=%d/%d\r\n", pktrx.idata[1], duplicate, filtered_packet()); //Serial.flush();
                    pktrx.iseqno = pktrx.idata[1];
                    pktrx.ipayload = pktrx.idata+8;
                    if (ota_packet(pkt_type(&pktrx))) last_ota_pkt_time = millis();
                    process = promiscuous || (!duplicate && !filtered_packet());
                    if (process) pktrx.sender = rs485_arbaddr();
                    return process;
                }
            }
            break;

        default:
            pktrx.istate = STX_STATE;
            break;
        }
    }
    #endif

    return FALSE;
}


static void pkt_receive_task(void* unused)
{
   while (true) {
        uint32_t status;
        if (xTaskNotifyWait(0x00, 0x00, &status, portMAX_DELAY)) {
            if (pkt_receive()) {
                uint8_t next = (qtail + 1) & (QMAX-1);
                if (qhead == next) {
                    LOGE(TAG, "%d -- %s(): PACKET Q OVERFLOW! head=%d, tail=%d\r\n", millis(), __FUNCTION__, qhead, qtail);
                } else {
                    pktq[qtail] = pktrx;
                    qtail = next;
                }
            }
        }
    }
}


void pkt_init(uint8_t* mac, board_type_t board_type)
{
    LOGD(TAG, "%d -- %s(mac=%02x.%02x.%02x, board_type=0x%x)\r\n", millis(), __FUNCTION__, mac[3], mac[4], mac[5], board_type);
    pkt_set_boardtype(board_type);
    pkt_set_ourmac(mac+3);
    memset(pktq, 0, sizeof(pktq));
    qhead = qtail = 0;
    xTaskCreate(pkt_receive_task, "pkt_receive_task", TASK_STACK_SIZE, NULL, PKT_PRIORITY, &task_handle);
}


#if defined(__cplusplus)
}
#endif

