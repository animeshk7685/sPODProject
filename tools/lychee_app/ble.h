#ifndef __BLE_H__
#define __BLE_H__

#include <Arduino.h>
#include <esp_bt.h>
#include <esp_bt_device.h>
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>

#define BLE_GATT_DB_INDEX_COUNT                    (0x001Bu)
#define BLE_GATT_DB_ATT_VAL_COUNT                  (0x15u)
#define BLE_GATT_DB_MAX_VALUE_LEN                  (0x0014u)

#define BLE_GATT_DB_CCCD_COUNT                     (0x0Cu)

#define BLE_GATT_DB_FLASH_CCCD_COUNT          (BLE_GATT_DB_CCCD_COUNT)


/* Generic Access Profile */
#define BLE_UUID_GAP_SERVICE                       (0x1800u)
/* Generic Attribute Profile */
#define BLE_UUID_GATT_SERVICE                      (0x1801u)

/* UUID: BLE Other Services */
#define BLE_UUID_IMMEDIATE_ALERT_SERVICE           (0x1802u)
#define BLE_UUID_LINK_LOSS_SERVICE                 (0x1803u)
#define BLE_UUID_TX_POWER_SERVICE                  (0x1804u)
#define BLE_UUID_CURRENT_TIME_SERVICE              (0x1805u)
#define BLE_UUID_REF_TIME_UPDATE_SERVICE           (0x1806u)
#define BLE_UUID_NEXT_DST_CHANGE_SERVICE           (0x1807u)
#define BLE_UUID_GLUCOSE_SERVICE                   (0x1808u)
#define BLE_UUID_HEALTH_THERMOMETER_SERVICE        (0x1809u)
#define BLE_UUID_DEVICE_INFO_SERVICE               (0x180Au)
#define BLE_UUID_NWA_SERVICE                       (0x180Bu)
#define BLE_UUID_WATCH_DOG_SERVICE                 (0x180Cu)
#define BLE_UUID_HEART_RATE_SERVICE                (0x180Du)
#define BLE_UUID_PHONE_ALERT_STATUS_SERVICE        (0x180Eu)
#define BLE_UUID_BAS_SERVICE                       (0x180Fu)
#define BLE_UUID_BLOOD_PRESSURE_SERVICE            (0x1810u)
#define BLE_UUID_ALERT_NOTIFICATION_SERVICE        (0x1811u)
#define BLE_UUID_HIDS_SERVICE                      (0x1812u)
#define BLE_UUID_SCAN_PARAM_SERVICE                (0x1813u)
#define BLE_UUID_RUNNING_SPEED_AND_CADENCE_SERVICE (0x1814u)
#define BLE_UUID_AUTOMATION_INPUT_OUTPUT_SERVICE   (0x1815u)
#define BLE_UUID_CYCLING_SPEED_AND_CADENCE_SERVICE (0x1816u)
#define BLE_UUID_PULSE_OXIMETER_SERVICE            (0x1817u)
#define BLE_UUID_CPS_SERVICE                       (0x1818u)
#define BLE_UUID_LOCATION_AND_NAVIGATION_SERVICE   (0x1819u)
#define BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE     (0x181Au)
#define BLE_UUID_BODY_COMPOSITION_SERVICE          (0x181Bu)
#define BLE_UUID_USER_DATA_SERVICE                 (0x181Cu)
#define BLE_UUID_WEIGHT_SCALE_SERVICE              (0x181Du)
#define BLE_UUID_BOND_MANAGEMENT_SERVICE           (0x181Eu)
#define BLE_UUID_CGM_SERVICE                       (0x181Fu)
#define BLE_UUID_INTERNET_PROTOCOL_SUPPORT_SERVICE (0x1820u)
#define BLE_UUID_IPS_SERVICE                       (0x1821u)
#define BLE_UUID_PLX_SERVICE                       (0x1822u)
#define BLE_UUID_HTTP_PROXY_SERVICE                (0x1823u)
#define BLE_UUID_OTS_SERVICE                       (0x1825u)
#define BLE_UUID_FIND_ME_SERVICE                   (0x18A3u)
#define BLE_UUID_WIRELESS_POWER_TRANSFER_SERVICE   (0xFFFEu)


/***************************************
* UUID: GATT Attribute Types defined by GATT Profile 
***************************************/

/* Primary Service Declaration */
#define BLE_UUID_PRIMARY_SERVICE                   (0x2800u)
/* Secondary Service Declaration */
#define BLE_UUID_SECONDARY_SERVICE                 (0x2801u)
/* Include Declaration */
#define BLE_UUID_INCLUDE                           (0x2802u)
/* Characteristic Declaration */
#define BLE_UUID_CHARACTERISTIC                    (0x2803u)


/***************************************
* UUID: GATT Characteristic Descriptors
***************************************/

