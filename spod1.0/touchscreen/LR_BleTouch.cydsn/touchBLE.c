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

//#include "touchBLE.h"


#include "common.h"
#include "simpleHsm.h"
#include "pubSub.h"
#include <stdio.h>
//#include "sPodCanTouch.h"
    
#define FLASH_SPEED     20      // __ms = 1 "tick"    

uint16_t sizeOfMtu = 23;

uint8_t sendOtaPacket = false;
uint8_t sendProPacket = 0;

uint32 ioAddress1 = 0x20;
uint32 ioAddress2 = 0x24;
bool isConnected = 0;
bool sendNotifications = 0;
bool pairingNeeded = 0;
bool pinGenerationNeeded = 0;
bool pinClearNeeded = 0;
uint16 tempPin = 0;
//uint8 i2cBuffer[3];

//uint8 bleRxBuffer[32];
//uint8 bleTxBuffer[20];
uint8 bleRxBuffer[100];
uint8 bleTxBuffer[100];

uint8 bleFoundDelimiter = 0;
uint8 bleBufferAddress = 0;
int blePacketLength = 0;


uint32 crc32(uint32 crc, const void *buf, size_t size);
    
void pairDevice(void);
    
void ProcessPacket(uint8 packetLength);

void StackEventHandler( uint32 eventCode, void *eventParam );
void clear512array(void);
void updateAuth(void);


//unsigned int rand_interval(unsigned int min, unsigned int max);
void generatePasskey(void);
bool readPasskey(void);
void writePassKey(uint32 thisKey);
void write_millis(void);



typedef struct settingsBLE
{
    uint8 appSourceAddress;
    uint8 sourceAddress;
    uint16 pin;
//    uint8 sleepTimer;
    uint32 crc;
    
}tSettingsBLE;

tSettingsBLE settingsStructBLE;
tSettingsBLE* settingsBLE = &settingsStructBLE;

void* paramsBLE;


static uint32 crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};//*/

uint32 crc32(uint32 crc, const void *buf, size_t size)
{
	const uint8_t *p;

	p = buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}


static uint8 iconData[512] = {0};


enum sPOD_Device_Type {
    BANTAM_V1,
    TOUCHSCREEN_V1,
    SWITCH_HD_V1
};

#define DEVICE_TYPE TOUCHSCREEN_V1

//uint8 lastTemp[4] = {0};
//uint8 lastBatt[4] = {0};
//
//uint8 lastSwitchValue[32] = {0};
//uint8 lastOutputState[32] = {0};
//uint16 lastOutputValue[32] = {0};
uint32 passkey = 1234;



CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};

#define PASS_DBG_TIMEOUT    30000       //    (30 * 1000)
#define PASSKEY_TIMEOUT     180000      //(3 * 60 * 1000)

static uint32_t passkeyTimer = 0;
//static bool passkey_init = false;


//unsigned int rand_interval(unsigned int min, unsigned int max) 
//{
//	unsigned int r;
//	const unsigned int range = 1 + max - min;
//	const unsigned int buckets = RAND_MAX/range;
//	const unsigned int limit = buckets * range;
//
//	do
//	{
//		r = rand();
//	}while (r >= limit);
//
//	return min + (r/buckets);
//}


void enterBootloader(void)
{
    
    uint32_t boardRead;
//    uint32_t boardRx, boardRead, stkBoardRead;
//    uint16_t appId, appVer, stkId, stkVer;
    uint16_t svdAppVer, svdStkVer;
//    uint16_t boardId;
//    char boardRev;
    
    
    char boardString[] = {"Hardware: "};
    char appVrString[] = {"Application: v"};
    char stackString[] = {"Stack: v"};
    char buff[2] = {0};
    
    boardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    svdAppVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    svdStkVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
    
    uint16_t boardId;
    char boardRev, boardRev2;
    
    boardId = (boardRead & 0xFFFF0000) >> 16;
    
    boardRev = boardRead & 0x0000FFFF;
    
    if(boardRev > 26)
    {
        boardRev2 = boardRev / 26 + ('A' - 1);
        buff[0] = (char)boardRev2;
        boardRev = boardRev % 26 + ('A' - 1);
        buff[1] = (char)boardRev;
    }
    else
    {
        boardRev = boardRev + ('A' - 1);
        buff[0] = (char)boardRev;
    }
    
    
    strcpy(ScratchArray, boardString);
	Dec2Ascii(ScratchArray, (boardId));
	strcat(ScratchArray,"rev");
    strcat(ScratchArray,buff);
//    Dec2Ascii(ScratchArray, (boardRev));

//    strcpy(ScratchArray1, appVrString);
//	Dec2Ascii(ScratchArray1, (svdAppVer / 0x0100));
//	strcat(ScratchArray1,".");
//    if((svdAppVer % 0x0100) < 10)
//        strcat(ScratchArray1,"0");
//    Dec2Ascii(ScratchArray1, (svdAppVer % 0x0100));
//    
//    strcpy(ScratchArray2, stackString);
//	Dec2Ascii(ScratchArray2, (svdStkVer / 0x0100));
//	strcat(ScratchArray2,".");
//    if((svdStkVer % 0x0100) < 10)
//        strcat(ScratchArray2,"0");
//    Dec2Ascii(ScratchArray2, (svdStkVer % 0x0100));
    
    strcpy(ScratchArray1, appVrString);
	Dec2Ascii(ScratchArray1, (svdAppVer / 0x0100));
	strcat(ScratchArray1,".");
    Dec2Ascii(ScratchArray1, (svdAppVer % 0x0100 / 0x0010));
    strcat(ScratchArray1,".");
    Dec2Ascii(ScratchArray1, (svdAppVer % 0x0010));
    
    strcpy(ScratchArray2, stackString);
	Dec2Ascii(ScratchArray2, (svdStkVer / 0x0100));
	strcat(ScratchArray2,".");
    Dec2Ascii(ScratchArray2, (svdStkVer % 0x0100 / 0x0010));
    strcat(ScratchArray2,".");
    Dec2Ascii(ScratchArray2, (svdStkVer % 0x0010));
    
    myPrintf("Enter Bootloader Mode \n");
    

    FT_GC_DLStart();
    
    FT_GC_ClearColorRGB_int(0x00);
		FT_GC_Clear(1, 1, 1);
		FT_GC_TagMask(1);

        drawTriText( 30, 10, 0, 0, 27,  
                        "Updating Firmware Over Bluetooth", "", "",
				false, 0, settings.color, true);
        
        
        
        
//        drawTriText( 15, 120, 0, 75, 26, 
//                        "1. Open CySmart and select 'OTA Bootloader'", 
//                        "2. Select 'OTA Bootloader' service         ", 
//                        "3. Follow onscreen instructions            ",
//				false, 0, settings.color, true);
        
        
        
        drawTriText( 15, 190, 0, 40, 20,  
                        "* See user manual for more information",
                        "",//(push calibration button to exit mode without updating)", 
                        "",
				false, 0, settings.greyColor, true);
    
        
        drawTriText( 15, 140, 0, 0, 20, ScratchArray, "", "",
				false, 0, settings.greyColor, true);
        drawTriText( 15, 150, 0, 0, 20, ScratchArray1, "", "",
				false, 0, settings.greyColor, true);
        drawTriText( 15, 160, 0, 0, 20, ScratchArray2, "", "",
				false, 0, settings.greyColor, true);
        
//        FT_GC_Tag(0);
        FT_GC_Cmd_Spinner(160, 75, 0, 0);
        
        FT_GC_DLEnd();
    		FT_GC_Finish();
        
//        while(1);
//        
//        while(1);
    
    Bootloadable_SetActiveApplication(0);
    Bootloadable_Load();
    CySoftwareReset();
    
}

