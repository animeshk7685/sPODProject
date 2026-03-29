#include "spod.h"
#include "crc16.h"
#include "crc32.h"
#include "main.h"
#include "ota.h"
#include "pkt_process.h"
#include "status.h"


static const char* TAG = "BLE";

#define NIMBLE_SPROPS (NIMBLE_PROPERTY::READ        |\
                       NIMBLE_PROPERTY::READ_ENC    |\
                       NIMBLE_PROPERTY::READ_AUTHEN |\
                       NIMBLE_PROPERTY::WRITE       |\
                       NIMBLE_PROPERTY::NOTIFY      |\
                       NIMBLE_PROPERTY::INDICATE)
#define NIMBLE_PROPS  (NIMBLE_PROPERTY::READ        |\
                       NIMBLE_PROPERTY::WRITE       |\
                       NIMBLE_PROPERTY::NOTIFY      |\
                       NIMBLE_PROPERTY::INDICATE)

int blePacketLength = 0;
bool bleDisconnect = true;
static uint32_t ota_length;
static bool proxy_ota_active;

static uint8_t ble_address[] = "sPOD Link #00123456";
static uint8_t ble_attValuesCCCD[BLE_GATT_DB_CCCD_COUNT];

static BLEServer*         ble_server = NULL;
static BLEService*        spod_service = NULL;

static BLECharacteristic* comm_char;
static BLEDescriptor*     comm_desc;

static BLECharacteristic* passkey_char;
static BLEDescriptor*     passkey_desc;

static BLECharacteristic* promode_char;
static BLEDescriptor*     promode_desc;

static BLECharacteristic* unsecured_char;
static BLEDescriptor*     unsecured_desc;

static BLECharacteristic* ota_ctrl_char;
static BLEDescriptor*     ota_ctrl_desc;

static BLECharacteristic* ota_data_char;
static BLEDescriptor*     ota_data_desc;


static uint8_t ble_attValues[0x73u+0x80/*+0x14+0x12*/] = {
    /* Device Name */
/*0*/(uint8_t)'s', (uint8_t)'P', (uint8_t)'O', (uint8_t)'D', (uint8_t)' ', (uint8_t)'L', (uint8_t)'i', (uint8_t)'n', (uint8_t)'k',
    (uint8_t)' ', (uint8_t)'#', (uint8_t)'0', (uint8_t)'0', (uint8_t)'1', (uint8_t)'2', (uint8_t)'3', (uint8_t)'4', (uint8_t)'5',
    (uint8_t)'6',

    /* Appearance */
/*19*/0x00u, 0x00u,

    /* Peripheral Preferred Connection Parameters */
/*21*/0x06u, 0x00u, 0x28u, 0x00u, 0x00u, 0x00u, 0xE8u, 0x03u,

    /* Service Changed */
/*29*/0x00u, 0x00u, 0x00u, 0x00u,

    /* COMM */
/*33*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* Passkey */
/*53*/0x00u, 0x00u, 0x00u, 0x00u,

    /* OTA Control */
/*57*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u,

    /* Pro Mode */
/*75*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* Unsecured Status */
/*95*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* OTA-DATA */
/*115*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
#if 0
    /* PROXY-COMM */
/*243*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u,

    /* PROXY OTA Bootloader */
/*263*/0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u,
#endif
};


static const uint8_t ble_attUuid128[][16u] = {
/*0-sPOD*/     { 0x5Bu, 0x68u, 0x56u, 0x60u, 0x9Fu, 0xE9u, 0x04u, 0x84u, 0x7Bu, 0x44u, 0x0Du, 0x8Cu, 0xECu, 0xF9u, 0x3Au, 0x7Eu },
/*1-COMM*/     { 0xF4u, 0xC6u, 0x6Fu, 0x64u, 0x02u, 0xECu, 0x5Bu, 0xA6u, 0x6Fu, 0x4Cu, 0x00u, 0x4Cu, 0x64u, 0x47u, 0x06u, 0xB9u },
/*2-Passkey*/  { 0x86u, 0x74u, 0x26u, 0x03u, 0xEEu, 0x73u, 0x28u, 0x8Eu, 0xB7u, 0x45u, 0x5Cu, 0xF1u, 0xFBu, 0x52u, 0x9Eu, 0x1Bu },
/*3-OTA CTRL*/ { 0x75u, 0x16u, 0x74u, 0x8Eu, 0x97u, 0x60u, 0xE6u, 0xA6u, 0x9Eu, 0x43u, 0x93u, 0xFEu, 0xA5u, 0x93u, 0x89u, 0xDFu },
/*4-Pro Mode*/ { 0xCBu, 0xB8u, 0xC9u, 0xCCu, 0x22u, 0x77u, 0xD9u, 0xBFu, 0xA4u, 0x43u, 0xAFu, 0x9Bu, 0xC1u, 0x25u, 0x41u, 0x02u },
/*5-Unsecured*/{ 0xDEu, 0x6Cu, 0xFDu, 0xCBu, 0xA1u, 0xCCu, 0xE8u, 0x82u, 0x36u, 0x45u, 0x58u, 0x7Bu, 0xD9u, 0x57u, 0xBAu, 0xA0u },
/*6-OTA-DATA*/ { 0x55u, 0xA3u, 0xe0u, 0x28u, 0x19u, 0x5Cu, 0x69u, 0x90u, 0x58u, 0x4Cu, 0xAEu, 0xFEu, 0xA9u, 0x82u, 0x47u, 0xF9u },
#if 0
/*7-PROXY-COMM-DATA*/{ 0xA7u, 0x4Au, 0xA1u, 0xBCu, 0x1Fu, 0xA9u, 0x5Cu, 0xA3u, 0x66u, 0x48u, 0xA4u, 0x76u, 0x0Bu, 0x3Bu, 0x24u, 0x53u },
//  { 0x53u, 0x24u, 0x3Bu, 0x0Bu, 0x76u, 0xA4u, 0x48u, 0x66u, 0xA3u, 0x5Cu, 0xA9u, 0x1Fu, 0xBCu, 0xA1u, 0x4Au, 0xA7u },
//53243b0b-76a4-4866-a35c-a91fbca14aa7
/*8-PROXY-OTA*/{ 0x8Bu, 0x21u, 0x57u, 0x7Du, 0xB3u, 0xA1u, 0x08u, 0x99u, 0x87u, 0x45u, 0xC6u, 0x18u, 0x39u, 0x71u, 0xA9u, 0xB4u },
//b4a97139-18c6-4587-9908-a1b37d57218b
#endif
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
 /*12*/  { 0x0010u, (void *)&ble_attUuid128[3] }, /* OTA Control UUID */
 /*13*/  { 0x0012u, (void *)&ble_attValues[57] }, /* OTA Control */
 /*14*/  { 0x0002u, (void *)&ble_attValuesCCCD[6] }, /* Client Characteristic Configuration */
 /*15*/  { 0x0010u, (void *)&ble_attUuid128[4] }, /* Pro Mode UUID */
 /*16*/  { 0x0014u, (void *)&ble_attValues[75] }, /* Pro Mode */
 /*17*/  { 0x0002u, (void *)&ble_attValuesCCCD[8] }, /* Client Characteristic Configuration */
 /*18*/  { 0x0010u, (void *)&ble_attUuid128[5] }, /* Unsecured Status UUID */
 /*19*/  { 0x0014u, (void *)&ble_attValues[95] }, /* Unsecured Status */
 /*20*/  { 0x0002u, (void *)&ble_attValuesCCCD[10] }, /* Client Characteristic Configuration */
 /*21*/  { 0x0080u, (void *)&ble_attValues[115] },/* OTA DATA */
 #if 0
 /*22*/  { 0x0014u, (void *)&ble_attValues[115+128] },/* PROXY COMM DATA */
 /*23*/  { 0x0012u, (void *)&ble_attValues[115+128+20] },/* PROXY OTA */
#endif
};


