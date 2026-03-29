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

#include "stdFlash.h"

#define EEPROM_NUMOF_ROWS              (2)
#define EEPROM_TOTAL_SIZE               (EEPROM_NUMOF_ROWS * CY_FLASH_SIZEOF_ROW)

// main "eeprom" declaration
const uint8_t eepromBlock[EEPROM_TOTAL_SIZE] CY_ALIGN(CY_FLASH_SIZEOF_ROW) = {0};

#define EEPROM_FLASH_ADDR_ROW           (((int)eepromBlock - CY_FLASH_BASE) / CY_FLASH_SIZEOF_ROW)

#define PASSKEY_OFFSET                  (0x00)      // 1 * uint32_t     -> 0x04
#define SECURITY_MODE_OFFSET            (0x04)      // 1 * uint8_t      -> 0x05
#define IS_SEC_RESET_OFFSET             (0x05)      // 1 * bool         -> 0x06
#define IS_DEEP_SLEEP_EN                (0x06)      // 1 * bool         -> 0x07

#define PRO_MODE_OFFSET                 (0x08)      // 1 * uint8_t      -> 0x09
#define PRO_WRITABLE_OFFSET             (0x09)      // 1 * uint8_t      -> 0x0A
#define PRO_SLEEPDISABLED_OFFSET        (0x0A)      // 1 * uint8_t      -> 0x0B
#define PRO_ENABLELINKING_OFFSET        (0x0B)      // 1 * uint8_t      -> 0x0C

#define PRO_ALWAYSON_OFFSET             (0x10)      // 8 * uint8_t      -> 0x18
#define PRO_IGNCTRL_OFFSET              (0x18)      // 8 * uint8_t      -> 0x20
#define PRO_LOCKOUT_OFFSET              (0x20)      // 8 * uint8_t      -> 0x28
#define PRO_LASTSWVAL_OFFSET            (0x28)      // 8 * uint8_t      -> 0x30
#define PRO_TIMERS_OFFSET               (0x30)      // 8 * uint16_t     -> 0x40
#define PRO_INPUTLATCH_OFFSET           (0x40)      // 8 * uint8_t      -> 0x48
#define PRO_CURRENTLIMIT_OFFSET         (0x48)      // 8 * uint8_t      -> 0x50
#define PRO_CURRENTRESTEART_OFFSET      (0x50)      // 8 * uint8_t      -> 0x58
#define PRO_INPUTENABLED_OFFSET         (0x58)      // 8 * uint8_t      -> 0x60
#define PRO_INPUTLOCKOUT_OFFSET         (0x60)      // 8 * uint8_t      -> 0x68
//#define PRO_INPUTLOCKINVERT_OFFSET      (0x68)      // 8 * uint8_t      -> 0x70
#define PRO_INPUTLINKS_OFFSET           (0x80)      // 32 * uint32_t      -> 0x100
#define PRO_LAST_VALS_OFFSET            (0x100)      // 32 * 3 * uint8_t      -> 0x160

uint32 readPasskey(void)
{
    uint32_t* tempArray32 = (void *) eepromBlock;
    uint8_t* tempArray8 = (void *) eepromBlock;
    
    passkey = tempArray32[PASSKEY_OFFSET / 4];
    
    status.isUnsecured = tempArray8[SECURITY_MODE_OFFSET];
    status.isResetForSecUpdate = tempArray8[IS_SEC_RESET_OFFSET];
    
    status.isDeepSleepEnabled = tempArray8[IS_DEEP_SLEEP_EN];
    
//    passkey_init = tempArray[PASS_INIT_OFFSET / 4];
    
    myPrintf("Read passkey: %d\n", passkey);
    
    return passkey;
}

