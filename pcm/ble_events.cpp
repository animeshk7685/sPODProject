/* ========================================
 *
 * Copyright Star Technologies, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF STAR TECHNOLOGIES.
 *
 *  Author: Luke Williams
 *
 *  Version 1.00    12-16-2017
 *  Version 1.01    12-19-2017
 *  Version 1.02    12-26-2017
 *  Version 1.03    01-11-2018
 *  Version 1.04    01-11-2018
 *  Version 1.05    01-15-2018
 *  Version 1.06    01-15-2018
 *
 * ========================================
*/

#include "spod.h"
#include "crc32.h"
#include "main.h"

#if 0
uint8_t bleFoundDelimiter = 0;
uint8_t bleBufferAddress = 0;
int blePacketLength = 0;

bool bleDisconnect = true;
#endif


#if 0
#define IS_MOMENTARY_MASK       0x01
#define IS_DIMABLE_MASK         0x02
#define IS_STROBE_MASK          0x04   
#define IS_FLASH_MASK           0x08

static void ProcessPacket(uint8_t packetLength)
{
    myPrintf("ProcessPacket(): ");
    for(int i = 0 ; i < packetLength ; i++) {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    myPrintf("\n");
    
    uint32_t calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32_t tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(tempCrc == calcCrc)
    {
        //myPrintf("Packet Good...\n");
        if(bleRxBuffer[2] == 0) //CAN PACKET
        {
            int index = (bleRxBuffer[5] & 0x0F) * 8 + canToI(bleRxBuffer[6]);
            
            if((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255 && bleRxBuffer[7] > 0) {
            
                status.pro.lastRecVals[index].dimVal = bleRxBuffer[7];
                status.pro.lastRecVals[index].onVal = bleRxBuffer[8];
                status.pro.lastRecVals[index].offVal = bleRxBuffer[9];
            }
            
            if((bleRxBuffer[5] & 0x0F) == status.address)
            {
                if((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255)
                {
                    uint8_t i = canToI(bleRxBuffer[6]);
                    
                    status.out[i].outCmd = bleRxBuffer[7];
                    
                    if(status.out[i].outCmd == 0xFE)
                    {
                        status.out[i].outCmd = 0xFF;
                    }
                    
                    if(status.pro.isEnabled && status.out[i].outCmd != 0)
                    {
                        status.pro.lastSwVal[i] = status.out[i].outCmd;
                    }
                    
                    status.out[i].blinkOn = bleRxBuffer[8] * FLASH_SPEED; 
                    status.out[i].blinkOff = bleRxBuffer[9] * FLASH_SPEED;
                    
                    isAwake = 1;
                    bleAliveTimer = millis();
                }
                else if(status.pro.isEnabled && (bleRxBuffer[5] & 0xF0) == PRO_PACKET && 
                        (bleRxBuffer[6] == 0 || canToI(bleRxBuffer[6]) != 255))
                {
                    uint8_t i = canToI(bleRxBuffer[6]);
                    
                    if(bleRxBuffer[6] != 0)
                    {
                        status.pro.isInputEnabled[i] = (bleRxBuffer[7] & INPUT_EN_CAN_MASK) ? true : false;
                        status.pro.isInputLockout[i] = (bleRxBuffer[7] & INPUT_LCK_CAN_MASK) ? true : false;
                        
                        uint8_t linkPacketIndex = 0;
                        
                        if((bleRxBuffer[7] & 0xC0) == INPUT_LINK_CAN_MASK)
                        {
                            linkPacketIndex = 1;
                        }
                        
                        if(bleRxBuffer[7] & LINKS_FLAG_MASK)
                        {
                            uint32_t linkHalf = (bleRxBuffer[8] << 8) | bleRxBuffer[9];
                            
                            if(linkPacketIndex == 0)
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0xffff0000) | linkHalf;
                            }
                            else
                            {
                                status.pro.links[index] = (status.pro.links[index] & 0x0000ffff) | (linkHalf << 16);
                            }
                        }
                    }
                    
                    writeFlash();
                    
                    pushCanFifo(&bleRxBuffer[5]);   // just transmit through rather than try to reassemble/index...
                    
                    needsInputsUpdate = true;
                }
            }
            else        // dump onto CAN
            {
                pushCanFifo(&bleRxBuffer[5]);       // move to canBroadcast to avoid possibility of contention
            }
        }
        else if(bleRxBuffer[2] == 1) //LED PACKET
        {
        }
        else if(bleRxBuffer[2] == 2) //SETTINGS PACKET
        {
            if((bleRxBuffer[3] & 0x02) != 0)
            {
                sendSettingsPacket = true;
            }
            else
            {
                status.isDeepSleepEnabled = bleRxBuffer[3] & 0x01;
                
                writeFlash();  
            }
        }
        else if(bleRxBuffer[2] == 3) //LINK PACKET
        {
        }
        else if(bleRxBuffer[2] == 4) //PAIR PACKET
        {
        }
        else if(bleRxBuffer[2] == 5) //altSettings
        {
        }
        else if(bleRxBuffer[2] == 8) //SWITCH SETTINGS PACKET
        {
            printByteArray("switchSettingsPacket():", &bleRxBuffer[0], packetLength);
            
            uint8_t swIndex = bleRxBuffer[3];
            if(swIndex > 32)
                return;
            
            if(bleRxBuffer[4] != 0)     // for versioning
                return;
            
            uint8_t srcAddr = swIndex / 8;
            uint8_t swAddr = swIndex % 8;
            uint8_t isDim = bleRxBuffer[6] & (IS_DIMABLE_MASK);
            
            if(isDim && bleRxBuffer[5] > 0 && bleRxBuffer[5] < 0xFE)
            {
                status.pro.lastRecVals[swIndex].dimVal = bleRxBuffer[5];
            }
            
            uint8_t isBlink = bleRxBuffer[6] & (IS_FLASH_MASK | IS_STROBE_MASK);
            
            if(isBlink && bleRxBuffer[7] != 0 && bleRxBuffer[8] != 0)
            {
                status.pro.lastRecVals[swIndex].onVal = bleRxBuffer[7] * FLASH_SPEED;
                status.pro.lastRecVals[swIndex].offVal = bleRxBuffer[8] * FLASH_SPEED;
                
                if(srcAddr == status.address)
                {
                    status.out[swAddr].blinkOn = bleRxBuffer[7] * FLASH_SPEED;
                    status.out[swAddr].blinkOff = bleRxBuffer[8] * FLASH_SPEED;
                }
            }
            
            status.pro.links[swIndex] = bleRxBuffer[9];
            status.pro.links[swIndex] |= bleRxBuffer[10] << 8;
            status.pro.links[swIndex] |= bleRxBuffer[11] << 16;
            status.pro.links[swIndex] |= bleRxBuffer[12] << 24;
        }
    }
    else
    {
       myPrintf("CRC Error... %d/%d\n", calcCrc, tempCrc);    
    }
}


static void processOtaPacket(void)
{
#if 0
    uint32_t boardRx, boardRead;
    uint16_t appId, appVer, stkId, stkVer;
    uint16_t svdAppId, svdAppVer, svdStkId, svdStkVer;
   
    boardRead = CY_GET_REG32(Bootloadable_MD_BASE_ADDR(1) + Bootloadable_META_APP_CUST_ID_OFFSET);
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
    }
#endif
}

