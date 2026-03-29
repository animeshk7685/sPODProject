#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include "lib.h"
#include "common.h"


#ifdef __cplusplus
extern "C"
{
#endif

extern int verbosity;


int dbg_printf(int level, const char *format, ...)
{
   if (level <= verbosity) {
       va_list list;
       va_start(list, format);
       char workbuf[512];
       int len = vsnprintf(workbuf, sizeof(workbuf), format, list);
       va_end(list);

       fprintf(stderr, "%s", workbuf);
       fflush(stderr);
       return len;
   }
   return 0;
}


void dbg_drain(void)
{
   fflush(stdout);
   fflush(stderr);
}


// return running time in milliseconds
uint32_t now()
{
   struct timespec spec;
   uint64_t usec;
   clock_gettime(CLOCK_MONOTONIC_RAW, &spec);
   
   usec = spec.tv_sec*1000000 + spec.tv_nsec/1000;

   return usec/1000;
}


#if 0
void allcaps(char* str)
{
    for (; *str != '\0'; ++str) {
        if (*str >= 'a' && *str <= 'z') *str = *str - 'a' + 'A';
    }
}


bool is_mac_address(char* data, uint8_t mac[3])
{
    if (data[2] == ':' && data[5] == ':') {
        for (int i = 0; i < 3; ++i) mac[i] = strtoul(data+(i*3), NULL, 16);
        return true;
    }
    return false;
}
#endif


void base_filename(char* filename, char* basename)
{
   char* slash = strrchr(filename, '/');

   if (slash != NULL) {
      memcpy(basename, slash, MAXSTRING);
   } else {
      memcpy(basename, filename, MAXSTRING);
   }
}


#if 0
const char* board_type_string(board_type_t btype)
{
   switch (btype) {
   case TSB_BOARD_TYPE: return "TSB";
   case PDB_BOARD_TYPE: return "PDB";
   case SDB_BOARD_TYPE: return "SDB";
   default:             return "UNS";
   }
}


bool determine_board_type(char* board_name, board_desc_t* desc)
{
   allcaps(board_name);
   if (is_mac_address(board_name, desc->mac)) {
      dbg_printf(0, "%s(): board %s (%x:%x:%x)\n", __FUNCTION__, 
      board_name, desc->mac[0], desc->mac[1], desc->mac[2]);
      return true;
   }

   memset(desc->mac, 0, sizeof(desc->mac));
   if (strncmp(board_name, "TOUCH", 4) == 0 || strncmp(board_name, "TSB", 3) == 0) desc->type = TSB_BOARD_TYPE;
   else 
   if (strncmp(board_name, "SPOD", 4) == 0 || strncmp(board_name, "PDB", 3) == 0) desc->type = PDB_BOARD_TYPE;
   else
   if (strncmp(board_name, "SWITCH", 4) == 0 || strncmp(board_name, "SDB", 3) == 0) desc->type = SDB_BOARD_TYPE;

   if (desc->type != UNSPECIFIED_BOARD_TYPE) {
      uint8_t addr = 0;

      for (char* ptr = board_name; *ptr != '\0'; ++ptr) {
         if (*ptr >= '1' && *ptr <= '4') {
            addr = *ptr - '1';
            break;
         }
      }
      desc->addr = addr;
   }

   dbg_printf(0, "%s(): board %s (board_type=%s, board_addr=%d)\n", __FUNCTION__, 
      board_name, board_type_string(desc->type), desc->addr);
   return false;
}
#endif

#ifdef __cplusplus
}
#endif
