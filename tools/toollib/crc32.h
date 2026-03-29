#ifndef __CRC32_H__
#define __CRC32_H__

#include "typedef.h"
#include <sys/types.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

extern uint32_t crc32(uint32_t crc, const void *buf, size_t size);

#if defined(__cplusplus)
}
#endif

#endif

