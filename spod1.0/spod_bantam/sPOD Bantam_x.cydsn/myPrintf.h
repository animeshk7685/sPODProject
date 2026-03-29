/******************************************************************************
*This file is for myPrintf()
*******************************************************************************/
#ifndef IPRINTF_H
#define IPRINTF_H
    
    //#include "common.h"
    
    #include <project.h>
    #include "stdDrivers.h"
    
    #define UART_ENABLED    (1)    
        
    #define TAG(f) if(f!=0)
    
    void myPrintf(char8 *pszFmt,...);
    
    #define FLT_DEC             2       // number of decimal places behind floats
    int32 fl(float32 val);
    
#endif
/* [] END OF FILE */
