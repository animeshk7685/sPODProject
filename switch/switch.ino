#include "switch.h"
#include "switch_pins.h"
#include "switch_common.h"
#include "ota.h"
#include "sleep.h"
#include "esp_mac.h"

static const char* TAG = "SDB";

SET_LOOP_TASK_STACK_SIZE(16*1024) // default stack size is 4K (which is probably enough!)

#define DEEPSLEEP_ENABLED (0)     // TODO: disable for now
#define PCM_WAIT_MS 500


// place the version of the firmware (including type, e.g. "PDB", "SDB", "TSB", for "sPOD", "switch", and "touchscreen")
// at a known location (offset 0x120) in the beginning of flash/binary image 
// this immediately follows the esp_image_header_t, esp_segment_header_t and esp_app_desc_t
// structures which are 24, 8 and 256 bytes respectively
//
// note: because arduino uses pre-compiled esp-idf libraries we could not take advantage of
// the esp_app_desc_t.version field 
const __attribute__((section(".rodata_custom_desc"))) char VERSION[32] = {"SDB-01.00.00"};

static uint8_t step = 0;
static uint32_t setup_complete_time;
bool deepsleep_wakeup;
bool ignition_on;
uint8_t our_mac[6];
Timer<> timer;


bool ota_active()
{
    return true; // TODO: sniff packets to determine this
}


static bool send_heartbeat_cb(void* unused)
{
    if (!pkt_ota_in_progress()) {
        send_heartbeat_pkt(rs485_getaddr());
        timer.in(5000, send_heartbeat_cb);
    }
    return true;
}


void timer_tick_delay(uint32_t msecs)
{
    uint32_t start = millis();
    pet_watchdog(NULL);
    while (millis() - start < msecs) {
        timer.tick();
    }
}


void goto_sleep()
{
    #if DEEPSLEEP_ENABLED
    // TODO: also need to check for OTA update, ignition on, etc.
    if (config.deepsleep_time > 0) {
        enter_deep_sleep(config.deepsleep_time);
    }
    deepsleep_wakeup = false;
    #endif
}


static void poll_rs485()
{
    uint8_t owner;
    if (pkt_get(&owner)) {
        pkt_process(owner);
        pkt_release();
    }
    pet_watchdog(NULL);

    step = config_init(step);
}


void setup()
{
    Serial.begin(115200);
    esp_read_mac(our_mac, ESP_MAC_BT);
    LOGI(TAG, "%d -- %s() -- %s, %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        millis(), __FUNCTION__, VERSION,
        our_mac[0], our_mac[1], our_mac[2],
        our_mac[3], our_mac[4], our_mac[5]);
    LOGI(TAG, "%d -- %s() -- Compiled: %s @ %s\r\n", millis(), __FUNCTION__, __DATE__, __TIME__);
    Serial.flush();

    ignition_on = false;
    #if DEEPSLEEP_ENABLED
    deepsleep_wakeup = esp_reset_reason() == ESP_RST_DEEPSLEEP;
    #endif

    LOGI(TAG, "%d -- %s RESET\r\n", millis(), reset_reason_string());
    enable_watchdog();
    timer.every(1000, pet_watchdog);
    
    serial_init(RS485_DE_PIN);
    i2c_init(I2C_DAT_PIN, I2C_CLK_PIN);

    fs_init();
    readSettings();
    switch_init();
    
    pkt_init(our_mac, SDB_BOARD_TYPE);
    pkt_process_init(VERSION, NULL, NULL);
    rs485_setup(SDB_BOARD_TYPE, RS485_CTRL_PIN, RS485_INTR_PIN);
    
    led_init();
    button_init();
    if (deepsleep_wakeup && button_read()) {
        deepsleep_wakeup = false;
        led_blink(0xFF, 0x00, 0xFF, 0x00, 100, 5);
        timer_tick_delay(100*5);
    }

    pcm_init(RS485_INVALID, NULL);
    send_pcm_info_pkt();
    led_post(poll_rs485);

    send_heartbeat_cb(NULL);
    timer.every(20, dim);

    LOGI(TAG, "%d -- %s() COMPLETE!\r\n", millis(), __FUNCTION__);
    Serial.flush();
    setup_complete_time = millis();
}


void loop()
{
    timer.tick();
    button_loop();
    rs485_receive();

    if (step != 0xFF) step = config_init(step);

    #if DEEPSLEEP_ENABLED
    if (deepsleep_wakeup && millis() - setup_complete_time > 750) {
        goto_sleep();
    }
    #endif
}