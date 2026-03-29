#include "spod_library.h"

static const char *TAG = "CMN";

static bool watchdog = false;
int verbosity = 4;


const char* reset_reason_string()
{
    switch (esp_reset_reason()) {
    case ESP_RST_UNKNOWN:   return "UNKNOWN";   //!< Reset reason can not be determined
    case ESP_RST_POWERON:   return "POWERON";   //!< Reset due to power-on event
    case ESP_RST_EXT:       return "EXT";       //!< Reset by external pin (not applicable for ESP32)
    case ESP_RST_SW:        return "SW";        //!< Software reset via esp_restart
    case ESP_RST_PANIC:     return "PANIC";     //!< Software reset due to exception/panic
    case ESP_RST_INT_WDT:   return "INT_WDT";   //!< Reset (software or hardware) due to interrupt watchdog
    case ESP_RST_TASK_WDT:  return "TASK_WDT";  //!< Reset due to task watchdog
    case ESP_RST_WDT:       return "WDT";       //!< Reset due to other watchdogs
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP"; //!< Reset after exiting deep sleep mode
    case ESP_RST_BROWNOUT:  return "BROWNOUT";  //!< Brownout reset (software or hardware)
    case ESP_RST_SDIO:      return "SDIO";      //!< Reset over SDIO
    default:                return "UNKNOWN2";
    }
}


bool pet_watchdog(void* unused)
{
#if WATCHDOG
    if (watchdog) esp_task_wdt_reset();
#endif
    return true;
}


void set_watchdog_timeout(uint32_t timeout_period_in_seconds)
{
#if WATCHDOG
    if (watchdog) esp_task_wdt_init(timeout_period_in_seconds, true);
#endif
}


void enable_watchdog()
{
#if WATCHDOG
    watchdog = true;
    set_watchdog_timeout(WT_TIMEOUT);
    esp_task_wdt_add(NULL);
    pet_watchdog(NULL);
#endif
}


void disable_watchdog()
{
#if WATCHDOG
    watchdog = false;
    esp_task_wdt_deinit();
#endif
}


void reboot(const char* reason)
{
    LOGC(TAG, "%s\r\n", reason);
    Serial.flush();
    esp_restart();
}


void slow_boot()
{
// TODO: for development only -- we do this so we can see all of the output on the serial monitor
  for (int i = 0; i < 5; ++i) {
    LOGI(TAG, "slow_boot -- %d\r\n", i);
    delay(1000);
  }
  Serial.flush();
}

void printByteArray(char* header, uint8_t* addr, uint8_t num)
{
    LOGD(TAG, "%s\r\n", header);
    LOG_HEXDUMP(TAG, addr, num, ESP_LOG_DEBUG);
}

void write_millis(uint32_t ms)
{
    myPrintf("(");
    
    if(ms/3600000)
    myPrintf("%d:",(ms/3600000));
    
    ms = ms % 3600000;
    
    if(ms/60000)
    myPrintf("%d:",(ms/60000));
    
    ms = ms % 60000;
    
    if(ms/10000 == 0)
    myPrintf("0");
    
    myPrintf("%d.",(ms/1000));
    
    ms = ms % 1000;
    
    if(ms/100 == 0)
    myPrintf("0");
    
    if(ms/10 == 0)
    myPrintf("0");
    
    myPrintf("%d)\n",ms);
    
}


uint32_t ms_to_ticks(uint32_t msecs)
{
    return msecs/portTICK_PERIOD_MS;
}


void ms_delay(int msecs)
{
    vTaskDelay(ms_to_ticks(msecs));
}


uint64_t get_microseconds()
{
    return esp_timer_get_time();
}


void us_delay(uint64_t usecs)
{
    delayMicroseconds(usecs);
}


#define CAN_SW0             0x08
#define CAN_SW1             0x10
#define CAN_SW2             0x20
#define CAN_SW3             0x40
#define CAN_SW4             0x80
#define CAN_SW5             0x01
#define CAN_SW6             0x02
#define CAN_SW7             0x04  

uint8_t iToCan(uint8_t sw)
{
    switch (sw) {
    case 0: return CAN_SW0;
    case 1: return CAN_SW1;
    case 2: return CAN_SW2;
    case 3: return CAN_SW3;
    case 4: return CAN_SW4;
    case 5: return CAN_SW5;
    case 6: return CAN_SW6;
    case 7: return CAN_SW7;
    default: return 0;
    }  
}

uint8_t canToI(uint8_t sw)
{
    switch (sw) {
    case CAN_SW0: return 0;
    case CAN_SW1: return 1;
    case CAN_SW2: return 2;
    case CAN_SW3: return 3;
    case CAN_SW4: return 4;
    case CAN_SW5: return 5;
    case CAN_SW6: return 6;
    case CAN_SW7: return 7;
    default:      return 0xff;
    }  
}


static char hexchar(uint8_t data)
{
    char ch = data;
    return ch >= ' ' && ch <= '~'? ch : '?';
}


static char dbgbuf[0x8000];
static int  dbgoff = 0;
static uint64_t dbg_last = 0;

int dbg_printf(int level, const char *format, ...)
{
    int len = 0;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    if (level <= verbosity && dbgoff < sizeof(dbgbuf)-64) {
        uint64_t now = get_microseconds();
        uint64_t delta = now - dbg_last;
        dbg_last = now;
        len = snprintf(dbgbuf+dbgoff, sizeof(dbgbuf)-dbgoff, "%lld -- ", delta);
        dbgoff += len;

        va_list list;
        va_start(list, format);
        len = vsnprintf(dbgbuf+dbgoff, sizeof(dbgbuf)-dbgoff, format, list);
        va_end(list);

        if (len <= sizeof(dbgbuf)-dbgoff) {
            dbgoff += len;
        } else {
            dbgoff = sizeof(dbgbuf);
        }
    }
    portEXIT_CRITICAL(&mux);
    return len;
}


void dbg_drain(void)
{
   if (dbgoff > 0) {
       Serial.printf("%s", dbgbuf);
       Serial.flush();
       dbgoff = 0;
   }
}


void dump_packet(char* name, uint8_t* data, int count)
{
    switch (count) {
    case 5:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x [%c%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], count, name,
            data[0], data[1], data[2], data[3], data[4], 
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
        break;
    case 7:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x %02x %02x [%c%c%c%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], count, name,
            data[0], data[1], data[2], data[3], data[4], data[5], data[6],
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), hexchar(data[3]), hexchar(data[4]), hexchar(data[5]), hexchar(data[6]));
        break;
    default:
        dump_memory(name, 0, data, count);
        break;
    }
    Serial.flush();
}


void dump_heartbeat_packet(uint8_t* data) 
{
    Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s MAC=%02x:%02x:%02x VERS=%02d.%02d.%02d\r\n",
        millis(), data[-6], data[-7], 7, board_name_string(data[-6], data[3]), 
        data[0], data[1], data[2], data[4], data[5], data[6]);
    Serial.flush();
}


void dump_can_packet(uint8_t* data)
{
    switch (data[0]) {
    case SWITCH_PACKET:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "SWITCH",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case DEBUG_PACKET:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "DEBUG ",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case SYSTEM_PACKET:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "SYSTEM",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case STATUS_PACKET:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "STATUS",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case PRO_PACKET:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "PRO   ",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    default:
        Serial.printf("%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x [%c%c%c%c%c]\r\n",
            millis(), data[-6], data[-7], 5, "CAN ",
            data[0], data[1], data[2], data[3], data[4], 
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
        break;
    }
    Serial.flush();
}