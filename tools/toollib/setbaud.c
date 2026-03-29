//#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <asm/termios.h>
#include "typedef.h"
#include "spkt.h"
#include "serial.h"


#ifdef __cplusplus
extern "C"
{
#endif

extern int ioctl(int fd, unsigned long control, void* ptr);


void setbaud(int fd, int speed)
{
   struct termios2 tio;
   ioctl(fd, TCGETS2, &tio);
   tio.c_cflag &= ~CBAUD;
   tio.c_cflag |= BOTHER;
   tio.c_ispeed = speed;
   tio.c_ospeed = speed;
   ioctl(fd, TCSETS2, &tio);
}

#ifdef __cplusplus
}
#endif