bool readProFlash(void)
{
    uint32_t* tempArray_uint32 = (void *) eepromBlock;
    uint16_t* tempArray_uint16 = (void *) eepromBlock;
    uint8_t* tempArray_uint8 = (void *) eepromBlock;
    
    status.pro.isEnabled = tempArray_uint8[PRO_MODE_OFFSET];
    
    
//    status.pro.isEnabled = 0;
    if(true)//status.pro.isEnabled)
    {
        status.pro.isWritable = tempArray_uint8[PRO_WRITABLE_OFFSET];
        status.pro.isDisableSleep = tempArray_uint8[PRO_SLEEPDISABLED_OFFSET];
        status.pro.isInputLinking = tempArray_uint8[PRO_ENABLELINKING_OFFSET];
        
        if(status.pro.isEnabled)
            myPrintf("Pro Enabled\n");
            
        for(int i = 0 ; i < 8 ; i++)
        {
            status.pro.alwaysOn[i] = tempArray_uint8[PRO_ALWAYSON_OFFSET + i];
            status.pro.isIgnCtrl[i] = tempArray_uint8[PRO_IGNCTRL_OFFSET + i]; 
            status.pro.isLockout[i] = tempArray_uint8[PRO_LOCKOUT_OFFSET + i]; 
            status.pro.lastSwVal[i] = tempArray_uint8[PRO_LASTSWVAL_OFFSET + i];
            status.pro.timers[i] = tempArray_uint16[PRO_TIMERS_OFFSET / 2 + i];
            
            //status.pro.links[i] = tempArray_uint8[PRO_INPUTLINKS_OFFSET + i];
            
            status.pro.isInputLatch[i] = tempArray_uint8[PRO_INPUTLATCH_OFFSET + i];
            status.pro.currentLimit[i] = tempArray_uint8[PRO_CURRENTLIMIT_OFFSET + i];
            status.pro.isCurrentRestart[i] = tempArray_uint8[PRO_CURRENTRESTEART_OFFSET + i];
            
            status.pro.isInputEnabled[i] = tempArray_uint8[PRO_INPUTENABLED_OFFSET + i];
            status.pro.isInputLockout[i] = tempArray_uint8[PRO_INPUTLOCKOUT_OFFSET + i];
//            status.pro.isInputLockInvert[i] = tempArray_uint8[PRO_INPUTLOCKINVERT_OFFSET + i];
            
            if(status.pro.lastSwVal[i] == 0)
                status.pro.lastSwVal[i] = 0xFF;
            
            status.out[i].outCmd = status.pro.alwaysOn[i] ? status.pro.lastSwVal[i] : 0x00;
        }
        
        for(int i = 0 ; i < 32 ; i++)
        {
            status.pro.links[i] = tempArray_uint32[PRO_INPUTLINKS_OFFSET/4 + i];
            
            status.pro.lastRecVals[i].dimVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 0];
            status.pro.lastRecVals[i].onVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 1];
            status.pro.lastRecVals[i].offVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 2];
        }
    }
    else
    {
        myPrintf("pro not enabled\n");
    }
    
    return status.pro.isEnabled;
}