void generatePasskey(void)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    static bool adcNotInit = true;
    uint32_t adcVal = 0;
    
    if(adcNotInit)
    {
        
        ADC_SAR_Seq_1_Start();
        ADC_SAR_Seq_1_SetChanMask(0b00000001);        
        ADC_SAR_Seq_1_StartConvert();
//        ADC_SAR_Seq_1_EnableInjection();
        
        CyDelayUs(25);
        
        adcVal = ADC_SAR_Seq_1_GetResult16(0);
        
//        myPrintf("temp val: %d \n",adcVal);
        
        srand(adcVal + 1);        // seed with random number      
        
        ADC_SAR_Seq_1_Stop();
        
        adcNotInit = false;
    }
    
    
    myPrintf("Wipe bonding\n");
    
    if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
    {
        CyBle_GappStopAdvertisement ();
    } 
        
    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
    
//    write_millis();
    
    while (CyBle_GetState() == CYBLE_STATE_CONNECTED)
 	{
 	    CyBle_ProcessEvents();
 	}
    
//    write_millis();
    
    apiResult = CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
    
    while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
        

    passkey = rand_interval(1, 999999);
//    passkey = 931136;
    
//    passkey_init = true;
    
    uint32_t* tempArray = (void *) &eepromLocalArray[PASSKEY_OFFSET];
    tempArray[0] = passkey;
    
//    eepromLocalArray[PASSKEY_OFFSET] = passkey;
    
    saveSettingRow(PASSKEY_OFFSET/CY_FLASH_SIZEOF_ROW);
    
    CyBle_GapFixAuthPassKey(1, passkey);
    writePassKey(passkey);
    passkeyTimer = millis();
    
    myPrintf("Generate new passkey: %d (%d)\n", passkey, apiResult);
     
    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
    
}

bool readPasskey(void)
{
    uint32_t* tempArray = (void *) &eepromLocalArray[PASSKEY_OFFSET];
    
    passkey = tempArray[0];
    
//    passkey = 888888;   ///
    
//    passkey_init = tempArray[PASS_INIT_OFFSET];
    
    myPrintf("Read passkey: %d\n", passkey);
    
    
    if(passkey == 0){
        return 0;
    } else {
        return 1;
    }
    
}



void clear512array(void)
{
    for(unsigned int i = 0 ; i < 512 ; i++)
    {
        iconData[i] = 0;
    }
}

static bool bleInit = false;

bool bleDisconnect = true;
bool authGood = false;


#define PRO_NUM_SW              (32)
#define PRO_NUM_OTHER           (1)
#define PRO_NUM_SW_TYPES        (3)
#define PRO_NUM_SW_PACKETS      (PRO_NUM_SW_TYPES * PRO_NUM_SW)

#define PRO_SW_SET_INDEX        (PRO_NUM_SW_PACKETS - 1)
#define PRO_OTH_INDEX           (PRO_SW_SET_INDEX + PRO_NUM_OTHER)
#define PRO_SW_INDEX            (PRO_OTH_INDEX + PRO_NUM_SW)
#define PRO_IN_INDEX            (PRO_SW_INDEX + PRO_NUM_SW)