/* Characteristic Extended Properties */
#define BLE_UUID_CHAR_EXTENDED_PROPERTIES          (0x2900u)
/* Characteristic User Description Descriptor */
#define BLE_UUID_CHAR_USER_DESCRIPTION             (0x2901u)
/* Client Characteristic Configuration Descriptor */
#define BLE_UUID_CHAR_CLIENT_CONFIG                (0x2902u)
/* Server Characteristic Configuration Descriptor */
#define BLE_UUID_CHAR_SERVER_CONFIG                (0x2903u)
/* Characteristic Format Descriptor */
#define BLE_UUID_CHAR_FORMAT                       (0x2904u)
/* Characteristic Aggregate Format Descriptor */
#define BLE_UUID_CHAR_AGGREGATE_FORMAT             (0x2905u)
/* Characteristic Valid Range */
#define BLE_UUID_CHAR_VALID_RANGE                  (0x2906u)
/* External Report Reference */
#define BLE_UUID_CHAR_EXTERNAL_REPORT_REF          (0x2907u)
/* Report Reference */
#define BLE_UUID_CHAR_REPORT_REFERENCE             (0x2908u)
/* Number of Digitals */
#define BLE_UUID_CHAR_NUMBER_OF_DIGITALS           (0x2909u)
/* Value Trigger Setting */
#define BLE_UUID_CHAR_VALUE_TRIGGER_SETTING        (0x290Au)
/* Environmental Sensing Configuration */
#define BLE_UUID_CHAR_ES_CONFIGURATION             (0x290Bu)
/* Environmental Sensing Measurement */
#define BLE_UUID_CHAR_ES_MEASUREMENT               (0x290Cu)
/* Environmental Sensing Trigger Setting */
#define BLE_UUID_CHAR_ES_TRIGGER_SETTING           (0x290Du)
/* Time Trigger Setting */
#define BLE_UUID_CHAR_TIME_TRIGGER_SETTING         (0x290Eu)


/***************************************
* UUID: Commonly used GATT Characteristic Types
***************************************/

/* Device Name Characteristic */
#define BLE_UUID_CHAR_DEVICE_NAME                  (0x2A00u)
/* Appearance Characteristic */
#define BLE_UUID_CHAR_APPEARANCE                   (0x2A01u)
/* Peripheral Privacy Flag Characteristic */
#define BLE_UUID_CHAR_PERIPH_PRIVCY_FLAG           (0x2A02u)
/* Reconnection Address Characteristic */
#define BLE_UUID_CHAR_RECONNECTION_ADDR            (0x2A03u)
/* Peripheral Preferred Connection Parameters Characteristic */
#define BLE_UUID_CHAR_PRFRRD_CNXN_PARAM            (0x2A04u)
/* Service Changed Characteristic */
#define BLE_UUID_CHAR_SERVICE_CHANGED              (0x2A05u)
/* Central Address Resolution Characteristic */
#define BLE_UUID_CHAR_CENTRAL_ADDRESS_RESOLUTION   (0x2AA6u)
/* Resolvable Private Address Only Characteristic */
#define BLE_UUID_CHAR_RESOLVABLE_PRIV_ADDR_ONLY    (0x2AC9u)


/***************************************
* UUID: Other Characteristic Types
***************************************/

