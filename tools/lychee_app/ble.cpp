#if 0
/*
 * GAP  -- generic advertising profile
 * GATT -- generic attribute profile
 */

#include "ble.h"

#define DEVICE_TYPE 0


static uint8_t ble_address[] = "sPOD Link #00123456";
static uint8_t ble_attValuesCCCD[BLE_GATT_DB_CCCD_COUNT];

static BLEServer*         ble_server = NULL;
static BLEService*        spod_service = NULL;

static BLECharacteristic* spod_comm_char;
static BLEDescriptor*     spod_comm_desc;
static BLEDescriptor*     spod_comm_cccd_desc;

static BLECharacteristic* spod_passkey_char;
static BLEDescriptor*     spod_passkey_desc;
static BLEDescriptor*     spod_passkey_cccd_desc;

static BLECharacteristic* spod_OTA_bootloader_char;
static BLEDescriptor*     spod_OTA_bootloader_desc;
static BLEDescriptor*     spod_OTA_bootloader_cccd_desc;

static BLECharacteristic* spod_promode_char;
static BLEDescriptor*     spod_promode_desc;
static BLEDescriptor*     spod_promode_cccd_desc;

static BLECharacteristic* spod_unsecured_status_char;
static BLEDescriptor*     spod_unsecured_status_desc;
static BLEDescriptor*     spod_unsecured_cccd_desc;

static bool connected = false;
static bool oldconnected = false;


static uint8_t ble_attValues[0x73u] = {
    /* Device Name */
    (uint8_t)'s', (uint8_t)'P', (uint8_t)'O', (uint8_t)'D', (uint8_t)' ', (uint8_t)'L', (uint8_t)'i', (uint8_t)'n', (uint8_t)'k',
    (uint8_t)' ', (uint8_t)'#', (uint8_t)'0', (uint8_t)'0', (uint8_t)'1', (uint8_t)'2', (uint8_t)'3', (uint8_t)'4', (uint8_t)'5',
    (uint8_t)'6',

    /* Appearance */
    0x00u, 0x00u,

    /* Peripheral Preferred Connection Parameters */
    0x06u, 0x00u, 0x28u, 0x00u, 0x00u, 0x00u, 0xE8u, 0x03u,

    /* Service Changed */
    0x00u, 0x00u, 0x00u, 0x00u,

    /* COMM */
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* Passkey */
    0x00u, 0x00u, 0x00u, 0x00u,

    /* OTA Bootloader */
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u,

    /* Pro Mode */
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* Unsecured Status */
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,
};


static const uint8_t ble_attUuid128[][16u] = {
    /* sPOD */
    { 0x5Bu, 0x68u, 0x56u, 0x60u, 0x9Fu, 0xE9u, 0x04u, 0x84u, 0x7Bu, 0x44u, 0x0Du, 0x8Cu, 0xECu, 0xF9u, 0x3Au, 0x7Eu },
    /* COMM */
    { 0xF4u, 0xC6u, 0x6Fu, 0x64u, 0x02u, 0xECu, 0x5Bu, 0xA6u, 0x6Fu, 0x4Cu, 0x00u, 0x4Cu, 0x64u, 0x47u, 0x06u, 0xB9u },
    /* Passkey */
    { 0x86u, 0x74u, 0x26u, 0x03u, 0xEEu, 0x73u, 0x28u, 0x8Eu, 0xB7u, 0x45u, 0x5Cu, 0xF1u, 0xFBu, 0x52u, 0x9Eu, 0x1Bu },
    /* OTA Bootloader */
    { 0x75u, 0x16u, 0x74u, 0x8Eu, 0x97u, 0x60u, 0xE6u, 0xA6u, 0x9Eu, 0x43u, 0x93u, 0xFEu, 0xA5u, 0x93u, 0x89u, 0xDFu },
    /* Pro Mode */
    { 0xCBu, 0xB8u, 0xC9u, 0xCCu, 0x22u, 0x77u, 0xD9u, 0xBFu, 0xA4u, 0x43u, 0xAFu, 0x9Bu, 0xC1u, 0x25u, 0x41u, 0x02u },
    /* Unsecured Status */
    { 0xDEu, 0x6Cu, 0xFDu, 0xCBu, 0xA1u, 0xCCu, 0xE8u, 0x82u, 0x36u, 0x45u, 0x58u, 0x7Bu, 0xD9u, 0x57u, 0xBAu, 0xA0u },
};


