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
    
#include "spod_library.h"
#include "status.h"
    
extern status_t status;
extern uint32_t passkey;
    
extern uint32_t readPasskey(void);
extern bool readFlash(void);
extern bool writeFlash(void);
extern void loadFlashDefaults(void);
    
#endif

/* [] END OF FILE */
