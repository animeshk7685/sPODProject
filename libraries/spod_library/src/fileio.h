/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef __FILEIO_H__
#define __FILEIO_H__
    
extern bool fs_init(void);
extern bool fs_open_for_reading(fs::FS &fs, const char* path, File &file);
extern bool fs_open_for_writing(fs::FS &fs, const char* path, File &file);
extern size_t fs_read(File& file, uint8_t* data, size_t count);
extern size_t fs_write(File& file, uint8_t* data, size_t count);
extern void fs_close(File& file);
extern bool fs_remove(const char *path);
extern void fs_rename(const char* old_name, const char* new_name);
extern uint32_t fs_filesize(const char* file_name);
extern bool fs_list(fs::FS &fs, uint8_t* smac, bool (*cb)(uint8_t* smac, const char* name, size_t size));
extern int fs_read_file(const char* path, uint8_t* data, size_t nbytes);
extern bool fs_write_file(const char* path, uint8_t* data, size_t nbytes);
extern void fs_format(fs::FS &fs);

#endif