static BLE_GATT_GEN_VAL_LEN_T ble_attValuesLen[BLE_GATT_DB_ATT_VAL_COUNT] = {
 /*0*/   { 0x0013u, (void *)&ble_attValues[0] }, /* Device Name */
 /*1*/   { 0x0002u, (void *)&ble_attValues[19] }, /* Appearance */
 /*2*/   { 0x0008u, (void *)&ble_attValues[21] }, /* Peripheral Preferred Connection Parameters */
 /*3*/   { 0x0004u, (void *)&ble_attValues[29] }, /* Service Changed */
 /*4*/   { 0x0002u, (void *)&ble_attValuesCCCD[0] }, /* Client Characteristic Configuration */
 /*5*/   { 0x0010u, (void *)&ble_attUuid128[0] }, /* sPOD UUID */
 /*6*/   { 0x0010u, (void *)&ble_attUuid128[1] }, /* COMM UUID */
 /*7*/   { 0x0014u, (void *)&ble_attValues[33] }, /* COMM */
 /*8*/   { 0x0002u, (void *)&ble_attValuesCCCD[2] }, /* Client Characteristic Configuration */
 /*9*/   { 0x0010u, (void *)&ble_attUuid128[2] }, /* Passkey UUID */
 /*10*/  { 0x0004u, (void *)&ble_attValues[53] }, /* Passkey */
 /*11*/  { 0x0002u, (void *)&ble_attValuesCCCD[4] }, /* Client Characteristic Configuration */
 /*12*/  { 0x0010u, (void *)&ble_attUuid128[3] }, /* OTA Bootloader UUID */
 /*13*/  { 0x0012u, (void *)&ble_attValues[57] }, /* OTA Bootloader */
 /*14*/  { 0x0002u, (void *)&ble_attValuesCCCD[6] }, /* Client Characteristic Configuration */
 /*15*/  { 0x0010u, (void *)&ble_attUuid128[4] }, /* Pro Mode UUID */
 /*16*/  { 0x0014u, (void *)&ble_attValues[75] }, /* Pro Mode */
 /*17*/  { 0x0002u, (void *)&ble_attValuesCCCD[8] }, /* Client Characteristic Configuration */
 /*18*/  { 0x0010u, (void *)&ble_attUuid128[5] }, /* Unsecured Status UUID */
 /*19*/  { 0x0014u, (void *)&ble_attValues[95] }, /* Unsecured Status */
 /*20*/  { 0x0002u, (void *)&ble_attValuesCCCD[10] }, /* Client Characteristic Configuration */
};

// 01 0x2800 -- BLE_UUID_PRIMARY_SERVICE           0x1800 -- BLE_UUID_GAP_SERVICE
// 02 0x2803 -- BLE_UUID_CHARACTERISTIC            0x2A00 -- BLE_UUID_CHAR_DEVICE_NAME
// 03 0x2A00 -- BLE_UUID_CHAR_DEVICE_NAME          0x0013 -- device name "sPOD Link #00123456"
// 04 0x2803 -- BLE_UUID_CHARACTERISTIC            0x2A01 -- BLE_UUID_CHAR_APPEARANCE
// 05 0x2A01 -- BLE_UUID_CHAR_APPEARANCE           0x0002 -- /* Appearance */
// 06 0x2803 -- BLE_UUID_CHARACTERISTIC            0x2A04 -- BLE_UUID_CHAR_PRFRRD_CNXN_PARAM
// 07 0x2A04 -- BLE_UUID_CHAR_PRFRRD_CNXN_PARAM    0x0008 -- /* Peripheral Preferred Connection Parameters */

// 08 0x2800 -- BLE_UUID_PRIMARY_SERVICE           0x1801 -- BLE_UUID_GATT_SERVICE
// 09 0x2803 -- BLE_UUID_CHARACTERISTIC            0x2A05 -- BLE UUID CHAR SERVICE CHANGED
// 0A 0x2A05 -- BLE_UUID_CHAR_SERVICE_CHANGED      0x0004 -- /* Service Changed */
// 0B 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCCD[0&1]

