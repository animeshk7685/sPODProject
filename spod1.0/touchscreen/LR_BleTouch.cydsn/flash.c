///* ========================================
// *
// * Copyright YOUR COMPANY, THE YEAR
// * All Rights Reserved
// * UNPUBLISHED, LICENSED SOFTWARE.
// *
// * CONFIDENTIAL AND PROPRIETARY INFORMATION
// * WHICH IS THE PROPERTY OF your company.
// *
// * ========================================
//*/
//
//#include "flash.h"
//#include "common.h"
//
//
//#define FLASH_NUMBER_ROWS               1024
//#define FLASH_ROW_SIZE                  128
//
//#define FLASH_TOP_SPACE                 16
//
////#define EEPROM_CALI_BYTE_SIZE           1*128
//#define EEPROM_BYTE_SIZE                10*128
//
//#define EEPROM_FLASH_ADDR_ROW           (FLASH_NUMBER_ROWS - (10 + FLASH_TOP_SPACE))
//#define EEPROM_CALI_FLASH_ADDR_ROW      (EEPROM_FLASH_ADDR_ROW + 9)
//
//#define EEPROM_FLASH_ADDR               (EEPROM_FLASH_ADDR_ROW * FLASH_ROW_SIZE)             
//#define EEPROM_CALI_FLASH_ADDR          (EEPROM_CALI_FLASH_ADDR_ROW * FLASH_ROW_SIZE) 
//
////static uint8 eepromLocalCaliArray[EEPROM_CALI_BYTE_SIZE];
//extern uint8 eepromLocalArray[EEPROM_BYTE_SIZE];
//static uint8 eepromLocalArray128[FLASH_ROW_SIZE];
//
//void clear128array(void)
//{
//    for(int i = 0 ; i < 128 ; i++)
//    {
//        eepromLocalArray128[i] = 0;
//    }
//}
//
//void writeRowToEeprom(uint32_t addr, uint8 * data)
//{
//    unsigned int length = 32;
//    unsigned int row = 0;
//    
//    if(addr == ADR_MASK_OFFSET)
//    length = 5;
//    
//    row = addr / 128;
//    
//    for(unsigned int i = 0 ; i < length ; i++)
//    {
//        eepromLocalArray[addr + i] = data[i];        
//    }
//    
//    for(unsigned int i = 0 ; i < 128 ; i++)
//    {
//        eepromLocalArray128[i] = eepromLocalArray[i + (128 * row)];
//    }
//    
//    CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + row, (const unsigned char *) eepromLocalArray128);
//}
//
//void loadSettings()
//{
//
//	settings.fps = 30;
//	settings.sleepTimer = 600000;
//	//settings.aliveTimer = 1000;//rand_interval(500,1500);
//	settings.dayBrightness = 128;
//	settings.nightBrightness = 64;
//	settings.dayNightThreshold = 128;
//	settings.fgColor = 0x000000;
//	settings.bgColor = 0x303030;
//
//	settings.color = 0xffffff;
//	settings.gradColor = 0xffffff;
//	settings.highlightColor = 0xEE0000;
//
//	settings.sliderBgColor = 0x101010;
//	settings.sliderFgColor = 0xEE0000;
//	settings.sliderColor = 0x404040;
//
//	settings.pageBgColor = 0x202020;
//    
//    myPrintf("loadSettings()\n");
//}
//
//uint8 *eeprom = (void *)EEPROM_FLASH_ADDR;
//
//static void loadSavedValues(void){
//	///static int sizeOf = 0;
//
////    eepromLocalArray; ///load local array from flash
//    
//    
//    uint8 *localArrayP = (uint8 *) eepromLocalArray;
//    
//     
//    for(unsigned int h = 0 ; h < sizeof(eepromLocalArray) ; h++)
//    {
//        localArrayP[h] = eeprom[h];
//    }
//    
////    CySysFlashWriteRow( );
////    
////    readFlash(EEPROM_FLASH_ADDR, eepromLocalArray, EEPROM_BYTE_SIZE);
//    
////    clearLocalEepromArray();
//    
//	for(int i = 0; i< 32; i++){
//		switchIsDimable[i] = eepromLocalArray[IS_DIMABLE_OFFSET+i];
//	}
//
//	for(int j = 0; j<32; j++){
//		switchisMomentary[j] = eepromLocalArray[IS_MOMENTARY_OFFSET+j];
//	}
//
//	sourceAdrMask = eepromLocalArray[ADR_MASK_OFFSET];
//
//	for(int k = 1; k <= 4; k++){
//		sourceAddr[k - 1] = eepromLocalArray[ADR_MASK_OFFSET+k];
//	}
//
//	for(int m = 0; m<32; m++){
//		switchisStrobe[m] = eepromLocalArray[IS_STROBE_OFFSET + m];
//	}
//
//	for(int n = 0; n<32; n++){
//		switchisFlash[n] = eepromLocalArray[IS_FLASH_OFFSET + n];
//	}
//
//	for(int o = 0; o<32; o++){
//	    switchIsLinked[o] = eepromLocalArray[IS_LINKED_OFFSET + o];
//	}
//
//	myPrintf("loadSavedValues()\n");
////	myPrintf("%d\n", sizeof(buttonDefaultLabels));
////	myPrintf("%d\n", sizeof(buttonLabels));
//
//    uint8 *tempArray = (uint8 *) buttonLabels;
//    
//    for(unsigned int p = 0; p < sizeof(buttonLabels) ; p++)
//    {
//        tempArray[p] = eepromLocalArray[BUTTON_LABELS_OFFSET + p];
//    }
//    
//
/////	localEepromReadAnything(BUTTON_LABELS_OFFSET, buttonLabels);
//
//}
//
//static void loadDefaults(void){
//	///static int addrIndex = 0;
//
//	for(int i = 0; i<32; i++){
//		eepromLocalArray[IS_DIMABLE_OFFSET + i] = 0;
//	}
//	for(int j = 0; j<32; j++){
//		eepromLocalArray[IS_MOMENTARY_OFFSET + j] = 0;
//	}
//
//	///EEPROM.write(ADR_MASK_OFFSET,0);
//    eepromLocalArray[ADR_MASK_OFFSET] = 0;
//
//	for(int k = 1; k<=4; k++){
//        eepromLocalArray[ADR_MASK_OFFSET + k] = 0;
//		///EEPROM.write(ADR_MASK_OFFSET+k ,0);
//	}
//
//    
//	///EEPROM.write(ADR_MASK_OFFSET +1 ,(uint8_t) 1);
//    eepromLocalArray[ADR_MASK_OFFSET + 1] = (uint8_t) 1;
//    
//	for(int m = 0; m<32; m++){
//		///EEPROM.write(IS_STROBE_OFFSET+m,0);
//        eepromLocalArray[IS_STROBE_OFFSET + m] = 0;
//	}
//
//	for(int n = 0; n<32; n++){
//		///EEPROM.write(IS_FLASH_OFFSET+n,0);
//        eepromLocalArray[IS_FLASH_OFFSET + n] = 0;
//	}
//
//	for(int o = 0; o<32; o++){
//		///EEPROM.write(IS_LINKED_OFFSET + o, 0);
//        eepromLocalArray[IS_LINKED_OFFSET + o] = 0;
//	}
//
//	myPrintf("loadDefaults()\n");
////	myPrintf("%d\n", sizeof(buttonDefaultLabels));
////	myPrintf("%d\n", sizeof(buttonLabels));
//
////	for(int p = 0; p < sizeof(buttonDefaultLabels); p++){
////			///char c = pgm_read_byte_near(buttonDefaultLabels + p);
////			///myPrintf("%c\n", c);
////			///EEPROM.write(BUTTON_LABELS_OFFSET + p, c);
////		}
////	myPrintf("\n");
//
//	//EEPROM_writeAnything(BUTTON_LABELS_OFFSET,buttonLabels);
//
//    uint8 *tempArray = (uint8 *) buttonDefaultLabels;
//    
//    for(unsigned int p = 0; p < sizeof(buttonLabels) ; p++)
//    {
//        eepromLocalArray[BUTTON_LABELS_OFFSET + p] = tempArray[p];
//    }
//    
////    eepromLocalArray; /// load into flash
//    
////    tempArray = (uint8 *)eepromLocalArray128;
//    
//    
//    
//    for(unsigned int q = 0 ; q < 9 ; q++)
//    {   
//        
//        for(unsigned int r = 0 ; r < FLASH_ROW_SIZE ; r++)
//        {
//            eepromLocalArray128[r] = eepromLocalArray[q * FLASH_ROW_SIZE + r];
//        }
//        
//        
//        CySysFlashWriteRow(EEPROM_FLASH_ADDR_ROW + q, (const unsigned char *) eepromLocalArray128);
//    }   
//}
//
//tCalibrate *caliEeprom = (void *)(EEPROM_CALI_FLASH_ADDR);
//
//static void loadConfig(void) {
//	///static int sizeOf = 0;
//    
//    unsigned int i;
//	CyDelay(10);
//    
//    unsigned char *calibrateP = (unsigned char *) &calibrate;
//    
////    tCalibrate *eeprom = (void *) EEPROM_CALI_FLASH_ADDR;
//    unsigned char *eepromP = (unsigned char *) caliEeprom;
//
//
//    
//    for(i = 0 ; i < sizeof(calibrate) ; i++)
//    {
//        calibrateP[i] = eepromP[i];
//    }
//    
/////	sizeOf = EEPROM_readyAnything(0, calibrate);
//	CyDelay(10);
//
//    myPrintf("loadConfig() \n");
//}
//
//static void saveConfig(void) {
//    ///remove
//    myPrintf("saveConfig... \n");
//	CyDelay(10);
//
//	calibrate.crc = getConfigCrc();
//
//    uint8 *tempArray = (uint8 *) &calibrate;        // cast to bytes
//    
//    clear128array();
//    
//    for(unsigned int i = 0 ; i < sizeof(calibrate) ; i++)
//    {
//        eepromLocalArray128[i] = tempArray[i];        // move to 128B array
//    }
//    
//	CyDelay(10);
//    
//    CySysFlashWriteRow(EEPROM_CALI_FLASH_ADDR_ROW, (const unsigned char *)eepromLocalArray128);
//    
//	///EEPROM_writeAnything(0,calibrate);
//	CyDelay(10);
//
//
//}
//
//
//
///* [] END OF FILE */
//