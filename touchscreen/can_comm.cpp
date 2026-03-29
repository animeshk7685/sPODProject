/*
* canComm.h
*
*  Created on: Oct 7, 2014
*      Author: drewrawlings
*/

#include "touchscreen.h"
#include "simple_hsm.h"
#include "pub_sub.h"


static const char* TAG = "CAN";

#define CAN_OK 0
#define CAN_NOMSG 1


struct canBuffer {
    uint8_t rx_buf[5];
};

#define CAN_TX_FIFO_NUM (16)
static struct canBuffer canTxFifoBuffer[CAN_TX_FIFO_NUM] = {0};
static uint8_t canTxFifoHead = 0, canTxFifoTail = 0;


// return true if no receive data pending on the "CAN" bus,
// which in this case is the RS485 bus parading as a CAN
int CANINT_Read()
{
    return !pkt_pending() && pcm1_known;
}


void pushCanFifo(uint8_t* buf)
{
    for (int i = 0; i < 5; i++) {
        canTxFifoBuffer[canTxFifoHead].rx_buf[i] = buf[i];
    }
    
    canTxFifoHead++;
    canTxFifoHead %= CAN_TX_FIFO_NUM;
    
    if (canTxFifoHead == canTxFifoTail) {     // to avoid overtaking
        canTxFifoTail++;
        canTxFifoTail %= CAN_TX_FIFO_NUM;
    }
}

uint8_t popCanFifo(uint8_t* buf)
{
    if (canTxFifoHead != canTxFifoTail) {
        for (int j = 0; j < 5; j++) {
            buf[j] = canTxFifoBuffer[canTxFifoTail].rx_buf[j];
        }
        
        canTxFifoTail++;
        canTxFifoTail %= CAN_TX_FIFO_NUM;
        return 1;
    }
    return 0;
}


#define MY_ID_TAG           0x80
#define DATA_BUFFER_SIZE    5

#define LVB_SW  0    // switch packet
#define LVB_ST  24   // status packet
#define LVB_SY  48   // system packet
#define LVB_DB  52   // debug packet

#define BUF_SZ1 55

#define SYSTEM_PACKET   0xA0
#define SWITCH_PACKET   0x80
#define STATUS_PACKET   0xB0
#define DEBUG_PACKET    0x90
#define PRO_PACKET_ID   0xC0


static uint8_t receive_buffer[DATA_BUFFER_SIZE] = {0,0,0,0,0};
static uint8_t send_buffer[DATA_BUFFER_SIZE] = {0,0,0,0,0};
static uint32_t tick = 0;
static uint8_t lastValBuf[BUF_SZ1 * 4] = {0};
static uint8_t addr;


static void sig_init(machineParams* mParams, machineEvent* mEvt)
{
}


static void receive_system_packet(machineParams* mParams)
{
    static uint32_t lastAddrSys = 0;

    uint8_t srcAddr = receive_buffer[4] & 0x03;
    if (srcAddr == sourceAdrMask && (millis() - lastAddrSys) > 2500) {
        lastAddrSys = millis();
        
        settings.is24v_not12 = (receive_buffer[4] & 0xFC) == 0x80 ? 1 : 0;
        voltage = (((double)receive_buffer[1] * 5 / 255) * 3) + 0.7;
        
        if (settings.is24v_not12 && (voltage * 2) < 20.0) {
            voltage *= 2;
            settings.is24v_not12 = 0;
        }
        
        temperature = receive_buffer[2];
    }

    for (int i = 0; i < 4 ; i++) {
        lastValBuf[(srcAddr * BUF_SZ1) + LVB_SY + i] = receive_buffer[1 + i];
    }
}


static void receive_status_packet(machineParams* mParams)
{
    uint8_t srcAddr = receive_buffer[0] & 0x03;
    if (srcAddr == sourceAdrMask) {
        machineEvent evt;
        evt.asPtrs[0] = (int) &receive_buffer;
        mParams->mEvt = &evt;
        publish(SWITCH_STATUS2_FROM_CAN_SIG, mParams);
    }

    addr = indexPos(receive_buffer[1]);
    for (int i = 0 ; i < 3 ; i++) {
        lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + i] = receive_buffer[2 + i];
    }
}


static void can_tick_sig(machineParams* mParams, machineEvent* mEvt)
{
    if (CANINT_Read() != 0) {
        if (pcm1_known && popCanFifo(send_buffer)) {
            if (pkt_put(pcm1_mac, CAN_PKT_TYPE, send_buffer, sizeof(send_buffer), 2)) {
                LOGC(TAG, "%d -- %s().%d: pkt_put(pcm1_mac, CAN_PKT_TYPE) FAILED!\r\n", millis(), __FUNCTION__, __LINE__);
            }
        }
        return;
    }

    uint8_t owner;
    if (!pkt_get(&owner)) return;
   
    led_toggle(GREEN_LED);

    uint8_t rtype = pkt_type(NULL);
    pkt_type_t ptype = (pkt_type_t) (rtype & MASK_PKT_TYPE);
    board_type_t btype = (board_type_t) (rtype & MASK_BOARD_TYPE);
    
    //LOGD(TAG, "%d -- %s(): type=%x, leng=%d\r\n", millis(), __FUNCTION__, rtype, pkt_leng(NULL));
    
    if (ptype == CAN_PKT_TYPE) {
        memcpy(receive_buffer, pkt_data(NULL), sizeof(receive_buffer));
        LOG_HEXDUMP("PKTRX", pkt_data(NULL), pkt_leng(NULL), ESP_LOG_DEBUG);
        if (receive_buffer[0] == SYSTEM_PACKET) {
            receive_system_packet(mParams);
        } else 
        if ((receive_buffer[0] & 0xF0) == STATUS_PACKET && indexPos(receive_buffer[1]) != 255) {
            receive_status_packet(mParams);
        }
    } else {
        pkt_process(owner);
        if (!pcm1_known) {
            board_info_t bi;
            pcm1_known = board_tad_search(PDB_BOARD_TYPE, RS485_PCM1, &bi);
            if (pcm1_known) {
                memcpy(pcm1_mac, bi.mac, sizeof(pcm1_mac));
            }
        }
    }
    pkt_release();
}


static void send_alive_can_packet(machineParams* mParams, machineEvent* mEvt)
{
    send_heartbeat_pkt(rs485_getaddr());
}


stnext canComm(int signal, void* params)
{
    if (signal != SIG_NULL) {
        machineParams *mParams = (machineParams *) params;
        machineEvent *mEvt = (machineEvent *) mParams->mEvt;
        
        switch (signal) {
        case SIG_INIT:                   sig_init(mParams, mEvt); break;
        case CAN_TICK_SIG:               can_tick_sig(mParams, mEvt); break;
        case SEND_ALIVE_CAN_PACKET:      send_alive_can_packet(mParams, mEvt); break;
        }
    }

	return (void*) currentPage;
}