// 0C 0x2800 -- BLE_UUID_PRIMARY_SERVICE           0x0010 -- sPOD UUID -- ble_attUuid128[0]
// 0D 0x2803 -- BLE_UUID_CHARACTERISTIC            0x0010 -- COMM UUID -- ble_attUuid128[1]
// 0E 0x4764 -- COMM                               0x0014 -- /* COMM */
// 0F 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCD[2&3]
// 10 0x2803 -- BLE_UUID_CHARACTERISTIC            0x0010 -- passkey UUID
// 11 0x52FB -- Passkey                            0x0004 -- passkey value
// 12 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCD[4&5]
// 13 0x2803 -- BLE_UUID_CHARACTERISTIC            0x0010 -- OTA bootloader UUID
// 14 0x93A5 -- OTA Bootloader                     0x0012 -- OTA bootloader value
// 15 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCD[6&7]
// 16 0x2803 -- BLE_UUID_CHARACTERISTIC            0x0010 -- Pro mode UUID
// 17 0x25C1 -- Pro Mode                           0x0014 -- Pro mode value
// 18 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCD[8&9]
// 19 0x2803 -- BLE_UUID_CHARACTERISTIC            0x0010 -- unsecured status UUID
// 1A 0x57D9 -- Unsecured Status                   0x0014 -- unsecured status value
// 1B 0x2902 -- BLE_UUID_CHAR_CLIENT_CONFIG        0x0002 -- attValuesCCD[10&11]


static const BLE_GATT_DB_T ble_gattDB[0x1Bu] = {
    { 0x0001u, 0x2800u /* Primary service                     */, 0x00000001u /*                   */, 0x0007u, {{0x1800u, NULL}}                           },
    { 0x0002u, 0x2803u /* Characteristic                      */, 0x00020001u /* rd                */, 0x0003u, {{0x2A00u, NULL}}                           },
    { 0x0003u, 0x2A00u /* Device Name                         */, 0x01020001u /* rd                */, 0x0003u, {{0x0013u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[0]}} },
    { 0x0004u, 0x2803u /* Characteristic                      */, 0x00020001u /* rd                */, 0x0005u, {{0x2A01u, NULL}}                           },
    { 0x0005u, 0x2A01u /* Appearance                          */, 0x01020001u /* rd                */, 0x0005u, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[1]}} },
    { 0x0006u, 0x2803u /* Characteristic                      */, 0x00020001u /* rd                */, 0x0007u, {{0x2A04u, NULL}}                           },
    { 0x0007u, 0x2A04u /* Peripheral Preferred Connection Par */, 0x01020001u /* rd                */, 0x0007u, {{0x0008u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[2]}} },
    
    { 0x0008u, 0x2800u /* Primary service                     */, 0x00000001u /*                   */, 0x000Bu, {{0x1801u, NULL}}                           },
    { 0x0009u, 0x2803u /* Characteristic                      */, 0x00220001u /* rd,ind            */, 0x000Bu, {{0x2A05u, NULL}}                           },
    { 0x000Au, 0x2A05u /* Service Changed                     */, 0x01220001u /* rd,ind            */, 0x000Bu, {{0x0004u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[3]}} },
    { 0x000Bu, 0x2902u /* Client Characteristic Configuration */, 0x010A0701u /* rd,wr             */, 0x000Bu, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[4]}} },
    
    { 0x000Cu, 0x2800u /* Primary service                     */, 0x08000001u /*                   */, 0x001Bu, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[5]}} },
    { 0x000Du, 0x2803u /* Characteristic                      */, 0x003E0001u /* rd,wr,wwr,ntf,ind */, 0x000Fu, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[6]}} },
    { 0x000Eu, 0x4764u /* COMM                                */, 0x093E0707u /* rd,wr,wwr,ntf,ind */, 0x000Fu, {{0x0014u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[7]}} },
    { 0x000Fu, 0x2902u /* Client Characteristic Configuration */, 0x010A0101u /* rd,wr             */, 0x000Fu, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[8]}} },
 
    { 0x0010u, 0x2803u /* Characteristic                      */, 0x000E0001u /* rd,wr,wwr         */, 0x0012u, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[9]}} },
    { 0x0011u, 0x52FBu /* Passkey                             */, 0x090E0101u /* rd,wr,wwr         */, 0x0012u, {{0x0004u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[10]}} },
    { 0x0012u, 0x2902u /* Client Characteristic Configuration */, 0x010A0701u /* rd,wr             */, 0x0012u, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[11]}} },
 
    { 0x0013u, 0x2803u /* Characteristic                      */, 0x002E0001u /* rd,wr,wwr,ind     */, 0x0015u, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[12]}} },
    { 0x0014u, 0x93A5u /* OTA Bootloader                      */, 0x092E0707u /* rd,wr,wwr,ind     */, 0x0015u, {{0x0012u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[13]}} },
    { 0x0015u, 0x2902u /* Client Characteristic Configuration */, 0x010A0701u /* rd,wr             */, 0x0015u, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[14]}} },
 
    { 0x0016u, 0x2803u /* Characteristic                      */, 0x003E0001u /* rd,wr,wwr,ntf,ind */, 0x0018u, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[15]}} },
    { 0x0017u, 0x25C1u /* Pro Mode                            */, 0x093E0707u /* rd,wr,wwr,ntf,ind */, 0x0018u, {{0x0014u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[16]}} },
    { 0x0018u, 0x2902u /* Client Characteristic Configuration */, 0x010A0701u /* rd,wr             */, 0x0018u, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[17]}} },
 
    { 0x0019u, 0x2803u /* Characteristic                      */, 0x003E0001u /* rd,wr,wwr,ntf,ind */, 0x001Bu, {{0x0010u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[18]}} },
    { 0x001Au, 0x57D9u /* Unsecured Status                    */, 0x093E0101u /* rd,wr,wwr,ntf,ind */, 0x001Bu, {{0x0014u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[19]}} },
    { 0x001Bu, 0x2902u /* Client Characteristic Configuration */, 0x010A0101u /* rd,wr             */, 0x001Bu, {{0x0002u, (BLE_GATT_GEN_VAL_LEN_T *)&ble_attValuesLen[20]}} },
};