#define BLE_UUID_CHAR_ALERT_LEVEL                  (0x2A06u)
#define BLE_UUID_CHAR_TX_POWER_LEVEL               (0x2A07u)
#define BLE_UUID_CHAR_DATE_TIME                    (0x2A08u)
#define BLE_UUID_CHAR_DAY_OF_WEEK                  (0x2A09u)
#define BLE_UUID_CHAR_DAY_DATE_TIME                (0x2A0Au)
#define BLE_UUID_CHAR_EXACT_TIME_100               (0x2A0Bu)
#define BLE_UUID_CHAR_EXACT_TIME_256               (0x2A0Cu)
#define BLE_UUID_CHAR_DST_OFFSET                   (0x2A0Du)
#define BLE_UUID_CHAR_TIME_ZONE                    (0x2A0Eu)
#define BLE_UUID_CHAR_LOCAL_TIME_INFO              (0x2A0Fu)
#define BLE_UUID_CHAR_SEC_TIME_ZONE                (0x2A10u)
#define BLE_UUID_CHAR_TIME_WITH_DST                (0x2A11u)
#define BLE_UUID_CHAR_TIME_ACCURACY                (0x2A12u)
#define BLE_UUID_CHAR_TIME_SOURCE                  (0x2A13u)
#define BLE_UUID_CHAR_REF_TIME_INFO                (0x2A14u)
#define BLE_UUID_CHAR_TIME_BROADCAST               (0x2A15u)
#define BLE_UUID_CHAR_TIME_UPDATE_CONTROL_POINT    (0x2A16u)
#define BLE_UUID_CHAR_TIME_UPDATE_STATE            (0x2A17u)
#define BLE_UUID_CHAR_GLUCOSE_MSRMNT               (0x2A18u)
#define BLE_UUID_CHAR_BATTERY_LEVEL                (0x2A19u)
#define BLE_UUID_CHAR_TEMPERATURE_MSMNT            (0x2A1Cu)
#define BLE_UUID_CHAR_TEMPERATURE_TYPE             (0x2A1Du)
#define BLE_UUID_CHAR_INTERMEDIATE_TEMP            (0x2A1Eu)
#define BLE_UUID_CHAR_MSMNT_INTERVAL               (0x2A21u)
#define BLE_UUID_CHAR_HIDS_BOOT_KEYBOARD_IN_RPT    (0x2A22u)
#define BLE_UUID_CHAR_SYSTEM_ID                    (0x2A23u)
#define BLE_UUID_CHAR_MODEL_NUMBER                 (0x2A24u)
#define BLE_UUID_CHAR_SERIAL_NUMBER                (0x2A25u)
#define BLE_UUID_CHAR_FIRMWARE_REV                 (0x2A26u)
#define BLE_UUID_CHAR_HARDWARE_REV                 (0x2A27u)
#define BLE_UUID_CHAR_SOFTWARE_REV                 (0x2A28u)
#define BLE_UUID_CHAR_MANUFACTURER_NAME            (0x2A29u)
#define BLE_UUID_CHAR_REG_CERT_DATA                (0x2A2Au)
#define BLE_UUID_CHAR_CURRENT_TIME                 (0x2A2Bu)
#define BLE_UUID_CHAR_MAGNETIC_DECLINATION         (0x2A2Cu)
#define BLE_UUID_CHAR_SCAN_REFRESH                 (0x2A31u)
#define BLE_UUID_CHAR_HIDS_BOOT_KEYBOARD_OUT_RPT   (0x2A32u)
#define BLE_UUID_CHAR_HIDS_BOOT_MOUSE_IN_RPT       (0x2A33u)
#define BLE_UUID_CHAR_GL_MSRMT_CTX                 (0x2A34u)
#define BLE_UUID_CHAR_BP_MSRMT                     (0x2A35u)
#define BLE_UUID_CHAR_INTRMDT_CUFF_PRSR            (0x2A36u)
#define BLE_UUID_CHAR_HR_MSRMT                     (0x2A37u)
#define BLE_UUID_CHAR_BODY_SENSOR_LOC              (0x2A38u)
#define BLE_UUID_CHAR_HR_CNTRL_POINT               (0x2A39u)
#define BLE_UUID_CHAR_NWA                          (0x2A3Eu)
#define BLE_UUID_CHAR_ALERT_STATUS                 (0x2A3Fu)
#define BLE_UUID_CHAR_RINGER_CONTROL_POINT         (0x2A40u)
#define BLE_UUID_CHAR_RINGER_SETTING               (0x2A41u)
#define BLE_UUID_CHAR_ALERT_ID_BIT_MASK            (0x2A42u)
#define BLE_UUID_CHAR_ALERT_ID                     (0x2A43u)
#define BLE_UUID_CHAR_ALERT_NTF_CONTROL_POINT      (0x2A44u)
#define BLE_UUID_CHAR_UNREAD_ALRT_STATUS           (0x2A45u)
#define BLE_UUID_CHAR_NEW_ALERT                    (0x2A46u)
#define BLE_UUID_CHAR_SPRTD_NEW_ALRT_CTGRY         (0x2A47u)
#define BLE_UUID_CHAR_SPRT_UNRD_ALRT_CTGRY         (0x2A48u)
#define BLE_UUID_CHAR_BP_FEATURE                   (0x2A49u)
#define BLE_UUID_CHAR_HIDS_INFORMATION             (0x2A4Au)
#define BLE_UUID_CHAR_HIDS_REPORT_MAP              (0x2A4Bu)
#define BLE_UUID_CHAR_HIDS_CONTROL_POINT           (0x2A4Cu)
#define BLE_UUID_CHAR_HIDS_REPORT                  (0x2A4Du)
#define BLE_UUID_CHAR_HIDS_PROTOCOL_MODE           (0x2A4Eu)
#define BLE_UUID_CHAR_SCAN_WINDOW                  (0x2A4Fu)
#define BLE_UUID_CHAR_PNP_ID                       (0x2A50u)
#define BLE_UUID_CHAR_GL_FEATURE                   (0x2A51u)
#define BLE_UUID_CHAR_RACP                         (0x2A52u) /* Record Access Control Point */
#define BLE_UUID_CHAR_RSC_MSRMT                    (0x2A53u)
#define BLE_UUID_CHAR_RSC_FEATURE                  (0x2A54u)
#define BLE_UUID_CHAR_SC_CONTROL_POINT             (0x2A55u)
#define BLE_UUID_CHAR_DIGITAL                      (0x2A56u)
#define BLE_UUID_CHAR_ANALOG                       (0x2A58u)
#define BLE_UUID_CHAR_AGGREGATE                    (0x2A5Au)
#define BLE_UUID_CHAR_CSC_MSRMT                    (0x2A5Bu)
#define BLE_UUID_CHAR_CSC_FEATURE                  (0x2A5Cu)
#define BLE_UUID_CHAR_SENSOR_LOCATION              (0x2A5Du)
#define BLE_UUID_CHAR_CONTINUOUS_MEASUREMENT       (0x2A5Fu)
#define BLE_UUID_CHAR_FEATURES                     (0x2A60u)
#define BLE_UUID_CHAR_CPS_MSRMT                    (0x2A63u)
#define BLE_UUID_CHAR_CPS_VECTOR                   (0x2A64u)
#define BLE_UUID_CHAR_CPS_FEATURE                  (0x2A65u)
#define BLE_UUID_CHAR_CPS_CP                       (0x2A66u)
#define BLE_UUID_CHAR_LOCATION_AND_SPEED           (0x2A67u)
#define BLE_UUID_CHAR_NAVIGATION                   (0x2A68u)
#define BLE_UUID_CHAR_POSITION_QUALITY             (0x2A69u)
#define BLE_UUID_CHAR_LN_FEATURE                   (0x2A6Au)
#define BLE_UUID_CHAR_LN_CONTROL_POINT             (0x2A6Bu)
#define BLE_UUID_CHAR_ELEVATION                    (0x2A6Cu)
#define BLE_UUID_CHAR_PRESSURE                     (0x2A6Du)
#define BLE_UUID_CHAR_THEMPERATURE                 (0x2A6Eu)
#define BLE_UUID_CHAR_HUMIDITY                     (0x2A6Fu)
#define BLE_UUID_CHAR_TRUE_WIND_SPEED              (0x2A70u)
#define BLE_UUID_CHAR_TRUE_WIND_DIRECTION          (0x2A71u)
#define BLE_UUID_CHAR_APPARENT_WIND_SPEED          (0x2A72u)
#define BLE_UUID_CHAR_APPARENT_WIND_DIRECTION      (0x2A73u)
#define BLE_UUID_CHAR_GUST_FACTOR                  (0x2A74u)
#define BLE_UUID_CHAR_POLLEN_CONCENTRATION         (0x2A75u)
#define BLE_UUID_CHAR_UV_INDEX                     (0x2A76u)
#define BLE_UUID_CHAR_IRRADIANCE                   (0x2A77u)
#define BLE_UUID_CHAR_RAINFALL                     (0x2A78u)
#define BLE_UUID_CHAR_WIND_CHILL                   (0x2A79u)
#define BLE_UUID_CHAR_HEAT_INDEX                   (0x2A7Au)
#define BLE_UUID_CHAR_DEW_POINT                    (0x2A7Bu)
#define BLE_UUID_CHAR_DESCR_VALUE_CHANGED          (0x2A7Du)
#define BLE_UUID_CHAR_AEL                          (0x2A7Eu) /* Aerobic Heart Rate Lower Limit */
#define BLE_UUID_CHAR_AEROBIC_THRESHOLD            (0x2A7Fu)
#define BLE_UUID_CHAR_AGE                          (0x2A80u)
#define BLE_UUID_CHAR_ANL                          (0x2A81u) /* Anaerobic Heart Rate Lower Limit */
#define BLE_UUID_CHAR_ANU                          (0x2A82u) /* Anaerobic Heart Rate Upper Limit */
#define BLE_UUID_CHAR_ANAEROBIC_THRESHOLD          (0x2A83u)
#define BLE_UUID_CHAR_AEU                          (0x2A84u) /* Anaerobic Heart Rate Upper Limit */
#define BLE_UUID_CHAR_DATE_OF_BIRTH                (0x2A85u)
#define BLE_UUID_CHAR_DATE_OF_THRESHOLD_ASSESSMENT (0x2A86u)
#define BLE_UUID_CHAR_EMAIL_ADDRESS                (0x2A87u)
#define BLE_UUID_CHAR_FBL                          (0x2A88u) /* Fat Burn Heart Rate Lower Limit */
#define BLE_UUID_CHAR_FBU                          (0x2A89u) /* Fat Burn Heart Rate Upper Limit */
#define BLE_UUID_CHAR_FIRST_NAME                   (0x2A8Au)
#define BLE_UUID_CHAR_FIVE_ZONE_HEART_RATE_LIMITS  (0x2A8Bu)
#define BLE_UUID_CHAR_GENDER                       (0x2A8Cu)
#define BLE_UUID_CHAR_HEART_RATE_MAX               (0x2A8Du)
#define BLE_UUID_CHAR_HEIGHT                       (0x2A8Eu)
#define BLE_UUID_CHAR_HIP_CIRCUNFERENCE            (0x2A8Fu)
#define BLE_UUID_CHAR_LAST_NAME                    (0x2A90u)
#define BLE_UUID_CHAR_MRH                          (0x2A91u) /* Maximum Recommended Heart Rate */
#define BLE_UUID_CHAR_RESTING_HEART_RATE           (0x2A92u)
#define BLE_UUID_CHAR_STP                          (0x2A93u) /* Sport Type for Aerobic and Anaerobic Thresholds */
#define BLE_UUID_CHAR_THREE_ZONE_HEART_RATE_LIMITS (0x2A94u)
#define BLE_UUID_CHAR_TWO_ZONE_HEART_RATE_LIMIT    (0x2A95u)
#define BLE_UUID_CHAR_VO2_MAX                      (0x2A96u)
#define BLE_UUID_CHAR_WAIST_CIRCUMFERENCE          (0x2A97u)
#define BLE_UUID_CHAR_WEIGHT                       (0x2A98u)
#define BLE_UUID_CHAR_DATABASE_CHANGE_INCREMENT    (0x2A99u)
#define BLE_UUID_CHAR_USER_INDEX                   (0x2A9Au)
#define BLE_UUID_CHAR_BODY_COMPOSITION_FEATURE     (0x2A9Bu)
#define BLE_UUID_CHAR_BODY_COMPOSITION_MEASUREMENT (0x2A9Cu)
#define BLE_UUID_CHAR_WEIGHT_MEASUREMENT           (0x2A9Du)
#define BLE_UUID_CHAR_WEIGHT_SCALE_FEATURE         (0x2A9Eu)
#define BLE_UUID_CHAR_USER_CONTROL_POINT           (0x2A9Fu)
#define BLE_UUID_CHAR_MAGNETIC_FLUX_DENSITY_2D     (0x2AA0u)
#define BLE_UUID_CHAR_MAGNETIC_FLUX_DENSITY_3D     (0x2AA1u)
#define BLE_UUID_CHAR_LANGUAGE                     (0x2AA2u)
#define BLE_UUID_CHAR_BAR_PRESSURE_TREND           (0x2AA3u)
/* Bond Management Characteristics defines */
#define BLE_UUID_BOND_MANAGEMENT_CONTROL_POINT     (0x2AA4u)
#define BLE_UUID_BOND_MANAGEMENT_FEATURE           (0x2AA5u)
/* CGM Characteristics defines */
#define BLE_UUID_CHAR_CGM_MEASUREMENT              (0x2AA7u)
#define BLE_UUID_CHAR_CGM_FEATURE                  (0x2AA8u)
#define BLE_UUID_CHAR_CGM_STATUS                   (0x2AA9u)
#define BLE_UUID_CHAR_CGM_SESSION_START_TIME       (0x2AAAu)
#define BLE_UUID_CHAR_CGM_SESSION_RUN_TIME         (0x2AABu)
#define BLE_UUID_CHAR_CGM_SOCP                     (0x2AACu) /* CGM Specific Ops Control Point */
/* Pulse Oximeter Characteristics defines */
#define BLE_UUID_CHAR_PLX_SPOT_CHK_MSRMT           (0x2A5Eu) /**< PLX Spot-Check Measurement Characteristic UUID */
#define BLE_UUID_CHAR_PLX_CONTINUOUS_MSRMT         (0x2A5Fu) /**< PLX Continuous Measurement Characteristic UUID */
#define BLE_UUID_CHAR_PLX_FEATURES                 (0x2A60u) /**< PLX Features Characteristic UUID */
/* CPM Characteristics Measurements */
#define BLE_UUID_CHAR_CPM_MSRMT                    (0x2A63u)
#define BLE_UUID_CHAR_CPM_VECTOR                   (0x2A64u)
#define BLE_UUID_CHAR_CPM_FEATURE                  (0x2A65u)
#define BLE_UUID_CHAR_CPM_CNTRL_PNT                (0x2A66u)
/* IPS Characteristics defines */
#define BLE_UUID_CHAR_INDOOR_POSITINING_CONFIG     (0x2AADu)
#define BLE_UUID_CHAR_LATITUDE                     (0x2AAEu)  
#define BLE_UUID_CHAR_LONGITUDE                    (0x2AAFu)   
#define BLE_UUID_CHAR_LOCAL_NORTH_COORDINATE       (0x2AB0u)     
#define BLE_UUID_CHAR_LOCAL_EAST_COORDINATE        (0x2AB1u)   
#define BLE_UUID_CHAR_FLOOR_NUMBER                 (0x2AB2u)    
#define BLE_UUID_CHAR_ALTITUDE                     (0x2AB3u)   
#define BLE_UUID_CHAR_UNCERTAINTY                  (0x2AB4u)       
#define BLE_UUID_CHAR_LOCATION_NAME                (0x2AB5u)
/* HPS Characteristics defines */
#define BLE_UUID_CHAR_URI                          (0x2AB6u)
#define BLE_UUID_CHAR_HTTP_HEADERS                 (0x2AB7u)
#define BLE_UUID_CHAR_HTTP_ENTITY_BODY             (0x2AB9u)
#define BLE_UUID_CHAR_HTTP_CP                      (0x2ABAu)
#define BLE_UUID_CHAR_HTTP_STATUS_CODE             (0x2AB8u)
#define BLE_UUID_CHAR_HTTPS_SECURITY               (0x2ABBu)