#if 0
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


#if 0
// note: third entry of BLE_GATT_DB_T are the permissions, refer to BLE_1_StackGattDb.h
// /* GATT Database Properties Field Description
    1. Attribute read permissions <B0>: Bluetooth Spec Defined
    2. Attribute write permissions <B1>: Bluetooth Spec Defined
    3. Characteristic properties <B2>: Bluetooth Spec Defined
    4. <B3>Implementation Specific */

/* Attribute Read Permissions <B0>: Bluetooth Spec Defined */
/**\note: It is mandatory to set 'CY_BLE_GATT_DB_ATTR_PROP_RD_SEC_ENCRYPT' bit if either 
          'CY_BLE_GATT_DB_ATTR_PROP_RD_SEC_AUTHENTICATE' or 'CY_BLE_GATT_DB_ATTR_PROP_RD_SEC_SC_AUTHENTICATE'
          bits are set. Otherwise these bits will be ignored. */
#define CYBLE_GATT_DB_ATTR_PROP_READ                    (0x00000001u)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SEC_ENCRYPT          (0x00000002u)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SEC_AUTHENTICATE     (0x00000004u)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SEC_AUTHORIZE        (0x00000008u)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SEC_SC_AUTHENTICATE  (0x00000010u)

#define CYBLE_GATT_DB_ATTR_PROP_RD_MASK                 (0x0000001Fu)
#define CYBLE_GATT_DB_ATTR_PROP_RD_BIT_SHIFT            (0x0u)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SECURITY_MASK        (0x0000001Eu)
#define CYBLE_GATT_DB_ATTR_PROP_RD_SECURITY_BIT_SHIFT   (0x1u)

/* Attribute Write Permissions <B1>: Bluetooth Spec Defined */
/**\note: It is mandatory to set 'CY_BLE_GATT_DB_ATTR_PROP_WR_SEC_ENCRYPT' bit if either 
          'CY_BLE_GATT_DB_ATTR_PROP_WR_SEC_AUTHENTICATE' or 'CY_BLE_GATT_DB_ATTR_PROP_WR_SEC_SC_AUTHENTICATE'
          bits are set. Otherwise these bits will be ignored. */
#define CYBLE_GATT_DB_ATTR_PROP_WRITE                   (0x00000100u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SEC_ENCRYPT          (0x00000200u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SEC_AUTHENTICATE     (0x00000400u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SEC_AUTHORIZE        (0x00000800u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SEC_SC_AUTHENTICATE  (0x00001000u)

#define CYBLE_GATT_DB_ATTR_PROP_WR_MASK                 (0x00001F00u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_BIT_SHIFT            (0x8u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SECURITY_MASK        (0x00001E00u)
#define CYBLE_GATT_DB_ATTR_PROP_WR_SECURITY_BIT_SHIFT   (0x9u)

/* Characteristic properties <B2>: Bluetooth Spec Defined */
#define CYBLE_GATT_DB_CH_PROP_BROADCAST                 (0x00010000u)
#define CYBLE_GATT_DB_CH_PROP_READ                      (0x00020000u)
#define CYBLE_GATT_DB_CH_PROP_RD_WRITE_WO_RESP          (0x00040000u)
#define CYBLE_GATT_DB_CH_PROP_WRITE                     (0x00080000u)
#define CYBLE_GATT_DB_CH_PROP_NOTIFY                    (0x00100000u)
#define CYBLE_GATT_DB_CH_PROP_INDICATE                  (0x00200000u)
#define CYBLE_GATT_DB_CH_PROP_SEC_AUTH_SIGNED_WRITE     (0x00400000u)
#define CYBLE_GATT_DB_CH_PROP_EXT_PROP                  (0x00800000u)
#define CYBLE_GATT_DB_CHAR_PROP_MASK                    (0x00FF0000u)
#define CYBLE_GATT_DB_CHAR_PROP_BIT_SHIFT               (0x10u)

/* Extended properties values. Note: These properties are not
   part of permission field. The peer GATT layer or local GATT
   layer needs to read "Characteristic Extended Properties 
   Descriptor" value. */
#define CYBLE_GATT_DB_CH_PROP_EXT_PROP_RELIABLE_WRITE   (0x0001u)
#define CYBLE_GATT_DB_CH_PROP_EXT_PROP_WRITABLE_AUX     (0x0002u)

/* Implementation permissions <B3>: Implementation specific
   defines to access runtime characteristics. */
