



#include "touchscreen.h"
#include "mcp_can.h"

//#define DEBUG_MODE 1
#define DEBUG_RXANY 1


static void STBY_Write(int wtf) {} // TODO
static void SCB_SpiUartClearRxBuffer() {} // TODO
static void SCB_SpiUartWriteTxData(uint8_t data) {} // TODO
static void SCB_SpiUartPutArray(uint8_t*, int) {} // TODO
static uint8_t SCB_SpiUartReadRxData() {return 0;} // TODO
static void SPI_ss0_m_Write(int stuff) {} // TODO
static int SCB_SpiUartGetRxBufferSize() {return 0;} // TODO


void mcpCanSleep(bool sleep)
{
    uint8_t res;
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ);
    
    if(sleep)
    {
        STBY_Write(1);
        CyDelay(100);
        mcpCanReset();
        CyDelay(500);
        res = mcpCanSetCanCtrlMode(MODE_SLEEP);
        
         if(res != MCP2515_OK)
        {
            while((mcpCanReadRegister(MCP_CANSTAT) & 0xE0) != MODE_SLEEP)
            {
                CyDelay(100);
            }
        }
        myPrintf("CAN sleep\n");
    }
    else
    {
        STBY_Write(0);
        CyDelay(1);
        res = mcpCanSetCanCtrlMode(MODE_NORMAL);
         if(res != MCP2515_OK)
        {
            myPrintf("CAN Wake Error...\n");
            while(1){};
        }
        CyDelay(1);
        res = mcpCanInit(CAN_250KBPS);
         if(res != MCP2515_OK)
        {
            myPrintf("CAN Config Error...\n");
            while(1){};
        }
        myPrintf("CAN wake \n");
    }
    
    GC_ChangeClock(FT_SPI_CLK_FREQ);
    
}

void mcpCanReset()
{
    GC_ChangeClock(CAN_SPI_CLK_FREQ);
    SCB_SpiUartClearRxBuffer();
    
    SPI_ss0_m_Write(0);
    SCB_SpiUartWriteTxData(MCP_RESET);
    while(SCB_SpiUartGetRxBufferSize() < 1){};
    SPI_ss0_m_Write(1);
    CyDelay(10);
}

uint8_t mcpCanReadRegister(uint8_t address)
{
    uint8_t buffer[] = {MCP_READ, address, 0xFF};
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ);
    SCB_SpiUartClearRxBuffer();
     
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, 3);
    
    while(SCB_SpiUartGetRxBufferSize() < 3){};
    SPI_ss0_m_Write(1);
      
    buffer[0] = SCB_SpiUartReadRxData();
    buffer[1] = SCB_SpiUartReadRxData();
    buffer[2] = SCB_SpiUartReadRxData();
    return buffer[2];
}

void mcpCanReadRegisters(const uint8_t address, uint8_t *outBuffer, const uint8_t n)
{
	uint8_t i;
    uint8_t packetLength = n + 2;
    uint8_t buffer[packetLength];
    buffer[0] = MCP_READ;
    buffer[1] = address;
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ);
	SCB_SpiUartClearRxBuffer();
    
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, packetLength);
    
    while(SCB_SpiUartGetRxBufferSize() < packetLength){};
    SPI_ss0_m_Write(1);
    SCB_SpiUartReadRxData();
    SCB_SpiUartReadRxData();
    
	for (i=0; i < n; i++) 
    {
		outBuffer[i] = SCB_SpiUartReadRxData();
	}
}

void mcpCanSetRegister(uint8_t address, uint8_t value)
{
    uint8_t buffer[] = {MCP_WRITE, address, value};
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ); 
    SCB_SpiUartClearRxBuffer();
    
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, 3);     
    while(SCB_SpiUartGetRxBufferSize() < 3){};
    SPI_ss0_m_Write(1);
}

