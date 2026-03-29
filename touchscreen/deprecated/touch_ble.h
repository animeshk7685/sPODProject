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
#include "simple_hsm.h"
#include "pub_sub.h"
#include "main.h"
    
#define FLASH_SPEED     20      // __ms = 1 "tick"    
    
    
uint32_t crc32(uint32_t crc, const void *buf, size_t size);
    
void pairDevice(void);
    
void ProcessPacket(uint8_t packetLength);

void StackEventHandler( uint32_t eventCode, void *eventParam );
  
    

#endif

/* [] END OF FILE */