stnext bleComm(int signal, void* params)
{
    static CYBLE_API_RESULT_T                  bleApiResult;
//    static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;
    static CYBLE_GATTS_HANDLE_VALUE_IND_T      characteristicIndicate;
    
    paramsBLE = params;                 // make global for file to pass to ProcessPacket()
    
    machineParams *mParams = (machineParams *) params;
	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
    
    uint8_t blePairVal;
    
    
    
    switch(signal)
    {
        case SIG_INIT:
        {
            if(!bleInit)
            { 
//                enterBootloader();
                
                CyBle_Start( StackEventHandler );
                
                if(readPasskey() == 0) {
                    generatePasskey();
                }
                
             	while (CyBle_GetState() == CYBLE_STATE_INITIALIZING){
             	    CyBle_ProcessEvents();
             	}
                
                myPrintf("BLE Initialized \n");
                
                bleInit = true;
                
                
            }
        
//            while(pairingNeeded){
//                pairDevice();
//                CyBle_ProcessEvents();
//            }
        }    
        break;
        case CAN_TICK_SIG:
        {
//            if(currentPage == onSettingsPage)
//            {
//                enterBootloader();
//            }
            
            if(sendOtaPacket)
            {
                newEvt.asChars[0] = 0;
    			mParams->mEvt = &newEvt;
                publish(BLE_SEND_OTA_SIG, mParams);
            }
            else if(sendProPacket != 0)
            {
                sendProPacket--;
                
//                if(sendProPacket > (65+32)) // input data
                
                if(sendProPacket > PRO_IN_INDEX)
                {
                    myPrintf("bad send pro: %d\n", sendProPacket);
                    sendProPacket = 0;
                }
                else if(sendProPacket > PRO_SW_INDEX) // input data
                {
//                    uint8_t index = (65+32+32) - sendProPacket;
                    uint8_t index = PRO_IN_INDEX - sendProPacket;
//                    sendProPacket--;
                    
                    machineEvent evt;

                    evt.asPtrs[0] = (int) &inputIsEnabled;
                	evt.asPtrs[1] = (int) &inputIsLockout;
                	evt.asPtrs[2] = (int) &inputIsInvert;
                    evt.asPtrs[3] = (int) &switchIsLinked;
                    
                	evt.asChars[31] = index;

                	mParams->mEvt = &evt;

                	publish(SEND_PRO_CAN_PACKET, mParams);
                    
                }
//                else if(sendProPacket > 65)     // basic switch data
                else if(sendProPacket > PRO_OTH_INDEX)     // basic switch data
                {
                    uint8_t buf[5];
//                    uint8_t index = 97 - sendProPacket;
                    uint8_t index = PRO_SW_INDEX - sendProPacket;
//                    sendProPacket--;
                    
                    buf[0] = 0x80 | (index / 8);
                    buf[1] = oneHot(index % 8);
                    buf[2] = switchStatus[index] ? 0xFF : 0;
                    buf[3] = switchStrobeOn[index];
                    buf[4] = switchStrobeOff[index];
                    
                    machineEvent evt;
                    evt.asPtrs[0] = (int) & buf;
					mParams->mEvt = &evt;
					publish(BLE_SEND_SIG, mParams);
                     
//                    machineEvent evt;
//					evt.asPtrs[0] = (int) & receive_buffer;
//					mParams->mEvt = &evt;
//					publish(SWITCH_STATUS_FROM_CAN_SIG, mParams);
                }
                else
                {
                    newEvt.asChars[0] = 0;
        			mParams->mEvt = &newEvt;
                    publish(BLE_SEND_PRO_SIG, mParams);
                }
            }

            
            updateAuth();
//                    myPrintf("BLE: %d ", millis());
            CyBle_ProcessEvents();
        }
        break;
        case BLE_SEND_SIG:
        {
//            myPrintf(".");
//            break;
            if(CyBle_GetState() == CYBLE_STATE_CONNECTED) 
            {
                //iprintf("BLE Connected...\n");
          
                /* if stack is free, handle UART traffic */
                if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
                {
                    //iprintf("BLE Stack Free...\n");
//                            sendBlePacket = 0;
                    if(sendNotifications){
                        
                        //iprintf("BLE Notification Enabled...\n");
                        int txLength = 5;
                        int i = 0;
                        
                        bleTxBuffer[0] = 0x55;
                        bleTxBuffer[1] = txLength + 5;
                        bleTxBuffer[2] = 0x00;
                        
                        uint8_t* temp = (void *) mEvt->asPtrs[0];
                        
                        for(i = 0 ; i < txLength; i++)
                        {
                            bleTxBuffer[3 + i] = temp[i];
                        }
                        
                        uint32 crc = crc32(0, bleTxBuffer, txLength + 3);
                        
                        uint32 packetLength = txLength + 7;
                        
                        bleTxBuffer[packetLength - 4] = crc & 0xFF;
                        bleTxBuffer[packetLength - 3] = crc >> 8;
                        bleTxBuffer[packetLength - 2] = crc >> 16;
                        bleTxBuffer[packetLength - 1] = crc >> 24;
                        
//                        myPrintf("Send Packet: ");
//                        
//                        for(int i = 0 ; i < packetLength ; i++)
//                        {
//                            myPrintf("%x ", bleTxBuffer[i]);
//                        }
//                        
//                        myPrintf("\n");
                        
//                        characteristicNotify.value.val = bleTxBuffer;
//                        characteristicNotify.value.len = packetLength;
//                        characteristicNotify.attrHandle = CYBLE_SPOD_COMM_CHAR_HANDLE;
                        
                        characteristicIndicate.value.val = bleTxBuffer;
                        characteristicIndicate.value.len = packetLength;
                        characteristicIndicate.attrHandle = CYBLE_SPOD_COMM_CHAR_HANDLE;
                        
                        do
                        {
//                            bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                            bleApiResult = CyBle_GattsIndication(cyBle_connHandle, &characteristicIndicate);
                            CyBle_ProcessEvents();
                        }
                        while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                        
                    }
                }
            }
            
            
            
        }
        break;
        case BLE_PAIR_SIG:
        {
//        myPrintf("ping \n");
        
            blePairVal = mEvt->asChars[0];
            
            if(blePairVal == BLE_PAIR)
            {
                passkeyTimer = millis();
                
                myPrintf("ble pair \n");
            }
            
            if(blePairVal == BLE_RESET)
            {
                generatePasskey();
                
                myPrintf("ble reset \n");
            }
        }
        break;
        case BLE_SEND_OTA_SIG:
        {
            
            if(CyBle_GetState() == CYBLE_STATE_CONNECTED) 
            {
                //iprintf("BLE Connected...\n");
          
                /* if stack is free, handle UART traffic */
                if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
                {
                    sendOtaPacket = false;
            
                    int packetLength = 20;
                    
                    uint32_t appBoard, stkBoard;
                    uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
                    
                    
                    appBoard = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
                    stkBoard = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
                    
                    svdAppId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
                    svdAppVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
                    svdStkId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
                    svdStkVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
                    
                    
                    
                    bleTxBuffer[0] = packetLength;
                    bleTxBuffer[1] = 0x00;
                    
                    bleTxBuffer[2] = (appBoard >> 24) & 0xFF;
                    bleTxBuffer[3] = (appBoard >> 16) & 0xFF;
                    bleTxBuffer[4] = (appBoard >> 8) & 0xFF;
                    bleTxBuffer[5] = (appBoard >> 0) & 0xFF;
                    
                    bleTxBuffer[6] = (svdAppId >> 8) & 0xFF;
                    bleTxBuffer[7] = (svdAppId >> 0) & 0xFF;
                    
                    bleTxBuffer[8] = (svdAppVer >> 8) & 0xFF;
                    bleTxBuffer[9] = (svdAppVer >> 0) & 0xFF;
                    
                    bleTxBuffer[10] = (stkBoard >> 24) & 0xFF;
                    bleTxBuffer[11] = (stkBoard >> 16) & 0xFF;
                    bleTxBuffer[12] = (stkBoard >> 8) & 0xFF;
                    bleTxBuffer[13] = (stkBoard >> 0) & 0xFF;
                    
                    bleTxBuffer[14] = (svdStkId >> 8) & 0xFF;
                    bleTxBuffer[15] = (svdStkId >> 0) & 0xFF;
                    
                    bleTxBuffer[16] = (svdStkVer >> 8) & 0xFF;
                    bleTxBuffer[17] = (svdStkVer >> 0) & 0xFF;
                    
                    
                    bleTxBuffer[18] = sizeOfMtu >> 8;
                    bleTxBuffer[19] = sizeOfMtu & 0xFF;
                    
                    
                    
                    
//                    characteristicNotify.value.val = bleTxBuffer;
//                    characteristicNotify.value.len = packetLength;
//                    characteristicNotify.attrHandle = CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE;
                    
                    characteristicIndicate.value.val = bleTxBuffer;
                    characteristicIndicate.value.len = packetLength;
                    characteristicIndicate.attrHandle = CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE;
                    
                    
                    myPrintf("Send OTA Packet\n");
                    
                    do
                    {
                        
//                        bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                        bleApiResult = CyBle_GattsIndication(cyBle_connHandle, &characteristicIndicate);
                        CyBle_ProcessEvents();
                    }
                    while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                    
                    myPrintf("Success \n");
                        
                    }
                
                
            }
        }
        break;
        case BLE_SEND_PRO_SIG:
        {
            if(authGood && CyBle_GetState() == CYBLE_STATE_CONNECTED)
            {            
                static CYBLE_API_RESULT_T                  bleApiResult;
//                static CYBLE_GATTS_HANDLE_VALUE_NTF_T      characteristicNotify;
//                static CYBLE_GATTS_HANDLE_VALUE_IND_T      characteristicIndicate;
                
                int packetLength = 20;
                
//                sendProPacket--;
                
                
                if(sendProPacket == PRO_OTH_INDEX)
                {
                    packetLength = 4;
                    
                    bleTxBuffer[0] = packetLength;
                    bleTxBuffer[1] = DEVICE_TYPE << 6;
                    bleTxBuffer[2] = sendProPacket;
                    bleTxBuffer[3] = isDeepSleepDisabled;
                    
                    // deep sleep
                    // sleep timer
                    // backlight
                    // address
                    // on/off rd
                    // other?
                    
                }
                else
                {
                    if(sendProPacket > PRO_SW_SET_INDEX)
                    {
                        myPrintf("bad send pro: %d\n", sendProPacket);
                        
                        sendProPacket = 0;
                        
                        break;
                    }
                    
//                    uint8_t index2 = 63 - sendProPacket;
//                    uint8_t index = index2 / 2;
//                    uint8_t isSecond = index2 % PRO_NUM_SW_TYPES;//2;
                    uint8_t index3 = PRO_SW_SET_INDEX - sendProPacket;
                    uint8_t index = index3 / PRO_NUM_SW_TYPES;
                    uint8_t typeIndex = index3 % PRO_NUM_SW_TYPES;
                    
//                    packetLength = 20;
//                    
//                    uint8_t offset = 3 - sendProPacket;
//                    uint8_t packetType = 0x02 + 1 + offset;
                    bool isErr = false;
                    
                    switch (typeIndex)
                    {
                        case 0:
                        {
                            packetLength = 9;
                            bleTxBuffer[0] = packetLength;
                            bleTxBuffer[1] = 2 | (DEVICE_TYPE << 6);
                            bleTxBuffer[2] = index3;
                            
                            bleTxBuffer[3] = 
                                ((switchIsDimmable[index] & 0x01) << 0) | 
                                ((switchIsMomentary[index] & 0x01) << 1) | 
                                ((switchIsFlash[index] & 0x01) << 2) | 
                                ((switchIsStrobe[index] & 0x01) << 3);
                                
//                            bleTxBuffer[4] = 0; 
                            
                            bleTxBuffer[4] = iconIsOn[index] != 0 ? iconId[index] + 1 : 0;
                              
                            bleTxBuffer[5] = (switchIsLinked[index] >> 0) & 0xff;
                            bleTxBuffer[6] = (switchIsLinked[index] >> 8) & 0xff;
                            bleTxBuffer[7] = (switchIsLinked[index] >> 16) & 0xff;
                            bleTxBuffer[8] = (switchIsLinked[index] >> 24) & 0xff;
                            
    //                        bleTxBuffer[5] = switchIsLinked[index];     /// todo, fix ble linking 
                            
//                            bleTxBuffer[6] = iconIsOn[index] != 0 ? iconId[index] + 1 : 0;
                            
                            if(sizeOfMtu > (39 + 3))
                            {
                                for(int i = 0 ; i < 10 ; i++)
                                {
                                    bleTxBuffer[9 + i] = buttonLabels[index].line1[i];
                                    bleTxBuffer[19 + i] = buttonLabels[index].line2[i];
                                    bleTxBuffer[29 + i] = buttonLabels[index].line3[i];
                                }
                            
//                                sendProPacket--;
                                sendProPacket -= 2;
                            
                                packetLength = 39;
                                bleTxBuffer[0] = packetLength;
                            }
                            
                        }
                        break;
                        case 1:
                        {
                            packetLength = 18;
                            bleTxBuffer[0] = packetLength;
                            bleTxBuffer[1] = 2 | (DEVICE_TYPE << 6);
                            bleTxBuffer[2] = index3;
                            
                            if(sizeOfMtu > (30 + 3))
                            {
                                for(int i = 0 ; i < 10 ; i++)
                                {
                                    bleTxBuffer[3 + i] = buttonLabels[index].line1[i];
                                    bleTxBuffer[13 + i] = buttonLabels[index].line2[i];
                                    bleTxBuffer[23 + i] = buttonLabels[index].line3[i];
                                }
                            
                                sendProPacket--;
                            
                                packetLength = 33;
                                bleTxBuffer[0] = packetLength;
                            }
                            else
                            {
                                for(int i = 0 ; i < 10 ; i++)
                                {
                                    bleTxBuffer[3 + i] = buttonLabels[index].line1[i];
                                }
                                
                                for(int i = 0 ; i < 5 ; i++)
                                {
                                    bleTxBuffer[13 + i] = buttonLabels[index].line2[i];
                                }
                            }
                        }
                        break;
                        case 2:
                        {
                            packetLength = 18;
                            bleTxBuffer[0] = packetLength;
                            bleTxBuffer[1] = 2 | (DEVICE_TYPE << 6);
                            bleTxBuffer[2] = index3;
                            
                            for(int i = 5 ; i < 10 ; i++)
                            {
                                bleTxBuffer[3 + (i - 5)] = buttonLabels[index].line2[i];
                            }
                            
                            for(int i = 0 ; i < 10 ; i++)
                            {
                                bleTxBuffer[8 + i] = buttonLabels[index].line3[i];
                            }
                        }
                        break;
                        default:
                            myPrintf("bad pro indexing: %d/%d\n", typeIndex, sendProPacket);
                            sendProPacket = 0;
                            isErr = true;
                        break;
                    }
                    
                    if(isErr)
                        break;
                    
//                    if(!isSecond)
//                    {
//                        packetLength = 20;
//                        bleTxBuffer[0] = packetLength;
//                        bleTxBuffer[1] = 1 | (DEVICE_TYPE << 6);
//                        bleTxBuffer[2] = index3;
//                        
//                        bleTxBuffer[3] = 
//                            ((switchIsDimmable[index] & 0x01) << 0) | 
//                            ((switchIsMomentary[index] & 0x01) << 1) | 
//                            ((switchIsFlash[index] & 0x01) << 2) | 
//                            ((switchIsStrobe[index] & 0x01) << 3);
//                            
//                        bleTxBuffer[4] = 0; 
//                          
//                        bleTxBuffer[5] = 0;
//                        
////                        bleTxBuffer[5] = switchIsLinked[index];     /// todo, fix ble linking 
//                        
//                        bleTxBuffer[6] = iconIsOn[index] != 0 ? iconId[index] + 1 : 0;
//                        
//                        
//                        
//                        if(sizeOfMtu > (37 + 3))
//                        {
//                            for(int i = 0 ; i < 10 ; i++)
//                            {
//                                bleTxBuffer[7 + i] = buttonLabels[index].line1[i];
//                                bleTxBuffer[17 + i] = buttonLabels[index].line2[i];
//                                bleTxBuffer[27 + i] = buttonLabels[index].line3[i];
//                            }
//                            
//                            sendProPacket--;
//                            
//                            packetLength = 37;
//                            bleTxBuffer[0] = packetLength;
//                        }
//                        else
//                        {
//                            
//                            for(int i = 0 ; i < 10 ; i++)
//                            {
//                                bleTxBuffer[7 + i] = buttonLabels[index].line1[i];
//                            }
//                            
//                            for(int i = 0 ; i < 3 ; i++)
//                            {
//                                bleTxBuffer[17 + i] = buttonLabels[index].line2[i];
//                            }
//                            
//                        }
//                        
//                    }
//                    else
//                    {
//                        packetLength = 20;
//                        bleTxBuffer[0] = packetLength;
//                        bleTxBuffer[1] = 1 | (DEVICE_TYPE << 6);
//                        bleTxBuffer[2] = index3;
//                        
//                        for(int i = 3 ; i < 10 ; i++)
//                        {
//                            bleTxBuffer[3 + (i - 3)] = buttonLabels[index].line2[i];
//                        }
//                        
//                        for(int i = 0 ; i < 10 ; i++)
//                        {
//                            bleTxBuffer[10 + i] = buttonLabels[index].line3[i];
//                        }
//                    }
                }
//                
//                characteristicNotify.value.val = bleTxBuffer;
//                characteristicNotify.value.len = packetLength;
//                characteristicNotify.attrHandle = CYBLE_SPOD_PRO_MODE_CHAR_HANDLE;
                
                characteristicIndicate.value.val = bleTxBuffer;
                characteristicIndicate.value.len = packetLength;
                characteristicIndicate.attrHandle = CYBLE_SPOD_PRO_MODE_CHAR_HANDLE;
                
                myPrintf("Send PRO Packet: %d ", sendProPacket);
                
                do
                {
//                    bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &characteristicNotify);
                    bleApiResult = CyBle_GattsIndication(cyBle_connHandle, &characteristicIndicate);
                    CyBle_ProcessEvents();
                }
                while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
                
                myPrintf("Success \n");
            }
            else
            {
                sendProPacket = 0;
            }
        }
    }
    
    return (void*) canComm;
}


