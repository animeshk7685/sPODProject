//
// The purpose of this program is to test BLE OTA for the sPOD. This program basically
// OTAs an image of itself to the PDB. It determines the active partition and uses 
// the headers in that partition to clone to the PDB. Note, the PDB, SDB and TSB all
// contain a VERSION character array at offset 0x120 by using the following magic:
//
//     const __attribute__((section(".rodata_custom_desc"))) char VERSION[32] = {"PDB-01.00.01"};
//
// This string contains the board type ("PDB") and version number. This is used by the 
// OTA library to determine if this image is older/newer or the same.
//
// The initial version of this program took about 7.5 minutes to clone the image over BLE.
// That time has been improved to just under a minute by sending 4Kbytes of data without 
// waiting for an ACK. The data is sent as 32 130-byte packets, where the first 2 bytes are 
// the offset into the 4K image data and the last 128 bytes are the data. The OTA sub-sysem 
// maintains a 4Kbyte image cache and a 32-bit image scoreboard. When a 130-byte packet is 
// received, the bit corresponding to that packet's offset is set and the 128 bytes of data 
// are copied into the 4K image at the specified offset. After the host has sent all of the 
// data for the current 4K chunk a chunk complete packet is sent with the crc32 of the 4K image. 
// The PDB compares the received crc32 to the calculated one for the current 4K image chunk.
// If they match an "ACK" is returned and the host can send the next contiguous chunk of 4K data. 
// If not, an "NAK" is returned along with the 32-bit scoreboard and a 64-byte array of the crc16 
// values for each of the 32 packets. The host can use this to retransmit the missing or incorrect 
// packets.
//

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

SET_LOOP_TASK_STACK_SIZE(16*1024) // default stack size is 4K (which is probably enough!)

#define VERSION_OFFSET 0x120
#define VERSION_SIZE   32

// place the version of the firmware (including type, e.g. "PDB", "SDB", "TSB", for "sPOD", "switch", and "touchscreen")
// at a known location (offset 0x120) in the beginning of flash/binary image 
// this immediately follows the esp_image_header_t, esp_segment_header_t and esp_app_desc_t
// structures which are 24, 8 and 256 bytes respectively
//
// note: because arduino uses pre-compiled esp-idf libraries we could not take advantage of
// the esp_app_desc_t.version field 
const __attribute__((section(".rodata_custom_desc"))) char VERSION[VERSION_SIZE] = {"PDB-01.00.01"};


#define ERROR_CHECK(x) do {                   \
        esp_err_t err_rc_ = (x);              \
        if (unlikely(err_rc_ != ESP_OK)) {    \
            panic(err_rc_, __LINE__);         \
        }                                     \
    } while(0)


#define CHUNK_SIZE 0x1000   // transfer 4Kbyte chunks with 32 128-byte packet writes
#define PKT_SIZE   0x80


typedef enum {
  OTA_START, START_ACK, WRITE_CHUNK, CHUNK_ACK, OTA_COMPLETE, OTA_FAILED
} loader_state_t;


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
    /* COMM-DATA */
    { 0x55u, 0xA3u, 0xe0u, 0x28u, 0x19u, 0x5Cu, 0x69u, 0x90u, 0x58u, 0x4Cu, 0xAEu, 0xFEu, 0xA9u, 0x82u, 0x47u, 0xF9u },

};


static BLEUUID serviceUUID((uint8_t*)ble_attUuid128[0], sizeof(ble_attUuid128[0]), false); // remote service to connect to
static BLEUUID commUUID((uint8_t*)ble_attUuid128[1], sizeof(ble_attUuid128[1]), false); // COMM characteristic UUID
static BLEUUID otaUUID((uint8_t*)ble_attUuid128[3], sizeof(ble_attUuid128[3]), false); // OTA characteristic UUID
static BLEUUID dataUUID((uint8_t*)ble_attUuid128[6], sizeof(ble_attUuid128[6]), false); // OTA characteristic UUID

static boolean doScan = false;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* comm_char;
static BLERemoteCharacteristic* data_char;
static BLERemoteCharacteristic* ota_char;
static BLEAdvertisedDevice* myDevice;

static const esp_partition_t *running_partition;
static esp_image_metadata_t image_metadata;
static uint8_t image_data[CHUNK_SIZE];
static size_t image_offset;
static int comm_ack = 0;
static int data_ack = 0;
static int ota_ack = 0;
static loader_state_t state;
static uint32_t ota_start_time, ota_stop_time;
static uint64_t char_start;


#if 0
static uint32_t ms_to_ticks(uint32_t msecs)
{
    return msecs/portTICK_PERIOD_MS;
}


static void ms_delay(int msecs)
{
    vTaskDelay(ms_to_ticks(msecs));
}


static uint64_t get_microseconds()
{
    return esp_timer_get_time();
}
#endif


static int pretty_time_units(char* buffer, int sizeof_buffer, long* secs, long unit_secs, const char* unit_name)
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