void mcpCanSetRegisters(uint8_t address, uint8_t *values, uint8_t n)
{
    uint8_t i;
    uint8_t packetLength = n + 2;
    uint8_t buffer[packetLength];
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ);
    buffer[0] = MCP_WRITE;
    buffer[1] = address;
    
    for (i=0; i<n; i++) 
    {
        buffer[i + 2] = values[i];
    }
    SCB_SpiUartClearRxBuffer();
     
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, packetLength); 
    
    while(SCB_SpiUartGetRxBufferSize() < packetLength){};
    SPI_ss0_m_Write(1);
}

void mcpCanModifyRegister(uint8_t address, uint8_t mask, uint8_t data)
{
    uint8_t buffer[] = {MCP_BITMOD, address, mask, data};
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ); 
    SCB_SpiUartClearRxBuffer();
    
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, 4); 
    while(SCB_SpiUartGetRxBufferSize() < 4){};
    SPI_ss0_m_Write(1);
}


uint8_t mcpCanReadStatus(void)                             
{
	uint8_t buffer[] = {MCP_READ_STATUS, 0xFF};
    
    GC_ChangeClock(CAN_SPI_CLK_FREQ); 
    SCB_SpiUartClearRxBuffer();
    
    SPI_ss0_m_Write(0);
    SCB_SpiUartPutArray(buffer, 2);
    while(SCB_SpiUartGetRxBufferSize() < 2){};
    SPI_ss0_m_Write(1);
    buffer[0] = SCB_SpiUartReadRxData();
    buffer[1] = SCB_SpiUartReadRxData();
    
    return buffer[1];
}

uint8_t mcpCanSetCanCtrlMode(const uint8_t newmode)
{
    uint8_t i;

    mcpCanModifyRegister(MCP_CANCTRL, MODE_MASK, newmode);

    i = mcpCanReadRegister(MCP_CANCTRL);
    i &= MODE_MASK;

    if ( i == newmode ) 
    {
        return MCP2515_OK;
    }
    return MCP2515_FAIL;
}

uint8_t mcpCanConfigRate(const uint8_t canSpeed)            
{
    uint8_t set, cfg1, cfg2, cfg3;
    set = 1;
    switch (canSpeed) 
    {
        case (CAN_5KBPS):
        cfg1 = MCP_16MHz_5kBPS_CFG1;
        cfg2 = MCP_16MHz_5kBPS_CFG2;
        cfg3 = MCP_16MHz_5kBPS_CFG3;
        break;

        case (CAN_10KBPS):
        cfg1 = MCP_16MHz_10kBPS_CFG1;
        cfg2 = MCP_16MHz_10kBPS_CFG2;
        cfg3 = MCP_16MHz_10kBPS_CFG3;
        break;

        case (CAN_20KBPS):
        cfg1 = MCP_16MHz_20kBPS_CFG1;
        cfg2 = MCP_16MHz_20kBPS_CFG2;
        cfg3 = MCP_16MHz_20kBPS_CFG3;
        break;
        
        case (CAN_31K25BPS):
        cfg1 = MCP_16MHz_31k25BPS_CFG1;
        cfg2 = MCP_16MHz_31k25BPS_CFG2;
        cfg3 = MCP_16MHz_31k25BPS_CFG3;
        break;

        case (CAN_40KBPS):
        cfg1 = MCP_16MHz_40kBPS_CFG1;
        cfg2 = MCP_16MHz_40kBPS_CFG2;
        cfg3 = MCP_16MHz_40kBPS_CFG3;
        break;

        case (CAN_50KBPS):
        cfg1 = MCP_16MHz_50kBPS_CFG1;
        cfg2 = MCP_16MHz_50kBPS_CFG2;
        cfg3 = MCP_16MHz_50kBPS_CFG3;
        break;

        case (CAN_80KBPS):
        cfg1 = MCP_16MHz_80kBPS_CFG1;
        cfg2 = MCP_16MHz_80kBPS_CFG2;
        cfg3 = MCP_16MHz_80kBPS_CFG3;
        break;

        case (CAN_100KBPS):                                             /* 100KBPS                  */
        cfg1 = MCP_16MHz_100kBPS_CFG1;
        cfg2 = MCP_16MHz_100kBPS_CFG2;
        cfg3 = MCP_16MHz_100kBPS_CFG3;
        break;

        case (CAN_125KBPS):
        cfg1 = MCP_16MHz_125kBPS_CFG1;
        cfg2 = MCP_16MHz_125kBPS_CFG2;
        cfg3 = MCP_16MHz_125kBPS_CFG3;
        break;

        case (CAN_200KBPS):
        cfg1 = MCP_16MHz_200kBPS_CFG1;
        cfg2 = MCP_16MHz_200kBPS_CFG2;
        cfg3 = MCP_16MHz_200kBPS_CFG3;
        break;

        case (CAN_250KBPS):
        cfg1 = MCP_16MHz_250kBPS_CFG1;
        cfg2 = MCP_16MHz_250kBPS_CFG2;
        cfg3 = MCP_16MHz_250kBPS_CFG3;
        break;

        case (CAN_500KBPS):
        cfg1 = MCP_16MHz_500kBPS_CFG1;
        cfg2 = MCP_16MHz_500kBPS_CFG2;
        cfg3 = MCP_16MHz_500kBPS_CFG3;
        break;
        
        case (CAN_1000KBPS):
        cfg1 = MCP_16MHz_1000kBPS_CFG1;
        cfg2 = MCP_16MHz_1000kBPS_CFG2;
        cfg3 = MCP_16MHz_1000kBPS_CFG3;
        break;  

        default:
        set = 0;
        break;
    }

    if (set) {
        mcpCanSetRegister(MCP_CNF1, cfg1);
        mcpCanSetRegister(MCP_CNF2, cfg2);
        mcpCanSetRegister(MCP_CNF3, cfg3);
        return MCP2515_OK;
    }
    else {
        return MCP2515_FAIL;
    }
}