void updateAuth(void)
{
//    write_millis();
    
    static bool passOn = 1;
    static bool passOff = 1;
    static CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    if(passkeyTimer)
    {
        if((millis() - passkeyTimer) > PASSKEY_TIMEOUT)
        {
            passkeyTimer = 0;
        }
        
//    }
//    if((millis() - passkeyTimer) < PASSKEY_TIMEOUT) {
        if(passOn) {
            writePassKey(passkey);
            passOn = 0;
            passOff = 1;
            bleDisconnect = false;
            
            pairOn = true;
        }
    } else {
        if(passOff){
            writePassKey(0);
            passOn = 1;
            passOff = 0;
            bleDisconnect = true;
            
            pairOn = false;
        }
    }
    
    if(cyBle_pendingFlashWrite != 0u)
    {
        
        // Store Bonding informtation to flash 
        apiResult = CyBle_StoreBondingData(0u);
        if ( apiResult == CYBLE_ERROR_OK)
        {
            myPrintf("Bonding data stored\r\n");
        }
        else
        {
            myPrintf ("Bonding data storing pending\r\n");
        }
    }
    
}

#define ICON_PACKET_LENGTH      32

void ProcessPacket(uint8 packetLength)//, void* params)
{
    
	machineParams *mParams = (machineParams *) paramsBLE;
//	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
    
//    static uint8_t receive_buffer[5] = {0};
    
   // int i;
    uint32 calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32 tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    
//    static uint16_t dataByte = 0;
//    static uint8_t packetNum = 1;
//    static uint8_t err = 0;
//    static uint8_t i = 0;
//    static uint8_t addr = 0;
//    static uint16_t length = 0;
    
    
    
    
    if(tempCrc == calcCrc)
    {
        //myPrintf("Packet Good...\n");
        if(bleRxBuffer[2] == 0) //CAN PACKET
        {  
//            uint16 pin = 0;
//            int index, i = 0;
//            uint8 t1,t2;
//            pin = pin | bleRxBuffer[4];
//            pin = (pin << 8) | bleRxBuffer[3];
      
            //myPrintf("Packet Good... %d\n", pin);
            
//            if(((bleRxBuffer[5] & 0x0F) == status.address) && (settingsBLE->pin == 0 || pin == settingsBLE->pin))
//            if(((bleRxBuffer[5] & 0x0F) == sourceAdrMask) && (settingsBLE->pin == 0 || pin == settingsBLE->pin))
//            if(1)//settingsBLE->pin == 0 || pin == settingsBLE->pin)
//            {
                
//                myPrintf("BLE Rec: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
//                    bleRxBuffer[0], bleRxBuffer[1], bleRxBuffer[2], bleRxBuffer[3], bleRxBuffer[4], 
//                    bleRxBuffer[5], bleRxBuffer[6], bleRxBuffer[7], bleRxBuffer[8], bleRxBuffer[9], 
//                    bleRxBuffer[10], bleRxBuffer[11], bleRxBuffer[12], bleRxBuffer[13], bleRxBuffer[14] );
                
            uint8_t addr = bleRxBuffer[5]; 
            uint8_t sw = bleRxBuffer[6]; 
            uint8_t on = bleRxBuffer[7]; 
            uint8_t bOn = bleRxBuffer[8];
            uint8_t bOff = bleRxBuffer[9];
            
            if(isPro && 
                (addr & 0xF0) == 0x80 &&    // SWITCH_PACKET
                bOff != 0 &&
                indexPos(sw) != 255)
            {
                int index = indexPos(sw) + 8 * (addr & 0x0F);
                
                switchStrobeOn[index] = bOn;
                switchStrobeOff[index] = bOff;
                
                eepromLocalArray[STROBE_OFF_OFFSET + index] = switchStrobeOff[index];
                eepromLocalArray[STROBE_ON_OFFSET + index] = switchStrobeOn[index];
            
                saveSettingRow(STROBE_OFF_OFFSET / CY_FLASH_SIZEOF_ROW);
                
                myPrintf("BLE strb sv: (%d) %x %x %x %x %x", index, addr, sw, on, bOn, bOff);
                
                if(bOff != 0 && on == 0)     // if "settings" packet, don't send on CAN bus
                {
                    myPrintf(" - skip send \n");
                    return;
                }
                
                myPrintf("\n");
            }
            
                	machineEvent evt;
                
//					evt.asPtrs[0] = (int) &bleRxBuffer[5];  
                    evt.asChars[0] = bleRxBuffer[5]; 
                    evt.asChars[1] = bleRxBuffer[6];  
                    evt.asChars[2] = bleRxBuffer[7];  
                    evt.asChars[3] = bleRxBuffer[8];  
                    evt.asChars[4] = bleRxBuffer[9];  

					mParams->mEvt = &evt;

					publish(SWITCH_STATUS_FROM_BLE_SIG, mParams);
                    
                    

//            }
        }
        else if(bleRxBuffer[2] == 1) //LED PACKET
        {
        }
        else if(bleRxBuffer[2] == 2) //SETTINGS PACKET
        {
        }
        else if(bleRxBuffer[2] == 3) //LINK PACKET
        {
        }
        else if(bleRxBuffer[2] == 4) //PAIR PACKET
        {
        }
        else if(0 && bleRxBuffer[2] == 5) // ICON PACKET
        {
        }
        
    
    }else
    {
       myPrintf("CRC Error...\n");    
    }
    
}