void writeFlash(void)
{
    static uint8_t localEeprom[EEPROM_TOTAL_SIZE] = {0};
    static uint8_t *localEeprom_uint8 = (uint8_t *)localEeprom;
    static uint16_t *localEeprom_uint16 = (uint16_t *)localEeprom;
    static uint32_t *localEeprom_uint32 = (uint32_t *)localEeprom;

    localEeprom_uint32[PASSKEY_OFFSET/4] = passkey;
    
    localEeprom_uint8[SECURITY_MODE_OFFSET] = status.isUnsecured;
    localEeprom_uint8[IS_SEC_RESET_OFFSET] = status.isResetForSecUpdate;
    
    localEeprom_uint8[IS_DEEP_SLEEP_EN] = status.isDeepSleepEnabled;
    
    
    localEeprom_uint8[PRO_MODE_OFFSET] = status.pro.isEnabled ? 1 : 0;
    
    if(true)//status.pro.isEnabled)
    {   
        localEeprom_uint8[PRO_WRITABLE_OFFSET] = status.pro.isWritable ? 1 : 0;
        localEeprom_uint8[PRO_SLEEPDISABLED_OFFSET] = status.pro.isDisableSleep ? 1 : 0;
        localEeprom_uint8[PRO_ENABLELINKING_OFFSET] = status.pro.isInputLinking ? 1 : 0;
        
        for(unsigned int i = 0 ; i < 8 ; i++)
        {
            localEeprom_uint8[PRO_ALWAYSON_OFFSET + i] = status.pro.alwaysOn[i];
            localEeprom_uint8[PRO_IGNCTRL_OFFSET + i] = status.pro.isIgnCtrl[i];
            localEeprom_uint8[PRO_LOCKOUT_OFFSET + i] = status.pro.isLockout[i];
            
            localEeprom_uint8[PRO_LASTSWVAL_OFFSET + i] = status.pro.lastSwVal[i];
            
            localEeprom_uint16[PRO_TIMERS_OFFSET/2 + i] = status.pro.timers[i];
            
            //localEeprom_uint8[PRO_INPUTLINKS_OFFSET + i] = status.pro.links[i];
            
            localEeprom_uint8[PRO_INPUTLATCH_OFFSET + i] = status.pro.isInputLatch[i];
            localEeprom_uint8[PRO_CURRENTLIMIT_OFFSET + i] = status.pro.currentLimit[i];
            localEeprom_uint8[PRO_CURRENTRESTEART_OFFSET + i] = status.pro.isCurrentRestart[i];
            
            localEeprom_uint8[PRO_INPUTENABLED_OFFSET + i] = status.pro.isInputEnabled[i];
            localEeprom_uint8[PRO_INPUTLOCKOUT_OFFSET + i] = status.pro.isInputLockout[i];
//            localEeprom_uint8[PRO_INPUTLOCKINVERT_OFFSET + i] = status.pro.isInputLockInvert[i];
            
        }
        
        for(int i = 0 ; i < 32 ; i++)
        {
            localEeprom_uint32[PRO_INPUTLINKS_OFFSET/4 + i] = status.pro.links[i];
            
            localEeprom_uint8[PRO_LAST_VALS_OFFSET + 3*i + 0] = status.pro.lastRecVals[i].dimVal;
            localEeprom_uint8[PRO_LAST_VALS_OFFSET + 3*i + 1] = status.pro.lastRecVals[i].onVal;
            localEeprom_uint8[PRO_LAST_VALS_OFFSET + 3*i + 2] = status.pro.lastRecVals[i].offVal;
            
            //status.pro.links[i] = tempArray_uint32[PRO_INPUTLINKS_OFFSET/4 + i];
            
//            status.pro.lastRecVals[i].dimVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 0];
//            status.pro.lastRecVals[i].onVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 1];
//            status.pro.lastRecVals[i].offVal = tempArray_uint8[PRO_LAST_VALS_OFFSET + 3*i + 2];
        }
    }
    
//    for(int i = 0; i < CY_FLASH_SIZEOF_ROW - 4; i++)
//    {
//        // crc
//    }
    
    for (int i = 0; i < EEPROM_NUMOF_ROWS; i++)
    {
        CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + i, (const unsigned char *) &localEeprom[i * CY_FLASH_SIZEOF_ROW]);
    }
    
    //CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW, (const unsigned char *) localEeprom);
}

void loadFlashDefaults(void)
{
    for(int i = 0; i < 8; i++)
    {
        status.pro.isInputLatch[i] = true;
        status.pro.currentLimit[i] = 30;
        
        status.pro.isInputEnabled[i] = true;
    }
    
    for(int i = 0; i < 32; i++)
    {
        status.pro.links[i] = 0;
        
        status.pro.lastRecVals[i].dimVal = 0xff;
        status.pro.lastRecVals[i].onVal = 0xff;
        status.pro.lastRecVals[i].offVal = 0;
    }
}
/* [] END OF FILE */