static void processProPacket(void)
{
    bool updateFlash = 0;
    uint8_t index;
    
    myPrintf("processProPacket(): ");
    for(int i = 0 ; i < bleRxBuffer[0] ; i++)
    {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    myPrintf("\n");
    
    if(((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) // check that the app sent the correct type of packet
    {   
        if((bleRxBuffer[1] & 0x01) != 0)
        {
            // read 
            sendProPacket = PRO_NUM_SYNC_PACKETS;
            globalUpdateProCanPackets = true;        // to update inputs settings
            return;         //ignore the rest of the packet
        }
        
        if(status.pro.isEnabled == 0)     // set into "pro" mode
        {
            status.pro.isEnabled = 1;
            updateFlash = 1;
            myPrintf("PRO Mode enabled!\n");
        }
        
        if((bleRxBuffer[1] & 0x02) != 0)
        {
            if(status.pro.isWritable != true)
            {
                status.pro.isWritable = true;
                updateFlash = true;
            }
        }
        else
        {
            if(status.pro.isWritable != false)
            {
                status.pro.isWritable = false;
                updateFlash = true;
            }
        }
        
        if((bleRxBuffer[1] & 0x04) != 0)
        {
        }
        
        uint8_t len = bleRxBuffer[0];
        
        if(len <= 3)
        {
            if((bleRxBuffer[2] & 0x02) != 0) // update deep sleep flag
            {
                uint8_t nds = bleRxBuffer[2] & 0x01;
                if(status.pro.isDisableSleep != nds)
                {
                    myPrintf("Deep Sleep disabled %d \n", nds);
                    status.pro.isDisableSleep = nds;
                    updateFlash = 1;
                }
            }
            
            if((bleRxBuffer[2] & 0x08) != 0) // update input linking flag
            {
                uint8_t il = (bleRxBuffer[2]) & 0x04 ? 1 : 0;
                if(status.pro.isInputLinking != il)
                {
                    myPrintf("Input linking enabled %d \n", il);
                    status.pro.isInputLinking = il;
                    updateFlash = 1;
                }
            }
            
            if(updateFlash)
            {
                writeFlash();
            }
            
            return;
        }
        else if(len >= 8 && bleRxBuffer[2] == 0x00)
        {
            uint8_t addr = bleRxBuffer[3] >> 3;
        
            if(addr == status.address)
            {
                updateFlash = 1;
                
                index = bleRxBuffer[3] & 0x07;
                
                status.pro.alwaysOn[index] = ((bleRxBuffer[4] & 0x10) != 0) ? 1 : 0;
                status.pro.isIgnCtrl[index] = ((bleRxBuffer[4] & 0x08) != 0) ? 1 : 0;
                status.pro.isLockout[index] = ((bleRxBuffer[4] & 0x04) != 0) ? 1 : 0;
                status.pro.isInputLatch[index] = ((bleRxBuffer[4] & 0x02) != 0) ? 1 : 0;
                status.pro.isCurrentRestart[index] = ((bleRxBuffer[4] & 0x01) != 0) ? 1 : 0;
                
                if(status.pro.alwaysOn[index])
                {
                    status.out[index].outCmd = 0xFF;
                }
                
                status.pro.timers[index] = (bleRxBuffer[5] << 8) | bleRxBuffer[6];
                
                if(status.pro.curTime[index] > 0)
                {
                    status.pro.curTime[index] = millis(); // reset timer if running
                }
                
                status.pro.currentLimit[index] = bleRxBuffer[7];
                
                if(status.pro.currentLimit[index] == 0) // error...
                {
                    status.pro.currentLimit[index] = 30;
                }
                
                if(len >= 13)
                {
                    status.pro.isInputEnabled[index] = (bleRxBuffer[8] & INPUT_EN_CAN_MASK) ? true : false;
                    status.pro.isInputLockout[index] = (bleRxBuffer[8] & INPUT_LCK_CAN_MASK) ? true : false;
                    
                    if(bleRxBuffer[8] & LINKS_FLAG_MASK)
                    {
                        status.pro.links[index] = bleRxBuffer[9];
                        status.pro.links[index] |= bleRxBuffer[10] << 8;
                        status.pro.links[index] |= bleRxBuffer[11] << 16;
                        status.pro.links[index] |= bleRxBuffer[12] << 24;
                    }
                }
            }
        }
        
        if(updateFlash)
        {
            writeFlash();
        }
    }
}

enum securityPacketType {SEC_READ_STATUS, SEC_SET_MODE, SEC_WRITE_PASSKEY};

static void processSecurityPacket(void)
{
    uint8_t packetLength = bleRxBuffer[0];
    
    myPrintf("processSecurityPacket(): ");
    for(int i = 0 ; i < packetLength ; i++) {
        myPrintf("%x ", bleRxBuffer[i]);
    }
    myPrintf("\n");
    
    uint32_t calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    
    uint32_t tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if(tempCrc == calcCrc)
    {
        if(bleRxBuffer[1] == SEC_READ_STATUS)
        {
            sendSecPacket = true;
        }
        
        if (authGood)
        {
            if(bleRxBuffer[1] == SEC_SET_MODE)
            {
                if((bleRxBuffer[2] & 0x01) == 0)
                {
                    setSecurityLevel(SECURED);
                }
                else
                {
                    setSecurityLevel(UNSECURED);
                }
            }
        }
        else
        {
            if(status.isUnsecured)
            {
                // listen for passkey -> set authGood
                
                if(bleRxBuffer[1] == SEC_WRITE_PASSKEY)
                {
                    uint32_t readPasskey = 0;
                    
                    readPasskey = readPasskey | bleRxBuffer[5];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[4];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[3];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[2];
                    
                    myPrintf("try passkey: %x/%x - ", readPasskey, passkey);
                    
                    if(readPasskey == passkey)
                    {
                        authGood = true;
                        myPrintf("accepted!!\n");
                    }
                    else
                    {
                        myPrintf("failed...\n");
                    }
                    
                    sendSecPacket = true;
                }
            }
        }
    }
    else
    {
        myPrintf(" -> crc bad\n");
    }
}
#endif


void StackEventHandler( uint32_t eventCode, void *eventParam )
{
#if 0
    switch( eventCode )
    {
        case CYBLE_EVT_STACK_ON:
        {
//            myPrintf("CYBLE_EVT_STACK_ON:...\n");
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
            
//            if(status.isUnsecured)
//            {
//                CyBle_GapFixAuthPassKey(1, 0);
//            }
//            else
//            {
                CyBle_GapFixAuthPassKey(1, passkey);
//            }
            CyBle_GapSetLocalName((const char8*)&localName[0]);
            CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST );
        }
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
                
                uint32_t thisKey = *(uint32_t*)eventParam;
                
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
//            myPrintf("CYBLE_EVT_GAP_DEVICE_CONNECTED:...\n");
            myPrintf("BLE Connected...\n");
        break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
//            myPrintf("CYBLE_EVT_GAP_DEVICE_DISCONNECTED:...\n");
            myPrintf("BLE Disconnected...\n");
            sendNotifications = 0;
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
//            myPrintf("CYBLE_EVT_GAP_ENCRYPT_CHANGE:...\n");
        break;

        case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
//            myPrintf("CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:...\n");
        break;

        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
//            myPrintf("CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:...\n");
        break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
//            myPrintf("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:...\n");
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        break;

        case CYBLE_EVT_GATTS_WRITE_REQ:                        
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
          {
                CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
                
                lastActivityMs = millis();
//                myPrintf("BLE Write...\n");
                
                // Handling Notification Enable 
    			if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    	        {
                    //myPrintf("CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE...\n");
                    
    				CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
    				uint8_t ccdValue[2];
    				
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
                        
                        uint8_t * inData = wrReqParam->handleValPair.value.val;
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
                else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PASSKEY_CHAR_HANDLE)
    	        {
                        passkeyTimer = millis();
                        myPrintf("Passkey timer reset %d\n", passkeyTimer);
    	        }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_OTA_BOOTLOADER_CHAR_HANDLE)
                {
                    uint8_t * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processOtaPacket();
                }
                else if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PRO_MODE_CHAR_HANDLE)
                {
                    uint8_t * inData = wrReqParam->handleValPair.value.val;
                    
                    for(int i = 0 ; i < wrReqParam->handleValPair.value.len ; i++)
                    {
                        bleRxBuffer[i] = inData[i];
                    }
                    
                    processProPacket();
                }
                    
            }
        break;

        default:
        break;
    }
#endif
}

