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

#ifndef COMMON_H
#define COMMON_H
    
    // includes
    
    #include <project.h>

    #include <stdbool.h>
    #include <stdlib.h>     // used for rand
    #include <stdio.h>      // for 1 use of sprintf...
       
    #include "typedefs.h"
    #include "pt.h"
    #include "timer.h"
    #include "ota_mandatory.h"
    
    #include "mcpCan.h"
    #include "myPrintf.h"
    #include "stdDrivers.h"
    #include "stdFlash.h"

    // defines
    
    
    // typedefs (included in typedefs.h to call before .h files)
    
    
    
    
    // externs
    
    //extern status_t status;
    extern void InitializeBootloaderSRAM(void);
    
    extern uint32_t millis(void);
    
    // declarations
    
    
    double approxRollingAverage (double avg, double new_sample, double window);
    
    void printByteArray(char* header, uint8_t* addr, uint8_t num);
    void write_millis(uint32_t mil);
    void printBoard(uint32_t boardRaw);
    
    uint8 canToI(uint8 sw);
    uint8 iToCan(uint8 sw);
    
#endif

/* [] END OF FILE */
