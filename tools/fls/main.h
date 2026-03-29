#ifndef _MAIN_H_
#define _MAIN_H_

#include "lib.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int verbosity;
extern struct pollfd fds[2];
extern board_info_t board_info;
extern void die(int error);

#ifdef __cplusplus
}
#endif

#endif