/* OTS Characteristics defines */
#define BLE_UUID_CHAR_OBJECT_FEATURE               (0x2ABDu)
#define BLE_UUID_CHAR_OBJECT_NAME                  (0x2ABEu)
#define BLE_UUID_CHAR_OBJECT_TYPE                  (0x2ABFu)
#define BLE_UUID_CHAR_OBJECT_SIZE                  (0x2AC0u)
#define BLE_UUID_CHAR_OBJECT_FIRST_CREATED         (0x2AC1u)
#define BLE_UUID_CHAR_OBJECT_LAST_MODIFIED         (0x2AC2u)
#define BLE_UUID_CHAR_OBJECT_ID                    (0x2AC3u)
#define BLE_UUID_CHAR_OBJECT_PROPERTIES            (0x2AC4u)
#define BLE_UUID_CHAR_OBJECT_ACTION_CONTROL_POINT  (0x2AC5u)
#define BLE_UUID_CHAR_OBJECT_LIST_CONTROL_POINT    (0x2AC6u)
#define BLE_UUID_CHAR_OBJECT_LIST_FILTER           (0x2AC7u)
#define BLE_UUID_CHAR_OBJECT_CHANGED               (0x2AC8u)

/* GATT Characteristic Properties bit field */
#define BLE_CHAR_PROP_BROADCAST                    (0x01u)
#define BLE_CHAR_PROP_READ                         (0x02u)
#define BLE_CHAR_PROP_WRITE_WITHOUT_RSP            (0x04u)   /* Write Without Response */
#define BLE_CHAR_PROP_WRITE                        (0x08u)
#define BLE_CHAR_PROP_NOTIFY                       (0x10u)
#define BLE_CHAR_PROP_INDICATE                     (0x20u)
#define BLE_CHAR_PROP_SIGNED_WRITE                 (0x40u)   /* Authenticated Signed Writes */
#define BLE_CHAR_PROP_EXTENDED_PROPERTIES          (0x80u)

