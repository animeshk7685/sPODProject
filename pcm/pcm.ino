#include "spod.h"
#include "main.h"
#include "stdDrivers.h"
#include "xi.h"
#include "xo.h"
#include "rs485.h"
#include "ota.h"
#include "crc16.h"
#include "crc32.h"


static const char* TAG = "PDB";


SET_LOOP_TASK_STACK_SIZE(16*1024) // default stack size is 4K (which is probably enough!)

// place the version of the firmware (including type, e.g. "PDB", "SDB", "TSB", for "sPOD", "switch", and "touchscreen")
// at a known location (offset 0x120) in the beginning of flash/binary image 
// this immediately follows the esp_image_header_t, esp_segment_header_t and esp_app_desc_t
// structures which are 24, 8 and 256 bytes respectively
//
// note: because arduino uses pre-compiled esp-idf libraries we could not take advantage of
// the esp_app_desc_t.version field 
const __attribute__((section(".rodata_custom_desc"))) char VERSION[32] = {"PDB-01.00.01"};

static uint8_t step = 0;
uint8_t our_mac[6];
Timer<> timer;


void setup()
{
    extern void main_init();
    extern void ble_ack_ota_ctrl_packet(uint8_t mac[3], uint8_t* data, uint8_t leng);

    Serial.begin(115200);
    esp_read_mac(our_mac, ESP_MAC_BT);
    enable_watchdog();
    timer.every(1000, pet_watchdog);
    
    LOGI(TAG, "%d -- %s() -- %s, %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        millis(), __FUNCTION__, VERSION, 
        our_mac[0], our_mac[1], our_mac[2], 
        our_mac[3], our_mac[4], our_mac[5]);
    LOGI(TAG, "%d -- %s RESET\r\n", millis(), reset_reason_string());
    
    serial_init(RS485_DE_PIN);
    i2c_init(I2C_DAT_PIN, I2C_CLK_PIN);
    xo_init();
    xi_init();

    #if 0
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    //xo_set_iref(0, 0x80);
    //xo_set_pwm(0,  0x80);
    //xo_set_ledout(0, XO_LEDOUT_PWM);
    xo_dump_regs();
    LOGD(TAG, "%s().%d\r\n", __FUNCTION__, __LINE__); Serial.flush();
    for (;;);
    #endif
  
    fs_init();
    readFlash();

    LOGD(TAG, "%d -- %s(): pkt_init()...\r\n", millis(), __FUNCTION__);
    pkt_init(our_mac, PDB_BOARD_TYPE);
    pkt_process_init(VERSION, ble_ack_ota_ctrl_packet, NULL);
    uint8_t addr = rs485_enumerate(PDB_BOARD_TYPE, RS485_CTRL_PIN, RS485_INTR_PIN);
    if (addr == RS485_PCM1) {
        board_set_pcm1(our_mac+3, true);
        send_arbiter_rebooted_pkt();
    }
    status.address = addr;
    current_pcm = addr & (PCMS-1);

    LOGD(TAG, "%d -- %s(): pcm_discover_type()...\r\n", millis(), __FUNCTION__);
    pcm_init(addr, our_mac);
    pcm_discover_type();
    send_pcm_info_pkt();

    ble_init();
    main_init();

    // wait around until the other PCMs have filled the pcm_circuits array
    // so config_init() knows the number of switches to read from
    // the flash file system
    LOGD(TAG, "%d -- %s(): wait for other PCMS...\r\n", millis(), __FUNCTION__);
    for (uint32_t start = millis(); millis() - start < 750; ) {
        uint8_t owner;
        if (pkt_get(&owner)) {
            pkt_process(owner);
            pkt_release();
        }
        pet_watchdog(NULL);
        step = config_init(step);
    }
    LOGD(TAG, "%d -- %s(): COMPLETE!!!\r\n", millis(), __FUNCTION__);
}


void loop()
{
    timer.tick();
    ble_loop();
    main_loop();

    if (step != 0xFF) step = config_init(step);
}