class MyServerCallbacks: public BLEServerCallbacks {
   void onConnect(BLEServer* server) {
      Serial.printf("Client connected!\n");
      connected = true;
    };

    void onDisconnect(BLEServer* server) {
      Serial.printf("Client disconnected\n");
      connected = false;
    }

    // void onMtuChanged(BLEServer* server, esp_ble_gatts_cb_param_t* param) {
    //    
    //}
};


class DescCallbacks: public BLEDescriptorCallbacks {
   void onRead(BLEDescriptor* desc) {
      Serial.printf("DESC onRead()\n");
      if (desc == spod_comm_desc) {
        Serial.printf("spod_comm_desc\n");
        return;
      }
      if (desc == spod_comm_cccd_desc) {
        Serial.printf("spod_comm_cccd_desc\n");
        return;
      }
      if (desc == spod_passkey_desc) {
        Serial.printf("spod_passkey_desc\n");
        return;
      }
      if (desc == spod_passkey_cccd_desc) {
        Serial.printf("spod_passkey_cccd_desc\n");
        return;
      }
      if (desc == spod_OTA_bootloader_desc) {
        Serial.printf("spod_OTA_bootloader_desc\n");
        return;
      }
      if (desc == spod_OTA_bootloader_cccd_desc) {
        Serial.printf("spod_OTA_bootloader_cccd_desc\n");
        return;
      }
      if (desc == spod_promode_desc) {
        Serial.printf("spod_promode_desc\n");
        return;
      }
      if (desc == spod_promode_cccd_desc) {
        Serial.printf("spod_promode_cccd_desc\n");
        return;
      }
      if (desc == spod_unsecured_status_desc) {
        Serial.printf("spod_unsecured_status_desc\n");
        return;
      }
      if (desc == spod_unsecured_cccd_desc) {
        Serial.printf("spod_unsecured_cccd_desc\n");
        return;
      }
    };

    void onWrite(BLEDescriptor* desc) {
      Serial.printf("DESC onWrite()\n");
    }
};


static void build_ble_address()
{
    Serial.printf("%d\n", __LINE__); Serial.flush();
    const uint8_t* addr = esp_bt_dev_get_address();
    //BLEAddress address = BLEDevice::getAddress();
    //Serial.printf("%d\n", __LINE__); Serial.flush();
    //esp_bd_addr_t* addr = address.getNative();
    Serial.printf("%d -- addr %p\n", __LINE__, addr); Serial.flush();
    sprintf((char *)&ble_address[11], "%02X%02X%02X%02X", DEVICE_TYPE, addr[3], addr[4], addr[5]);
    Serial.printf("%d\n", __LINE__); Serial.flush();
}


