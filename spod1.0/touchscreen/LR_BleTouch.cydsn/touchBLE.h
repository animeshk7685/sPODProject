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

#ifndef TOUCHBLE_H_
#define TOUCHBLE_H_

#include "common.h"
#include "simpleHsm.h"
#include "pubSub.h"
#include "sPodCanTouch.h"
    
#define FLASH_SPEED     20      // __ms = 1 "tick"    
    
    
uint32 crc32(uint32 crc, const void *buf, size_t size);
    
void pairDevice(void);
    
void ProcessPacket(uint8 packetLength);

void StackEventHandler( uint32 eventCode, void *eventParam );
  
    

#endif

/* [] END OF FILE */