/* Client Characteristic Configuration Descriptor values */
#define BLE_CCCD_DEFAULT                           (0x0000u)
#define BLE_CCCD_NOTIFICATION                      (0x0001u)
#define BLE_CCCD_INDICATION                        (0x0002u)
#define BLE_CCCD_LEN                               (0x02u)

/* Server Characteristic Configuration Descriptor values */
#define BLE_SCCD_DEFAULT                           (0x0000u)
#define BLE_SCCD_BROADCAST                         (0x0001u)
#define BLE_SCCD_LEN                               (0x02u)

/* Characteristic Presentation Format Descriptor values */
#define BLE_CPFD_BOOLEAN                           (0x01u)
#define BLE_CPFD_UINT2                             (0x02u)
#define BLE_CPFD_UINT4                             (0x03u)
#define BLE_CPFD_UINT8                             (0x04u)
#define BLE_CPFD_UINT12                            (0x05u)
#define BLE_CPFD_UINT16                            (0x06u)
#define BLE_CPFD_UINT24                            (0x07u)
#define BLE_CPFD_UINT32                            (0x08u)
#define BLE_CPFD_UINT48                            (0x09u)
#define BLE_CPFD_UINT64                            (0x0au)
#define BLE_CPFD_UINT128                           (0x0bu)
#define BLE_CPFD_INT8                              (0x0cu)
#define BLE_CPFD_INT12                             (0x0du)
#define BLE_CPFD_INT16                             (0x0eu)
#define BLE_CPFD_INT24                             (0x0fu)
#define BLE_CPFD_INT32                             (0x10u)
#define BLE_CPFD_INT48                             (0x11u)
#define BLE_CPFD_INT64                             (0x12u)
#define BLE_CPFD_INT128                            (0x13u)
#define BLE_CPFD_IEEE754_FP32                      (0x14u)
#define BLE_CPFD_IEEE754_FP64                      (0x15u)
#define BLE_CPFD_IEEE11073_SFLOAT16                (0x16u)
#define BLE_CPFD_IEEE11073_FLOAT32                 (0x17u)
#define BLE_CPFD_IEEE20601_FORMAT                  (0x18u)
#define BLE_CPFD_UTF8_STRING                       (0x19u)
#define BLE_CPFD_UTF16_STRING                      (0x1au)
#define BLE_CPFD_OPAQUE_STRUCTURE                  (0x1bu)
#define BLE_CPFD_LEN                               (0x01u)

