#include "spod_library.h"
#include "ota.h"
#include "esp_app_format.h"


static const char *TAG = "OTA";

#define OTA_TIMEOUT_MSECS   ONE_MINUTE

#define HASH_LEN 32 /* SHA-256 digest length */
#define SIZEOF_HDRS (sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
#define SIZEOF_VERS 32


typedef struct ota_client_struct {
    bool running;
    TaskHandle_t task_handle;
    SemaphoreHandle_t data_ready;
    esp_ota_handle_t update_handle;
    size_t total_length;
    size_t received_length;
    uint8_t rxring_buffer[0x2000];
    ring_t rxring;
    uint8_t buffer[0x400];
} ota_client_t;


static ota_client_t* client = NULL;


bool ota_packet(uint8_t type)
{
    type &= MASK_PKT_TYPE;
    return type == OTA_CTRL_PKT_TYPE || type == OTA_DATA_PKT_TYPE || type == OTA_CTRL_PKT_ACK_TYPE;
}


static void free_resources()
{
    if (client) {
        client->update_handle = 0;
        client->running = false;
        vSemaphoreDelete(client->data_ready);
        free(client);
        client = NULL;
    }
}


static void __attribute__((noreturn)) task_fatal_error(esp_err_t err, int lineno)
{
    LOGE(TAG, "Exiting task due to fatal error... (err=%s, lineno=%d)\r\n", esp_err_to_name(err), lineno);
    free_resources();
    (void)vTaskDelete(NULL);
    while (1) continue;
}


void ota_abort()
{
    if (client) {
        vTaskDelete(client->task_handle);
        esp_ota_abort(client->update_handle);
        free_resources();
    }
}


bool ota_running()
{
    return client && client->running;
}


static inline bool all_received()
{
    return client && client->received_length >= client->total_length;
}


static int blocking_read(uint8_t* buffer, size_t length)
{
    errno = 0;

    if (client == NULL || all_received()) {
        return 0;
    }

    do {
        size_t valid = ring_valid(&client->rxring);

        // the OTA thread expects to get the headers and version number as contiguous
        // data (all in one read).  after that there are no restrictions 
        if (valid != 0 && client->received_length + valid >= 288) { /* sizeof headers and version number*/
            size_t nread = (valid > length)? length : valid;
            if (!ring_read(&client->rxring, buffer, nread, true)) return -1;
            client->received_length += nread;
            return nread;
        }
    } while (xSemaphoreTake(client->data_ready, ms_to_ticks(OTA_TIMEOUT_MSECS)));

    // OTA_TIMEOUT_MSECS milliseconds transpired before any OTA data was received, abort OTA!
    LOGE(TAG, "%s(): OTA TIMEOUT!\r\n", __FUNCTION__);
    errno = ENOTCONN;
    return 0;
}


esp_err_t ota_write(uint8_t* data, size_t length)
{
    LOGD(TAG, "%s(leng=%x)\r\n", __FUNCTION__, length);
    if (client == NULL || !client->running) {
        LOGE(TAG, "%s(length=%x): client==NULL or !client->running\r\n", __FUNCTION__, length);
        return ESP_FAIL;
    }
    if (!ring_write(&client->rxring, data, length, NULL)) {
        LOGE(TAG, "%s(length=%x): ring_write() failed!\r\n", __FUNCTION__, length);
        ESP_FAIL;
    }
    xSemaphoreGive(client->data_ready);
    return ESP_OK;
}


static esp_err_t read_app_version(const esp_partition_t* partition, char* version)
{
    return partition == NULL? ESP_FAIL : esp_partition_read(partition, SIZEOF_HDRS, version, SIZEOF_VERS);
}