#define CYBLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT            (0x01000000u)
#define CYBLE_GATT_DB_ATTR_UUID_FMT_32                  (0x04000000u)
#define CYBLE_GATT_DB_ATTR_UUID_FMT_128                 (0x08000000u)
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_7            (0x10000000u) /* Encryption Key Size 7 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_8            (0x20000000u)/* Encryption Key Size 8 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_9            (0x30000000u)/* Encryption Key Size 9 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_10           (0x40000000u)/* Encryption Key Size 10 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_11           (0x50000000u)/* Encryption Key Size 11 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_12           (0x60000000u)/* Encryption Key Size 12 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_13           (0x70000000u)/* Encryption Key Size 13 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_14           (0x80000000u)/* Encryption Key Size 14 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_15           (0x90000000u)/* Encryption Key Size 15 Needed for the Service */
#define CYBLE_GATT_DB_SER_ENCRYPT_KEY_SIZE_16           (0xA0000000u)/* Encryption Key Size 16 Needed for the Service */
#define CYBLE_GATT_DB_SER_NO_ENCRYPT_PROPERTY           (0x00000000u)/* No Encryption Needed for the Service */
#define CYBLE_GATT_DB_ENC_KEY_SIZE_MASK                 (0xF0000000u)/*  Encryption Key Size Mask Needed for the Service */

/* By default entries in CYBLE_GATT DB is 16-bit UUID
   This mask is defined to decide UUID length */
#define CYBLE_GATT_DB_ATTR_UUID_FMT_16_MASK             (0x03000000u)
#endif

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
#endif


bool ota_active()
{
    return ota_running() || proxy_ota_active;
}


void updateAuth(void)
{
    static bool passOn = 1;
    static bool passOff = 1;
    
    if (status.isUnsecured) bleDisconnect = false;
    
    if (passkeyTimer) {
        if ((millis() - passkeyTimer) > PASSKEY_TIMEOUT) { // 3 minute timeout
            passkeyTimer = 0;
        }
        
        if (passOn) {
            writePassKey(passkey);
            passOn = 0;
            passOff = 1;
            bleDisconnect = false;
        }
    } else {
        if (passOff) {
            writePassKey(0);
            passOn = 1;
            passOff = 0;
            
            if (!status.isUnsecured) {
                bleDisconnect = true;
            }
        }
    }
    
    if(!status.isUnsecured /* TODO -- && cyBle_pendingFlashWrite != 0u */) {
#if 0 // TODO -- STORE BLE BONDING INFORMATION TO FLASH
        static CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;

        // Store Bonding information to flash 
        apiResult = CyBle_StoreBondingData(0u);
        if ( apiResult == CYBLE_ERROR_OK)
            LOGD(TAG, "Bonding data stored\n");
        else
            LOGD(TAG, "Bonding data storing pending\n");
#endif
    }
}


void writePassKey(uint32_t thisKey)
{
    passkey_char->setValue((uint8_t*)&thisKey, sizeof(thisKey));

    LOGD(TAG, "Service Changed Attribute DB write success: %s -- %d\r\n",
        thisKey? "PASSKEY ON " : "PASSKEY OFF ", millis());
}


void do_send_sec_packet()
{
    int packetLength = 9;

    LOGD(TAG, "%s(): status.isUnsecured=%d, authGood=%d, passkeyTimer=%d\r\n", __FUNCTION__, status.isUnsecured, authGood, passkeyTimer);
    Serial.flush();
   
    sendSecPacket = false;
    bleTxBuffer[0] = packetLength;
    bleTxBuffer[1] = 0x00;
    bleTxBuffer[2] = status.isUnsecured;
    bleTxBuffer[3] = authGood;
    bleTxBuffer[4] = passkeyTimer > 0 ? 1 : 0;
    
//    if(passkeyTimer && passkey > 0)
//    {
//        bleTxBuffer[5] = passkey & 0xFF;
//        bleTxBuffer[6] = passkey >> 8;
//        bleTxBuffer[7] = passkey >> 16;
//        bleTxBuffer[8] = passkey >> 24;
//        
//        packetLength = 13;
//        bleTxBuffer[0] = packetLength;
//    }
    
    uint32_t crc = crc32(0, bleTxBuffer, packetLength - 4);
    bleTxBuffer[packetLength - 4] = crc & 0xFF;
    bleTxBuffer[packetLength - 3] = crc >> 8;
    bleTxBuffer[packetLength - 2] = crc >> 16;
    bleTxBuffer[packetLength - 1] = crc >> 24;
    
    unsecured_char->setValue(bleTxBuffer, packetLength);
    unsecured_char->notify();
    LOGD(TAG, "Send Security Packet: Success \r\n");
}


void do_send_ble_packet()
{
    /* if stack is free, handle UART traffic */
    if (ble_client_connected) {
        // TODO: limit BLE traffic if OTA in progress
        // If this is too restrictive we could make mutually exclusive BLE characteristics
        // so OTA and system operation do not overlap
        sendBlePacket = 0;
        if (!ota_active()) {
            if (authGood && sendNotifications) {
                //LOGD(TAG, "BLE Notification Enabled...\r\n");
                int txLength = 5;
                int i = 0;
                
                bleTxBuffer[0] = 0x55;
                bleTxBuffer[1] = txLength + 5;
                bleTxBuffer[2] = 0x00;
                
                for(i = 0 ; i < txLength; i++) {
                    bleTxBuffer[3 + i] = tx_buffer[i];
                }
                
                uint32_t crc = crc32(0, bleTxBuffer, txLength + 3);
                
                uint32_t packetLength = txLength + 7;
                
                bleTxBuffer[packetLength - 4] = crc & 0xFF;
                bleTxBuffer[packetLength - 3] = crc >> 8;
                bleTxBuffer[packetLength - 2] = crc >> 16;
                bleTxBuffer[packetLength - 1] = crc >> 24;
                
                comm_char->setValue(bleTxBuffer, packetLength);
                comm_char->notify();
            }
        }
    }
}


void do_send_settings_packet()
{
    sendSettingsPacket = false;
    
    if (authGood) {
        bleTxBuffer[0] = 0x55;
        bleTxBuffer[1] = 6;
        bleTxBuffer[2] = 0x02;
        bleTxBuffer[3] = status.isDeepSleepEnabled? 0x01 : 0x00;
        
        uint32_t crc = crc32(0, bleTxBuffer, 4);
        bleTxBuffer[4] = crc & 0xFF;
        bleTxBuffer[5] = crc >> 8;
        bleTxBuffer[6] = crc >> 16;
        bleTxBuffer[7] = crc >> 24;
        
        comm_char->setValue(bleTxBuffer, 8);
        comm_char->notify();       
        LOGD(TAG, "Send Setting Packet: (%d) Success \r\n", status.isDeepSleepEnabled);
    }
}


