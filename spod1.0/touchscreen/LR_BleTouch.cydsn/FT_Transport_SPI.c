/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "FT_Transport_SPI.h"
#include "iprintf.h"

//de assert CSpin
void FT_Transport_SPI_EndTransfer()
{
//    CyDelayUs(8);
	FTCS_Write(1);
}

//api to set the cs pin number 
void FT_Transport_SPI_SetCSpin(uint8_t cspin)
{
	
}

void FT_Transport_SPI_Init()
{
	/* Initialize SPI library */
    

}
void FT_Transport_SPI_Exit()
{
	/* close the spi channel */
	
}
void FT_Transport_SPI_ChangeClock( uint32_t ClockValue)
{
    static uint32_t lastDivider = 0; 
    uint32_t clkDivider;
    
    clkDivider = (8 * 1000000 / ClockValue) - 1;
    
    if((clkDivider != lastDivider) && (clkDivider < 0xFFFF))
    {
//        myPrintf("%dHz, %d \n", ClockValue, clkDivider);
        Clock_1_SetDividerRegister(clkDivider, 1); 
        lastDivider = clkDivider;
    } 
    
}


/* APIs related to memory read & write/transport */
void FT_Transport_SPI_StartRead( uint32_t Addr)
{
	//make sure the union array indexes are modified for big endian usecase
	union Data32 data;
//	data.UUint32 = Addr;
    data.UUint32 = Addr << 8;
    SCB_SpiUartClearRxBuffer();
    SCB_SpiUartClearTxBuffer();
	FTCS_Write(0);
    
//    myPrintf("%x, %x%x%x%x\n", data.UUint32, data.A[0], data.A[1], data.A[2], data.A[3]);
    
    SCB_SpiUartWriteTxData((uint32) data.A[3]);
    SCB_SpiUartWriteTxData((uint32) data.A[2]);
    SCB_SpiUartWriteTxData((uint32) data.A[1]);
    SCB_SpiUartWriteTxData((uint32) 0x00);
    
//    while(SCB_SpiUartGetTxBufferSize() > 0){};
    while(SCB_SpiUartGetRxBufferSize() < 4){};
    SCB_SpiUartClearRxBuffer();
//    CyDelayUs(4);

}

void FT_Transport_SPI_HostCmd( uint8_t hostCmd)
{

    if(hostCmd != 0){       // power up (0x00) doesn't have the cmd bit
        hostCmd = hostCmd | 0x40;
    }
    
    FTCS_Write(0);
    
    SCB_SpiUartClearTxBuffer();
    SCB_SpiUartClearRxBuffer();
//	
//    while(SCB_SpiUartGetTxBufferSize() > 0){};
        
    
    SCB_SpiUartWriteTxData((uint32) hostCmd);
    SCB_SpiUartWriteTxData((uint32) 0x00);
    SCB_SpiUartWriteTxData((uint32) 0x00);
    
//    while(SCB_SpiUartGetTxBufferSize() > 0){};
    while(SCB_SpiUartGetRxBufferSize() < 3){};
    SCB_SpiUartClearRxBuffer();
    
    FTCS_Write(1);
}

void FT_Transport_SPI_StartWrite( uint32_t Addr)
{
	union Data32 data;
	data.UUint32 = Addr;
    data.A[2] = data.A[2] | 0x80;
    SCB_SpiUartClearTxBuffer();
    SCB_SpiUartClearRxBuffer();
	FTCS_Write(0);
        
    
    SCB_SpiUartWriteTxData((uint32) data.A[2]);
    SCB_SpiUartWriteTxData((uint32) data.A[1]);
    SCB_SpiUartWriteTxData((uint32) data.A[0]);
    
//    while(SCB_SpiUartGetTxBufferSize() > 0){};
    while(SCB_SpiUartGetRxBufferSize() < 3){};
    SCB_SpiUartClearRxBuffer();
}

//Read a byte from Addr location
uint8_t FT_Transport_SPI_Read( uint32_t Addr)
{
	union Data16 data;
    data.UUint16 = 0;
	FT_Transport_SPI_StartRead( Addr);
    SCB_SpiUartPutArray(data.A, 1);
    while(SCB_SpiUartGetRxBufferSize() < 1){};
	data.A[0] = SCB_SpiUartReadRxData();		
	FT_Transport_SPI_EndTransfer();
	return (data.A[0]);
}

