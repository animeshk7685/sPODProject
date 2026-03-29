#ifndef __TOUCHSCREEN_H__
#define __TOUCHSCREEN_H__

#include "spod_library.h"
#include "common.h"
#include "gregs-timer.h"
#include "fileio.h"
#include "portx.h"
#include "led.h"
#include "main.h"
#include <GD23Z.h>

#define LEGACY 0
#define WATCHDOG 0
#define CAN_ENABLED 0
#define BLE_ENABLED 0
#define TASK_STACK_SIZE 8192

extern uint8_t pcm1_mac[3];
extern Timer<> timer;
extern const char VERSION[];

#if defined(__cplusplus)
extern "C" {
#endif

//extern void main_loop(void);
extern void process_pkt();

#if defined(__cplusplus)
}
#endif

#endif