void do_send_pro_packet()
{
    // TODO: so PRO_NUM_SYNC_PACKETS is 8, which means the 'if' case below can *NEVER* happen
    sendProPacket -= 1;
    if (sendProPacket >= PRO_NUM_SYNC_PACKETS)
        sendProPacket = 0;
    else  {
        int packetLength = 9;
        if (sendProPacket == 8) { // TODO: sendProPacket takes on the values 7 to 0
            packetLength = 4;
            
            bleTxBuffer[0] = packetLength;
            bleTxBuffer[1] = (0x00) | (DEVICE_TYPE << 6);
            bleTxBuffer[2] = 0x00;
            bleTxBuffer[3] = (status.isDisableSleep ? 0x01 : 0);
        } else {
            packetLength = 13;
                
            uint8_t index = 7 - sendProPacket;      //0;
            
            bleTxBuffer[0] = packetLength;
            bleTxBuffer[1] = (0x03) | (DEVICE_TYPE << 6);
            bleTxBuffer[2] = 0x00;
            bleTxBuffer[3] = status.address << 3 | index;
            bleTxBuffer[4] = (
                    (status.pro[index].alwaysOn ? 0x10 : 0) | 
                    (status.pro[index].isIgnCtrl ? 0x08 : 0) | 
                    (status.pro[index].isLockout ? 0x04 : 0) | 
                    /*(status.pro[index].isInputLatch ? 0x02 : 0) | */
                    (status.pro[index].isCurrentRestart ? 0x01 : 0) 
                    );
            bleTxBuffer[5] = status.pro[index].timers >> 8;
            bleTxBuffer[6] = status.pro[index].timers & 0xFF;
            bleTxBuffer[7] = status.pro[index].currentLimit;
            bleTxBuffer[8] = 0x20 /*| 
                    status.pro[index].isInputEnabled ? INPUT_EN_CAN_MASK : 0 | 
                    status.pro[index].isInputLockout ? INPUT_LCK_CAN_MASK : 0*/; 
        }
        
        promode_char->setValue(bleTxBuffer, packetLength);
        promode_char->notify();       
        LOGD(TAG, "Send Pro Packet (%d): Success \r\n", sendProPacket);
    }
}


enum securityPacketType {SEC_READ_STATUS, SEC_SET_MODE, SEC_WRITE_PASSKEY};

static void processSecurityPacket(void)
{
    uint8_t packetLength = bleRxBuffer[0];
    
    LOGD(TAG, "processSecurityPacket():\r\n");
    LOG_HEXDUMP(TAG, bleRxBuffer, packetLength, ESP_LOG_DEBUG);
    
    uint32_t calcCrc = crc32(0, bleRxBuffer, packetLength - 4);
    uint32_t tempCrc = 0;
    tempCrc = tempCrc | bleRxBuffer[packetLength - 1];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 2];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 3];
    tempCrc = (tempCrc << 8) | bleRxBuffer[packetLength - 4];
    
    if (tempCrc != calcCrc)
        LOGE(TAG, "%s(): crc bad\r\n", __FUNCTION__);
    else {
        if (bleRxBuffer[1] == SEC_READ_STATUS) sendSecPacket = true;
        
        if (authGood) {
            if(bleRxBuffer[1] == SEC_SET_MODE) {
              bool level = (bleRxBuffer[2] & 0x01) == 0? SECURED : UNSECURED;
              setSecurityLevel(level);
            }
        } else {
            LOGD(TAG, "%s(): status.isUnsecured=%d\r\n", __FUNCTION__, status.isUnsecured);
            if (status.isUnsecured) {
                // listen for passkey -> set authGood
                if (bleRxBuffer[1] == SEC_WRITE_PASSKEY) {
                    uint32_t readPasskey = 0;
                    readPasskey = readPasskey | bleRxBuffer[5];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[4];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[3];
                    readPasskey = (readPasskey << 8) | bleRxBuffer[2];
                    
                    LOGD(TAG, "try passkey: %x/%x - ", readPasskey, passkey);
                    
                    if(readPasskey == passkey) {
                        authGood = true;
                        LOGD(TAG, "accepted!!\r\n");
                    } else {
                        LOGD(TAG, "failed...\r\n");
                    }
                    
                    sendSecPacket = true;
                }
            }
        }
    }
}


#define CAN_PACKET              0
#define LED_PACKET              1
#define SETTINGS_PACKET         2
#define LINK_PACKET             3
#define PAIR_PACKET             4
#define ALT_SETTINGS_PACKET     5
#define SWITCH_SETTINGS_PACKET  8
#define BOARD_INFO_PACKET       9

#define IS_MOMENTARY_MASK       0x01
#define IS_DIMABLE_MASK         0x02
#define IS_STROBE_MASK          0x04   
#define IS_FLASH_MASK           0x08

// For processCANPacket(), 
//     processSettingsPacket(), 
//     processSwitchSettingsPacket(),
//
// index 0 -- 0x55
//       1 -- packet length (includes 3-byte header and 4-byte trailer (CRC32))
//       2 -- COMM packet type (CAN_PACKET/LED_PACKET...)