#if 0
#define    ESP_GATT_PERM_READ                  (1 << 0)   /* bit 0 -  0x0001 */    /* relate to BTA_GATT_PERM_READ in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_READ_ENCRYPTED        (1 << 1)   /* bit 1 -  0x0002 */    /* relate to BTA_GATT_PERM_READ_ENCRYPTED in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_READ_ENC_MITM         (1 << 2)   /* bit 2 -  0x0004 */    /* relate to BTA_GATT_PERM_READ_ENC_MITM in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_WRITE                 (1 << 4)   /* bit 4 -  0x0010 */    /* relate to BTA_GATT_PERM_WRITE in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_WRITE_ENCRYPTED       (1 << 5)   /* bit 5 -  0x0020 */    /* relate to BTA_GATT_PERM_WRITE_ENCRYPTED in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_WRITE_ENC_MITM        (1 << 6)   /* bit 6 -  0x0040 */    /* relate to BTA_GATT_PERM_WRITE_ENC_MITM in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_WRITE_SIGNED          (1 << 7)   /* bit 7 -  0x0080 */    /* relate to BTA_GATT_PERM_WRITE_SIGNED in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_WRITE_SIGNED_MITM     (1 << 8)   /* bit 8 -  0x0100 */    /* relate to BTA_GATT_PERM_WRITE_SIGNED_MITM in bta/bta_gatt_api.h */
#define    ESP_GATT_PERM_READ_AUTHORIZATION    (1 << 9)   /* bit 9 -  0x0200 */
#define    ESP_GATT_PERM_WRITE_AUTHORIZATION   (1 << 10)  /* bit 10 - 0x0400 */
#define    ESP_GATT_PERM_ENCRYPT_KEY_SIZE(keysize)     (((keysize - 6) & 0xF) << 12)    /* bit 12:15 - 0xF000 */
typedef uint16_t esp_gatt_perm_t;

BLEDescriptor::setValue(std::string value);
BLEDescriptor::setValue(uint8_t* data, size_t length);
void BLEDescriptor::setAccessPermissions(esp_gatt_perm_t perm);
void BLEDescriptor::setCallbacks(BLEDescriptorCallbacks* pCallback);

class BLEDescriptorCallbacks {
public:
	virtual ~BLEDescriptorCallbacks();
	virtual void onRead(BLEDescriptor* pDescriptor);
	virtual void onWrite(BLEDescriptor* pDescriptor);
};
#endif


