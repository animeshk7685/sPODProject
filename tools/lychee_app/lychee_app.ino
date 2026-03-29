#include "ble.h"
#include <stdbool.h>
#include <esp_err.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_image_format.h>
#include <esp_app_format.h>
#include <esp_ota_ops.h>
#include <esp_assert.h>
#include "crc16.h"
#include "crc32.h"
#include "spod_library.h"
#include "ota.h"


SET_LOOP_TASK_STACK_SIZE(16*1024) // default stack size is 4K (which is probably enough!)

#define VERSION_OFFSET 0x120
#define VERSION_SIZE   16

// place the version of the firmware (including type, e.g. "PDB", "SDB", "TSB", for "sPOD", "switch", and "touchscreen")
// at a known location (offset 0x120) in the beginning of flash/binary image 
// this immediately follows the esp_image_header_t, esp_segment_header_t and esp_app_desc_t
// structures which are 24, 8 and 256 bytes respectively
//
// note: because arduino uses pre-compiled esp-idf libraries we could not take advantage of
// the esp_app_desc_t.version field 
const __attribute__((section(".rodata_custom_desc"))) char VERSION[VERSION_SIZE] = {"APP-01.00.01"};


#define ERROR_CHECK(x) do {                   \
        esp_err_t err_rc_ = (x);              \
        if (unlikely(err_rc_ != ESP_OK)) {    \
            panic(err_rc_, __LINE__);         \
        }                                     \
    } while(0)

#define CHUNK_SIZE 0x1000   // transfer 4Kbyte chunks with 32 128-byte packet writes
#define PKT_SIZE   0x80


static const uint8_t ble_attUuid128[][16u] = {
/*0-sPOD*/     { 0x5Bu, 0x68u, 0x56u, 0x60u, 0x9Fu, 0xE9u, 0x04u, 0x84u, 0x7Bu, 0x44u, 0x0Du, 0x8Cu, 0xECu, 0xF9u, 0x3Au, 0x7Eu },
/*1-COMM */    { 0xF4u, 0xC6u, 0x6Fu, 0x64u, 0x02u, 0xECu, 0x5Bu, 0xA6u, 0x6Fu, 0x4Cu, 0x00u, 0x4Cu, 0x64u, 0x47u, 0x06u, 0xB9u },
/*2-Passkey*/  { 0x86u, 0x74u, 0x26u, 0x03u, 0xEEu, 0x73u, 0x28u, 0x8Eu, 0xB7u, 0x45u, 0x5Cu, 0xF1u, 0xFBu, 0x52u, 0x9Eu, 0x1Bu },
/*3-OTA CTRL*/ { 0x75u, 0x16u, 0x74u, 0x8Eu, 0x97u, 0x60u, 0xE6u, 0xA6u, 0x9Eu, 0x43u, 0x93u, 0xFEu, 0xA5u, 0x93u, 0x89u, 0xDFu },
/*4-Pro Mode*/ { 0xCBu, 0xB8u, 0xC9u, 0xCCu, 0x22u, 0x77u, 0xD9u, 0xBFu, 0xA4u, 0x43u, 0xAFu, 0x9Bu, 0xC1u, 0x25u, 0x41u, 0x02u },
/*5-Unsecured*/{ 0xDEu, 0x6Cu, 0xFDu, 0xCBu, 0xA1u, 0xCCu, 0xE8u, 0x82u, 0x36u, 0x45u, 0x58u, 0x7Bu, 0xD9u, 0x57u, 0xBAu, 0xA0u },
/*6-OTA-DATA*/ { 0x55u, 0xA3u, 0xe0u, 0x28u, 0x19u, 0x5Cu, 0x69u, 0x90u, 0x58u, 0x4Cu, 0xAEu, 0xFEu, 0xA9u, 0x82u, 0x47u, 0xF9u },
};


static BLEUUID service_uuid((uint8_t*)ble_attUuid128[0], sizeof(ble_attUuid128[0]), false); // remote service to connect to
static BLEUUID comm_uuid((uint8_t*)ble_attUuid128[1], sizeof(ble_attUuid128[1]), false); // COMM characteristic UUID
static BLEUUID ota_ctrl_uuid((uint8_t*)ble_attUuid128[3], sizeof(ble_attUuid128[3]), false); // OTA characteristic UUID
static BLEUUID ota_data_uuid((uint8_t*)ble_attUuid128[6], sizeof(ble_attUuid128[6]), false); // OTA characteristic UUID

static boolean doScan = false;
static boolean doConnect = false;
static boolean connected = false;
static BLEAdvertisedDevice* myDevice;
static BLERemoteCharacteristic* comm_char;
BLERemoteCharacteristic* ota_data_char;
BLERemoteCharacteristic* ota_ctrl_char;

volatile int comm_ack = 0;
volatile int ota_data_ack = 0;
volatile int ota_ctrl_ack = 0;

uint8_t our_mac[6];

board_desc_t board_desc;
char* file_name;
File file;