void mcpCanWriteId( const uint8_t mcp_addr, const uint8_t ext, const uint32_t id )
{
    uint16_t canid;
    uint8_t tbufdata[4];

    canid = (uint16_t)(id & 0x0FFFF);

    if ( ext == 1) 
    {
        tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
        tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
        tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5 );
    }
    else 
    {
        tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3 );
        tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07 ) << 5);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
    mcpCanSetRegisters( mcp_addr, tbufdata, 4 );
}

void mcpCanInitCANBuffers(void)
{
    uint8_t i, a1, a2, a3;
    
    uint8_t std = 0;               
    uint8_t ext = 1;
    uint32_t ulMask = 0x00, ulFilt = 0x00;


    mcpCanWriteId(MCP_RXM0SIDH, ext, ulMask);			/*Set both masks to 0           */
    mcpCanWriteId(MCP_RXM1SIDH, ext, ulMask);			/*Mask register ignores ext bit */
    
                                                                        /* Set all filters to 0         */
    mcpCanWriteId(MCP_RXF0SIDH, ext, ulFilt);			/* RXB0: extended               */
    mcpCanWriteId(MCP_RXF1SIDH, std, ulFilt);			/* RXB1: standard               */
    mcpCanWriteId(MCP_RXF2SIDH, ext, ulFilt);			/* RXB2: extended               */
    mcpCanWriteId(MCP_RXF3SIDH, std, ulFilt);			/* RXB3: standard               */
    mcpCanWriteId(MCP_RXF4SIDH, ext, ulFilt);
    mcpCanWriteId(MCP_RXF5SIDH, std, ulFilt);

                                                                        /* Clear, deactivate the three  */
                                                                        /* transmit buffers             */
                                                                        /* TXBnCTRL -> TXBnD7           */
    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;
    for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
        mcpCanSetRegister(a1, 0);
        mcpCanSetRegister(a2, 0);
        mcpCanSetRegister(a3, 0);
        a1++;
        a2++;
        a3++;
    }
    mcpCanSetRegister(MCP_RXB0CTRL, 0);
    mcpCanSetRegister(MCP_RXB1CTRL, 0);
}

