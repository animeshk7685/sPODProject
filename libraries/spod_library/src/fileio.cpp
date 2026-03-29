#include "spod_library.h"
#include "pkt_process.h"
#include "crc16.h"
#include "crc32.h"

static const char* TAG = "FS";

static bool initialized = false;


bool fs_init(void)
{
  if (!initialized) {
    LittleFS.end();
    if (!LittleFS.begin(true)) {
      LOGE(TAG, "Unable to mount/format SPI-FFS1\r\n");
      initialized = false;
      return false;
    }

    initialized = true;
  }
  return true;
}


static char* fullpath(const char* name)
{
  if (strchr(name, '/') == NULL) {
    static char path[128];
    snprintf(path, sizeof(path), "/%s", name);
    return path;
  }
  return (char*)name;
}


bool fs_open_for_reading(fs::FS &fs, const char* path, File &file)
{
  if (!initialized) return false;
  file = fs.open(fullpath(path), "r");
  return file? true : false;
}


bool fs_open_for_writing(fs::FS &fs, const char* path, File &file)
{
  if (!initialized) return false;
  file = fs.open(fullpath(path), "w");
  Serial.printf("%s(%s), file=%p\r\n", __FUNCTION__, fullpath(path), file);
  return file? true : false;
}


size_t fs_read(File& file, uint8_t* data, size_t count)
{
  if (!initialized) return 0;
  return file.read(data, count);
}


size_t fs_write(File& file, uint8_t* data, size_t count)
{
  if (!initialized) return 0;
  return file.write(data, count);
}


void fs_close(File& file)
{
  if (file && initialized) file.close();
}


int fs_read_file(const char* path, uint8_t* data, size_t nbytes)
{
  File file;
  Serial.printf("%s(path='%s', nbytes=0x%x)\r\n", __FUNCTION__, path, nbytes);
  if (!fs_open_for_reading(LittleFS, path, file)) {
    return -1;
  }

  size_t n = file.read(data, nbytes);
  file.close();

  return n;
}


bool fs_write_file(const char* path, uint8_t* data, size_t nbytes)
{
  File file;
  if (!fs_open_for_writing(LittleFS, path, file)) {
    return false;
  }

  size_t n = file.write(data, nbytes);
  file.close();

  return n == nbytes;
}


bool fs_remove(const char *path)
{
  if (initialized) {
    char* p = fullpath(path);
    if (LittleFS.exists(p)) {
      LittleFS.remove(p);
      return true;
    }
  }

  return false;
}


void fs_rename(const char* old_name, const char* new_name)
{
  if (initialized) {
    char* p = fullpath(new_name);
    fs_remove(p);
    LittleFS.rename(old_name, p); // we assume old_name is fully qualified!
  }
}


uint32_t fs_filesize(const char* file_name)
{
#if 0
  struct stat sb;
  return stat(fullpath(file_name), &sb) == 0? sb.st_size : -1;
#else
  uint32_t file_size = 0;
  File file;
  if (fs_open_for_reading(LittleFS, fullpath(file_name), file)) {
    file_size = file.size();
    file.close();
  }
  return file_size;
#endif
}


bool fs_list(fs::FS &fs, uint8_t* smac, bool (*cb)(uint8_t* smac, const char* name, size_t size))
{
  if (!initialized) {
    cb(smac, nullptr, 0);
    return false;
  } else {
    File root = fs.open("/");
    if (!root) {
      LOGE(TAG, "NO ROOT DIR!\r\n");
      return false;
    }

    for (File file = root.openNextFile(); file; file = root.openNextFile()) {
      if (cb(smac, file.name(), file.size())) break;
    }

    root.close();
    return true;
  }
}


void fs_format(fs::FS &fs)
{
  // TODO: erase everything in the partition
  //fs.format();
}