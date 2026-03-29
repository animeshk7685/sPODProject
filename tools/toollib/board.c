#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "board_type.h"
#include "pkt.h"
#include "board.h"
#include "pkt_process.h"
#include "common.h"


board_info_t board[BOARD_MAX];


void allcaps(char* str)
{
    for (; *str != '\0'; ++str) {
        if (*str >= 'a' && *str <= 'z') *str = *str - 'a' + 'A';
    }
}


static uint8_t hexer(char data)
{
    return data >= 'A' && data <= 'F'? data - 'A' + 10 : data - '0';
}


static uint8_t hexit(char* data)
{
    return (hexer(data[0]) << 4) + hexer(data[1]);
}


bool is_mac_address(char* data, uint8_t mac[3])
{
    if (data[2] == ':' && data[5] == ':') {
        for (int i = 0; i < 3; ++i) {
            mac[i] = hexit(data+(i*3));
            printf("%s(board='%s'): mac[%d]=%02x\n", __FUNCTION__, data, i, mac[i]);
        }
        return true;
    }
    return false;
}


const char* board_type_string(board_type_t btype)
{
   switch (btype) {
   case TSB_BOARD_TYPE: return "TSB";
   case PDB_BOARD_TYPE: return "PDB";
   case SDB_BOARD_TYPE: return "SDB";
   default:             return "UNS";
   }
}


bool determine_board_type(char* name, board_desc_t* desc)
{
   allcaps(name);
   if (is_mac_address(name, desc->mac)) {
      return true;
   }

   memset(desc, 0, sizeof(*desc));
   if (strncmp(name, "TOUCH", 4) == 0  || strncmp(name, "TSB", 3) == 0) desc->type = TSB_BOARD_TYPE; else 
   if (strncmp(name, "SPOD", 4) == 0   || strncmp(name, "PDB", 3) == 0) desc->type = PDB_BOARD_TYPE; else
   if (strncmp(name, "SWITCH", 4) == 0 || strncmp(name, "SDB", 3) == 0) desc->type = SDB_BOARD_TYPE;
   else return false;

    uint8_t addr = 0;

    for (char* ptr = name; *ptr != '\0'; ++ptr) {
        if (*ptr >= '1' && *ptr <= '4') {
        addr = *ptr - '1';
        break;
        }
    }
    desc->addr = addr;

   return false;
}


bool pkt_mac_match(uint8_t a[3], uint8_t b[3])
{
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2];
}


bool pkt_mac_zero(uint8_t* mac)
{
    return mac[0] == 0 && mac[1] == 0 && mac[2] == 0;   
}


void board_init()
{
    memset(board, 0, sizeof(board));
}


static int empty_slot()
{
    for (int i = 0; i < BOARD_MAX; ++i) {
        if (pkt_mac_zero(board[i].mac)) return i;
    }
    return -1;
}


static int find_board(uint8_t* mac)
{
    if (!pkt_mac_zero(mac)) {
        for (int i = 0; i < BOARD_MAX; ++i) {
            if (pkt_mac_match(board[i].mac, mac)) return i;
        }
    }
    return -1;   
}


void board_process_heartbeat_packet(uint8_t* mac, uint8_t* data, uint8_t ptype, uint8_t leng)
{
    pkt_heartbeat_t heartbeat;
    int slot;
   
    dbg_printf(0, "%s(mac=%02x:%02x:%02x, ptype=0x%x, leng=%d)\n", __FUNCTION__, mac[0], mac[1], mac[2], ptype, leng);
    slot = find_board(mac);
    if (slot == -1) {
        dbg_printf(0, "%s(): BOARD NOT FOUND!\n", __FUNCTION__);
        slot = empty_slot();
        if (slot == -1) {
            dbg_printf(0, "%s(): NO EMPTY SLOTS!\n", __FUNCTION__);
            return;
        }
    }

    dbg_printf(0, "%s(): FILLING SLOT %d\n", __FUNCTION__, slot);
    memcpy((uint8_t*)&heartbeat, data, sizeof(heartbeat));
    memcpy(board[slot].mac, mac, sizeof(board[slot].mac));
    board[slot].board_type = ptype & MASK_BOARD_TYPE;
    board[slot].addr       = heartbeat.addr;
    board[slot].major      = heartbeat.major;
    board[slot].minor      = heartbeat.minor;
    board[slot].revision   = heartbeat.revision;
    board[slot].last_seen  = now()/1000;
    board[slot].beats     += 1;
}


bool board_mac_search(uint8_t* mac, board_info_t* info)
{
    int slot = find_board(mac);
    if (slot != -1 && info != NULL) {
        *info = board[slot];
    }
    return slot != -1;
}


bool board_tad_search(board_type_t btype, uint8_t addr, board_info_t* info)
{
    for (int i = 0; i < BOARD_MAX; ++i) {
        if (((board_type_t)board[i].board_type) == btype && board[i].addr == addr) {
            *info = board[i];
            dbg_printf(0, "%s(): FOUND BOARD!\n", __FUNCTION__);
            return true;
        }
    }
    dbg_printf(7, "%s(): BOARD 0x%x, ADDR %d NOT FOUND!\n", __FUNCTION__, btype, addr);
    return false;    
}