void ble_init()
{
    static uint8_t cccd_value[2] = {0,0};

    Serial.printf("%d\n", __LINE__); Serial.flush();
    std::string address = (char *) ble_address;
    Serial.printf("%d\n", __LINE__); Serial.flush();
    //build_ble_address();
    Serial.printf("%d\n", __LINE__); Serial.flush();
    BLEDevice::init(address);
    Serial.printf("%d\n", __LINE__); Serial.flush();

    ble_server = BLEDevice::createServer();
    ble_server->setCallbacks(new MyServerCallbacks());

    DescCallbacks* cbs = new DescCallbacks();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // three services
    spod_service = ble_server->createService(BLEUUID((uint8_t*)ble_attUuid128[0], sizeof(ble_attUuid128[0]), false));

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // sPOD specific characteristics and descriptors
    spod_comm_char = spod_service->createCharacteristic(
        BLEUUID((uint8_t*)ble_attUuid128[1], sizeof(ble_attUuid128[1]), false),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);

    spod_comm_desc = new BLEDescriptor(BLEUUID((uint16_t) 0x4764));
    spod_comm_desc->setValue((uint8_t*)ble_attValuesLen[4].attGenericVal, ble_attValuesLen[4].actualLength);
    spod_comm_char->addDescriptor(spod_comm_desc);

    spod_comm_cccd_desc = new BLEDescriptor(BLEUUID((uint16_t) BLE_UUID_CHAR_PRFRRD_CNXN_PARAM));
    spod_comm_cccd_desc->setValue(cccd_value, sizeof(cccd_value));
    spod_comm_char->addDescriptor(spod_comm_cccd_desc);

    spod_passkey_char = spod_service->createCharacteristic(
        BLEUUID((uint8_t*)ble_attUuid128[2], sizeof(ble_attUuid128[2]), false),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE);
    spod_passkey_desc = new BLEDescriptor(BLEUUID((uint16_t) 0x52FB));
    spod_passkey_desc->setValue((uint8_t*)ble_attValuesLen[5].attGenericVal, ble_attValuesLen[5].actualLength);
    spod_passkey_char->addDescriptor(spod_passkey_desc);

    spod_passkey_cccd_desc = new BLEDescriptor(BLEUUID((uint16_t) BLE_UUID_CHAR_PRFRRD_CNXN_PARAM));
    spod_passkey_cccd_desc->setValue(cccd_value, sizeof(cccd_value));
    spod_passkey_char->addDescriptor(spod_passkey_cccd_desc);

    spod_OTA_bootloader_char = spod_service->createCharacteristic(
        BLEUUID((uint8_t*)ble_attUuid128[3], sizeof(ble_attUuid128[3]), false),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);
    spod_OTA_bootloader_desc = new BLEDescriptor(BLEUUID((uint16_t) 0x93A5));
    spod_OTA_bootloader_desc->setValue((uint8_t*)ble_attValuesLen[6].attGenericVal, ble_attValuesLen[6].actualLength);
    spod_OTA_bootloader_char->addDescriptor(spod_OTA_bootloader_desc);

    spod_OTA_bootloader_cccd_desc = new BLEDescriptor(BLEUUID((uint16_t) BLE_UUID_CHAR_PRFRRD_CNXN_PARAM));
    spod_OTA_bootloader_cccd_desc->setValue(cccd_value, sizeof(cccd_value));
    spod_OTA_bootloader_char->addDescriptor(spod_OTA_bootloader_cccd_desc);

    spod_promode_char = spod_service->createCharacteristic(
        BLEUUID((uint8_t*)ble_attUuid128[4], sizeof(ble_attUuid128[4]), false),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);
    spod_promode_desc = new BLEDescriptor(BLEUUID((uint16_t) 0x25C1));
    spod_promode_desc->setValue((uint8_t*)ble_attValuesLen[7].attGenericVal, ble_attValuesLen[7].actualLength);
    spod_promode_desc->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    spod_promode_char->addDescriptor(spod_promode_desc);

    spod_promode_cccd_desc = new BLEDescriptor(BLEUUID((uint16_t) BLE_UUID_CHAR_PRFRRD_CNXN_PARAM));
    spod_promode_cccd_desc->setValue(cccd_value, sizeof(cccd_value));
    spod_promode_cccd_desc->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    spod_promode_char->addDescriptor(spod_promode_cccd_desc);

    spod_unsecured_status_char = spod_service->createCharacteristic(
        BLEUUID((uint8_t*)ble_attUuid128[5], sizeof(ble_attUuid128[5]), false),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);
    spod_unsecured_status_desc = new BLEDescriptor(BLEUUID((uint16_t) 0x57D9));
    spod_unsecured_status_desc->setValue((uint8_t*)ble_attValuesLen[8].attGenericVal, ble_attValuesLen[8].actualLength);
    spod_unsecured_status_desc->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    spod_unsecured_status_char->addDescriptor(spod_unsecured_status_desc);

    spod_unsecured_cccd_desc = new BLEDescriptor(BLEUUID((uint16_t) BLE_UUID_CHAR_PRFRRD_CNXN_PARAM));
    spod_unsecured_cccd_desc->setValue(cccd_value, sizeof(cccd_value));
    spod_unsecured_cccd_desc->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    spod_unsecured_status_char->addDescriptor(spod_unsecured_cccd_desc);
 
    spod_comm_desc->setCallbacks(cbs);
    spod_comm_cccd_desc->setCallbacks(cbs);
    spod_passkey_desc->setCallbacks(cbs);
    spod_passkey_cccd_desc->setCallbacks(cbs);
    spod_OTA_bootloader_desc->setCallbacks(cbs);
    spod_OTA_bootloader_cccd_desc->setCallbacks(cbs);
    spod_promode_desc->setCallbacks(cbs);
    spod_promode_cccd_desc->setCallbacks(cbs);
    spod_unsecured_status_desc->setCallbacks(cbs);
    spod_unsecured_cccd_desc->setCallbacks(cbs);

    // Start the service
    spod_service->start();

    // Start advertising
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(BLEUUID((uint8_t*)ble_attUuid128[0], sizeof(ble_attUuid128[0]), false));
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();

    Serial.println("ble_init() complete!!!");
    Serial.flush();    
}
#endif
