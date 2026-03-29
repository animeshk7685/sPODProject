#include "touchscreen.h"
#include "touchscreen_pins.h"
#include "config.h"
#include "ota.h"
#include "crc16.h"
#include "crc32.h"
#include "portx.h"


SET_LOOP_TASK_STACK_SIZE(16*1024) // default stack size is 4K (which is probably enough!)

static const char* TAG = "TSB";

#define ERSATZ_ARBITER 0

// place the version of the firmware (including type, e.g. "PDB", "SDB", "TSB", for "sPOD", "switch", and "touchscreen")
// at a known location (offset 0x120) in the beginning of flash/binary image 
// this immediately follows the esp_image_header_t, esp_segment_header_t and esp_app_desc_t
// structures which are 24, 8 and 256 bytes respectively
//
// note: because arduino uses pre-compiled esp-idf libraries we could not take advantage of
// the esp_app_desc_t.version field 
const __attribute__((section(".rodata_custom_desc"))) char VERSION[32] = {"TSB-01.00.01"};

static uint8_t step = 0;
uint8_t our_mac[6];
Timer<> timer;


bool ota_active()
{
    return true; // TODO: sniff packets to determine this
}


static bool watchdog(void* context)
{
    led_toggle(BLUE_LED);
    return pet_watchdog(context);
}


void setup()
{

    Serial.begin(115200);
    esp_read_mac(our_mac, ESP_MAC_BT);
    LOGI(TAG, "%d -- %s() -- %s, %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        millis(), __FUNCTION__, VERSION,
        our_mac[0], our_mac[1], our_mac[2],
        our_mac[3], our_mac[4], our_mac[5]);
        Serial.flush();
        
    LOGI(TAG, "%d -- %s RESET\r\n", millis(), reset_reason_string());
    enable_watchdog();
    timer.every(1000, watchdog);
    
    serial_init(RS485_DE_PIN);
    i2c_init(I2C_DAT_PIN, I2C_CLK_PIN);
    fs_init();

    portx_init();
    led_init();
    led_set(GREEN_LED, true);
    
    pkt_init(our_mac, TSB_BOARD_TYPE);
    pkt_process_init(VERSION, NULL, NULL);

    #if ERSATZ_ARBITER
    LOGI(TAG, "%s -- %s(): ERSATZ ARBITER!!!\r\n", millis(), __FUNCTION__);
    rs485_enumerate(PDB_BOARD_TYPE, RS485_CTRL_PIN, RS485_INTR_PIN);
    #else
    rs485_setup(TSB_BOARD_TYPE, RS485_CTRL_PIN, RS485_INTR_PIN);
    #endif

    main_setup();
    led_set(GREEN_LED, false);

    pcm_init(RS485_INVALID, NULL);
    #if ERSATZ_ARBITER
    pcm_discover_type();
    board_set_pcm1(our_mac+3, true);
    #endif
    send_pcm_info_pkt();

    // wait around until the PCMs have filled the pcm_circuits array
    // so config_init() knows the number of switches to read from
    // the flash file system
    for (uint32_t start = millis(); millis() - start < 750 || step != 0xFF; ) {
        uint8_t owner;
        if (pkt_get(&owner)) {
            pkt_process(owner);
            pkt_release();
        }
        pet_watchdog(NULL);
        step = config_init(step);
    }

    LOGI(TAG, "%d -- %s() COMPLETE1\r\n", millis(), __FUNCTION__);
    Serial.flush();
}


void loop()
{
    timer.tick();
    //if (step != 0xFF) config_init(step);
    main_loop();
}