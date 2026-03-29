#ifndef __LOG_H__
#define __LOG_H__

#include "console.h"

#define CPP_LOG_LEVEL 4

#  if CPP_LOG_LEVEL >= 5
#    define LOGV( tag, format, ... ) console_printf("V %s: " format, tag, ##__VA_ARGS__)
#  else
#    define LOGV( tag, format, ... ) (void)tag
#  endif

#  if CPP_LOG_LEVEL >= 4
#    define LOGD( tag, format, ... ) console_printf("D %s: " format, tag, ##__VA_ARGS__)
#  else
#    define LOGD( tag, format, ... ) (void)tag
#  endif

#  if CPP_LOG_LEVEL >= 3
#    define LOGI( tag, format, ... ) console_printf("I %s: " format, tag, ##__VA_ARGS__)
#  else
#    define LOGI( tag, format, ... ) (void)tag
#  endif

#  if CPP_LOG_LEVEL >= 2
#    define LOGW( tag, format, ... ) console_printf("W %s: " format, tag, ##__VA_ARGS__)
#  else
#    define LOGW( tag, format, ... ) (void)tag
#  endif

#  if CPP_LOG_LEVEL >= 1
#    define LOGE( tag, format, ... ) console_printf("E %s: " format, tag, ##__VA_ARGS__)
#    define LOGC( tag, format, ... ) console_printf("C %s: " format, tag, ##__VA_ARGS__)
#  else
#    define LOGE( tag, format, ... ) (void)tag
#    define LOGC( tag, format, ... ) (void)tag
#  endif

/**
 * @brief Dump a buffer to the log at specified level.
 *
 * The dump log shows just like the one below:
 *
 *      W (195) log_example: 0x3ffb4280   45 53 50 33 32 20 69 73  20 67 72 65 61 74 2c 20  |ESP32 is great, |
 *      W (195) log_example: 0x3ffb4290   77 6f 72 6b 69 6e 67 20  61 6c 6f 6e 67 20 77 69  |working along wi|
 *      W (205) log_example: 0x3ffb42a0   74 68 20 74 68 65 20 49  44 46 2e 00              |th the IDF..|
 *
 * It is highly recommend to use terminals with over 102 text width.
 *
 * @param tag description tag
 * @param buffer Pointer to the buffer array
 * @param buff_len length of buffer in bytes
 * @param level level of the log
 */
#define LOG_HEXDUMP( tag, buffer, buff_len, level ) \
    do { \
        if ( CPP_LOG_LEVEL >= (level) ) { \
            log_hexdump( tag, buffer, buff_len, level); \
        } \
    } while(0)

extern void log_hexdump(const char *tag, const void *buffer, uint16_t buff_len, esp_log_level_t log_level);

#endif
