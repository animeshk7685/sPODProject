#include "lib.h"
#include "common.h"


#ifdef __cplusplus
extern "C"
{
#endif


timespec get_time(void)
{
    timespec time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
    return time;
}


uint32_t delta_time(timespec start, timespec end)
{
    timespec delta;

    if (end.tv_nsec - start.tv_nsec < 0) {
        delta.tv_sec = end.tv_sec - start.tv_sec - 1;
        delta.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        delta.tv_sec = end.tv_sec - start.tv_sec;
        delta.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    while (delta.tv_sec > 0) {
        delta.tv_sec -= 1;
        delta.tv_nsec += 1000000000;
    }

    return delta.tv_nsec / 1000;
}


void record_pid(char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
        syslog(LOG_CRIT, "%s():fopen(%s, \"r\") -- '%m'\n", __FUNCTION__, filename);
    else {
        fprintf(file, "%d\n", getpid());
        fclose(file);
    }
}


void make_daemon(char *daemon_logname)
{
    // make nlmon a legit daemon
    pid_t pid, sid;
    char pid_filename[64];

    closelog();
    pid = fork();
    if (pid < 0) {
        syslog(LOG_CRIT, "fork() failed -- '%m'\n");
        die(__LINE__);
    }
    if (pid > 0)
        exit(0);

    umask(0);
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_CRIT, "setsid() failed -- '%m'\n");
        die(__LINE__);
    }
    daemon(0, 0);

    openlog(daemon_logname, 0, LOG_DAEMON);
    snprintf(pid_filename, sizeof(pid_filename), "/var/run/%s.pid", daemon_logname);
    record_pid(pid_filename);
}


int do_system(const char* fmt, ...)
{
    va_list vargs;
    char cmd[MAX_PATHNAME];
    int rc, error;

    va_start(vargs, fmt);
    vsnprintf(cmd, sizeof(cmd), fmt, vargs);
    va_end(vargs);

    errno = 0;
    rc = system(cmd);
    error = errno;

    syslog(LOG_DEBUG, "%s():do_system(%s) rc=%d, errno=%d WIFEXITED(rc)=%d, WEXITSTATUS(rc)=%d -- '%m'\n", 
           __FUNCTION__, cmd, rc, error, WIFEXITED(rc), WEXITSTATUS(rc));

    if (rc == -1) {
        errno = error;
        return -error;
    }

    // note: this is the "exit(#);" or "main(): return(#);" lower 8 bits of
    // the return code (>=0)
    if (WIFEXITED(rc) && WEXITSTATUS(rc)) {
        syslog(LOG_CRIT, "%s():system(%s) WEXITSTATUS(%d)=%d -- '%m'\n", __FUNCTION__, cmd, rc, WEXITSTATUS(rc));
        return WEXITSTATUS(rc);
    }

    return 0;
}


char* trim_bol(char* line)
{
    if (line) {
        for (; *line; ++line) {
            switch (*line) {
            case '\n': case '\r': case '\t': case ' ': continue;
            }
            break;
        }
    }
    return line;
}


char* trim_eol(char* line)
{
    if (line) {
        int leng;
        for (leng = strlen(line); leng > 0; --leng) {
            switch (line[leng-1]) {
            case '\n': case '\r': case '\t': case ' ': continue;
            }
            break;
        }
        line[leng] = '\0';
    }
    return line;
}


char* local_strncpy(char* dst, const char* src, size_t nbytes)
{
    if (dst && nbytes > 0) {
        strncpy(dst, src, nbytes);
        dst[nbytes-1] = '\0';
    }
    return dst;
}