static void processCANPacket(uint8_t packetLength)
{
  LOGI(TAG, "%s()\r\n", __FUNCTION__);
  int index = (bleRxBuffer[5] & 0x0F) * 8 + canToI(bleRxBuffer[6]);
  if ((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255 && bleRxBuffer[7] > 0) {
      lastRecVals[index].dimVal = bleRxBuffer[7];
      lastRecVals[index].onVal = bleRxBuffer[8];
      lastRecVals[index].offVal = bleRxBuffer[9];
  }

  if((bleRxBuffer[5] & 0x0F) != status.address)
      pushCanFifo(&bleRxBuffer[5]);       // move to canBroadcast to avoid possibility of contention
  else {
      if((bleRxBuffer[5] & 0xF0) == SWITCH_PACKET && canToI(bleRxBuffer[6]) != 255) {
          uint8_t i = canToI(bleRxBuffer[6]);
          
          status.out[i].outCmd = bleRxBuffer[7];
          if (status.out[i].outCmd == 0xFE) status.out[i].outCmd = 0xFF;
          
          if (status.out[i].outCmd != 0) status.pro[i].lastSwVal = status.out[i].outCmd;
          
          status.out[i].blinkOn = bleRxBuffer[8] * FLASH_SPEED; 
          status.out[i].blinkOff = bleRxBuffer[9] * FLASH_SPEED;
          
          isAwake = 1;
          bleAliveTimer = millis();
      } else 
      if ((bleRxBuffer[5] & 0xF0) == PRO_PACKET && 
          (bleRxBuffer[6] == 0 || canToI(bleRxBuffer[6]) != 255)) {
          uint8_t i = canToI(bleRxBuffer[6]);
          
          if (bleRxBuffer[6] != 0) {
            #if 0
              status.pro[i].isInputEnabled = (bleRxBuffer[7] & INPUT_EN_CAN_MASK) ? true : false;
              status.pro[i].isInputLockout = (bleRxBuffer[7] & INPUT_LCK_CAN_MASK) ? true : false;
            #endif
          }
          
          writeFlash();
          
          // just transmit through rather than try to reassemble/index... 
          // TODO: with CAN bus this may mean this packet will be re-received, we may have to add code
          // to inject this packet back into the "CAN" (AKA RS485) sub-system for processing (because
          // we don't re-receive with RS485 bus)
          pushCanFifo(&bleRxBuffer[5]);   
          needsInputsUpdate = true;
      }
  }
}


// index 0 -- 0x55
//       1 -- packet length (includes 3-byte header and 4-byte trailer (CRC32))
//       2 -- COMM packet type
static void processSettingsPacket(uint8_t packetLength)
{
  LOGI(TAG, "%s()\r\n", __FUNCTION__);
  if ((bleRxBuffer[3] & 0x02) != 0) {
      sendSettingsPacket = true;
  } else {
      status.isDeepSleepEnabled = bleRxBuffer[3] & 0x01;
      writeFlash();  
  }
}


// index 0 -- 0x55
//       1 -- packet length (includes 3-byte header and 4-byte trailer (CRC32))
//       2 -- COMM packet type
static void processSwitchSettingsPacket(uint8_t packetLength)
{
  LOGI(TAG, "%s()\r\n", __FUNCTION__);
  printByteArray((char*)"switchSettingsPacket():", &bleRxBuffer[0], packetLength);

  uint8_t swIndex = bleRxBuffer[3];
  if (swIndex > 32) return;
  if (bleRxBuffer[4] != 0) return; // for versioning

  uint8_t srcAddr = swIndex / 8;
  uint8_t swAddr = swIndex % 8;
  uint8_t isDim = bleRxBuffer[6] & (IS_DIMABLE_MASK);

  if (isDim && bleRxBuffer[5] > 0 && bleRxBuffer[5] < 0xFE) {
      lastRecVals[swIndex].dimVal = bleRxBuffer[5];
  }

  uint8_t isBlink = bleRxBuffer[6] & (IS_FLASH_MASK | IS_STROBE_MASK);
  if (isBlink && bleRxBuffer[7] != 0 && bleRxBuffer[8] != 0) {
      lastRecVals[swIndex].onVal = bleRxBuffer[7] * FLASH_SPEED;
      lastRecVals[swIndex].offVal = bleRxBuffer[8] * FLASH_SPEED;
      
      if(srcAddr == status.address) {
          status.out[swAddr].blinkOn = bleRxBuffer[7] * FLASH_SPEED;
          status.out[swAddr].blinkOff = bleRxBuffer[8] * FLASH_SPEED;
      }
  }
}


static void process_board_info_packet(uint8_t packet_length)
{
    board_short_info_t info[BOARD_MAX];
    uint8_t data[7+sizeof(info)];
    board_info_query(info);

    LOGI(TAG, "%s(): sizeof(data)=%d\r\n", __FUNCTION__, sizeof(data));
    data[0] = 0x55;
    data[1] = sizeof(info)+2;
    data[2] = BOARD_INFO_PACKET;
    memcpy(data+3, (uint8_t*)&info, sizeof(info));
    
    uint32_t crc = crc32(0, data, sizeof(info)+3);
    data[3+sizeof(info)] = crc & 0xFF;
    data[4+sizeof(info)] = crc >> 8;
    data[5+sizeof(info)] = crc >> 16;
    data[6+sizeof(info)] = crc >> 24;
    
    comm_char->setValue(data, sizeof(data));
    comm_char->notify();       
}


// index 0 -- 0x55
//       1 -- packet length (includes 3-byte header and 4-byte trailer (CRC32))
//       2 -- COMM packet type (CAN_PACKET/LED_PACKET...)
static void processCOMMPacket(uint8_t packetLength)
{
    LOGI(TAG, "%s():\r\n", __FUNCTION__);
    LOG_HEXDUMP(TAG, bleRxBuffer, packetLength, ESP_LOG_INFO);
    
    uint32_t calcCrc = crc32(0, bleRxBuffer, packetLength - sizeof(calcCrc));
    uint32_t crc = bleRxBuffer[packetLength - 1];
    crc = (crc << 8) | bleRxBuffer[packetLength - 2];
    crc = (crc << 8) | bleRxBuffer[packetLength - 3];
    crc = (crc << 8) | bleRxBuffer[packetLength - 4];
    
    if (crc != calcCrc)
       LOGE(TAG, "%s(): CRC Error... calculated=%d expected=%d\r\n", __FUNCTION__, calcCrc, crc);    
    else {
        switch (bleRxBuffer[2]) {
        case CAN_PACKET: processCANPacket(packetLength); break;
        case LED_PACKET: break;
        case LINK_PACKET: break;
        case PAIR_PACKET: break;
        case SETTINGS_PACKET: processSettingsPacket(packetLength); break;
        case ALT_SETTINGS_PACKET: break;
        case SWITCH_SETTINGS_PACKET: processSwitchSettingsPacket(packetLength); break;
        case BOARD_INFO_PACKET: process_board_info_packet(packetLength); break;
        default: break;
        }
    }
}


// "data" can be several COMM packets delimited with 0x55,
// break up into individual packets and send to
//   processCOMMPacket()
static void processCOMMData(uint8_t* data, size_t length)
{
  bool delimiter = false;
  uint8_t index = 0;

  //LOGD(TAG, "%s(length=0x%x)\r\n", __FUNCTION__, length);
  //LOG_HEXDUMP(TAG, data, 32, ESP_LOG_DEBUG);

  blePacketLength = -1;
  for (int i = 0; i < length; ++i) {
    if (delimiter) {
        if (blePacketLength == -1) {
            if (data[i] <= 250) {
                blePacketLength = data[i] + 2;
                bleRxBuffer[index++] = data[i];
            } else {
                delimiter = false;
            }
        } else {
            bleRxBuffer[index++] = data[i];
            if (index == blePacketLength) {
                processCOMMPacket(blePacketLength);
                delimiter = false;
            }
        }
    } else {
        if (data[i] == 0x55) {
            delimiter = true;
            blePacketLength = -1;
            index = 1;
            bleRxBuffer[0] = data[i];
        }
    }
  }
}


static void processProPacket(void)
{
    LOGI(TAG, "processProPacket():\r\n");
    LOG_HEXDUMP(TAG, bleRxBuffer, bleRxBuffer[0], ESP_LOG_INFO);
    
    if (((bleRxBuffer[1] & 0xC0) >> 6) == DEVICE_TYPE) { // check that the app sent the correct type of packet
        bool updateFlash = 0;

        if ((bleRxBuffer[1] & 0x01) != 0) { // read
            sendProPacket = PRO_NUM_SYNC_PACKETS;
            globalUpdateProCanPackets = true;        // to update inputs settings
            return;         //ignore the rest of the packet
        }
        
        uint8_t len = bleRxBuffer[0];
        if (len <= 3) {
            if ((bleRxBuffer[2] & 0x02) != 0) { // update deep sleep flag
                uint8_t nds = bleRxBuffer[2] & 0x01;
                if (status.isDisableSleep != nds) {
                    LOGD(TAG, "Deep Sleep disabled %d \n", nds);
                    status.isDisableSleep = nds;
                    updateFlash = 1;
                }
            }
            
            if (updateFlash) writeFlash();
            return;
        } else 
        if (len >= 8 && bleRxBuffer[2] == 0x00) {
            uint8_t addr = bleRxBuffer[3] >> 3;
            if (addr == status.address) {
                updateFlash = 1;
                
                uint8_t index = bleRxBuffer[3] & 0x07;
                status.pro[index].alwaysOn = ((bleRxBuffer[4] & 0x10) != 0) ? 1 : 0;
                status.pro[index].isIgnCtrl = ((bleRxBuffer[4] & 0x08) != 0) ? 1 : 0;
                status.pro[index].isLockout = ((bleRxBuffer[4] & 0x04) != 0) ? 1 : 0;
                /*status.pro[index].isInputLatch = ((bleRxBuffer[4] & 0x02) != 0) ? 1 : 0;*/
                status.pro[index].isCurrentRestart = ((bleRxBuffer[4] & 0x01) != 0) ? 1 : 0;
                
                if (status.pro[index].alwaysOn) status.out[index].outCmd = 0xFF;
                
                status.pro[index].timers = (bleRxBuffer[5] << 8) | bleRxBuffer[6];
                
                if (status.pro[index].curTime > 0) status.pro[index].curTime = millis(); // reset timer if running
                
                status.pro[index].currentLimit = bleRxBuffer[7];
                if (status.pro[index].currentLimit == 0) status.pro[index].currentLimit = 30; // error...
                
                if (len >= 13) {
                #if 0
                    status.pro[index].isInputEnabled = (bleRxBuffer[8] & INPUT_EN_CAN_MASK) ? true : false;
                    status.pro[index].isInputLockout = (bleRxBuffer[8] & INPUT_LCK_CAN_MASK) ? true : false;
                #endif
                }
            }
        }
        
        if (updateFlash) writeFlash();
    }
}


void ble_ack_ota_ctrl_packet(uint8_t mac[3], uint8_t* data, uint8_t leng)
{
    if (leng == 0) proxy_ota_active = false; // zero length denotes OTA complete

    leng += PKT_MACLENG;
    data -= PKT_MACLENG;
    memcpy(data, mac, PKT_MACLENG);

    LOGD(TAG, "%s(leng=%d)\r\n", __FUNCTION__, leng);
    //LOG_HEXDUMP(TAG, data, leng, ESP_LOG_INFO);

    ota_ctrl_char->setValue(data, leng);
    ota_ctrl_char->notify();
}


static void ble_process_ota_data_packet(uint8_t* data, size_t length)
{
    uint8_t dstmac[3];
    memcpy(dstmac, data, sizeof(dstmac));

    #if 0
    uint16_t offset;
    memcpy((uint8_t*)&offset, data+3, sizeof(offset));
    LOGI(TAG, "%s(): pleng=%d, mac=%02x:%02x:%02x (offset=0x%x)\r\n", 
        __FUNCTION__, length, dstmac[0], dstmac[1], dstmac[2], offset);
    //LOG_HEXDUMP(TAG, data, 32, ESP_LOG_INFO);
    #endif

    if (!pkt_mac_match(pkt_ourmac(), dstmac)) {
        proxy_ota_active = true;
        pkt_put(dstmac, PKT_SRCMAC|PKT_NOACK|OTA_DATA_PKT_TYPE, data+sizeof(dstmac), length-sizeof(dstmac), -1);               
    } else {
        process_ota_data_packet(data+sizeof(dstmac), length-sizeof(dstmac));
    }
}


static void ble_process_ota_ctrl_packet(uint8_t* data, uint8_t pleng)
{
    uint8_t dstmac[3] = {data[0], data[1], data[2]};

    LOGI(TAG, "%s(): pleng=%d, mac=%02x:%02x:%02x\r\n", __FUNCTION__, pleng, dstmac[0], dstmac[1], dstmac[2]);
    //LOG_HEXDUMP(TAG, data, 32, ESP_LOG_INFO);

    if (!pkt_mac_match(pkt_ourmac(), dstmac)) {
        // SPOD is an OTA proxy for another device, forward the packet
        proxy_ota_active = true;
        pkt_put(dstmac, PKT_SRCMAC|OTA_CTRL_PKT_TYPE, data+sizeof(dstmac), pleng-sizeof(dstmac), 2);
    } else {
        process_ota_ctrl_packet(dstmac, data+sizeof(dstmac), pleng-sizeof(dstmac), ble_ack_ota_ctrl_packet);
    }
}


class ServerCallbacks: public BLEServerCallbacks {
   void onConnect(BLEServer* server) {
      LOGI(TAG, "onConnect(): Client connected!\r\n");
      ble_client_connected = true;
      server->stopAdvertising();
    };

    void onDisconnect(BLEServer* server) {
      LOGI(TAG, "onDisconnect(): Client disconnected\r\n");
      ble_client_connected = false;
      server->startAdvertising();
    }

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        LOGE(TAG, "MTU updated: %u for connection ID: %u\r\n", MTU, desc->conn_handle);
    };

    uint32_t onPassKeyRequest(){
        LOGD(TAG, "Server Passkey Request\n");
        // TODO: not sure what to do here... (do we return the current passkey or 123456, which has special meaning in the BLE stack)

        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key){
        LOGD(TAG, "The passkey YES/NO number: %x\n", pass_key);
        writePassKey(pass_key); // TODO: is this correct?
        return true;  // false if passkeys don't match
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        if(!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            LOGE(TAG, "Encrypt connection failed - disconnecting client\r\n");
            authGood = false;
            return;
        }
        LOGI(TAG, "Encryption good -- Starting BLE work!\r\n");
        writePassKey(0);
        authGood = true;
    };
};