//read a short from Addr location
uint16_t FT_Transport_SPI_Read16( uint32_t Addr)
{
	union Data16 data;
    data.UUint16 = 0;
	FT_Transport_SPI_StartRead( Addr);
    SCB_SpiUartPutArray(data.A, 2);
    while(SCB_SpiUartGetRxBufferSize() < 2){};
	data.A[0] = SCB_SpiUartReadRxData();
	data.A[1] = SCB_SpiUartReadRxData();		
	FT_Transport_SPI_EndTransfer();
	return (data.UUint16);
}
//read a word from Addr location
uint32_t FT_Transport_SPI_Read32( uint32_t Addr)
{
	union Data32 data;
    data.UUint32 = 0;
	FT_Transport_SPI_StartRead( Addr);
    
    SCB_SpiUartClearRxBuffer();
    
    SCB_SpiUartPutArray(data.A, 4);
    while(SCB_SpiUartGetRxBufferSize() < 4){};
    
	data.A[0] = SCB_SpiUartReadRxData();
	data.A[1] = SCB_SpiUartReadRxData();	
    data.A[2] = SCB_SpiUartReadRxData();
    data.A[3] = SCB_SpiUartReadRxData();
    
	FT_Transport_SPI_EndTransfer();
	return (data.UUint32);
}
//read N bytes from Addr location
void FT_Transport_SPI_Read_Src( uint32_t Addr, uint8_t *Src, uint32_t NBytes)
{
	uint32_t i;
	FT_Transport_SPI_StartRead( Addr);
	for(i=0;i<NBytes;i++)
    {
        SCB_SpiUartWriteTxData(0);
        while(SCB_SpiUartGetRxBufferSize() < 1){};
		*Src++ = SCB_SpiUartReadRxData();
    }
	FT_Transport_SPI_EndTransfer();
}

//write a byte to Addr location
void FT_Transport_SPI_Write( uint32_t Addr, uint8_t Value8)
{	
	FT_Transport_SPI_StartWrite( Addr);
	SCB_SpiUartWriteTxData(Value8);
    while(SCB_SpiUartGetRxBufferSize() < 1){};
	FT_Transport_SPI_EndTransfer();
}
//write a short to Addr location
void FT_Transport_SPI_Write16( uint32_t Addr, uint16_t Value16)
{
	union Data16 data;
    
	data.UUint16 = Value16;
	///little endian read
	FT_Transport_SPI_StartWrite( Addr);
    
	SCB_SpiUartPutArray(data.A, 2);
    while(SCB_SpiUartGetRxBufferSize() < 2){};
	FT_Transport_SPI_EndTransfer();
}	
//write a word to Addr location
void FT_Transport_SPI_Write32( uint32_t Addr, uint32_t Value32)
{
	union Data32 data;
    
	data.UUint32 = Value32;
	///little endian read
	FT_Transport_SPI_StartWrite( Addr);
	SCB_SpiUartPutArray(data.A, 4);
    while(SCB_SpiUartGetRxBufferSize() < 4){};
	FT_Transport_SPI_EndTransfer();
}	

void FT_Transport_SPI_Write_Src( uint32_t Addr, uint8_t *Src, uint32_t NBytes)
{
	uint32_t i;
	FT_Transport_SPI_StartWrite( Addr);
	for(i=0;i<NBytes;i++)
    {
        SCB_SpiUartClearRxBuffer();
        SCB_SpiUartWriteTxData(*Src++);
        while(SCB_SpiUartGetRxBufferSize() < 1){};
    }
	FT_Transport_SPI_EndTransfer();
}

/*
void FT_Transport_SPI_Writefromflash( uint32_t Addr, prog_uchar *Src, uint32_t NBytes)
{
	uint32_t i;
	StartWrite(Addr);
	for(i=0;i<NBytes;i++)
	{
		SPI.transfer(pgm_read_byte_near(Src));
		Src++;
	}		
	EndTransfer();
}
*/

//apis useful for performance via spi - write only functionality

//assert CSpin and send write command
void FT_Transport_SPI_StartTransfer( uint32_t Addr)
{
	union Data32 data;
	data.UUint32 = Addr;
    data.A[2] = data.A[2] | 0x80;
    SCB_SpiUartClearRxBuffer();
    SCB_SpiUartClearTxBuffer();
	FTCS_Write(0);
    
//    SCB_SpiUartPutArray(data.A, 3);
    
    SCB_SpiUartWriteTxData((uint32) data.A[2]);
    SCB_SpiUartWriteTxData((uint32) data.A[1]);
    SCB_SpiUartWriteTxData((uint32) data.A[0]);    
    
    while(SCB_SpiUartGetTxBufferSize() > 0){};
    SCB_SpiUartClearRxBuffer();
}


//transfer a single byte
void FT_Transport_SPI_Transfer( uint8_t Value8)
{
	SCB_SpiUartClearRxBuffer();
    SCB_SpiUartWriteTxData(Value8);
    while(SCB_SpiUartGetRxBufferSize() < 1){};
}
//transfer 2 bytes
void FT_Transport_SPI_Transfer16( uint16_t Value16)
{
	union Data16 data;
    
	data.UUint16 = Value16;
           
    SCB_SpiUartClearRxBuffer();
    SCB_SpiUartPutArray(data.A, 2);
    while(SCB_SpiUartGetRxBufferSize() < 2){};
}
//transfer 4 bytes
void FT_Transport_SPI_Transfer32( uint32_t Value32)
{
	union Data32 data;
    
	data.UUint32 = Value32;
           
    SCB_SpiUartClearRxBuffer();
    SCB_SpiUartPutArray(data.A, 4);
    while(SCB_SpiUartGetRxBufferSize() < 4){};		
}

//transfer N bytes
void FT_Transport_SPI_Transfer_Buff( uint8_t *Buff,uint32_t NBytes)
{
	uint32_t i;
	for(i=0;i<NBytes;i++)
	{
        SCB_SpiUartClearRxBuffer();
        SCB_SpiUartWriteTxData(*Buff++);
        while(SCB_SpiUartGetRxBufferSize() < 1){};
	}
}

/* [] END OF FILE */