int pretty_time_units(char* buffer, int sizeof_buffer, long* secs, long unit_secs, const char* unit_name)
{
    int count = *secs / unit_secs;
    if (count) {
        *secs %= unit_secs;
        snprintf(buffer, sizeof_buffer, "%d %s%s%s", count, unit_name, count > 1? "s":"", secs? ", ":"");
    } else {
        *buffer = '\0';
    }

    return strlen(buffer);
}


const char* pretty_time(long seconds)
{
    static char buffer[128];
    int length = 0;
    length += pretty_time_units(buffer+length, sizeof(buffer)-length, &seconds, 365*24*60*60, "year");
    length += pretty_time_units(buffer+length, sizeof(buffer)-length, &seconds, 24*60*60, "day");
    length += pretty_time_units(buffer+length, sizeof(buffer)-length, &seconds, 60*60, "hour");
    length += pretty_time_units(buffer+length, sizeof(buffer)-length, &seconds, 60, "minute");
    if (seconds || length == 0) {
        snprintf(buffer+length, sizeof(buffer)-length, "%ld second%s", seconds, seconds == 1? "":"s");
    }
    return buffer;
}


static void panic(esp_err_t err, int lineno)
{
    Serial.printf("PANIC @ line %d, err=%d\n", lineno, err);
    Serial.flush();
    for(;;) continue;
}


static void comm_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  comm_ack = 1;
}


static void ota_data_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  ota_data_ack = 1;
}


static void ota_ctrl_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  ota_ctrl_ack = 1;
}


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* client) {
    Serial.println("Connected");
  }

  void onDisconnect(BLEClient* client) {
    connected = false;
    Serial.print(client->getPeerAddress().toString().c_str());
    Serial.println(" Disconnected");
  }

  uint32_t onPassKeyRequest() {
    Serial.println("Client Passkey Request");
    return 123456;
  }

  bool onConfirmPIN(uint32_t pass_key) {
    Serial.print("The passkey YES/NO number: ");
    Serial.println(pass_key);
    return true; // return true if passkeys match, false otherwise
  }

  void onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (!desc->sec_state.encrypted) {
      Serial.println("Encrypt connection failed -- disconnecting");
      BLEDevice::getClientByID(desc->conn_handle)->disconnect();
      return;
    }
  }
};


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice* dev) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(dev->toString().c_str());

    Serial.printf("onResult(): haveServiceUUID()=%d, isAdvertisingService()=%d\n",
      dev->haveServiceUUID(),
      dev->isAdvertisingService(service_uuid));
  
    // found device, does it contain the required service?
    if (dev->haveServiceUUID() && dev->isAdvertisingService(service_uuid)) {
      Serial.printf("onResult(): doConnect --> true\n");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(*dev);
      doConnect = true;
      doScan = true;
    }
  }
};


