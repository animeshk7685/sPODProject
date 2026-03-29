#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern int dbg_printf(int x, const char* fmt, ...);
extern void dbg_drain(void);

#define watchdog_kick()

extern uint32_t now();
extern void die(int lineno);
extern void allcaps(char* str);
extern bool is_mac_address(char* data, uint8_t mac[3]);
#if 0
extern const char* board_type_string(board_type_t btype);
extern bool determine_board_type(char* board_name, board_desc_t* desc);
#endif
extern void base_filename(char* filename, char* basename);


extern int verbosity;

#ifdef __cplusplus
}
#endif

#endif
