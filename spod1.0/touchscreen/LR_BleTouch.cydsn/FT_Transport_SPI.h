/*****************************************************************************
* Copyright (c) Future Technology Devices International 2014
* propriety of Future Technology devices International.
*
* Software License Agreement
*
* This code is provided as an example only and is not guaranteed by FTDI. 
* FTDI accept no responsibility for any issues resulting from its use. 
* The developer of the final application incorporating any parts of this 
* sample project is responsible for ensuring its safe and correct operation 
* and for any consequences resulting from its use.
*****************************************************************************/
/**
* @file                           FT_Transport_SPI.h
* @brief                          Contains FT transport layer specific to SPI for arduino platform.
								  Tested platform version: Arduino 1.0.4 and later
								  Dependencies: arduino standard SPI library
* @version                        1.1.0
* @date                           2014/02/05
*
*/

#ifndef _FT_TRANSPORT_SPI_H_
#define _FT_TRANSPORT_SPI_H_
    
#include <project.h>
#include "bridge.h"

//de assert CSpin
void FT_Transport_SPI_EndTransfer();

//api to set the cs pin number 
void FT_Transport_SPI_SetCSpin(uint8_t cspin);

void FT_Transport_SPI_Init();
void FT_Transport_SPI_Exit();
void FT_Transport_SPI_ChangeClock( uint32_t ClockValue);
/* APIs related to memory read & write/transport */
void FT_Transport_SPI_StartRead( uint32_t Addr);
void FT_Transport_SPI_StartWrite( uint32_t Addr);
void FT_Transport_SPI_HostCmd( uint8_t hostCmd);
//Read a byte from Addr location
uint8_t FT_Transport_SPI_Read( uint32_t Addr);
//read a short from Addr location
uint16_t FT_Transport_SPI_Read16( uint32_t Addr);
//read a word from Addr location
uint32_t FT_Transport_SPI_Read32( uint32_t Addr);
//read N bytes from Addr location
void FT_Transport_SPI_Read_Src( uint32_t Addr, uint8_t *Src, uint32_t NBytes);

//write a byte to Addr location
void FT_Transport_SPI_Write( uint32_t Addr, uint8_t Value8);
//write a short to Addr location
void FT_Transport_SPI_Write16( uint32_t Addr, uint16_t Value16);
//write a word to Addr location
void FT_Transport_SPI_Write32( uint32_t Addr, uint32_t Value32);

void FT_Transport_SPI_Write_Src( uint32_t Addr, uint8_t *Src, uint32_t NBytes);

/*
void FT_Transport_SPI_Writefromflash( uint32_t Addr, prog_uchar *Src, uint32_t NBytes);
*/

//apis useful for performance via spi - write only functionality

//assert CSpin and send write command
void FT_Transport_SPI_StartTransfer( uint32_t Addr);

//transfer a single byte
void FT_Transport_SPI_Transfer( uint8_t Value8);
//transfer 2 bytes
void FT_Transport_SPI_Transfer16( uint16_t Value16);
//transfer 4 bytes
void FT_Transport_SPI_Transfer32( uint32_t Value32);
//transfer N bytes
void FT_Transport_SPI_Transfer_Buff( uint8_t *Buff,uint32_t NBytes);


#endif /* _FT_TRANSPORT_SPI_H_ */