void dump_memory(char* banner, u32 addr, u8* data, int count)
{
    int offset = 0;
    int i;

    for (; count >= 16; count -= 16, offset += 16) {
        printf("%d -- %s%08lx: ", now(), banner, addr + offset);
        for (i=0; i<16; ++i)
            printf("%02x ", data[offset+i]);
        for (i=0; i<16; ++i) {
            char ch = data[offset+i];
            printf("%c", ch >= ' ' && ch <= '~'? ch : '?');
        }
        printf("\n");
    }

    if (count) {
        printf("%d -- %s%08lx: ", now(), banner, addr + offset);
        for (i=0; i<count; ++i)
            printf("%02x ", data[offset+i]);
        for (; i<16; ++i)
            printf("   ");
        for (i=0; i<count; ++i) {
            char ch = data[offset+i];
            printf("%c", ch >= ' ' && ch <= '~'? ch : '?');
        }
        printf("\n");
    }
}


static char hexchar(u8 data)
{
    char ch = data;
    return ch >= ' ' && ch <= '~'? ch : '?';
}


void dump_packet(char* name, u8* data, int count)
{
    switch (count) {
    case 5:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x [%c%c%c%c%c]\n",
            now(), data[-6], data[-7], count, name,
            data[0], data[1], data[2], data[3], data[4], 
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
        break;
    case 7:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x %02x %02x [%c%c%c%c%c%c%c]\n",
            now(), data[-6], data[-7], count, name,
            data[0], data[1], data[2], data[3], data[4], data[5], data[6],
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), hexchar(data[3]), hexchar(data[4]), hexchar(data[5]), hexchar(data[6]));
        break;
    default:
        dump_memory(name, 0, data, count);
        break;
    }
}


static char* board_name(u8 type, u8 addr)
{
    static char name[8];
    
    switch (type & MASK_BOARD_TYPE) {
    case PDB_BOARD_TYPE: sprintf(name, "PDB%03d", addr); break;
    case SDB_BOARD_TYPE: sprintf(name, "SDB%03d", addr); break;
    case TSB_BOARD_TYPE: sprintf(name, "TSB%03d", addr); break;
    default: sprintf(name, "%03x%03d", type, addr); break;
    }
    return name;
}


void dump_heartbeat_packet(u8* data) 
{
    dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s MAC=%02x:%02x:%02x VERS=%02d.%02d.%02d\n",
        now(), data[-6], data[-7], 7, board_name(data[-6], data[3]), 
        data[0], data[1], data[2], data[4], data[5], data[6]);
}


void dump_can_packet(u8* data)
{
    switch (data[0]) {
    case SWITCH_PACKET:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "SWITCH",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case DEBUG_PACKET:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "DEBUG ",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case SYSTEM_PACKET:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "SYSTEM",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case STATUS_PACKET:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "STATUS",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    case PRO_PACKET:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x [%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "PRO   ",
            data[1], data[2], data[3], data[4], 
            hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
         break;

    default:
        dbg_printf(0, "%d -- RX-T/SEQ=%x/%d (leng=%d) %s %02x %02x %02x %02x %02x [%c%c%c%c%c]\n",
            now(), data[-6], data[-7], 5, "CAN ",
            data[0], data[1], data[2], data[3], data[4], 
            hexchar(data[0]), hexchar(data[1]), hexchar(data[2]), 
            hexchar(data[3]), hexchar(data[4]), hexchar(data[5]));
        break;
    }
}


int temp_filename(char* filename, int maxlen)
{
    return snprintf(filename, maxlen, "/tmp/spod-%08lx", random());
}


size_t file_size(const char* filename)
{
    struct stat sbuf;
    return stat(filename, &sbuf) == 0? sbuf.st_size : 0;
}


uint32_t file_crc32(const char* filename)
{
    uint8_t buffer[512];
    uint32_t nbytes, crc = 0;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    do {
        nbytes = fread(buffer, 1, sizeof(buffer), file);
        if (nbytes > 0) {
            crc = crc32(crc, buffer, nbytes);
        }
    } while (nbytes == sizeof(buffer));
    fclose(file);

    return crc;
}

#ifdef __cplusplus
}
#endif