static void ota_task(void *unused)
{
    esp_err_t err;
    esp_ota_handle_t update_handle = 0 ; /* set by esp_ota_begin(), must be freed via esp_ota_end() */
    const esp_partition_t *update_partition = NULL;

    LOGI(TAG, "Starting OTA...\r\n");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x\r\n",
                 configured->address, running->address);
        LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)\r\n");
    }
    LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)\r\n",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    LOGI(TAG, "Writing to partition subtype %d at offset 0x%x\r\n",
             update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    bool image_header_was_checked = false;
    uint8_t* buffer = client->buffer;
    while (1) {
        int data_read = blocking_read(buffer, sizeof(client->buffer));
        if (data_read < 0) {
            LOGE(TAG, "Error: SSL data read error\r\n");
            task_fatal_error(ESP_ERR_INVALID_RESPONSE, __LINE__);
        } else if (data_read > 0) {
            LOGD(TAG, "%s(): blocking_read()=%x\r\n", __FUNCTION__, data_read);
            if (image_header_was_checked == false) {
                if (data_read >= (SIZEOF_HDRS+SIZEOF_VERS)) {
                    // check current version with downloading
                    char new_app_version[SIZEOF_VERS];
                    memcpy(new_app_version, &buffer[SIZEOF_HDRS], sizeof(new_app_version));
                    LOGI(TAG, "New firmware version: %s\r\n", new_app_version);

                    char running_app_version[SIZEOF_VERS];
                    if (read_app_version(running, running_app_version) == ESP_OK) {
                        LOGI(TAG, "Running firmware version: %s\r\n", running_app_version);
                    } else {
                        memcpy(running_app_version, "VERSION UNKNOWN", 16/*sizeof(running_app_version)*/);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    char invalid_app_version[SIZEOF_VERS];
                    if (read_app_version(last_invalid_app, invalid_app_version) == ESP_OK) {
                        LOGI(TAG, "Last invalid firmware version: %s\r\n", invalid_app_version);
                    } else {
                        memcpy(invalid_app_version, "VERSION UNKNOWN", 16/*sizeof(invalid_app_version)*/);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_version, new_app_version, sizeof(new_app_version)) == 0) {
                            LOGW(TAG, "New version is the same as invalid version.\r\n");
                            LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.\r\n", invalid_app_version);
                            LOGW(TAG, "The firmware has been rolled back to the previous version.\r\n");
                            task_fatal_error(ESP_ERR_INVALID_VERSION, __LINE__);
                        }
                    }
                    if (memcmp(new_app_version, running_app_version, sizeof(new_app_version)) == 0) {
                        LOGW(TAG, "Current running version (%s) is the same as a new. We will not continue the update.\r\n", new_app_version);
                        task_fatal_error(ESP_ERR_INVALID_VERSION, __LINE__);
                    }

                    image_header_was_checked = true;
            
                    // NOTE: specifying the image size, rather than OTA_WITH_SEQUENTIAL_WRITES caused the OTA performance for
                    // a 585K image to go from 5 minutes down to 1 minute 10 seconds! The loader is a little trickier because
                    // this initial esp_ota_begin() call takes an additional 3 seconds to erase the flash, so the loader has
                    // to delay AFTER it has loaded the first 3 packets (assuming the packets are 128 bytes of payload), this
                    // is when esp_ota_begin() is called, after it verifies the version number.
                    uint64_t start = get_microseconds();
                    err = esp_ota_begin(update_partition, client->total_length/*OTA_WITH_SEQUENTIAL_WRITES*/, &update_handle);
                    LOGI(TAG, "esp_ota_begin(): %lldus\r\n", get_microseconds()-start);

                    if (err != ESP_OK) {
                        LOGE(TAG, "esp_ota_begin failed (%s)\r\n", esp_err_to_name(err));
                        esp_ota_abort(update_handle);
                        task_fatal_error(err, __LINE__);
                    }
                    client->update_handle = update_handle;
                    LOGI(TAG, "%d -- esp_ota_begin succeeded\r\n", millis());
                } else {
                    LOGE(TAG, "received package is not fit len\r\n");
                    esp_ota_abort(update_handle);
                    task_fatal_error(ESP_ERR_INVALID_SIZE, __LINE__);
                }
            }
            //uint64_t start = get_microseconds();
            err = esp_ota_write(update_handle, (const void *)buffer, data_read);
            if (err != ESP_OK) {
                esp_ota_abort(update_handle);
                task_fatal_error(err, __LINE__);
            }
            //LOGW(TAG, "esp_ota_write(leng=%d): %lldus\r\n", data_read, get_microseconds()-start);
            binary_file_length += data_read;
            if ((binary_file_length % 0x1000) == 0)
                LOGI(TAG, "%d -- Wrote %d\r\n", millis(), binary_file_length);
        } else if (data_read == 0) {
           /*
            * As client->read() never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                LOGE(TAG, "Connection closed, errno = %d\r\n", errno);
                break;
            }
            if (all_received()) {
                LOGI(TAG, "Connection closed\r\n");
                break;
            }
        }
    }
    LOGI(TAG, "Total Write binary data length: %d\r\n", binary_file_length);
    if (!all_received()) {
        LOGE(TAG, "Error in receiving complete file\r\n");
        esp_ota_abort(update_handle);
        task_fatal_error(ESP_ERR_NOT_FINISHED, __LINE__);
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            LOGE(TAG, "Image validation failed, image is corrupted\r\n");
        } else {
            LOGE(TAG, "esp_ota_end failed (%s)!\r\n", esp_err_to_name(err));
        }
        task_fatal_error(err, __LINE__);
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!\r\n", esp_err_to_name(err));
        task_fatal_error(err, __LINE__);
    }
    LOGI(TAG, "OTA COMPLETE -- Prepare to restart system!\r\n");
    esp_restart();
}


static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    LOGI(TAG, "%s: %s\r\n", label, hash_print);
}


esp_err_t ota_start(size_t total_length)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;
    TaskHandle_t task_handle;

    if (client != NULL) {
        ota_abort();
    }

    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_partition_get_sha256(running, sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // If this happens, we erase NVS partition and initialize NVS again.
        if (err = nvs_flash_erase()) return err;
        if (err = nvs_flash_init()) return err;
    }

    client = (ota_client_t*)malloc(sizeof(*client));
    memset(client, 0, sizeof(*client));
    client->running = true;
    client->total_length = total_length;
    client->update_handle = 0;
    client->rxring.data = client->rxring_buffer;
    client->rxring.size = sizeof(client->rxring_buffer);
    client->data_ready = xSemaphoreCreateBinary();

    if (!xTaskCreate(&ota_task, "ota_task", 8192, NULL, 1, &client->task_handle)) {
        free_resources();
        return ESP_FAIL;
    }

    return ESP_OK;
}