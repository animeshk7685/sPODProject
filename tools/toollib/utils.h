#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef UTILS_FILE_COPY_COMMAND
#define UTILS_FILE_COPY_COMMAND "/bin/cp"
#endif

#ifndef UTILS_CMDLINE_PROCFILE
#define UTILS_CMDLINE_PROCFILE "/proc/cmdline"
#endif

extern timespec get_time(void);
extern uint32_t delta_time(timespec start, timespec end);

extern void  record_pid(char *filename);
extern void  make_daemon(char *daemon_logname);
extern int   do_system(const char* fmt, ...);
extern char* trim_bol(char* line);
extern char* trim_eol(char* line);
extern char* local_strncpy(char* dst, const char* src, size_t nbytes);
extern void  dump_memory(char* banner, u32 addr, u8* data, int count);
extern void  dump_packet(char* name, u8* data, int count);
extern void  dump_heartbeat_packet(u8* data);
extern void  dump_can_packet(u8* data);
extern int   temp_filename(char* filename, int maxlen);
extern size_t file_size(const char* filename);
extern uint32_t file_crc32(const char* filename);

#ifdef __cplusplus
}
#endif

#endif
