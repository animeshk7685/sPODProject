#ifndef __FILEIO_H__
#define __FILEIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern int get_file_size(const char *fname);
extern int read_file(const char* path, uint8_t* result, int sizeof_result, bool verbose);
extern bool write_file(const char * path, uint8_t* data, size_t nbytes);
extern void remove_file(const char *path);
extern void touch_file(const char* path);
extern bool file_exists(const char* path);
extern bool rename_file(const char* from, const char* to);
extern void littlefs_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
