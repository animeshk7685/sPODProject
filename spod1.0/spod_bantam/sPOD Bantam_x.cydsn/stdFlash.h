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

#ifndef STDFLASH_H
#define STDFLASH_H
    
    #include "common.h"
    
    // defines
    
    
    // externs
    
    extern status_t status;
    extern uint32 passkey;
    
    // declarations
    
    uint32 readPasskey(void);
    bool readProFlash(void);
    void writeFlash(void);
    void loadFlashDefaults(void);
    
#endif

/* [] END OF FILE */
