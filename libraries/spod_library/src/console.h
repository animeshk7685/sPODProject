#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define console_printf(_fmt, ...) do {printf(_fmt, ##__VA_ARGS__);} while (0)
extern char* board_name(uint8_t type, uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif /* __CONSOLE_H__ */