void mcpCanReadId( const uint8_t mcp_addr, uint8_t* ext, uint32_t* id )
{
    uint8_t tbufdata[4];

    *ext = 0;
    *id = 0;

    mcpCanReadRegisters( mcp_addr, tbufdata, 4 );

    *id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ( (tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M ) 
    {
                                                                        /* extended id                  */
        *id = (*id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id<<8) + tbufdata[MCP_EID8];
        *id = (*id<<8) + tbufdata[MCP_EID0];
        *ext = 1;
    }
}

void mcpCanWriteCanMsg( mcpCan *can, uint8_t buffer_sidh_addr)
{
    uint8_t mcp_addr;
    mcp_addr = buffer_sidh_addr;
    mcpCanSetRegisters(mcp_addr+5, can->m_nDta, can->m_nDlc );                  /* write data bytes             */
    if ( can->m_nRtr == 1)                                                   /* if RTR set bit in byte       */
    {
        can->m_nDlc |= MCP_RTR_MASK;  
    }
    mcpCanSetRegister((mcp_addr+4), can->m_nDlc );                        /* write the RTR and DLC        */
    mcpCanWriteId(mcp_addr, can->m_nExtFlg, can->m_nID );                     /* write CAN id                 */
}

void mcpCanReadCanMsg( mcpCan *can, uint8_t buffer_sidh_addr)        /* read can msg                 */
{
    uint8_t mcp_addr, ctrl;

    mcp_addr = buffer_sidh_addr;

    mcpCanReadId( mcp_addr, &can->m_nExtFlg, &can->m_nID );

    ctrl = mcpCanReadRegister( mcp_addr-1 );
    can->m_nDlc = mcpCanReadRegister( mcp_addr+4 );

    if ((ctrl & 0x08)) {
        can->m_nRtr = 1;
    }
    else {
        can->m_nRtr = 0;
    }

    can->m_nDlc &= MCP_DLC_MASK;
    mcpCanReadRegisters( mcp_addr+5, &(can->m_nDta[0]), can->m_nDlc );
}

void mcpCanStartTransmit(uint8_t mcp_addr)              /* start transmit               */
{
    mcpCanModifyRegister( mcp_addr-1 , MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M );
}

uint8_t mcpCanGetNextFreeTxBuf(uint8_t *txbuf_n)                 /* get Next free txbuf          */
{
    uint8_t res, i, ctrlval;
    uint8_t ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };

    res = MCP_ALLTXBUSY;
    *txbuf_n = 0x00;

                                                                        /* check all 3 TX-Buffers       */
    for (i=0; i<MCP_N_TXBUFFERS; i++) {
        ctrlval = mcpCanReadRegister( ctrlregs[i] );
        if ( (ctrlval & MCP_TXB_TXREQ_M) == 0 ) {
            *txbuf_n = ctrlregs[i]+1;                                   /* return SIDH-address of Buffe */
                                                                        /* r                            */
            res = MCP2515_OK;
            return res;                                                 /* ! function exit              */
        }
    }
    return res;
}

