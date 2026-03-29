#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <spod_library.h>
#include "gregs-timer.h"
#include "main.h"
#include "led.h"
#include "button.h"
#include "switch_common.h"
#include "config.h"


#define WATCHDOG 0
#define TASK_STACK_SIZE 8192

#define DEEP_SLEEP_TIME_MS (10*1000)


extern bool deepsleep_wakeup;
extern bool ignition_on;
extern uint8_t our_mac[6];
extern uint8_t pcm1_mac[3];
extern bool    pcm1_known;

extern Timer<> timer;

//#if defined(__cplusplus)
//extern "C" {
//#endif

extern void process_pkt();
extern void goto_sleep();
extern void timer_tick_delay(uint32_t msecs);

//#if defined(__cplusplus)
//}
//#endif

#endif
