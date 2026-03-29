#ifndef __SLEEP_H__
#define __SLEEP_H__

extern char* sleep_wakeup_reason(void);
extern void enter_deep_sleep(int msecs);
extern void enter_light_sleep(int msecs);
extern void sleep_testing(const char* mode, void (*sleep)(int msecs));
extern void deep_sleep_testing(void);
extern void light_sleep_testing(void);

#endif