/* Characteristic Extended Properties Descriptor values */
#define BLE_CEPD_DEFAULT                           (0x0000u)
#define BLE_CEPD_RELIABLE_WRITE                    (0x0001u)
#define BLE_CEPD_WRITABLE_AUXILIARIES              (0x0002u)
#define BLE_CEPD_LEN                               (0x02u)

#define BLE_ATTR_HANDLE_LEN                        (0x02u)
#define BLE_DB_ATTR_HANDLE_LEN                     (0x02u)

/* GATT Server Configuration values */
#define BLE_GATT_SER_CNFG_BROADCAST                (0x0001u)   /* Broadcasts enabled */
#define BLE_GATT_SER_CNFG_DEFAULT                  (0x0000u)

#define BLE_CHAR_EXT_PRPTS_DESCR_RLWR              (0x0001u)  /* Reliable Write enabled */
#define BLE_CHAR_EXT_PRPTS_DESCR_WRAUX             (0x0002u)  /* Writable Auxiliaries enabled */

/* PDU data length */
#define BLE_PDU_DATA_LEN_ZERO                      (0x00u)
#define BLE_PDU_DATA_LEN_ONE_BYTE                  (0x01u)
#define BLE_PDU_DATA_LEN_TWO_BYTES                 (0x02u)
#define BLE_PDU_DATA_LEN_THREE_BYTES               (0x03u)
#define BLE_PDU_DATA_LEN_FOUR_BYTES                (0x04u)
#define BLE_PDU_DATA_LEN_FIVE_BYTES                (0x05u)
#define BLE_PDU_DATA_LEN_SIX_BYTES                 (0x06u)
#define BLE_PDU_DATA_LEN_SEVEN_BYTES               (0x07u)
#define BLE_PDU_DATA_LEN_EIGHT_BYTES               (0x08u)
#define BLE_PDU_DATA_LEN_NINE_BYTES                (0x09u)