/* [] END OF FILE */


// TODO: DELETE WHEN HAPPY WITH BLE
// For reference only, moved from ble.cpp because I'm tried of looking at them...

#if 0
    void onConnect(NimBLEServer* pServer) {
        LOGD(TAG, "Client connected");
        LOGD(TAG, "Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };
    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        LOGD(TAG, "Client address: %s\n"
            NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer* pServer) {
        LOGD(TAG, "Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
    };
#endif


#if 0
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
    lastActivityMs = millis();
    
    // Handling Notification Enable 
    if(authGood && wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
    {
        CYBLE_GATT_HANDLE_VALUE_PAIR_T    notificationCCDHandle;
        
        // Extract CCCD Notification enable flag 
        sendNotifications = wrReqParam->handleValPair.value.val[0];
        
        // Write the present notification status to the local variable 
        uint8_t ccdValue[2] = {sendNotifications, 0x00};
        // Update CCCD handle with notification status data
        notificationCCDHandle.attrHandle = CYBLE_SPOD_COMM_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
        notificationCCDHandle.value.val = ccdValue;
        notificationCCDHandle.value.len = 2;
        
        // Report data to BLE component for sending data when read by Central device 
        CyBle_GattsWriteAttributeValue(&notificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
    }
    else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPOD_PASSKEY_CHAR_HANDLE)
    {
            passkeyTimer = millis();
            LOGD(TAG, "Passkey timer reset %d\r\n", passkeyTimer);
    }
#endif
