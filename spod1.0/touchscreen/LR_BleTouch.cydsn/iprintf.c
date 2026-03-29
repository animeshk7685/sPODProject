/******************************************************************************
*This file is for myPrintf()
*The myPrintf() is a simple printf() and only can print string with %s,%d,%c,%x.
*******************************************************************************/
#include "iprintf.h"
#include <project.h>

static void iputc(char8 ch)
{
	/*This function has to be replaced by user*/	

    UART_UartPutChar(ch);
}

int32 fl(float32 val)
{
    uint8 i;
    int32 nval = 10;
    
//    if(val < 0){
//        myPrintf("-");
//        val = val * -1;
//    }
    
    
    for(i = 1; i < FLT_DEC; i++)
    {
        nval *= 10; 
    }
    
    nval = (int32)(val * nval + 0.5);
    return nval;
}

static uint8* change(uint32 Index)
{
    return (uint8*)("0123456789abcdef" + Index);
}


void myPrintf(char8 *pszFmt,...)
{
    
    uint8 *pszVal;
    uint32 iVal, xVal, i = 0, buffer[12], index = 1;
    uint8 cVal;
    uint32 *pArg;
    pArg =(uint32 *)&pszFmt;

    while(*pszFmt)
    {
        if('%' != *pszFmt)
        {
            iputc(*pszFmt);
            pszFmt++;
            continue;
        }
        pszFmt++;

        if(*pszFmt == 's')
        {
            pszVal = (uint8*)pArg[index++];
            for(; *pszVal != '\0'; pszVal++)
                iputc(*pszVal);
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'd')
        {
            iVal = pArg[index++];
            i = 0;
            do{
                buffer[i++] = iVal % 10;
                iVal /= 10;
            }while(iVal);
            while(i > 0)
            {
                i--;
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'i')      // for negative integers
        {
            iVal = pArg[index++];
            i = 0;
            
            if(iVal & 0x8000)
            {
                iputc('-');
                iVal = ~iVal + 1;
            }
            
            do{
                buffer[i++] = iVal % 10;
                iVal /= 10;
            }while(iVal);
            while(i > 0)
            {
                i--;
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }  
        if(*pszFmt == 'f')      // floats must be passed through fl() to offset
        {
            iVal = pArg[index++];
            i = 0;
            
            if(iVal & 0x8000)
            {
                iputc('-');
                iVal = ~iVal + 1 + 1; // extra +1 otherwise it's randomly off by one...
            }
            
            do{
                buffer[i++] = (uint8)(iVal % 10);
                iVal /= 10;
            }while(iVal || (i < FLT_DEC + 1));  // make sure decimal and leading zero is displayed
            while(i > 0)
            {
                if(i == FLT_DEC)
                {
                    iputc('.');                 // add decimal place
                }
                i--;
                
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }       
        if(*pszFmt == 'c')
        {
            cVal = (uint8)pArg[index++];
            iputc(cVal);
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'x')
        {
            xVal = pArg[index++];
            i = 0;
            do{
                buffer[i++] = xVal % 16;
                xVal /= 16;
            }while(xVal);
            if(i%2!=0)
                buffer[i++]=0;
            if(i<2)
                buffer[i++]=0;

            while(i > 0)
            {
                i--;
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }
        if(pszFmt == '\0')
        {
            break;
        }

    }
}