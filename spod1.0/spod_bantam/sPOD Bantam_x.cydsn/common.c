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

#include "common.h"

void printByteArray(char* header, uint8_t* addr, uint8_t num)
{
    myPrintf(header);
    
    for(uint8_t *p = addr; p < (addr + num); p++)
        myPrintf(" %x", *p);
        
    myPrintf("\n");
}

void printBoard(uint32_t boardRaw)
{
    uint16_t boardId;
    char boardRev, boardRev2;
    
    boardId = (boardRaw & 0xFFFF0000) >> 16;
    
    myPrintf("%drev", boardId);
    
    boardRev = boardRaw & 0x0000FFFF;
    
    if(boardRev > 26)
    {
        boardRev2 = boardRev / 26 + ('A' - 1);
        
        myPrintf("%c", boardRev2);
        
        boardRev = boardRev % 26 + ('A' - 1);
    }
    else
    {
        boardRev = boardRev + ('A' - 1);
    }
    
    myPrintf("%c\n", boardRev);
    
}

void write_millis(uint32_t mil)
{
    uint32_t ms = mil;//millis();
    
    myPrintf("(");
    
    if(ms/3600000)
    myPrintf("%d:",(ms/3600000));
    
    ms = ms % 3600000;
    
    if(ms/60000)
    myPrintf("%d:",(ms/60000));
    
    ms = ms % 60000;
    
    if(ms/10000 == 0)
    myPrintf("0");
    
    myPrintf("%d.",(ms/1000));
    
    ms = ms % 1000;
    
    if(ms/100 == 0)
    myPrintf("0");
    
    if(ms/10 == 0)
    myPrintf("0");
    
    myPrintf("%d)\n",ms);
    
}

double approxRollingAverage (double avg, double new_sample, double window) {
	double sub = avg / window;
	double add = new_sample / window;
	
    avg = avg - sub;
    avg = avg + add;

    return avg;
}

#define CAN_SW0             0x08
#define CAN_SW1             0x10
#define CAN_SW2             0x20
#define CAN_SW3             0x40
#define CAN_SW4             0x80
#define CAN_SW5             0x01
#define CAN_SW6             0x02
#define CAN_SW7             0x04  

uint8 iToCan(uint8 sw)
{
    switch (sw) {
    case 0:
        return CAN_SW0;
        break;
    case 1:
        return CAN_SW1;
        break;
    case 2:
        return CAN_SW2;
        break;
    case 3:
        return CAN_SW3;
        break;
    case 4:
        return CAN_SW4;
        break;
    case 5:
        return CAN_SW5;
        break;
    case 6:
        return CAN_SW6;
        break;
    case 7:
        return CAN_SW7;
        break;
    default:
        return 0;
        break;
    }  
}

uint8 canToI(uint8 sw)
{
    switch (sw) {
    case CAN_SW0:
        return 0;
        break;
    case CAN_SW1:
        return 1;
        break;
    case CAN_SW2:
        return 2;
        break;        
    case CAN_SW3:
        return 3;
        break;        
    case CAN_SW4:
        return 4;
        break;
    case CAN_SW5:
        return 5;
        break;        
    case CAN_SW6:
        return 6;
        break;        
    case CAN_SW7:
        return 7;
        break;        
    default:
        return 0xff;
        break;
    }  
}

/* [] END OF FILE */