uint8_t mcpCanInit(uint8_t canSpeed)                       /* mcp2515init                  */
{

    
  uint8_t res;

    //SPI_SpiSetActiveSlaveSelect(SPI_SPIM_ACTIVE_SS1);
    mcpCanReset();

    res = mcpCanSetCanCtrlMode(MODE_CONFIG);
    if(res > 0)
    {
#if DEBUG_MODE
      while(1){}; 
#endif
      return res;
    }
#if DEBUG_MODE
    UART_PutString("Enter setting mode success \r\n");
#endif

                                                                        /* set boadrate                 */
    if(mcpCanConfigRate(canSpeed))
    {
#if DEBUG_MODE
      while(1){};
#endif
      return res;
    }
#if DEBUG_MODE
    UART_PutString("Set rate success!!\r\n");
#endif

    if ( res == MCP2515_OK ) {

                                                                        /* init canbuffers              */
        mcpCanInitCANBuffers();

                                                                        /* interrupt mode               */
        mcpCanSetRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

#if (DEBUG_RXANY==1)
                                                                        /* enable both receive-buffers  */
                                                                        /* to receive any message       */
                                                                        /* and enable rollover          */
        mcpCanModifyRegister(MCP_RXB0CTRL,
        MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
        MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
        
        mcpCanModifyRegister(MCP_RXB1CTRL, 
        MCP_RXB_RX_MASK,
        MCP_RXB_RX_ANY);
#else
                                                                        /* enable both receive-buffers  */
                                                                        /* to receive messages          */
                                                                        /* with std. and ext. identifie */
                                                                        /* rs                           */
                                                                        /* and enable rollover          */
        mcpCanModifyRegister(MCP_RXB0CTRL,
        MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
        MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK );
        
        mcpCanModifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
        MCP_RXB_RX_STDEXT);
#endif
                                                                        /* enter normal mode            */
        res = mcpCanSetCanCtrlMode(MODE_NORMAL);                                                                
        if(res)
        {
#if DEBUG_MODE        
          while(1){};
#endif           
          return res;
        }


#if DEBUG_MODE
          UART_PutString("Enter Normal Mode Success!!\r\n");
#endif

    }
    return res;

}

uint8_t mcpCanInitMask(uint8_t num, uint8_t ext, uint32_t ulData)
{
    uint8_t res = MCP2515_OK;
#if DEBUG_MODE
    UART_PutString("Begin to set Mask!!\r\n");
#endif
    res = mcpCanSetCanCtrlMode(MODE_CONFIG);
    if(res > 0){
#if DEBUG_MODE
    while(1){};
#endif
  return res;
}
    
    if (num == 0){
        mcpCanWriteId(MCP_RXM0SIDH, ext, ulData);

    }
    else if(num == 1){
        mcpCanWriteId(MCP_RXM1SIDH, ext, ulData);
    }
    else res =  MCP2515_FAIL;
    
    res = mcpCanSetCanCtrlMode(MODE_NORMAL);
    if(res > 0){
#if DEBUG_MODE
    while(1){}; 
#endif
    return res;
  }
#if DEBUG_MODE
    UART_PutString("Set Mask success!!\r\n");
#endif
    return res;
}

uint8_t mcpCanInitFilt(uint8_t num, uint8_t ext, uint32_t ulData)
{
    uint8_t res = MCP2515_OK;
#if DEBUG_MODE
    UART_PutString("Begin to set Filter!!\r\n");
#endif
    res = mcpCanSetCanCtrlMode(MODE_CONFIG);
    if(res > 0)
    {
#if DEBUG_MODE
      while(1){};
#endif
      return res;
    }
    
    switch( num )
    {
        case 0:
        mcpCanWriteId(MCP_RXF0SIDH, ext, ulData);
        break;

        case 1:
        mcpCanWriteId(MCP_RXF1SIDH, ext, ulData);
        break;

        case 2:
        mcpCanWriteId(MCP_RXF2SIDH, ext, ulData);
        break;

        case 3:
        mcpCanWriteId(MCP_RXF3SIDH, ext, ulData);
        break;

        case 4:
        mcpCanWriteId(MCP_RXF4SIDH, ext, ulData);
        break;

        case 5:
        mcpCanWriteId(MCP_RXF5SIDH, ext, ulData);
        break;

        default:
        res = MCP2515_FAIL;
    }
    
    res = mcpCanSetCanCtrlMode(MODE_NORMAL);
    if(res > 0)
    {
#if DEBUG_MODE
     while(1){}; 
#endif
      return res;
    }
#if DEBUG_MODE
    UART_PutString("set Filter success!!\r\n");
#endif
    
    return res;
}

uint8_t mcpCanSetMsg(mcpCan * can, uint32_t id, uint8_t ext, uint8_t len, uint8_t *pData)
{
    int i = 0;
    can->m_nExtFlg = ext;
    can->m_nID     = id;
    can->m_nDlc    = len;
    
    for(i = 0; i < MAX_CHAR_IN_MESSAGE; i++)
        can->m_nDta[i] = *(pData+i);
        
    return MCP2515_OK;
}