/***************************************
* Bluetooth Appearance values
***************************************/

#define BLE_APPEARANCE_UNKNOWN                            (0u) /* Unknown */
#define BLE_APPEARANCE_GENERIC_PHONE                      (64u) /* Generic Phone */
#define BLE_APPEARANCE_GENERIC_COMPUTER                   (128u) /* Generic Computer */
#define BLE_APPEARANCE_GENERIC_WATCH                      (192u) /* Generic Watch */
#define BLE_APPEARANCE_WATCH_SPORTS_WATCH                 (193u) /* Watch: Sports Watch */
#define BLE_APPEARANCE_GENERIC_CLOCK                      (256u) /* Generic Clock */
#define BLE_APPEARANCE_GENERIC_DISPLAY                    (320u) /* Generic Display */
#define BLE_APPEARANCE_GENERIC_REMOTE_CONTROL             (384u) /* Generic Remote Control */
#define BLE_APPEARANCE_GENERIC_EYE_GLASSES                (448u) /* Generic Eye-glasses */
#define BLE_APPEARANCE_GENERIC_TAG                        (512u) /* Generic Tag */
#define BLE_APPEARANCE_GENERIC_KEYRING                    (576u) /* Generic Keyring */
#define BLE_APPEARANCE_GENERIC_MEDIA_PLAYER               (640u) /* Generic Media Player */
#define BLE_APPEARANCE_GENERIC_BARCODE_SCANNER            (704u) /* Generic Barcode Scanner */
#define BLE_APPEARANCE_GENERIC_THERMOMETER                (768u) /* Generic Thermometer */
#define BLE_APPEARANCE_THERMOMETER_EAR                    (769u) /* Thermometer: Ear */
#define BLE_APPEARANCE_GENERIC_HEART_RATE_SENSOR          (832u) /* Generic Heart rate Sensor */
#define BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT  (833u) /* Heart Rate Sensor: Heart Rate Belt */
#define BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE             (896u) /* Generic Glucose */
#define BLE_APPEARANCE_BLOOD_PRESSURE_ARM                 (897u) /* Glucose: Arm */
#define BLE_APPEARANCE_BLOOD_PRESSURE_WRIST               (898u) /* Glucose: Wrist */
#define BLE_APPEARANCE_GENERIC_HID                        (960u) /* Human Interface Device (HID) */
#define BLE_APPEARANCE_HIDS_KEYBOARD                      (961u) /* Keyboard (HID Subtype) */
#define BLE_APPEARANCE_HIDS_MOUSE                         (962u) /* Mouse (HID Subtype) */
#define BLE_APPEARANCE_HIDS_JOYSTICK                      (963u) /* Joystiq (HID Subtype) */
#define BLE_APPEARANCE_HIDS_GAMEPAD                       (964u) /* Gamepad (HID Subtype) */
#define BLE_APPEARANCE_HIDS_DIGITIZERSUBTYPE              (965u) /* Digitizer Tablet (HID Subtype) */
#define BLE_APPEARANCE_HIDS_CARD_READER                   (966u) /* Card Reader (HID Subtype) */
#define BLE_APPEARANCE_HIDS_DIGITAL_PEN                   (967u) /* Digital Pen (HID Subtype) */
#define BLE_APPEARANCE_HIDS_BARCODE                       (968u) /* Barcode Scanner (HID Subtype) */
#define BLE_APPEARANCE_GENERIC_GLUCOSE_METER              (1024u) /* Generic Glucose Meter */
#define BLE_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR     (1088u) /* Generic Running Walking Sensor */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_IN_SHOE     (1089u) /* Running Walking Sensor: In-Shoe */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_ON_SHOE     (1090u) /* Running Walking Sensor: On-Shoe */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_ON_HIP      (1091u) /* Running Walking Sensor: On-Hip */
#define BLE_APPEARANCE_GENERIC_CYCLING                    (1152u) /* Generic Cycling */
#define BLE_APPEARANCE_CYCLING_CYCLING_COMPUTER           (1153u) /* Cycling: Cycling Computer */
#define BLE_APPEARANCE_CYCLING_SPEED_SENSOR               (1154u) /* Cycling: Speed Sensor */
#define BLE_APPEARANCE_CYCLING_CADENCE_SENSOR             (1155u) /* Cycling: Cadence Sensor */
#define BLE_APPEARANCE_CYCLING_POWER_SENSOR               (1156u) /* Cycling: Power Sensor */
#define BLE_APPEARANCE_CYCLING_SPEED_CADENCE_SENSOR       (1157u) /* Cycling: Speed and Cadence Sensor */