void ProcessTsPacket(uint16_t packetLength)//, void* params)
{
    
	machineParams *mParams = (machineParams *) paramsBLE;
//	machineEvent *mEvt = (machineEvent *) mParams->mEvt;
    
//    static uint8_t receive_buffer[5] = {0};
    
   // int i;
//    uint32 calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
//    uint32 tempCrc = 0;
//    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
//    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
//    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
//    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(isPro && !isProTempWritable)
    {
        return;
    }
    
    static uint16_t dataByte = 0;
    static uint8_t packetNum = 1;
    static uint8_t err = 0;
    static uint8_t i = 0;
    static uint8_t addr = 0;
    static uint8_t iAddr = 0;
    static uint16_t length = 0;
    
//    static uint8_t swPacketNum = 0;
//    static uint8_t swPacketNumNext = 0;
    
    static uint8_t addrCurr = 0;
    static uint8_t swRxBuf[35] = {0};
    
    
    if(1)//tempCrc == calcCrc)
    {
        //myPrintf("Packet Good...\n");
        if(bleRxBuffer[1] == 0) //TS Switch Update Packet
        { 
            addr = bleRxBuffer[2];
            
            uint8_t length = bleRxBuffer[0];
            
//            swPacketNum = bleRxBuffer[3];
            
            if(bleRxBuffer[3] == 0)
            {
                
//                if(addr == 32)
//                {
//                    fahOn = bleRxBuffer[9];   
//                    return;
//                }
                
                switchIsMomentary[addr] = bleRxBuffer[4];
                switchIsDimmable[addr] = bleRxBuffer[5];
                switchIsFlash[addr] = bleRxBuffer[6];
                switchIsStrobe[addr] = bleRxBuffer[7];
                
                if(length >= 12)    // check not old style
                {
                    switchIsLinked[addr] = bleRxBuffer[8];
                    switchIsLinked[addr] |= bleRxBuffer[9] << 8;
                    switchIsLinked[addr] |= bleRxBuffer[10] << 16;
                    switchIsLinked[addr] |= bleRxBuffer[11] << 24;
                }
                
//                switchIsLinked[addr] = bleRxBuffer[8];   // old ble linking 
                
                
                
                
//                eepromLocalArray[IS_MOMENTARY_OFFSET + addr] = switchIsMomentary[addr];
//                eepromLocalArray[IS_DIMABLE_OFFSET + addr] = switchIsDimmable[addr];
//                eepromLocalArray[IS_FLASH_OFFSET + addr] = switchIsFlash[addr];
//                eepromLocalArray[IS_STROBE_OFFSET + addr] = switchIsStrobe[addr];
                
                uint8_t lastVal = eepromLocalArray[SW_OPTIONS_OFFSET + addr];
        
                    lastVal = lastVal & ~(IS_MOMENTARY_MASK | IS_DIMABLE_MASK | IS_STROBE_MASK | IS_FLASH_MASK);//0xf0;
                
                eepromLocalArray[SW_OPTIONS_OFFSET + addr] = lastVal |
                    (switchIsMomentary[addr] ? IS_MOMENTARY_MASK : 0) |
                    (switchIsDimmable[addr] ? IS_DIMABLE_MASK : 0) |
                    (switchIsStrobe[addr] ? IS_STROBE_MASK : 0) | 
                    (switchIsFlash[addr] ? IS_FLASH_MASK : 0);
                    
                
//                eepromLocalArray[IS_LINKED_OFFSET + addr] = switchIsLinked[addr]; /// todo, fix ble linking 
                eepromLocalArray32[IS_LINKED_OFFSET/4 + addr] = switchIsLinked[addr];
                
//                saveSettingRow((IS_DIMABLE_OFFSET + addr)/CY_FLASH_SIZEOF_ROW); 
                saveSettingRow(SW_OPTIONS_OFFSET / CY_FLASH_SIZEOF_ROW);
                
                if(((SW_OPTIONS_OFFSET + addr)/CY_FLASH_SIZEOF_ROW) != ((IS_LINKED_OFFSET + addr)/CY_FLASH_SIZEOF_ROW))
                {
                    saveSettingRow((IS_LINKED_OFFSET + addr)/CY_FLASH_SIZEOF_ROW);     // if it wraps around to the next, save the second row
                }
                
            }

        }
        else if(bleRxBuffer[1] == 1) //TS Text Update Packet
        { 
            uint8_t needsParse = false;
            
            addr = bleRxBuffer[2];
            
            if(bleRxBuffer[3] == 0)
            {
                
                addrCurr = addr + 1;
                
                if(sizeOfMtu >= (34 + 3) && packetLength >= 34)
                {
                    for(i = 0 ; i < 30 ; i++)
                    {
                        swRxBuf[i] = bleRxBuffer[i + 4];
                    }
                    
                    needsParse = true;
                }
                else
                {
                    
                    for(i = 0 ; i < 15 ; i++)
                    {
                        swRxBuf[i] = bleRxBuffer[i + 4];
                    }
                }
            }
            else if(bleRxBuffer[3]== 1)
            {
                
                if(addrCurr == addr + 1)
                {
                    addrCurr = 0;
                    
                    for(i = 0 ; i < 15 ; i++)
                    {
                        swRxBuf[15 + i] = bleRxBuffer[i + 4];
                    }
                    
                    needsParse = true;
                
                }
                else
                {
                    addrCurr = 0;
                    // ignore and don't save if first half hasn't been received   
                }
                
            }

            if(needsParse)
            {
                for(i = 0 ; i < 10 ; i++)
                {
                    buttonLabels[addr].line1[i] = swRxBuf[i];
                }
                
                for(i = 0 ; i < 10 ; i++)
                {
                    buttonLabels[addr].line2[i] = swRxBuf[i + 10];
                }
                
                for(i = 0 ; i < 10 ; i++)
                {
                    buttonLabels[addr].line3[i] = swRxBuf[i + 20];
                }
              
                
                uint8 *tempArray = (uint8 *) buttonLabels;
                
                uint16_t tempPos = addr * 30;
                
                
                for(unsigned int i = 0 ; i < 30 ; i ++)
                {
                    eepromLocalArray[BUTTON_LABELS_OFFSET + tempPos + i] = tempArray[tempPos + i];
                }
                
                saveSettingRow((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW);          // save the first row
                
//                    myPrintf("row1: %d row2: %d \n", ((BUTTON_LABELS_OFFSET + tempPos)/128), ((BUTTON_LABELS_OFFSET + tempPos + 29)/128));
                
                if(((BUTTON_LABELS_OFFSET + tempPos)/CY_FLASH_SIZEOF_ROW) != ((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW))
                {
                    saveSettingRow((BUTTON_LABELS_OFFSET + tempPos + 29)/CY_FLASH_SIZEOF_ROW);     // if it wraps around to the next, save the second row
                }
            }
            
        }
        else if(bleRxBuffer[1] == 2) //ICON SELECT PACKET
        {
            addr = bleRxBuffer[2];
            
            bool iconWasOn = iconIsOn[addr];
            
            iconIsOn[addr] = bleRxBuffer[3];
            
            if(iconIsOn[addr] && packetLength > 4)
            {
                iconId[addr] = bleRxBuffer[4];
            }
            
            eepromLocalArray[IS_ICON_OFFSET + addr] = iconIsOn[addr];
            eepromLocalArray[ICON_ID_OFFSET + addr] = iconId[addr];
            saveSettingRow((IS_ICON_OFFSET + addr)/CY_FLASH_SIZEOF_ROW); 
            
            if(iconWasOn != iconIsOn[addr])
            {
            
                machineEvent evt;
    			evt.asChars[0] = 1;
    			mParams->mEvt = &evt;
    			publish(GOTO_PAGE_SIG, mParams);
            }
    
        }
        else if(bleRxBuffer[1] == 3) //ICON LOAD PACKET
        {
                           
//            myPrintf("BLE Rec: %x %x %x %x \n", 
//                    bleRxBuffer[0], bleRxBuffer[1], bleRxBuffer[2], bleRxBuffer[3] );
            
//            myPrintf("%d %d %d %d :pn: %d\n%d %d %d %d \n%d %d %d %d \n%d %d %d %d \n", 
//                    bleRxBuffer[4], bleRxBuffer[5], bleRxBuffer[6], bleRxBuffer[7],  bleRxBuffer[3], 
//                    bleRxBuffer[8], bleRxBuffer[9], bleRxBuffer[10], bleRxBuffer[11], 
//                    bleRxBuffer[12], bleRxBuffer[13], bleRxBuffer[14] , bleRxBuffer[15],
//                    bleRxBuffer[16], bleRxBuffer[17], bleRxBuffer[18] , bleRxBuffer[19]
//            );

            if(packetLength > (sizeOfMtu - 3)){
                err = true;
            }
            
            if(iAddr == bleRxBuffer[2] && bleRxBuffer[3] == packetNum)
            {
                
                
                for(i = 4 ; i < packetLength ; i++)
                {
                    iconData[dataByte] = bleRxBuffer[i];
                    dataByte++;
                    
                    if(dataByte >= length) {
                        break;
                    }
                }
                
                if(dataByte >= length) 
                {
                    
                    loadIcon(iAddr, iconData);//, 0);
                    
                    err = true;
                    
    				machineEvent evt;
    				evt.asChars[0] = 1;
    				mParams->mEvt = &evt;
    				publish(GOTO_PAGE_SIG, mParams);
                        
//                    myPrintf("Icon Loaded \n");
                }
                
                packetNum++;
                
                if(packetNum > 32)
                {
                    myPrintf("Error: icon packets out of bounds %d\n", packetNum);
                    err = true;
                }
                
            }
            else if(bleRxBuffer[3] == 0)        // first packet
            {
                clear512array();
                
                iAddr = bleRxBuffer[2];
                dataByte = 0;
                length = 511;
                
                    for(i = 4 ; i < packetLength ; i++)
                    {
                        iconData[dataByte] = bleRxBuffer[i];
                        dataByte++;
                    }
            }
            else
            {
                err = true;
            }
            
            if(err)
            {
                dataByte = 0;
                packetNum = 1;
                i = 0;
                iAddr = 0;
                length = 0;
                err = false;
            }   
        }
        
    
    }else
    {
       myPrintf("CRC Error...\n");    
    }
    
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

void processOtaPacket(void)
{
    uint32_t boardRx, boardRead;//, stkBoardRead;
    uint16_t appId, appVer, stkId, stkVer;
    uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
//    uint16_t boardId;
//    char boardRev;
    
    
    
    boardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
//    stkBoardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_CUST_ID_OFFSET);
    
    svdAppId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_ID_OFFSET);
    svdAppVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_VER_OFFSET);
    svdStkId = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_ID_OFFSET);
    svdStkVer = CY_GET_REG16(Bootloadable_MD_BASE_ADDR(0) + Bootloadable_META_APP_VER_OFFSET);
    
    if(bleRxBuffer[1] == 0)
    {
        boardRx = 0;
        
        
        for(int i = 0 ; i < 14 ; i++)
        {
            myPrintf("%x ", bleRxBuffer[i]);
        }
        
        myPrintf("\n");
        
        for(int j = 0 ; j < 4 ; j++)
        {
            boardRx = (bleRxBuffer[2 + j] << (8 * (3 - j))) | boardRx;
        }
        
        if(boardRx != boardRead)
        {
            myPrintf("Will Not Update: Incompatible hardware\n");
            
            myPrintf("Saved Hardware ID: ");
            printBoard(boardRead);
            
            myPrintf("Recieved Hardware ID: ");
            printBoard(boardRx);
            
            return;
        }
        
        myPrintf("Board# good: ");
        printBoard(boardRx);
        
        appId = (bleRxBuffer[6] << 8) | bleRxBuffer[7];
        appVer = (bleRxBuffer[8] << 8) | bleRxBuffer[9];
        stkId = (bleRxBuffer[10] << 8) | bleRxBuffer[11];
        stkVer = (bleRxBuffer[12] << 8) | bleRxBuffer[13];
        
        
        if(appId != svdAppId)
        {
            myPrintf("Will Not Update: Invalid App ID");
            
            return;
        }
        
        if(appVer < svdAppVer)
        {
            myPrintf("Will Not Update: Newer app firmware installed\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New App Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            return;
        }
        else if(appVer > svdAppVer)         
        {
//            if(stkVer < svdStkVer)
//            {
//                myPrintf("Will Not Update: Newer stack firmware installed\n");
//                myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
//                myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
//                return;
//            }
            
            myPrintf("Updating\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New App Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            
            enterBootloader();          // Should never return
            
            return;
        }
        else if(appVer == svdAppVer)
        {
            
        }
        else
        {
            myPrintf("APP VERSION ERROR\n");
            return;
        }
        
        
        if(stkId != svdStkId)
        {
            myPrintf("Will Not Update: Invalid Stack ID");
            
            return;
        }
        
        
        if(stkVer < svdStkVer)
        {
            myPrintf("Will Not Update: Newer stack firmware installed\n");
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            return;
        }
        else if(stkVer > svdStkVer)         
        {
            myPrintf("Updating\n");
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("New Stack Version: %d.%d\n", appVer / 0x100, appVer % 0x100);
            
            enterBootloader();          // Should never return
            
            return;
        }
        else if(stkVer == svdStkVer)
        {
            myPrintf("Will Not Update: App and Stack versions match installed versions\n");
            myPrintf("Current App Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
            myPrintf("Current Stack Version: %d.%d\n", svdAppVer / 0x100, svdAppVer % 0x100);
        }
        else
        {
            myPrintf("STACK VERSION ERROR\n");
        }
        
    }
    else if(bleRxBuffer[1] == 0xFF)
    {
        myPrintf("Forcing Bootloader Sequence\n");
        enterBootloader();          // Should never return
    }
    else
    {
        
        sendOtaPacket = true;
        
//        sendOtaPacket();
    }
    
    
}

void processProPacket(void)
{
    
    bool updateFlash = 0;
//    bool isPermWrite = eepromLocalArray[IS_PRO_MODE_OFFSET];
//    bool isTempWritable = 0;
    bool isPermWrite = false;
//    uint16_t swVal;
//    uint8_t index;
    
//    bleRxBuffer[0]; //length
    
    myPrintf("Pro rec: ");
    
    for(int i = 0 ; i < bleRxBuffer[0] ; i++)
    {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    
    myPrintf("\n");
    
    if(((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) // check that the app sent the correct type of packet
    {
        if((bleRxBuffer[1] & 0x01) == 1)    // read
        {
            if(bleRxBuffer[2] == 1)
            {
                sendProPacket = PRO_IN_INDEX;//(32 * 3 + 1) + 32 + 32;
            }
            else
            {
                sendProPacket = 0;
//                sendProPacket = 65 + 32 + 32;
            }
            
            return;
        }
        
        if((bleRxBuffer[1] & 0x02) > 0)    // isn't locked
        {
            isPermWrite = true;
        }
        else
        {
            isPermWrite = false;
        }
        
        if(isProWritable != isPermWrite)
        {
            isProWritable = isPermWrite;
            updateFlash = 1;
            
        }
        
        if((bleRxBuffer[1] & 0x04) > 0)    // is writable until ble disconnect
        {
            isProTempWritable = true;
        }
        else
        {
            isProTempWritable = false;
        }
        
        if(isProWritable != 0)
        {
            isProTempWritable = true;
        }
        
        if(isPro == 0)     // set into "pro" mode
        {
            isPro = 1;
            
//            writeFlash();
            updateFlash = 1;
            
            myPrintf("PRO Mode enabled!\n");
            
            
        }
        
        if((bleRxBuffer[2] & 0x02) != 0 && isProTempWritable) // update deep sleep flag
        {
            uint8_t nds = bleRxBuffer[2] & 0x01;
            if(isDeepSleepDisabled != nds)
            {
                myPrintf("Deep Sleep disabled %d \n", nds);
                isDeepSleepDisabled = nds;
                updateFlash = 1;
            }
        }
        
        
        if(updateFlash)
        {
            eepromLocalArray[IS_PRO_MODE_OFFSET] = isPro;
            eepromLocalArray[IS_PRO_WRITABLE_OFFSET] = isProWritable;
            eepromLocalArray[IS_NO_DEEP_SLEEP_OFFSET] = isDeepSleepDisabled;
            
            saveSettingRow((IS_PRO_MODE_OFFSET)/CY_FLASH_SIZEOF_ROW); 
        }
        
        
    }
    
    
}

//bool bleDisconnect = true;

void writePassKey(uint32 thisKey)
{
    
    
    
    CYBLE_GATT_ERR_CODE_T apiGattErrCode = 0;

    CYBLE_GATT_HANDLE_VALUE_PAIR_T handleValuePair;
    
    handleValuePair.value.val = (uint8 *)&thisKey;
    handleValuePair.value.len = sizeof(thisKey);
    handleValuePair.attrHandle = CYBLE_SPOD_PASSKEY_CHAR_HANDLE;
    
    while (CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_BUSY)
 	{
 	    CyBle_ProcessEvents();
 	}
    
    /* To register the service change in the Database of the GATT Server */
    apiGattErrCode = CyBle_GattsWriteAttributeValue(&handleValuePair, 0u, NULL, CYBLE_GATT_DB_LOCALLY_INITIATED);
    
    if (apiGattErrCode != 0)
    {
        myPrintf("Service Changed Attribute DB write failed: %d\n", apiGattErrCode );
    }
     else
    {
        myPrintf("Service Changed Attribute DB write success\n");
        
        if(thisKey) {
//            myPrintf("PASSKEY ON %d\n", millis());
            myPrintf("PASSKEY ON: %d \n", millis());
//            write_millis();
        } else {
            myPrintf("PASSKEY OFF: %d \n", millis());
//            write_millis();
        }
    }
}

uint8_t localName[] = "sPOD Link #00123456";

#define CYREG_SRSS_TST_DDFT_CTRL 0x40030008

void StackEventHandler( uint32 eventCode, void *eventParam )
{
//    myPrintf("b ");
    
    switch( eventCode )
    {

        case CYBLE_EVT_HOST_INVALID:
        break;

        case CYBLE_EVT_STACK_ON:
        {
            
            /* Configure the Link Layer to automatically switch PA control pin P3[2] and LNA control pin P3[3] */
                
                CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_BB_XO_CAPTRIM), 0xD0D0); 
            
            CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLESS_RF_CONFIG), 0x0331); 
            CY_SET_XTND_REG32((void CYFAR *)(CYREG_SRSS_TST_DDFT_CTRL), 0x80000302);
            
            CYBLE_GAP_BD_ADDR_T localAddr;
            
            localAddr.type = 0x00;
            
            CyBle_GetDeviceAddress(&localAddr);
            
          
            sprintf((char *)&localName[11], "%02X", DEVICE_TYPE);
            
            int index = 0;
            for(int i = 3; i > 0; i--)
            {
               myPrintf("%x ", localAddr.bdAddr[i-1]); 
               sprintf((char *)&localName[13 + index], "%02X", localAddr.bdAddr[i-1]);
               index += 2;
            }
            
            myPrintf("\n");
            
            myPrintf("%s\n", localName);
            
            CyBle_GapFixAuthPassKey(1, passkey);
            CyBle_GapSetLocalName((const char8*)&localName[0]);
            CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST );
        }
        break;

        case CYBLE_EVT_TIMEOUT:
        break;

        case CYBLE_EVT_HARDWARE_ERROR:
        break;

        case CYBLE_EVT_HCI_STATUS:
        break;

        case CYBLE_EVT_STACK_BUSY_STATUS:
        break;

        case CYBLE_EVT_PENDING_FLASH_WRITE:
        break;


        case CYBLE_EVT_GAP_AUTH_REQ:
            myPrintf("CYBLE_EVT_GAP_AUTH_REQ \n");
        break;

        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            myPrintf("CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST \n ");

        break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            {
                myPrintf("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST \n");
                
                if(bleDisconnect)
                {
                    myPrintf("Disconnecting...\n");
                    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                    break;
                }
                
                uint32 thisKey = *(uint32*)eventParam;
                
                writePassKey(thisKey);
                
                myPrintf("Passkey is: %d.\n", thisKey);
                myPrintf("Please enter the passkey on peer device.\n");
            
            
            }
        break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            myPrintf("CYBLE_EVT_GAP_AUTH_COMPLETE...\n");
            writePassKey(0);
            
            authGood = true;
            
        break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
            myPrintf("CYBLE_EVT_GAP_AUTH_FAILED...\n");
            
            authGood = false;
            
            if(bleDisconnect)
            {
                myPrintf("Disconnecting...\n");
                CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
            }
        break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            myPrintf("BLE Connected...\n");
            isConnected = 1;
        break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            myPrintf("BLE Disconnected...\n");
            isConnected = 0;
            sendNotifications = 0;
            authGood = false;
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
        break;

        case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        break;

        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
        break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        break;

        case CYBLE_EVT_GAPC_SCAN_START_STOP:
        break;

        case CYBLE_EVT_GATT_CONNECT_IND:
        break;

        case CYBLE_EVT_GATT_DISCONNECT_IND:
        break;

        case CYBLE_EVT_GATTC_ERROR_RSP:
        break;

        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BLOB_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_MULTI_RSP:
        break;

        case CYBLE_EVT_GATTC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
        break;

        case CYBLE_EVT_GATTC_INDICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
                myPrintf("CYBLE_EVT_GATTS_XCNHG_MTU_REQ:...");
                
                uint16 cyBle_mtuSize, receivedMtuSize;
                receivedMtuSize = ((CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam)->mtu;
                
                
                myPrintf("Recived Mtu Size: %d\n", receivedMtuSize);
                
                if(CYBLE_GATT_MTU > receivedMtuSize)
                {
                    cyBle_mtuSize = receivedMtuSize;
                }
                else
                {
                    cyBle_mtuSize = CYBLE_GATT_MTU;
                }
                
                sizeOfMtu = cyBle_mtuSize;
                
        break;
                

        case CYBLE_EVT_GATTS_WRITE_REQ:                        
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
          {
                CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
                
                
                
                //myPrintf("BLE Write...\n");
                
                 // Handling Notification Enable 
    			if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    	        {
                    //myPrintf("CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE...\n");
                    
    				CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
    				uint8 ccdValue[2];
    				
    	            // Extract CCCD Notification enable flag 
    	            sendNotifications = wrReqParam->handleValPair.value.val[0];
    				
    				// Write the present notification status to the local variable 
    				ccdValue[0] = sendNotifications;
    				
    				ccdValue[1] = 0x00;
    				
    				// Update CCCD handle with notification status data
    				notificationCCDHandle.attrHandle = CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    				
    				notificationCCDHandle.value.val = ccdValue;
    				
    				notificationCCDHandle.value.len = 2;
    				
    				// Report data to BLE component for sending data when read by Central device 
    				CyBle_GattsWriteAttributeValue(&notificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
    	        }
    				
    	        // Handling Write data from Client 
    	        else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CHAR_HANDLE)
    	        {
    					//myPrintf("CYBLE_SPOD_COMM_CHAR_HANDLE...\n");
                        
                    
                        uint8 * inData = wrReqParam->handleValPair.value.val;
                        int i;
                        for(i = 0; i < wrReqParam->handleValPair.value.len; i++)
                        {
                            if(bleFoundDelimiter)
                            {
                                if(blePacketLength == -1)
                                {
                                    if(inData[i] <= 32)
                                    {
                                        blePacketLength = inData[i] + 2;
                                        bleRxBuffer[bleBufferAddress] = inData[i];
                                        bleBufferAddress++;
                                    }
                                    else
                                    {
                                        bleFoundDelimiter = 0;
                                    }
                                }
                                else
                                {
                                    bleRxBuffer[bleBufferAddress] = inData[i];
                                    bleBufferAddress++;
                                    
                                    if(bleBufferAddress == blePacketLength)
                                    {
                                        //myPrintf("ProcessPacket...\n");
                                        ProcessPacket(blePacketLength);
                                        bleFoundDelimiter = 0;
                                    }
                                }
                            }
                            else
                            {
                                if(inData[i] == 0x55)
                                {
                                    bleBufferAddress = 0;
                                    blePacketLength = -1;
                                    bleFoundDelimiter = 1;
                                    bleRxBuffer[bleBufferAddress] = inData[i];
                                    bleBufferAddress++;
                                }
                            }
                        }
    	        }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_TOUCHSCREEN_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    	        {
                    //myPrintf("CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE...\n");
                    
    				CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
    				uint8 ccdValue[2];
    				
    	            // Extract CCCD Notification enable flag 
    	            sendNotifications = wrReqParam->handleValPair.value.val[0];
    				
    				// Write the present notification status to the local variable 
    				ccdValue[0] = sendNotifications;
    				
    				ccdValue[1] = 0x00;
    				
    				// Update CCCD handle with notification status data
    				notificationCCDHandle.attrHandle = CYBLE_SPOD_TOUCHSCREEN_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    				
    				notificationCCDHandle.value.val = ccdValue;
    				
    				notificationCCDHandle.value.len = 2;
    				
    				// Report data to BLE component for sending data when read by Central device 
    				CyBle_GattsWriteAttributeValue(&notificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
    	        }
    				
    	        // Handling Write data from Client 
    	        else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_TOUCHSCREEN_CHAR_HANDLE)
    	        {
    					//myPrintf("CYBLE_SPOD_COMM_CHAR_HANDLE...\n");
                        
                        uint8 * inData = wrReqParam->handleValPair.value.val;
                        int i;
                        for(i = 0; i < wrReqParam->handleValPair.value.len; i++)
                        {
                            bleRxBuffer[i] = inData[i];   
                            
//                            if(bleFoundDelimiter)
//                            {
//                                if(blePacketLength == -1)
//                                {
//                                    if(inData[i] <= 32)
//                                    {
//                                        blePacketLength = inData[i] + 2;
//                                        bleRxBuffer[bleBufferAddress] = inData[i];
//                                        bleBufferAddress++;
//                                    }
//                                    else
//                                    {
//                                        bleFoundDelimiter = 0;
//                                    }
//                                }
//                                else
//                                {
//                                    bleRxBuffer[bleBufferAddress] = inData[i];
//                                    bleBufferAddress++;
//                                    
//                                    if(bleBufferAddress == blePacketLength)
//                                    {
//                                        //myPrintf("ProcessPacket...\n");
//                                        ProcessTsPacket(blePacketLength);
//                                        bleFoundDelimiter = 0;
//                                    }
//                                }
//                            }
//                            else
//                            {
//                                if(inData[i] == 0x55)
//                                {
//                                    bleBufferAddress = 0;
//                                    blePacketLength = -1;
//                                    bleFoundDelimiter = 1;
//                                    bleRxBuffer[bleBufferAddress] = inData[i];
//                                    bleBufferAddress++;
//                                }
//                            }
                        }
                        
                        ProcessTsPacket(i);
    	        }
                else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PASSKEY_CHAR_HANDLE)
    	        {
                        passkeyTimer = millis();
                        myPrintf("Passkey timer reset %d\n", passkeyTimer);
    	        }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE)
//                else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE)
                {
                    uint8 * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processOtaPacket();
                    
//                    enterBootloader();
                }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PRO_MODE_CHAR_HANDLE)
                {
                    uint8 * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processProPacket();
                }
                    
                if (eventCode == CYBLE_EVT_GATTS_WRITE_REQ)
    			{
                    //myPrintf("CYBLE_EVT_GATTS_WRITE_REQ...\n");
    	            CyBle_GattsWriteRsp(cyBle_connHandle);
    			}
            }  //*/
        break;

        case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
        break;

        case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
        break;


        case CYBLE_EVT_GATTS_INDICATION_ENABLED:
        break;

        case CYBLE_EVT_GATTS_INDICATION_DISABLED:
        break;


        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
        break;

        case CYBLE_EVT_L2CAP_COMMAND_REJ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
        break;

        default:
        break;
    }
}

/* [] END OF FILE */
