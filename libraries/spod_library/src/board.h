#ifndef __BOARD_H__
#define __BOARD_H__


#define BOARD_MAX 16

typedef struct {
    board_type_t    type;
    uint16_t        addr;
    uint8_t         mac[3];
} board_desc_t;

typedef struct {
    uint8_t  mac[3];
    uint8_t  board_type;
    uint8_t  addr;
    uint8_t  major;
    uint8_t  minor;
    uint8_t  revision;
    uint32_t last_seen;
    uint32_t beats;
} board_info_t;


typedef struct {
    uint8_t  mac[3];
    uint8_t  board_type;
    uint8_t  addr;
    uint8_t  major;
    uint8_t  minor;
    uint8_t  revision;    
} board_short_info_t;


extern board_info_t board[BOARD_MAX];
extern bool    pcm1;
extern bool    pcm1_known;
extern uint8_t pcm1_mac[3];

extern bool i_am_arbiter();
extern const char* board_type_string(board_type_t btype);
extern char* board_name_string(uint8_t type, uint8_t addr);
extern bool determine_board_type(char* board_name, board_desc_t* desc);
extern void board_process_heartbeat_packet(uint8_t* mac, uint8_t* data, uint8_t ptype, uint8_t leng);
extern bool board_mac_search(uint8_t* mac, board_info_t* info);
extern bool board_tad_search(board_type_t btype, uint8_t addr, board_info_t* info);
extern void show_boards();
extern void board_info_query(board_short_info_t* info);
extern void board_set_pcm1(uint8_t* mac, bool arbiter);
extern void forward_packet(uint8_t owner, bool pcms, bool controllers, uint8_t ptype, uint8_t* data, uint8_t leng);

#endif