class unsecured_status_callbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pchar){
        LOGD(TAG, "%s: unsecured_status::onRead(), value: %s\r\n",
            pchar->getUUID().toString().c_str(),
            pchar->getValue().c_str());
    };
    
    void onWrite(NimBLECharacteristic* pchar) {
        LOGD(TAG, "%s: unsecured_status::onWrite()\n",
            pchar->getUUID().toString().c_str());

        lastActivityMs = millis();
                            
        size_t leng = pchar->getDataLength();
        if (leng > sizeof(bleRxBuffer)) leng = sizeof(bleRxBuffer);
        memcpy(bleRxBuffer, pchar->getValue().getValue(NULL), leng);           
        processSecurityPacket();
    };

    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        LOGD(TAG, "Sending notification to clients (unsecured status)");
    };
    
    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("(unsecured_status) Notification/Indication status code: "); 
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        LOGD(TAG, "%s\n", str);
    };
    
    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "(unsecured status) Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        LOGD(TAG, "%s\n", str);
    };
};


class promode_callbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pchar) {
        LOGD(TAG, "%s: promode::onWrite()\n",
            pchar->getUUID().toString().c_str());
      
        lastActivityMs = millis();
        if (authGood) {
            size_t leng = pchar->getDataLength();
            if (leng > sizeof(bleRxBuffer)) leng = sizeof(bleRxBuffer);
            memcpy(bleRxBuffer, pchar->getValue().getValue(NULL), leng);           
            processProPacket();
        }
    };
};


