#include "touchscreen.h"
#include <LittleFS.h>


static const char *TAG = "LITTLEFS";

fs::LittleFSFS LittleFS2;


int get_file_size(const char *fname)
{
    struct stat sb;
    return stat(fname, &sb) == 0? sb.st_size : -1;
}


static int fs_read(fs::FS &fs, const char* path, uint8_t* data, size_t nbytes)
{
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    if (file) file.close();
    return -1;
  }

  size_t n = file.read(data, nbytes);
  file.close();

  return n;
}


static int fs_write(fs::FS &fs, const char* path, uint8_t* data, size_t nbytes)
{
  LOGI(TAG, "Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    LOGE(TAG, "Failed to open file '%s' for writing\r\n", path);
    return false;
  }

  int n = file.write(data, nbytes);
  file.close();
  return n;
}


int read_file(const char* path, uint8_t* result, int sizeof_result, bool verbose)
{
  int length;
  uint8_t* mirror = (uint8_t*)malloc(sizeof_result);

  if (verbose) LOGI(TAG, "Reading file: %s\r\n", path);

  int n1 = fs_read(LittleFS, path, result, sizeof_result);
  int n2 = fs_read(LittleFS2, path, mirror, sizeof_result);

  if (n1 == n2 && memcmp(result, mirror, n1) == 0) {
    free(mirror);
    return n1;
  }
  free(mirror);

  if (n1 > 0) {
    LittleFS2.remove(path);
    fs_write(LittleFS2, path, result, n1);
    return n1;
  }

  if (n2 > 0) {
    LittleFS.remove(path);  
    fs_write(LittleFS, path, result, n2);
    return n2;
  }

  if (verbose) LOGW(TAG, "contents=<empty>\r\n");
  return -1;
}


bool write_file(const char * path, uint8_t* data, size_t nbytes)
{
  int rc1 = fs_write(LittleFS, path, data, nbytes);
  int rc2 = fs_write(LittleFS2, path, data, nbytes);
  return rc1==nbytes && rc2==nbytes;
}


void remove_file(const char *path)
{
  LittleFS.remove(path);
  LittleFS2.remove(path);
}


bool file_exists(const char* path)
{
  return LittleFS.exists(path);
}


void touch_file(const char* path)
{
  if (!file_exists(path)) write_file(path, (uint8_t*)path, strlen(path)+1);
}


bool rename_file(const char* from, const char* to)
{
  bool rc1 = LittleFS.rename(from, to);
  bool rc2 = LittleFS2.rename(from, to);
  return rc1 && rc2;
}


void littlefs_init(void)
{
  const esp_partition_t* part;

  LittleFS2.end();
  if (!LittleFS2.begin(false, "/spiffs2", 10, "spiffs2")) {
    LOGE(TAG, "Error mounting SPI-FFS2, FORMATTING\r\n");
    part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                    ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs2");
    ESP.partitionEraseRange(part, 0, part->size);

    LittleFS2.format();
    LittleFS2.begin(false, "/spiffs2", 10, "spiffs2");
  }

  LittleFS.end();
  if (!LittleFS.begin(false/*formatOnFail*/)) {
    LOGE(TAG, "Error mounting SPI-FFS, FORMATTING\r\n");
    part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                    ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
    ESP.partitionEraseRange(part, 0, part->size);
    LittleFS.format();
    LittleFS.begin(false);
  }

  LOGD(TAG, "%s() COMPLETE\r\n", __FUNCTION__);
}