#ifndef __SPOD_H__
#define __SPOD_H__

#include "spod_library.h"
#include "gregs-timer.h"
#include "ble.h"
#include "stdFlash.h"

#define WATCHDOG 0
#define TASK_STACK_SIZE 8192

extern const char VERSION[32];
extern Timer<> timer;

#if defined(__cplusplus)
extern "C" {
#endif

extern void process_pkt();
extern void main_loop(void);

#if defined(__cplusplus)
}
#endif

#endif
