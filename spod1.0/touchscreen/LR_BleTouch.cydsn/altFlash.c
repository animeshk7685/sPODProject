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

//#include "simplehsm.h"
#include "pubSub.h"
//#include "simpleTimer.h"
#include "sPodImageData.h"
#include "sPodCanTouch.h"

#include "common.h"

//#define FLASH_ROW_SIZE                  CY_FLASH_SIZEOF_ROW   // 256 now....


//#define EEPROM_CALI_BYTE_SIZE           1*128
//#define EEPROM_BYTE_SIZE                10*128
//#define EEPROM_BYTE_SIZE                6*256


#define EEPROM_FLASH_ADDR_ROW           (((int)eepromBlock - CY_FLASH_BASE) / CY_FLASH_SIZEOF_ROW)   //(FLASH_NUMBER_ROWS - (11 + FLASH_TOP_SPACE))

#define EEPROM_CALI_FLASH_ADDR_ROW      (EEPROM_FLASH_ADDR_ROW + (EEPROM_BYTE_SIZE / CY_FLASH_SIZEOF_ROW))

            
#define EEPROM_CALI_FLASH_ADDR          (EEPROM_CALI_FLASH_ADDR_ROW * CY_FLASH_SIZEOF_ROW) 
#define EEPROM_FLASH_ADDR               (EEPROM_FLASH_ADDR_ROW * CY_FLASH_SIZEOF_ROW)

//static uint8 eepromLocalCaliArray[EEPROM_CALI_BYTE_SIZE];
//static uint8 eepromLocalArray[EEPROM_BYTE_SIZE];
uint8_t eepromLocalArray[EEPROM_BYTE_SIZE] = {0};
uint32_t *eepromLocalArray32 = (uint32_t *) eepromLocalArray;

static uint8 eepromLocalArray_row[CY_FLASH_SIZEOF_ROW];


void clear128array(void)
{
    for(unsigned int i = 0 ; i < CY_FLASH_SIZEOF_ROW ; i++)
    {
        eepromLocalArray_row[i] = 0;
    }
}

// set "sourceAdrMask" to the new value, then call this function to update other variables and store in flash
void updateSourceAddress(void)     
{
    for(int i = 0 ; i < 4 ; i++)
    {
        if(i == sourceAdrMask)
        {
            sourceAddr[i] = true;
        }else{
            sourceAddr[i] = false;
        }
    }
    
    eepromLocalArray[ADR_MASK_OFFSET] = sourceAdrMask;

	for(int i = 1; i <= 4; i++){
        eepromLocalArray[ADR_MASK_OFFSET+i] = (uint8_t)sourceAddr[i - 1];
	}
    
    saveSettingRow(ADR_MASK_OFFSET / CY_FLASH_SIZEOF_ROW);
    
    
 
}

void setWakeFromHib(bool set)     
{
    wakeFromHib = set;
    
    eepromLocalArray[DID_WAKE_OFFSET] = wakeFromHib;
    
    saveSettingRow(DID_WAKE_OFFSET / CY_FLASH_SIZEOF_ROW);
}

void saveSettingRow(uint8 row)
{
    
    if(row >= (EEPROM_BYTE_SIZE / CY_FLASH_SIZEOF_ROW))
    {
        myPrintf("ERROR: settings flash row value out of bounds\n");
    }
    else
    {
//        myPrintf(" saving flash row %d \n", row);
        
        for(unsigned int i = 0 ; i < CY_FLASH_SIZEOF_ROW ; i++)
        {
            eepromLocalArray_row[i] = eepromLocalArray[i + (CY_FLASH_SIZEOF_ROW * row)];
        }
        
        CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + row, (const unsigned char *) eepromLocalArray_row);
//        myPrintf("CySysFlashWriteRow(%x) \n", EEPROM_FLASH_ADDR_ROW + row);
    }

}



