#ifndef __OTA_H__
#define __OTA_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "errno.h"
#include "ring.h"
#include "log.h"


#if defined(__cplusplus)
extern "C" {
#endif

extern uint64_t get_microseconds();

#if defined(__cplusplus)
}
#endif

extern bool ota_packet(uint8_t type);
extern esp_err_t ota_start(size_t total_length);
extern esp_err_t ota_write(uint8_t* buffer, size_t length);
extern void ota_abort();
extern bool ota_running();

#endif