/***************************************
* SMP Key size constants
***************************************/

#define BLE_SMP_USER_PASSKEY_SIZE                  (6u)    /* User Passkey size */
#define BLE_SMP_IDADDR_SIZE                        (7u)    /* Identity Address data size */
#define BLE_SMP_IRK_SIZE                           (16u)   /* Identity Resolving Key size */
#define BLE_SMP_CSRK_SIZE                          (16u)   /* Connection Signature Resolving key size */
#define BLE_SMP_LTK_SIZE                           (16u)   /* Long Term Key Size size */
#define BLE_SMP_EDIV_SIZE                          (2u)    /* Encrypted Diversifier size */
#define BLE_SMP_RAND_24B_SIZE                      (3u)    /* 24 Bit Random Number size */
#define BLE_SMP_RAND_64B_SIZE                      (8u)    /* 64 Bit Random Number size */
#define BLE_SMP_RAND_128B_SIZE                     (16u)   /* 128 Bit Random Number size */

#define BLE_GATT_16_BIT_UUID_LO_IDX                (3u)
#define BLE_GATT_16_BIT_UUID_HI_IDX                (2u)

#define BLE_8_BIT                                  (8u)
#define BLE_TWO_BYTES_SHIFT                        (2u)
#define BLE_NULL_CHARCTER                          ((char8) (0))


/** Attribute value type used in GATT database */
typedef struct {
        /** Length in number of bytes for attGenericVal */
        uint16_t actualLength;

        /** Buffer to the store generic characteristic value based on
         length or complete UUID value if the attribute is of type 128-bit
        UUID and 32-bit UUID type. */
        void * attGenericVal;
} BLE_GATT_GEN_VAL_LEN_T;

/** Attribute value type used in GATT database */
typedef struct {
        /** Length in number of bytes for attGenericVal */
        uint16_t maxAttrLength;

        /** Buffer to the store generic characteristic value based on
         length or complete UUID value if the attribute is of type 128-bit
        UUID and 32-bit UUID type. */
        BLE_GATT_GEN_VAL_LEN_T * attGenericValLen;
} BLE_GATT_PACK_VAL_LEN_T;

/** Attribute value type used in GATT database */
typedef union {    
        /** Buffer containing 32-bit or 128-bit UUID values for Service and
         Characteristic declaration.
        Attribute format structure: if entry is for characteristic value format, 
        then it has the "attribute format value" of pointer type to represent generic
        structure to cater wide formats of available list of characteristic formats. */
        BLE_GATT_PACK_VAL_LEN_T  	attFormatValue;

        /** Attribute UUID value */
        uint16_t 				attValueUuid;
} BLE_GATT_VALUE_T;

/** GATT database structure used in the GAP Server */
typedef struct {
        /** Start Handle: Act as an index for querying BLE GATT database */
        uint16_t 				attHandle;

        /** UUID: 16 bit UUID type for an attribute entry, for 32 bit and
         128 bit UUIDs the last 16 bits should be stored in this entry
        GATT DB access layer shall retrieve complete 128 bit UUID from
        BLE_GATT_GENERIC_VAL_T structure. */
        uint16_t 				attType;

        /** The permission bits are clubbed in to a 32-bit field. These 
        32-bits can be grouped in to 4 bytes. The lowest significant byte
        is byte 0 (B0) and the most significant byte is byte 3 (B3). The 
        bytes where the permissions have been grouped is as given below.
                * Attribute permissions for read (B0)
                * Attribute permissions for write (B1)
                * Characteristic properties (B2)
                * Implementation specific permission (B3)
                */
        uint32_t 				permission;

        /** Attribute end handle, indicating logical boundary of given attribute. */
        uint16_t 				attEndHandle;

        /** Attribute value format, it can be one of following:
        * uint16 16bit - UUID for 16bit service & characteristic declaration
        * BLE_GATTS_ATT_GENERIC_VAL_T attFormatValue - Buffer containing 32 bit
                or 128 bit UUID values for service & characteristic declaration
        * BLE_GATTS_ATT_GENERIC_VAL_T attFormatValue - Buffer containing generic 
                char definition value, or generic descriptor values
        */
        BLE_GATT_VALUE_T 	attValue;
 } BLE_GATT_DB_T;

extern void ble_setup();
extern void ble_loop();

#endif