// get the partition metadata, we use this to get the image_len (# bytes of image to OTA)
static esp_err_t get_partition_metadata(const esp_partition_t *partition, esp_image_metadata_t* metadata)
{
    const esp_partition_pos_t part_pos = {
        .offset = partition->address,
        .size = partition->size,
    };
    return esp_image_verify(ESP_IMAGE_VERIFY, &part_pos, metadata);
}


static void comm_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  comm_ack = 1;
}


static void data_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  data_ack = 1;
}


static void ota_write_cb(BLERemoteCharacteristic* pchar, uint8_t* pData, size_t length, bool isNotify)
{
  ota_ack = 1;
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


/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice* dev) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(dev->toString().c_str());

    Serial.printf("onResult(): haveServiceUUID()=%d, isAdvertisingService()=%d\n",
      dev->haveServiceUUID(),
      dev->isAdvertisingService(serviceUUID));
  
    // found device, does it contain the required service?
    if (dev->haveServiceUUID() && dev->isAdvertisingService(serviceUUID)) {
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
  
    BLERemoteService* remote_service = client->getService(serviceUUID);
    if (remote_service == nullptr) {
      Serial.print("Failed to find service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      client->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    comm_char = remote_service->getCharacteristic(commUUID);
    if (comm_char == nullptr) {
      Serial.print("Failed to find COMM characteristic UUID: ");
      Serial.println(commUUID.toString().c_str());
      client->disconnect();
      return false;
    }

    if(comm_char->canNotify())
      comm_char->registerForNotify(comm_write_cb);

    data_char = remote_service->getCharacteristic(dataUUID);
    if (data_char == nullptr) {
      Serial.print("Failed to find DATA characteristic UUID: ");
      Serial.println(dataUUID.toString().c_str());
      client->disconnect();
      return false;
    }

    if(data_char->canNotify())
      data_char->registerForNotify(data_write_cb);

    ota_char = remote_service->getCharacteristic(otaUUID);
    if (ota_char == nullptr) {
      Serial.print("Failed to find OTA characteristic UUID: ");
      Serial.println(otaUUID.toString().c_str());
      client->disconnect();
      return false;
    }
    ota_char->registerForNotify(ota_write_cb);

    connected = true;
    state = OTA_START;
    return true;
}


static void ota_start()
{
  uint8_t data[VERSION_SIZE];

  ota_start_time = millis();
  char_start = get_microseconds();

  data[0] = sizeof(data);
  data[1] = 0;
  data[2] = 0;
  ERROR_CHECK(esp_partition_read(running_partition, VERSION_OFFSET, data+2, sizeof(data)-2));
  memcpy(data+18, (uint8_t*)&(image_metadata.image_len), sizeof(uint32_t));

  Serial.printf("%d -- OTA PACKET:", millis());
  for (int i = 0; i < sizeof(data); ++i) {
    Serial.printf(" %02x", data[i]);
  }
  Serial.printf("\n");

  ota_char->writeValue(data, sizeof(data));
  state = START_ACK;
  comm_ack = 0;
  data_ack = 0;
  ota_ack = 0;
}


static void wait_for_start_ack()
{
  if (ota_ack) {
    uint8_t data[32];
    size_t  leng = ota_char->getValue().size();
    if (leng > sizeof(data)) leng = sizeof(data);
    memcpy(data, ota_char->getValue().getValue(NULL), leng);

    ota_ack = 0;
    uint64_t delta = get_microseconds() - char_start;
    if (data[2] == 'O' && data[3] == 'K') {
      Serial.printf("%d -- %s(): SUCCESS -- '%s' (%lldus)\n", millis(), __FUNCTION__, data+2, delta);
      image_offset = 0;
      state = WRITE_CHUNK;
    } else {
      Serial.printf("%d -- %s(): FAILED -- '%s' (%lldus)\n", millis(), __FUNCTION__, data+2, delta);
      state = OTA_FAILED;
    }
  }
}


static bool write_packet(uint16_t offset)
{
    uint8_t data[sizeof(offset)+PKT_SIZE];
    memcpy(data, (uint8_t*)&offset, sizeof(offset));
    memcpy(data+sizeof(offset), image_data + offset, PKT_SIZE);

    uint32_t retries = 0;
    while (!data_char->writeValue(data, sizeof(data), false)) {
      if (++retries < 5) {
        Serial.printf("%d -- %s(): writeValue() retry=%d, offset=%x image_offset=%x\n", millis(), __FUNCTION__, retries, offset, image_offset);
        ms_delay((retries*retries)*50); // wait an exponentially longer period of time to drain BLE output buffers
        continue;
      }
      Serial.printf("%d -- %s(): RETRIES EXHAUSED!\n", millis(), __FUNCTION__);
      return false;
    }

    ms_delay(10); // throttle transmitted BLE data (to avoid buffer overflow)
    return true;
}


static void chunk_complete()
{
  uint8_t cdata[11] = {0x55, sizeof(cdata)-2, 9};
  uint32_t image_crc = crc32(0, image_data, sizeof(image_data));
  memcpy(cdata+3, (uint8_t*)&image_crc, sizeof(image_crc));
  uint32_t crc = crc32(0, cdata, sizeof(cdata)-sizeof(crc));
  memcpy(cdata+(sizeof(cdata)-sizeof(crc)), (uint8_t*)&crc, sizeof(crc));
  comm_char->writeValue(cdata, sizeof(cdata));
  state = CHUNK_ACK;
}


static void write_next_chunk()
{
  Serial.printf("%d -- %s(): 0x%x of 0x%x\n", millis(), __FUNCTION__, image_offset, image_metadata.image_len);
  ERROR_CHECK(esp_partition_read(running_partition, image_offset, image_data, sizeof(image_data)));

  for (uint16_t offset = 0; offset < sizeof(image_data); offset += PKT_SIZE) {
    write_packet(offset);
  }
  chunk_complete();
}


static void handle_nak(uint8_t* data, size_t leng)
{
  uint32_t data_scoreboard;
  uint16_t data_crc16[32];

  Serial.printf("%d -- %s(): %s -- FAILED!\n", millis(), __FUNCTION__, data+3);
  memcpy((uint8_t*)&data_scoreboard, data+19, sizeof(data_scoreboard));
  memcpy((uint8_t*)data_crc16, data+19+sizeof(data_scoreboard), sizeof(data_crc16));

  for (int index = 0; index < 32; ++index) {
    uint16_t offset = index*PKT_SIZE;
    if ((data_scoreboard & (1<<index)) == 0 || data_crc16[index] != crc16(image_data+offset, PKT_SIZE)) {
      Serial.printf("%d -- %s(): resending offset=%x, image_offset=%x\n", millis(), __FUNCTION__, offset, image_offset);
      write_packet(offset);
    }
  }

  chunk_complete();
}


static void handle_ack()
{
  if (image_offset == 0) ms_delay(3000); // allow for flash erase
  if ((image_offset += sizeof(image_data)) < image_metadata.image_len) {
    state = WRITE_CHUNK;
  } else {
    state = OTA_COMPLETE;
    ota_stop_time = millis();
  }
}


static void wait_for_chunk_ack()
{
  if (comm_ack) {
    uint8_t data[256];
    size_t  leng = comm_char->getValue().size();
    if (leng > sizeof(data)) leng = sizeof(data);
    memcpy(data, comm_char->getValue().getValue(NULL), leng);
  
    comm_ack = 0;
    if (data[0] == 0x55 && data[2] == 9) {
      if (strncmp((const char*)(data+3), "ACK", 3) == 0) {
        handle_ack();
      } else
      if (strncmp((const char*)(data+3), "NAK", 3) == 0) {
        handle_nak(data, leng);
      } else {
        Serial.printf("%s(): unknown response\n", __FUNCTION__);
        state = OTA_FAILED;
      }
    } else {
      Serial.printf("%s(): data[0]=0x%x, data[2]=%d\n", __FUNCTION__, data[0], data[2]);
      state = OTA_FAILED;
    }
  }
}


static void ota_complete()
{
  Serial.printf("OTA COMPLETE! -- %s\n", pretty_time((ota_stop_time - ota_start_time)/1000));
  delay(15000);
}


static void ota_failed()
{
  Serial.printf("OTA FAILED!\n");
  delay(60000);
}


static void run_loader()
{
  switch (state) {
  case OTA_START:    ota_start();          break;
  case START_ACK:    wait_for_start_ack(); break;
  case WRITE_CHUNK:  write_next_chunk();   break;
  case CHUNK_ACK:    wait_for_chunk_ack(); break;
  case OTA_COMPLETE: ota_complete();       break;
  case OTA_FAILED:   ota_failed();         break;
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

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    run_loader();
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    delay(5000); // Delay a second between loops.
  }
}


void setup()
{
  esp_err_t err;
  uint8_t data[32];

  Serial.begin(115200);
  Serial.printf("%s(): VERSION='%s'\n", __FUNCTION__, VERSION);
  Serial.flush();

  image_offset = 0;
  running_partition = esp_ota_get_running_partition();
  if (running_partition == NULL) {
    Serial.printf("%s(): unable to get running partition\n", __FUNCTION__);
    panic(0, __LINE__);
  }

  ERROR_CHECK(get_partition_metadata(running_partition, &image_metadata));

  ERROR_CHECK(esp_partition_read(running_partition, 0, data, sizeof(data)));
  Serial.printf("IMAGE HEADER:");
  for (int i = 0; i < sizeof(data); ++i) {
    Serial.printf(" %02x", data[i]);
  }
  Serial.printf("\n");

  ERROR_CHECK(esp_partition_read(running_partition, VERSION_OFFSET, data, sizeof(data)));
  Serial.printf("IMAGE VERSION:");
  for (int i = 0; i < sizeof(data); ++i) {
    Serial.printf(" %02x", data[i]);
  }

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