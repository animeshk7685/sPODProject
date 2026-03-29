/*
 * canComm.h
 *
 *  Created on: Oct 7, 2014
 *      Author: drewrawlings
 */


#include "simpleHsm.h"
#include "pubSub.h"
#include "common.h"
#include "iprintf.h"

struct canBuffer {
    uint8_t rx_buf[5];
};

#define CAN_TX_FIFO_NUM (16)
static struct canBuffer canTxFifoBuffer[CAN_TX_FIFO_NUM] = {0};
static uint8_t canTxFifoHead = 0, canTxFifoTail = 0;

void pushCanFifo(uint8_t* buf)
{
    for(int i = 0; i < 5; i++)
    {
        canTxFifoBuffer[canTxFifoHead].rx_buf[i] = buf[i];
    }
    
    canTxFifoHead++;
    canTxFifoHead %= CAN_TX_FIFO_NUM;
    
    if(canTxFifoHead == canTxFifoTail)      // to avoid overtaking
    {
        canTxFifoTail++;
        canTxFifoTail %= CAN_TX_FIFO_NUM;
    }
}

uint8_t popCanFifo(uint8_t* buf)
{
    if(canTxFifoHead != canTxFifoTail)
    {
        for(int j = 0; j < 5; j++)
        {
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


#define LVB_SW  0
#define LVB_ST  24
#define LVB_SY  48
#define LVB_DB  52

#define BUF_SZ1 55

#define SYSTEM_PACKET   0xA0
#define SWITCH_PACKET   0x80
#define STATUS_PACKET   0xB0
#define DEBUG_PACKET    0x90
#define PRO_PACKET_ID       0xC0



stnext canComm(int signal, void* params)
{
	//return (void*) currentPage;

//	static uint8_t i = 0;
    static bool canInit = false;
    static uint32_t startConnectTime;

//    static bool canChanInit[32] = {0};
    
	static uint8_t receive_buffer[DATA_BUFFER_SIZE] = {0,0,0,0,0};
	//static st_cmd_t receive_msg;

	static uint8_t send_buffer[DATA_BUFFER_SIZE] = {0,0,0,0,0};
	//static st_cmd_t send_message;

	machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;

	static uint8_t tick = 0;

    
    static mcpCan can;
    static uint8 bytesRead = 0;

    static uint8_t lastValBuf[BUF_SZ1 * 4] = {0};
    static bool sendBlePacket = false;
    static uint8_t addr;
    static uint16_t lastCur = 0;
    static uint16_t nextCur = 0;
    static int diff = 0;
    static uint8_t srcAddr = 0;
    static uint8_t sysCount = 0;
    
    static uint32_t lastAddrSys = 0;
    
    static uint32_t lastCanCheck = 0;
    
	switch(signal)
	{
	case SIG_INIT:
	{

		if(!canInit){

        /*
            
		CAN.set_baudrate(250);
		CAN.init(0);

		receive_msg.pt_data = (unsigned char*)&receive_buffer[0];
		receive_msg.status = 0;


		for(i=0; i<DATA_BUFFER_SIZE; i++) {receive_buffer[i]=0;}
		receive_msg.id.std = 0x80;
		receive_msg.ctrl.ide = 0;
		receive_msg.ctrl.rtr = 0;
		receive_msg.dlc = DATA_BUFFER_SIZE;
		receive_msg.cmd = CMD_RX_DATA;

		while(CAN.cmd(&receive_msg) != CAN_CMD_ACCEPTED);
        
        */
            
            configureCan();

		myPrintf("CAN initialized.... \n");

		canInit = true;

		}

        simplehsm_transition_state(mParams->machine->hsm, bleComm, params);
        
		break;
	}
	case CAN_TICK_SIG:
	{
		//Can RX here
//		myPrintf("CAN tick...");

		//uint8_t status = CAN.get_status(&receive_msg);

//        myPrintf("CAN: %d \n", millis());
        
        bool isCanTimeout = false;
        
        if((millis() - lastCanCheck) > 1000)
        {
            isCanTimeout = true;
        }
        
        if(CANINT_Read() != 0 && !isCanTimeout)
        {
            //uint8_t txBuf[5];
            if(popCanFifo(send_buffer))
            {
//                myPrintf("CAN sent: %x %x %x %x %x \n", send_buffer[0], send_buffer[1], send_buffer[2], send_buffer[3], send_buffer[4]);

                FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);
                
                mcpCanSendMsgBuf(&can, 0x80, 0, DATA_BUFFER_SIZE, send_buffer);
                
                if(mcpCanCheckError() != CAN_OK)
                {
                    mcpCanClearError();   
                }
                
                FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
            }
//            myPrintf(".");
            break;
        }
        
        lastCanCheck = millis();
        
        FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);
        
        uint8 canStatus = mcpCanReadMsgBuf(&can, &bytesRead, receive_buffer);
        
        if(isCanTimeout)
        {
            myPrintf("%d CAN try read: %d\n", millis(), canStatus);
        }
        
        if(canStatus == CAN_OK)
        {

//            myPrintf("CAN recieve: %x %x %x %x %x \n", receive_buffer[0], receive_buffer[1], receive_buffer[2], receive_buffer[3], receive_buffer[4]);
//            myPrintf(",");
            
            if (receive_buffer[0] == SYSTEM_PACKET) {

//                myPrintf("CAN recieve: %x %x %x %x %x \n", receive_buffer[0], receive_buffer[1], receive_buffer[2], receive_buffer[3], receive_buffer[4]);
                
                srcAddr = receive_buffer[4] & 0x7F;
                
                if(srcAddr == sourceAdrMask || (millis() - lastAddrSys) > 2500)  // srcAddr == sourceAdrMask)
                {
                    if(srcAddr == sourceAdrMask)
                    {
                        lastAddrSys = millis();
                    }
                    
                    is24v_not12 = (receive_buffer[4] & 0xFC) == 0x80 ? 1 : 0;
                    
//                    is24v_not12 = (receive_buffer[4] >> 7) & 0x01;
                    
    				voltage = (((double)receive_buffer[1] * 5 / 255) * 3) + 0.7;
                    
                    if(is24v_not12)
                    {
                        if((voltage * 2) < 20.0)
                        {
                            voltage *= 2;
                            is24v_not12 = 0;
                        }
                        
                        
                    }
                    
    				temperature = receive_buffer[2];
                    
                    
                    sysCount++;
                
                    if(sysCount > 120)           // ping the system packet ~60s
                    {
                        sendBlePacket = true;
                        sysCount = 0;
                    }
                    
                }
                
                diff = receive_buffer[1] - lastValBuf[(srcAddr * BUF_SZ1) + LVB_SY + 0];
                    
                if(abs(diff) > 3)
                {
                    sendBlePacket = true;
                }
                
                diff = receive_buffer[2] - lastValBuf[(srcAddr * BUF_SZ1) + LVB_SY + 1];
                    
                if(abs(diff) > 3)
                {
                    sendBlePacket = true;
                }
                
                
                
                
                
                if(sendBlePacket && authGood)
                {
                    sendBlePacket = false;
                    sysCount = 0;
                
                    for(int i = 0 ; i < 4 ; i++)
                    {
                        lastValBuf[(srcAddr * BUF_SZ1) + LVB_SY + i] = receive_buffer[1 + i];
                    }
                    
//                    myPrintf("Send BLE system packet \n");
                    
                	machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                    
                }

			} 
            else if ((receive_buffer[0] & 0xF0) == SWITCH_PACKET && indexPos(receive_buffer[1]) != 255){
			
                srcAddr = receive_buffer[0] & 0x0F;
                
                if(true)//srcAddr == sourceAdrMask)
                {
					machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(SWITCH_STATUS_FROM_CAN_SIG, mParams);
                }   
                    
                    
                addr = indexPos(receive_buffer[1]);
                    
//                canChanInit[addr + srcAddr * 8] = 1;
                
                for(int i = 0 ; i < 3 ; i++)
                {
                    if((lastValBuf[(srcAddr * BUF_SZ1) + LVB_SW + (addr * 3) + i]) != (receive_buffer[2 + i]))
                    {
                        sendBlePacket = true;
                        break;
                    }
                }   
                
                if(sendBlePacket && authGood)
                {
                    sendBlePacket = false;
                
                    for(int i = 0 ; i < 3 ; i++)
                    {
                        lastValBuf[(srcAddr * BUF_SZ1) + LVB_SW + (addr * 3) + i] = receive_buffer[2 + i];
                    }
                    
                    myPrintf("Send BLE switch packet %d \n", addr);
                    
                	machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                    
                }

			} 
            else if ((receive_buffer[0] & 0xF0) == STATUS_PACKET && indexPos(receive_buffer[1]) != 255){
			
                srcAddr = receive_buffer[0] & 0x0F;
                
                if(srcAddr == sourceAdrMask)
                {
					machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(SWITCH_STATUS2_FROM_CAN_SIG, mParams);
                }   
                    
                    
                    
                addr = indexPos(receive_buffer[1]);
                
                if(0)//(lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3)]) != (receive_buffer[2]))
                {
                    sendBlePacket = true;
                } 
                else {
                    lastCur = lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + 2] | 
                            (lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + 1] << 8);
                    nextCur = receive_buffer[4] | (receive_buffer[3] << 8);
                        diff = lastCur - nextCur;
                    
                    if(abs(diff) > 3)
                    {
                        sendBlePacket = true;
                    }
                    
                    if(millis() - startConnectTime < 10000)
                    {
                        sendBlePacket = true;
                    }
                }
                 
                if(!isConnected)
                {
                    startConnectTime = millis();
                }
                
                
                if(sendBlePacket && authGood)
                {
                    sendBlePacket = false;
                
                    for(int i = 0 ; i < 3 ; i++)
                    {
                        lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + i] = receive_buffer[2 + i];
                    }
                    
//                    myPrintf("Send BLE status packet %d\n", addr);
                    
                	machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                    
                }    

			} 
            else if ((receive_buffer[0] & 0xF0) == PRO_PACKET_ID){
                
                srcAddr = receive_buffer[0] & 0x0F;
                
//                if(srcAddr == sourceAdrMask)
//                {
//					machineEvent evt;
//					evt.asPtrs[0] = (int) & receive_buffer;
//					mParams->mEvt = &evt;
//					publish(SWITCH_STATUS2_FROM_CAN_SIG, mParams);
//                }   
                    
                    
                addr = indexPos(receive_buffer[1]);
                
                uint8_t swIndex = srcAddr * 8 + addr;
                
                if(swIndex < 32 && addr != 255)
                {
                    inputIsEnabled[swIndex] = receive_buffer[2] & INPUT_EN_CAN_MASK ? true : false;
                    inputIsLockout[swIndex] = receive_buffer[2] & INPUT_LCK_CAN_MASK ? true : false;
                    inputIsInvert[swIndex] = receive_buffer[2] & INPUT_INV_CAN_MASK ? true : false;
                    
                    if(receive_buffer[2] & LINK_CAN_MASK)
                    {
                        uint8_t linkPacketIndex = (receive_buffer[2] & LINK_CAN_INDEX_MASK) == LINK_CAN_INDEX_VAL ? 1 : 0;
                        
                        uint32_t linkHalf = (receive_buffer[3] << 8) | receive_buffer[4];
                        
                        if(linkPacketIndex == 0)
                        {
                            switchIsLinked[swIndex] = (switchIsLinked[swIndex] & 0xffff0000) | linkHalf;
                        }
                        else
                        {
                            switchIsLinked[swIndex] = (switchIsLinked[swIndex] & 0x0000ffff) | (linkHalf << 16);
                        }
                    }
                    
//                    uint32_t tempLinks = receive_buffer[3];
//                    
//                    tempLinks = tempLinks & ~(0x01 << addr);
//                    tempLinks = tempLinks << (srcAddr * 8);
//                    
//                    uint32_t myLink = switchIsLinked[swIndex];
//                    uint32_t linkMask = ~(0xff << (srcAddr * 8));
//                    
//                    switchIsLinked[swIndex] = (myLink & linkMask) | tempLinks;      // replace just new byte
                    
                    
                    
                    if(isPro && isProWritable)
                    {
                        sendBlePacket = true;
                    }
                }
                else if(receive_buffer[1] == 0)
                {
                    
                }
                
                // save to flash?
               
//                    lastCur = lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + 2] | 
//                            (lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + 1] << 8);
//                    nextCur = receive_buffer[4] | (receive_buffer[3] << 8);
//                        diff = lastCur - nextCur;
//                    
//                    if(abs(diff) > 3)
//                    {
//                        sendBlePacket = true;
//                    }
//                    
//                    if(millis() - startConnectTime < 10000)
//                    {
//                        sendBlePacket = true;
//                    }
                
                 
//                if(!isConnected)
//                {
//                    startConnectTime = millis();
//                }
                
                
                if(sendBlePacket && authGood)      /// send when receive CAN, when changed, when sync
                {
                    sendBlePacket = false;
                
//                    for(int i = 0 ; i < 3 ; i++)
//                    {
//                        lastValBuf[(srcAddr * BUF_SZ1) + LVB_ST + (addr * 3) + i] = receive_buffer[2 + i];
//                    }
                    
//                    myPrintf("Send BLE status packet %d\n", addr);
                    
                	machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                    
                }    

			} 
            else if ((receive_buffer[0] == DEBUG_PACKET) && (receive_buffer[1] == 0x0A)) {
                
                for(int i = 0 ; i < 3 ; i++)
                {
                    if((lastValBuf[(srcAddr * BUF_SZ1) + LVB_DB + i]) != (receive_buffer[2 + i]))
                    {
                        sendBlePacket = true;
                        break;
                    }
                } 
                
                if(sendBlePacket && authGood)
                {
                    sendBlePacket = false;
                
                    for(int i = 0 ; i < 3 ; i++)
                    {
                        lastValBuf[(srcAddr * BUF_SZ1) + LVB_DB + i] = receive_buffer[2 + i];
                    }
                    
//                    myPrintf("Send BLE debug packet \n");
                    
                	machineEvent evt;
					evt.asPtrs[0] = (int) & receive_buffer;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                    
                }
			}
            
        }

        if(mcpCanCheckError() != CAN_OK)
        {
//            myPrintf(",");
            mcpCanClearError();   
        }

        FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
        
//        myPrintf("done... \n");
		break;
	}
	case SWITCH_STATUS_FROM_TS_SIG:
	{
//        if(millis() < 5000)
//        break;
        
		//Switch status has changes
		uint16_t *switchVal;
		bool *switchStatus;		// switch on/off
//		bool *switchIsDimmable;
		uint8_t index;
		uint8_t *onTime_buffer;
		uint8_t *offTime_buffer;

		switchVal = (uint16_t *) mEvt->asPtrs[0];
		switchStatus = (bool *) mEvt->asPtrs[1];
//		switchIsDimmable = (bool *) mEvt->asPtrs[2];
		onTime_buffer = (uint8_t *) mEvt->asPtrs[3];
		offTime_buffer = (uint8_t *) mEvt->asPtrs[4];
		index = mEvt->asChars[20];

//        FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);

		send_buffer[0] = (MY_ID_TAG | (index / 8)) ;
		send_buffer[1] = oneHot(index % 8);

		if(switchStatus[index])
			send_buffer[2] = (normalizeSlider(switchVal[index]) * .003891);
		else
			send_buffer[2] = 0x00;
        

        send_buffer[3] = onTime_buffer[index];
        send_buffer[4] = offTime_buffer[index];
            
        //myPrintf("CAN SWITCH_STATUS_FROM_TS_SIG queued\n");
        pushCanFifo(send_buffer);
        
//		myPrintf("CAN message sent...  ");
//		myPrintf(send_buffer[2]);
//        myPrintf("SWITCH_STATUS_FROM_TS_SIG: %x %x %x %x %x \n", send_buffer[0], send_buffer[1], send_buffer[2], send_buffer[3], send_buffer[4]);
//
//        mcpCanSendMsgBuf(&can, 0x80, 0, DATA_BUFFER_SIZE, send_buffer);
//        
//        if(mcpCanCheckError() != CAN_OK)
//        {
//            mcpCanClearError();   
//        }
//        
//        FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
        
		break;
	}
	case SWITCH_STATUS_FROM_BLE_SIG:
	{
//        if(millis() < 5000)
//        break;

        
		send_buffer[0] = mEvt->asChars[0];
		send_buffer[1] = mEvt->asChars[1];
		send_buffer[2] = mEvt->asChars[2];
		send_buffer[3] = mEvt->asChars[3];
		send_buffer[4] = mEvt->asChars[4];

//        FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);

        
        //myPrintf("CAN message queued\n");
        pushCanFifo(send_buffer);
        
//		myPrintf("CAN message sent: %x %x %x %x %x \n", send_buffer[0], send_buffer[1], send_buffer[2], send_buffer[3], send_buffer[4]);
//		myPrintf(send_buffer[2]);


//        mcpCanSendMsgBuf(&can, 0x80, 0, DATA_BUFFER_SIZE, send_buffer);
//        
//        if(mcpCanCheckError() != CAN_OK)
//        {
//            mcpCanClearError();   
//        }
//        
//        FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
        
		break;
	}
	case SEND_ALIVE_CAN_PACKET:
	{

//		myPrintf("Sending alive packet... ");
        
//        FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);
        
		//send_buffer[0] = (DEBUG_PACKET | sourceAdrMask);

		send_buffer[0] = (DEBUG_PACKET | (tick % 4));
		send_buffer[1] = CONNECTED;
		send_buffer[2] = TOUCHSCREEN_ADDR;
		send_buffer[3] = 0;
		send_buffer[4] = 0;


		tick++;

        pushCanFifo(send_buffer);
        
//        mcpCanSendMsgBuf(&can, 0x80, 0, DATA_BUFFER_SIZE, send_buffer);
//        
//        if(mcpCanCheckError() != CAN_OK)
//        {
//            mcpCanClearError();   
//        }
//        
//        FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
        
        break;
	}
    case SEND_PRO_CAN_PACKET:
    {
        myPrintf("SEND_PRO_CAN_PACKET \n");
        
        bool *inputEn;
        bool *inputLock;
        bool *inputInv;
        uint32_t *inputLinks;
        
        uint8_t index;

        
        inputEn = (bool *) mEvt->asPtrs[0];
        inputLock = (bool *) mEvt->asPtrs[1];
        inputInv = (bool *) mEvt->asPtrs[2];
        inputLinks = (uint32_t *) mEvt->asPtrs[3];

		index = mEvt->asChars[31];
        
//        myPrintf("pointers %d \n", inputInv);
        
//        myPrintf("%x %d %d-> ", PRO_PACKET_ID, sourceAdrMask, index);
//        myPrintf("(%x %x) ", mEvt->asPtrs[0], mEvt->asPtrs[3]);
//        if(index < 32)
//        {
//            //myPrintf("%x ", inputEn[index]);
//            myPrintf("%x\n", inputLinks[index]);
//        }
        if(index < 32 || index >= 255)
        {

//        FT_Transport_SPI_ChangeClock(CAN_SPI_CLK_FREQ);

        if(index == 255)
        {
            send_buffer[0] = (PRO_PACKET_ID | sourceAdrMask) ;
            
            send_buffer[1] = 0;
            send_buffer[2] = 0x04;
            send_buffer[3] = 0;
            send_buffer[4] = 0;
        }
        else
        {
            uint8_t addr = index/8;
            uint8_t sw = index%8;
            
            send_buffer[0] = (PRO_PACKET_ID | addr) ;
            
    		send_buffer[1] = oneHot(sw);
            
            uint8_t inputVal = LINK_CAN_MASK |
                            (inputEn[index] ? INPUT_EN_CAN_MASK : 0) |
                            (inputLock[index] ? INPUT_LCK_CAN_MASK : 0) |
                            (inputInv[index] ? INPUT_INV_CAN_MASK : 0);
            
            send_buffer[2] = inputVal;
            
            
//            uint8_t myLinks = (inputLinks[index] >> (addr * 8)) & 0xFF;
//            
//            myLinks = myLinks | (0x01 << sw);        // flag to indicate valid data
            
            send_buffer[3] = (inputLinks[index] >> 8) & 0xff;
            send_buffer[4] = (inputLinks[index] >> 0) & 0xff;
            
            printByteArray("CAN messagePro:", &send_buffer[0], 5);
            pushCanFifo(send_buffer);
            
            
            send_buffer[2] = inputVal | LINK_CAN_INDEX_VAL;
            
            send_buffer[3] = (inputLinks[index] >> 24) & 0xff;
            send_buffer[4] = (inputLinks[index] >> 16) & 0xff;
        }
        
        //myPrintf("CAN messagePro queued\n");
        printByteArray("CAN messagePro:", &send_buffer[0], 5);
        pushCanFifo(send_buffer);
        
            
//		myPrintf("CAN message sent...  ");
//		myPrintf(send_buffer[2]);
//        myPrintf("CAN messagePro sent: %x %x %x %x %x \n", send_buffer[0], send_buffer[1], send_buffer[2], send_buffer[3], send_buffer[4]);
//
//        
//        mcpCanSendMsgBuf(&can, 0x80, 0, DATA_BUFFER_SIZE, send_buffer);
//        
//        if(mcpCanCheckError() != CAN_OK)
//        {
//            mcpCanClearError();   
//        }
//        
//        FT_Transport_SPI_ChangeClock(FT_SPI_CLK_FREQ);
        }
        else
        {
            myPrintf("index error: %d \n", index);
        }
		break;
        
    }

	}

    
	return (void*) currentPage;
}