uint8_t mcpCanClearMsg(mcpCan * can)
{
    int i;
    
    can->m_nID       = 0;
    can->m_nDlc      = 0;
    can->m_nExtFlg   = 0;
    can->m_nRtr      = 0;
    can->m_nfilhit   = 0;
    
    for(i = 0; i<can->m_nDlc; i++ )
        can-> m_nDta[i] = 0x00;

    return MCP2515_OK;
}

uint8_t mcpCanSendMsg(mcpCan *can)
{
    uint8_t res, res1, txbuf_n;
    uint16_t uiTimeOut = 0;

    do {
        res = mcpCanGetNextFreeTxBuf(&txbuf_n);                       /* info = addr.                 */
        uiTimeOut++;
    } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

    if(uiTimeOut == TIMEOUTVALUE) 
    {   
        return CAN_GETTXBFTIMEOUT;                                      /* get tx buff time out         */
    }
    uiTimeOut = 0;
    mcpCanWriteCanMsg( can, txbuf_n);
    mcpCanStartTransmit( txbuf_n );
    do
    {
        uiTimeOut++;        
        res1= mcpCanReadRegister(txbuf_n);  			                /* read send buff ctrl reg 	*/
        res1 = res1 & 0x08;                               		
    }while(res1 && (uiTimeOut < TIMEOUTVALUE));   
    if(uiTimeOut == TIMEOUTVALUE)                                       /* send msg timeout             */	
    {
        return CAN_SENDMSGTIMEOUT;
    }
    return CAN_OK;

}

uint8_t mcpCanSendMsgBuf(mcpCan *can, uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf)
{
    mcpCanSetMsg(can, id, ext, len, buf);
    return mcpCanSendMsg(can);
}

uint8_t mcpCanReadMsg(mcpCan *can)
{
    uint8_t stat, res;

    stat = mcpCanReadStatus();

    if ( stat & MCP_STAT_RX0IF )                                        /* Msg in Buffer 0              */
    {
        mcpCanReadCanMsg(can, MCP_RXBUF_0);
        mcpCanModifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
        res = CAN_OK;
    }
    else if ( stat & MCP_STAT_RX1IF )                                   /* Msg in Buffer 1              */
    {
        mcpCanReadCanMsg(can, MCP_RXBUF_1);
        mcpCanModifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
        res = CAN_OK;
    }
    else 
    {
        res = CAN_NOMSG;
    }
    return res;
}

uint8_t mcpCanReadMsgBuf(mcpCan * can, uint8_t *len, uint8_t *buf)
{
    int i;
    uint8_t status = mcpCanReadMsg(can);
    *len = can->m_nDlc;
    for(i = 0; i < can->m_nDlc; i++)
    {
      buf[i] = can->m_nDta[i];
    }
    return status;
}

uint8_t mcpCanCheckReceive()
{
    uint8_t res;
    res = mcpCanReadStatus();                                         /* RXnIF in Bit 1 and 0         */
    if ( res & MCP_STAT_RXIF_MASK ) 
    {
        return CAN_MSGAVAIL;
    }
    else 
    {
        return CAN_NOMSG;
    }
}

uint8_t mcpCanCheckError()
{
    uint8_t eflg = mcpCanReadRegister(MCP_EFLG);

    if ( eflg & MCP_EFLG_ERRORMASK ) 
    {

#if DEBUG_MODE
        UART_PutString("E: ");
        UART_PutHexByte(eflg);  
        UART_PutString("\n");
#endif
        return CAN_CTRLERROR;
    }
    else 
    {
        return CAN_OK;
    }
}

void mcpCanClearError()
{
    mcpCanSetRegister(MCP_CANINTF, 0x00);
}

uint32_t mcpCanGetCanId(mcpCan * can)
{
    return can->m_nID;
}
