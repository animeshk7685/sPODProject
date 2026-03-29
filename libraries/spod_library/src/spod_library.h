#ifndef __SPOD_LIBRARY_H__
#define __SPOD_LIBRARY_H__

#include <Arduino.h>
//#include "spod_typedefs.h"
#include "typedef.h"
#include "board_type.h"
#include "ring.h"
#include "pkt.h"
#include "i2c.h"
#include "serial.h"

#include "log.h"
#include "board.h"
#include <LittleFS.h>
#include "fileio.h"
#include "pkt_process.h"
#include "serial.h"
#include "crc16.h"
#include "rs485.h"
#include "config.h"
#include "circuit.h"
#include "triggers.h"
#include "switch_common.h"
#include "pcm_common.h"
#include "esp_mac.h"


#define ONE_SECOND  (1000)          // # of milliseconds in a second
#define ONE_MINUTE  (60*ONE_SECOND) // # of milliseconds in a minutes
#define TWO_MINUTES (2*ONE_MINUTE)  // # of milliseconds in a 2 minutes
#define ONE_HOUR    (60*ONE_MINUTE) // # of milliseconds in an hour
#define ONE_DAY     (24*ONE_HOUR)   // # of milliseconds in a day
#define ONE_WEEK    (7*ONE_DAY)     // # of milliseconds in a week

#define myPrintf Serial.printf
#define CyDelay  delay


#if defined(__cplusplus)
extern "C" {
#endif

extern int verbosity;

extern const char* reset_reason_string();
extern void set_watchdog_timeout(uint32_t timeout_period_in_seconds);
extern void enable_watchdog();
extern bool pet_watchdog(void* unused);
extern void slow_boot();
extern void reboot(const char* reason);
extern void dump_memory(char* banner, uint32_t addr, uint8_t* data, int count);
extern unsigned long millis(void);
extern uint32_t ms_to_ticks(uint32_t msecs);
extern void ms_delay(int msecs);
extern void us_delay(uint64_t usecs);
extern uint64_t get_microseconds();
extern void dump_packet(char* name, uint8_t* data, int count);
extern void dump_heartbeat_packet(uint8_t* data);
extern void dump_can_packet(uint8_t* data);

extern void printByteArray(char* header, uint8_t* addr, uint8_t num);
extern void write_millis(uint32_t mil);

extern uint8_t canToI(uint8_t sw);
extern uint8_t iToCan(uint8_t sw);

extern int dbg_printf(int level, const char *format, ...);
extern void dbg_drain(void);

#if defined(__cplusplus)
}
#endif

#endif