class DescCallbacks: public BLEDescriptorCallbacks {
    void onWrite(BLEDescriptor* desc) {
      LOGD(TAG, "DESC onWrite()\n");
      lastActivityMs = millis();
    }
};


class comm_callbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pchar) {
        LOGV(TAG, "%s: comm::onWrite() -- authGood=%d\r\n",
            pchar->getUUID().toString().c_str(), authGood);
      
        lastActivityMs = millis();
        if (authGood) {
          size_t leng = pchar->getDataLength();
          uint8_t data[256];
          if (leng > sizeof(data)) leng = sizeof(data);
          memcpy(data, pchar->getValue().getValue(NULL), leng);           
          processCOMMData(data, leng);
        }                  
    };
};


class ota_ctrl_callbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pchar) {
        LOGV(TAG, "%s: OTA bootloader::onWrite()\r\n",
            pchar->getUUID().toString().c_str());
      
        lastActivityMs = millis();
        if (authGood) {
            size_t leng = pchar->getDataLength();
            uint8_t data[256];
            if (leng > sizeof(data)) leng = sizeof(data);
            memcpy(data, pchar->getValue().getValue(NULL), leng);           
            ble_process_ota_ctrl_packet(data, leng);
        }
    };
};

class ota_data_callbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pchar) {
        LOGV(TAG, "%s: comm::onWrite() -- authGood=%d\r\n",
            pchar->getUUID().toString().c_str(), authGood);
      
        lastActivityMs = millis();
        if (authGood) {
          size_t leng = pchar->getDataLength();
          uint8_t data[256];
          if (leng > sizeof(data)) leng = sizeof(data);
          memcpy(data, pchar->getValue().getValue(NULL), leng);           
          ble_process_ota_data_packet(data, leng);
        }                  
    };
};


class passkey_callbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pchar) {
        LOGD(TAG, "%s: passkey::onWrite()\r\n", pchar->getUUID().toString().c_str());
        lastActivityMs = millis();
    };
};


static void build_ble_address()
{    
    sprintf((char *)&ble_address[11], "%02X%02X%02X%02X", DEVICE_TYPE, our_mac[3], our_mac[4], our_mac[5]);
    LOGI(TAG, "%s(): device-name='%s'\r\n", __FUNCTION__, ble_address);
}