static bool connectToServer()
{
    Serial.print("Forming connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  client  = BLEDevice::createClient();
    Serial.println(" - Created client");

    client->setClientCallbacks(new MyClientCallback());

    client->connect(myDevice);
    Serial.println(" - Connected to server");
    BLEDevice::setMTU(517); // set maximum MTU (default is 23)
  
    BLERemoteService* remote_service = client->getService(service_uuid);
    if (remote_service == nullptr) {
      Serial.print("Failed to find service UUID: ");
      Serial.println(service_uuid.toString().c_str());
      client->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    comm_char = remote_service->getCharacteristic(comm_uuid);
    if (comm_char == nullptr) {
      Serial.print("Failed to find COMM characteristic UUID: ");
      Serial.println(comm_uuid.toString().c_str());
      client->disconnect();
      return false;
    }

    if(comm_char->canNotify())
      comm_char->registerForNotify(comm_write_cb);

    ota_data_char = remote_service->getCharacteristic(ota_data_uuid);
    if (ota_data_char == nullptr) {
      Serial.print("Failed to find DATA characteristic UUID: ");
      Serial.println(ota_data_uuid.toString().c_str());
      client->disconnect();
      return false;
    }

    if(ota_data_char->canNotify())
      ota_data_char->registerForNotify(ota_data_write_cb);

    ota_ctrl_char = remote_service->getCharacteristic(ota_ctrl_uuid);
    if (ota_ctrl_char == nullptr) {
      Serial.print("Failed to find OTA characteristic UUID: ");
      Serial.println(ota_ctrl_uuid.toString().c_str());
      client->disconnect();
      return false;
    }
    ota_ctrl_char->registerForNotify(ota_ctrl_write_cb);

    Serial.printf("%s(): connected set to true!\n", __FUNCTION__);
    connected = true;
    return true;
}


static int file_count;

static bool fls_cb(uint8_t* smac, const char* name, size_t size)
{
    if (name != NULL) {
        Serial.printf("%s %d\r\n", name, fs_filesize(name));
        Serial.flush();
        file_count += 1;
    }
    return false;
}


static void list_files()
{
  file_count = 0;
  Serial.printf("---------------------------------------------\r\n");
  if (fs_list(LittleFS, NULL, fls_cb)) {
      Serial.printf("OKAY, list complete (%d files)\r\n", file_count);
  } else {
      Serial.printf("OOPS, unable to list files\r\n");
  }
}


static void show_board(board_short_info_t* info)
{
    Serial.printf("%s %02x:%02x:%02x v%d.%d.%d\r\n", 
        board_name_string(info->board_type, info->addr),
        info->mac[0], info->mac[1], info->mac[2],
        info->major, info->minor, info->revision);
}


static void ble_show_boards()
{
  uint8_t data[256];

  data[0] = 0x55;
  data[1] = 5;
  data[2] = 9;  // get board info
  uint32_t crc = crc32(0, data, 3);
  data[3] = crc & 0xFF;
  data[4] = crc >> 8;
  data[5] = crc >> 16;
  data[6] = crc >> 24;

  comm_ack = 0;
  comm_char->writeValue(data, 7);

  for (uint32_t start = millis(); !comm_ack && millis() - start < 8000; ) continue;
  if (!comm_ack) {
    Serial.printf("TIMEDOUT waiting for comm_ack!!!\r\n");
    Serial.flush();
  }

  size_t  leng = comm_char->getValue().size();
  if (leng > sizeof(data)) leng = sizeof(data);
  memcpy(data, comm_char->getValue().getValue(NULL), leng);

  board_short_info_t info[BOARD_MAX];
  memcpy((uint8_t*)info, data+3, sizeof(info));

  bool some = false;

  Serial.printf("------------------------------\r\n");
  for (int slot = 0; slot < BOARD_MAX; ++slot) {
      if (!pkt_mac_zero(info[slot].mac)) {
          show_board(info+slot);
          some = true;
      }
  }

  if (!some) {
      Serial.printf("No boards detected\r\n");
  }
}


static void show_help()
{
  Serial.printf("\r\nTest app:\r\n");
  Serial.printf("  help   -- print this text\r\n");
  Serial.printf("  boards -- show all the boards configured in the system\r\n");
  Serial.printf("  files  -- show all the files available\r\n");
  Serial.printf("  mymac  -- shows the MAC address of this board\r\n");
  Serial.printf("  ota <board> <file> [force]-- download the binary image to the specified board\r\n");
  Serial.printf("    For example:\r\n");
  Serial.printf("      ota spod spodv1.bin\r\n");
  Serial.printf("      ota switch switchv2.bin\r\n");
  Serial.printf("      ota spod2 spodv1.bin\r\n\r\n");
}


static void run_shell()
{
  extern void do_ota(char* command);
  static char line[80];
  static int linex = 0;

  while (Serial.available()) {
    line[linex++] = Serial.read();
    if (line[linex-1] == '\n' || linex >= sizeof(line)) {
      line[linex-1] = '\0';
      Serial.printf("\r\n");
      if (strncmp(line, "mymac", 5) == 0) {
        Serial.printf("%s %02x:%02x:%02x:%02x:%02x:%02x\n", 
          VERSION, 
          our_mac[0], our_mac[1], our_mac[2], 
          our_mac[3], our_mac[4], our_mac[5]);
      } else
      if (strncmp(line, "ota ", 4) == 0) {
        do_ota(line);
      } else
      if (strncmp(line, "boards", 6) == 0) {
        //show_boards();
        ble_show_boards();
      } else
      if (strncmp(line, "files", 5) == 0) {
        list_files();
      } else {
        show_help();
      }
      linex = 0;
    }
  }
}


void loop()
{
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("Connected to the BLE Server.");
    } else {
      Serial.println("Failed to connect to BLE server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  uint8_t owner;
  while (pkt_get(&owner)) {
    uint8_t ptype = pkt_type(NULL) & MASK_PKT_TYPE;
    if (ptype == CAN_PKT_TYPE) {
      dump_can_packet(pkt_data(NULL));
    } else {
      pkt_process(owner);
    }
    pkt_release();
  }

  if (connected) {
    run_shell();
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    delay(5000); // Delay a second between loops.
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.printf("%s(): VERSION='%s'\n", __FUNCTION__, VERSION);
  Serial.flush();
  fs_init();

  esp_read_mac(our_mac, ESP_MAC_BT);
  Serial.printf("%s() -- %s, %02x:%02x:%02x:%02x:%02x:%02x\n", 
      __FUNCTION__, VERSION, 
      our_mac[0], our_mac[1], our_mac[2], 
      our_mac[3], our_mac[4], our_mac[5]);

  serial_init(5);
  pkt_init(our_mac, UNSPECIFIED_BOARD_TYPE);
  pkt_set_promiscuous(true);
  pkt_process_init(VERSION, NULL, NULL);

  Serial.printf("\n");
  Serial.println("BLE client setup...");
  BLEDevice::init("");
  BLEDevice::setSecurityAuth(true, true, true);
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  BLEDevice::setMTU(512);

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scan->setInterval(1349);
  scan->setWindow(449);
  scan->setActiveScan(true);
  scan->start(5, false);
}