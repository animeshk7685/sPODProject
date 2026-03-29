#ifndef _CRC16_H_
#define _CRC16_H_

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern uint16_t crc16(unsigned char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