void ble_init()
{
    BLEUUID service_uuid(         (uint8_t*)ble_attUuid128[0], sizeof(ble_attUuid128[0]), false);
    BLEUUID comm_uuid(            (uint8_t*)ble_attUuid128[1], sizeof(ble_attUuid128[1]), false);
    BLEUUID passkey_uuid(         (uint8_t*)ble_attUuid128[2], sizeof(ble_attUuid128[2]), false);
    BLEUUID promode_uuid(         (uint8_t*)ble_attUuid128[4], sizeof(ble_attUuid128[4]), false);
    BLEUUID unsecured_uuid(       (uint8_t*)ble_attUuid128[5], sizeof(ble_attUuid128[5]), false);
    BLEUUID ota_ctrl_uuid(        (uint8_t*)ble_attUuid128[3], sizeof(ble_attUuid128[3]), false);
    BLEUUID ota_data_uuid(        (uint8_t*)ble_attUuid128[6], sizeof(ble_attUuid128[6]), false);
    #if 0
    BLEUUID proxy_comm_uuid(      (uint8_t*)ble_attUuid128[7], sizeof(ble_attUuid128[7]), false);
    BLEUUID proxy_ota_uuid(       (uint8_t*)ble_attUuid128[8], sizeof(ble_attUuid128[8]), false);
    #endif

    LOGI(TAG, "%s(): service_uuid='%s', comm_uuid='%s'\r\n", __FUNCTION__,
        service_uuid.toString().c_str(),
        comm_uuid.toString().c_str()); 
    Serial.flush();
    build_ble_address();
    std::string address = (char *) ble_address;
    BLEDevice::init(address);
    BLEDevice::setPower(ESP_PWR_LVL_P9); /* +9db */
    BLEDevice::setSecurityAuth(true/*bonding*/, true/*mitm*/, true/*secure connections*/);
    BLEDevice::setSecurityPasskey(123456);
    BLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY /*passkey*/);
    BLEDevice::setMTU(512);

    ble_server = BLEDevice::createServer();
    ble_server->setCallbacks(new ServerCallbacks());

    DescCallbacks* cbs = new DescCallbacks();

    spod_service = ble_server->createService(service_uuid);

    comm_char = spod_service->createCharacteristic(comm_uuid, NIMBLE_SPROPS);
    comm_char->setValue((uint8_t*)ble_attValuesLen[7].attGenericVal, ble_attValuesLen[7].actualLength);
    comm_char->setCallbacks(new comm_callbacks);

    comm_desc = comm_char->createDescriptor(BLEUUID((uint16_t) 0x4764), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[7].actualLength);
    comm_desc->setValue((uint8_t*)ble_attValuesLen[7].attGenericVal, ble_attValuesLen[7].actualLength);
    comm_char->addDescriptor(comm_desc);

    passkey_char = spod_service->createCharacteristic(passkey_uuid, NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE);
    passkey_char->setValue((uint8_t*)ble_attValuesLen[10].attGenericVal, ble_attValuesLen[10].actualLength);
    passkey_char->setCallbacks(new passkey_callbacks);

    passkey_desc = passkey_char->createDescriptor(BLEUUID((uint16_t) 0x52FB), 
      NIMBLE_PROPERTY::READ, ble_attValuesLen[10].actualLength);
    passkey_desc->setValue((uint8_t*)ble_attValuesLen[10].attGenericVal, ble_attValuesLen[10].actualLength);
    passkey_char->addDescriptor(passkey_desc);

    promode_char = spod_service->createCharacteristic(promode_uuid, NIMBLE_SPROPS);
    promode_char->setValue((uint8_t*)ble_attValuesLen[16].attGenericVal, ble_attValuesLen[16].actualLength);
    promode_char->setCallbacks(new promode_callbacks);

    promode_desc = promode_char->createDescriptor(BLEUUID((uint16_t) 0x25C1), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[16].actualLength);
    promode_desc->setValue((uint8_t*)ble_attValuesLen[16].attGenericVal, ble_attValuesLen[16].actualLength);
    promode_char->addDescriptor(promode_desc);

    unsecured_char = spod_service->createCharacteristic(unsecured_uuid, NIMBLE_PROPS);
    unsecured_char->setValue((uint8_t*)ble_attValuesLen[19].attGenericVal, ble_attValuesLen[19].actualLength);
    unsecured_char->setCallbacks(new unsecured_status_callbacks);

    unsecured_desc = unsecured_char->createDescriptor(BLEUUID((uint16_t) 0x57D9), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[19].actualLength);
    unsecured_desc->setValue((uint8_t*)ble_attValuesLen[19].attGenericVal, ble_attValuesLen[19].actualLength);
    unsecured_char->addDescriptor(unsecured_desc);

    ota_ctrl_char = spod_service->createCharacteristic(ota_ctrl_uuid, NIMBLE_SPROPS);
    ota_ctrl_char->setValue((uint8_t*)ble_attValuesLen[13].attGenericVal, ble_attValuesLen[13].actualLength);
    ota_ctrl_char->setCallbacks(new ota_ctrl_callbacks);

    ota_ctrl_desc = ota_ctrl_char->createDescriptor(BLEUUID((uint16_t) 0x93A5), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[13].actualLength);
    ota_ctrl_desc->setValue((uint8_t*)ble_attValuesLen[13].attGenericVal, ble_attValuesLen[13].actualLength);
    ota_ctrl_char->addDescriptor(ota_ctrl_desc);

    ota_data_char = spod_service->createCharacteristic(ota_data_uuid, NIMBLE_SPROPS);
    ota_data_char->setValue((uint8_t*)ble_attValuesLen[21].attGenericVal, ble_attValuesLen[21].actualLength);
    ota_data_char->setCallbacks(new ota_data_callbacks);

    ota_data_desc = ota_data_char->createDescriptor(BLEUUID((uint16_t) 0x6447), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[21].actualLength);
    ota_data_desc->setValue((uint8_t*)ble_attValuesLen[21].attGenericVal, ble_attValuesLen[21].actualLength);
    ota_data_char->addDescriptor(ota_data_desc);

#if 0
    ota_data_char = spod_service->createCharacteristic(proxy_comm_uuid, NIMBLE_SPROPS);
    ota_data_char->setValue((uint8_t*)ble_attValuesLen[22].attGenericVal, ble_attValuesLen[22].actualLength);
    ota_data_char->setCallbacks(new ota_data_callbacks);

    ota_data_desc = ota_data_char->createDescriptor(BLEUUID((uint16_t) 0x6744), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[22].actualLength);
    ota_data_desc->setValue((uint8_t*)ble_attValuesLen[22].attGenericVal, ble_attValuesLen[22].actualLength);
    ota_data_char->addDescriptor(ota_data_desc);

    proxy_ota_char = spod_service->createCharacteristic(proxy_ota_uuid, NIMBLE_SPROPS);
    proxy_ota_char->setValue((uint8_t*)ble_attValuesLen[23].attGenericVal, ble_attValuesLen[23].actualLength);
    proxy_ota_char->setCallbacks(new proxy_ota_callbacks);

    proxy_ota_desc = proxy_ota_char->createDescriptor(BLEUUID((uint16_t) 0x6644), 
      NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE, ble_attValuesLen[23].actualLength);
    proxy_ota_desc->setValue((uint8_t*)ble_attValuesLen[23].attGenericVal, ble_attValuesLen[23].actualLength);
    proxy_ota_char->addDescriptor(proxy_ota_desc);
#endif

    comm_desc->setCallbacks(cbs);
    passkey_desc->setCallbacks(cbs);
    promode_desc->setCallbacks(cbs);
    unsecured_desc->setCallbacks(cbs);
    ota_ctrl_desc->setCallbacks(cbs);
    ota_data_desc->setCallbacks(cbs);

    spod_service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(service_uuid);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    advertising->start();

    LOGD(TAG, "ble_init(): complete!!!\n");
    Serial.flush();    
}


void ble_loop()
{
    if (!ble_client_connected && ota_running()) {
        LOGW(TAG, "BLE connection lost, terminating OTA\r\n");
        ota_abort();
    }
}