void loadSavedValues(void){

    uint8 *eeprom = (void *)EEPROM_FLASH_ADDR;
//    uint32_t *eepromLocalArray32 = (uint32_t *) eepromLocalArray;
    
    uint8 *localArrayP = (uint8 *) eepromLocalArray;
    
    for(unsigned int h = 0 ; h < sizeof(eepromLocalArray) ; h++)
    {
        localArrayP[h] = eeprom[h];
    }
    
    for(int i = 0; i< 32; i++){
		
        switchIsDimmable[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_DIMABLE_MASK ? 1 : 0;
        switchIsMomentary[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_MOMENTARY_MASK ? 1 : 0;
        switchIsStrobe[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_STROBE_MASK ? 1 : 0;
        switchIsFlash[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_FLASH_MASK ? 1 : 0;
        
        switchStrobeSet[i] = eepromLocalArray[SW_STATE_OFFSET+i] & IS_STROBE_SET_MASK ? 1 : 0;
        switchFlashSet[i] = eepromLocalArray[SW_STATE_OFFSET+i] & IS_FLASH_SET_MASK ? 1 : 0;
        
//        inputIsEnabled[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_INPUT_ENABLED_MASK ? 1 : 0;
//        inputIsLockout[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_INPUT_LOCKOUT_MASK ? 1 : 0;
//        inputIsInvert[i] = eepromLocalArray[SW_OPTIONS_OFFSET+i] & IS_INPUT_INVERT_MASK ? 1 : 0;
        
//        switchIsLinked[i] = eepromLocalArray[IS_LINKED_OFFSET + i];
        switchIsLinked[i] = eepromLocalArray32[(IS_LINKED_OFFSET / 4) + i];
        
        iconIsOn[i] = eepromLocalArray[IS_ICON_OFFSET + i];
        iconId[i] = eepromLocalArray[ICON_ID_OFFSET + i];
        
        switchVal[i] = eepromLocalArray[DIM_VALUE_OFFSET + i] * 255;
	}
    
    isOnRoad = eepromLocalArray[SW_STATE_OFFSET] & IS_OFF_ROAD_MASK ? 0 : 1;
    
    sourceAdrMask = eepromLocalArray[ADR_MASK_OFFSET];

	for(int k = 1; k <= 4; k++){
		sourceAddr[k - 1] = eepromLocalArray[ADR_MASK_OFFSET+k];
	}
    
    
    settings.sleepTimer = eepromLocalArray[TIMEOUT_OFFSET] * 60000;
	
    settings.dayBrightness = eepromLocalArray[BRIGHTNESS_OFFSET];
    
    wakeFromHib = eepromLocalArray[DID_WAKE_OFFSET];
    
    fahOn = eepromLocalArray[IS_FAHREN_OFFSET];
    
    
    if(!isPro)  // IS_PRO_TESTING
    {
        isPro = eepromLocalArray[IS_PRO_MODE_OFFSET];
    }
    
    isProWritable = eepromLocalArray[IS_PRO_WRITABLE_OFFSET];
    isDeepSleepDisabled = eepromLocalArray[IS_NO_DEEP_SLEEP_OFFSET];
    
    isSwipeOn = eepromLocalArray[IS_SWIPE_ENABLED_OFFSET];
    isLockoutOn = eepromLocalArray[IS_LOCKOUT_OFFSET];
    
//    isPro = 1;
    
    if(isPro){
        for(int j = 0; j < 32; j++){
    		switchStrobeOn[j] = eepromLocalArray[STROBE_ON_OFFSET + j];
            switchStrobeOff[j] = eepromLocalArray[STROBE_OFF_OFFSET + j];
    	}
    }
    

	myPrintf("loadSavedValues()\n");
//	myPrintf("%d\n", sizeof(buttonDefaultLabels));
//	myPrintf("%d\n", sizeof(buttonLabels));

    uint8 *tempArray = (uint8 *) buttonLabels;
    
    for(unsigned int p = 0; p < sizeof(buttonLabels) ; p++)
    {
        tempArray[p] = eepromLocalArray[BUTTON_LABELS_OFFSET + p];
    }

}

void loadDefaults(void){

	for(int i = 0; i<32; i++){
		eepromLocalArray[SW_OPTIONS_OFFSET + i] = 0;
        eepromLocalArray[SW_STATE_OFFSET + i] = 0;
//        eepromLocalArray[SW_OPTIONS_OFFSET + i] = IS_INPUT_ENABLED_MASK;
        
        eepromLocalArray32[IS_LINKED_OFFSET/4 + i] = 0;
        
        eepromLocalArray[IS_ICON_OFFSET + i] = 0;
        eepromLocalArray[ICON_ID_OFFSET + i] = 0xFF;
        
        eepromLocalArray[STROBE_ON_OFFSET + i] = 1;
        eepromLocalArray[STROBE_OFF_OFFSET + i] = 4;
        
        eepromLocalArray[DIM_VALUE_OFFSET + i] = 0;
	}    
    
    eepromLocalArray[ADR_MASK_OFFSET] = 0;

	for(int k = 1; k<=4; k++){
        eepromLocalArray[ADR_MASK_OFFSET + k] = 0;
	}

    eepromLocalArray[ADR_MASK_OFFSET + 1] = (uint8_t) 1;
    
    
    eepromLocalArray[TIMEOUT_OFFSET] = settings.sleepTimer / 60000;
    eepromLocalArray[BRIGHTNESS_OFFSET] = settings.dayBrightness;
    
    eepromLocalArray[DID_WAKE_OFFSET] = 0;
    
    eepromLocalArray[IS_FAHREN_OFFSET] = 1;
    
    eepromLocalArray[IS_PRO_MODE_OFFSET] = 0;
    eepromLocalArray[IS_PRO_WRITABLE_OFFSET] = 0;
    eepromLocalArray[IS_NO_DEEP_SLEEP_OFFSET] = IS_DEEP_SLEEP_DISABLED;
    
    eepromLocalArray[IS_SWIPE_ENABLED_OFFSET] = 1;
    eepromLocalArray[IS_LOCKOUT_OFFSET] = 0;
    

	myPrintf("loadDefaults()\n");

    uint8 *tempArray = (uint8 *) buttonDefaultLabels;
    
    for(unsigned int p = 0; p < sizeof(buttonLabels) ; p++)
    {
        eepromLocalArray[BUTTON_LABELS_OFFSET + p] = tempArray[p];
    }
    
    for(unsigned int q = 0 ; q < (EEPROM_BYTE_SIZE / CY_FLASH_SIZEOF_ROW) ; q++)
    {   
        for(unsigned int r = 0 ; r < CY_FLASH_SIZEOF_ROW ; r++)
        {
            eepromLocalArray_row[r] = eepromLocalArray[q * CY_FLASH_SIZEOF_ROW + r];
        }

        CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + q, (const unsigned char *) eepromLocalArray_row);
//            myPrintf("CySysFlashWriteRow(%x) \n", EEPROM_FLASH_ADDR_ROW + q);
    }   

}

// api to load new icon into flash
// cindex = switch# (0-31)
// data is the icon in L1 format (57p x 57p) in array
void loadIcon(uint8_t cindex, uint8_t* data)//[512])//, bool clr)
{
    static uint32_t buttonRawRow;
    buttonRawRow = ((int)button_Bitmap_RawData  - CY_FLASH_BASE) / CY_FLASH_SIZEOF_ROW;
        
    for(unsigned int i = 0 ; i < BUTTON_FLASH_RAWDATA_LENGTH / CY_FLASH_SIZEOF_ROW ; i++)
    {
        clear128array();
        
        for(unsigned int r = 0 ; r < CY_FLASH_SIZEOF_ROW ; r++)
        {
            eepromLocalArray_row[r] = data[(i * CY_FLASH_SIZEOF_ROW) + r];
        }
        
        CySysFlashWriteRow(buttonRawRow + (cindex * (BUTTON_FLASH_RAWDATA_LENGTH / CY_FLASH_SIZEOF_ROW)) + i, 
            (const unsigned char *) eepromLocalArray_row);
    }

//        iconIsOn[cindex] = 1;           /// << save to flash
        
//        eepromLocalArray[IS_ICON_OFFSET + cindex] = iconIsOn[cindex];
        
    saveSettingRow((uint8_t) IS_ICON_OFFSET/CY_FLASH_SIZEOF_ROW);
}


void loadConfig(void) {
    
    tCalibrate *caliEeprom = (void *)(EEPROM_CALI_FLASH_ADDR);
    
    unsigned int i;
	CyDelay(10);
    
    unsigned char *calibrateP = (unsigned char *) &calibrate;
    
//    tCalibrate *eeprom = (void *) EEPROM_CALI_FLASH_ADDR;
    unsigned char *eepromP = (unsigned char *) caliEeprom;
    
    for(i = 0 ; i < sizeof(calibrate) ; i++)
    {
        calibrateP[i] = eepromP[i];
    }
    
	CyDelay(10);

    myPrintf("loadConfig() \n");
}

void saveConfig(void) {
    myPrintf("saveConfig... \n");
	CyDelay(10);

	calibrate.crc = getConfigCrc();

    uint8 *tempArray = (uint8 *) &calibrate;        // cast to bytes
    
    clear128array();
    
    for(unsigned int i = 0 ; i < sizeof(calibrate) ; i++)
    {
        eepromLocalArray_row[i] = tempArray[i];        // move to 256B array
    }
    
	CyDelay(10);
    
    CySysFlashWriteRow(EEPROM_CALI_FLASH_ADDR_ROW, (const unsigned char *)eepromLocalArray_row);
//    myPrintf("CySysFlashWriteRow(%x) \n", EEPROM_CALI_FLASH_ADDR_ROW);
    
	CyDelay(10);


}


/* [] END OF FILE */
