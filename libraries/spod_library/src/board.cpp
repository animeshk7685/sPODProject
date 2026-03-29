#include "spod_library.h"
#include <PCA95x5.h>


#ifdef ARDUINO
#define printf Serial.printf
#endif

static const char* TAG = "PP485";

#define SHOW_HEARTBEAT 0


board_info_t board[BOARD_MAX];
bool    pcm1;
bool    pcm1_known;
uint8_t pcm1_mac[3];


bool i_am_arbiter()
{
    return pcm1;
}


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


const char* board_type_string(board_type_t btype)
{
   switch (btype & MASK_BOARD_TYPE) {
   case TSB_BOARD_TYPE: return "TSB";
   case PDB_BOARD_TYPE: return "PDB";
   case SDB_BOARD_TYPE: return "SDB";
   default:             return "UNS";
   }
}


char* board_name_string(uint8_t type, uint8_t addr)
{
    static char name[8];
    snprintf(name, sizeof(name), "%s-%d", board_type_string((board_type_t)type), addr);
    return name;
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

   return true;
}


void board_init()
{
    memset(board, 0, sizeof(board));
}


static int empty_slot()
{
    for (int i = 0; i < BOARD_MAX; ++i) {
        if (board[i].addr == RS485_INVALID/*pkt_mac_zero(board[i].mac)*/) return i;
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


static void show_board(int slot)
{
    printf("%s %02x:%02x:%02x v%d.%d.%d\r\n", 
        board_name_string(board[slot].board_type, board[slot].addr),
        board[slot].mac[0], board[slot].mac[1], board[slot].mac[2],
        board[slot].major, board[slot].minor, board[slot].revision);
}


void show_boards()
{
    bool some = false;

    printf("---------------------------------------------\r\n");
    for (int slot = 0; slot < BOARD_MAX; ++slot) {
        if (!pkt_mac_zero(board[slot].mac)) {
            show_board(slot);
            some = true;
        }
    }

    if (!some) {
        printf("No boards detected\r\n");
    }
}


void board_set_pcm1(uint8_t* mac, bool arbiter)
{
    pcm1 = arbiter;
    memcpy(pcm1_mac, mac, sizeof(pcm1_mac));
    if (!pcm1_known) LOGI(TAG, "%d -- %s(): pcm1_known!!!, pcm1_mac=%02x:%02x:%02x\r\n", millis(), __FUNCTION__, pcm1_mac[0], pcm1_mac[1], pcm1_mac[2]);
    pcm1_known = true;
}


void board_process_heartbeat_packet(uint8_t* mac, uint8_t* data, uint8_t ptype, uint8_t leng)
{
    pkt_heartbeat_t heartbeat;
    int slot = find_board(mac);

    if (slot == -1) {
        slot = empty_slot();
        if (slot == -1) return;
    }

    memcpy((uint8_t*)&heartbeat, data, sizeof(heartbeat));
    memcpy(board[slot].mac, mac, sizeof(board[slot].mac));

    #if SHOW_HEARTBEAT
    LOGD(TAG, "%d -- HEARTBEAT-%x: %s%02x:%02x:%02x:%02x (last seen %d secs ago)\r\n", 
        millis(), heartbeat.beat, board_type_string((board_type_t) ptype), heartbeat.addr, mac[0], mac[1], mac[2], 
        millis()/1000 - board[slot].last_seen);
    #endif

    board[slot].board_type = ptype & MASK_BOARD_TYPE;
    board[slot].addr       = heartbeat.addr;
    board[slot].major      = heartbeat.major;
    board[slot].minor      = heartbeat.minor;
    board[slot].revision   = heartbeat.revision;
    board[slot].last_seen  = millis()/1000;
    board[slot].beats     += 1;

    if (heartbeat.addr >= RS485_PCM1 && heartbeat.addr <= RS485_PCM4) {
        if (heartbeat.addr == RS485_PCM1) board_set_pcm1(mac, false);
        pcm_setmac(heartbeat.addr, mac);
    }

    //show_board(slot);

#ifdef ARDUINO
    if (pkt_get_boardtype() != PDB_BOARD_TYPE) {
        uint8_t our_addr = rs485_getaddr();
        if (heartbeat.addr == RS485_PCM1) {
            static uint8_t arbiter_beats = 0;

            // heartbeat from RS485 bus arbiter -- track beats to detect reboot
            if (heartbeat.beat < arbiter_beats) {
                // arbiter rebooted, clear our local rs485 address
                rs485_arbiter_lost();
            }
            arbiter_beats = heartbeat.beat;
        }
        if (our_addr >= RS485_CONTROLLER1 && our_addr == heartbeat.addr) {
            rs485_arbiter_lost();  // basically just resets our addr
        }
    }
#endif
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
            return true;
        }
    }
    return false;    
}


void board_info_query(board_short_info_t* info)
{
    memset(info, 0, sizeof(board_short_info_t)*BOARD_MAX);
    for (int i = 0; i < BOARD_MAX; ++i) {
        if (!pkt_mac_zero(board[i].mac)) {
            memcpy(info[i].mac, board[i].mac, sizeof(info[i].mac));
            info[i].board_type = board[i].board_type;
            info[i].addr = board[i].addr;
            info[i].major = board[i].major;
            info[i].minor = board[i].minor;
            info[i].revision = board[i].revision;    
        }
    }
}


void forward_packet(uint8_t owner, bool pcms, bool controllers, uint8_t ptype, uint8_t* data, uint8_t leng)
{
    if (i_am_arbiter() && owner != RS485_INVALID) {
        LOGD(TAG, "%d -- %s(owner=%x, pcms=%d, controllers=%d, ptype=%x, leng=%x)\r\n", millis(), __FUNCTION__, owner, pcms, controllers, ptype, leng);
        for (int i = 0; i < BOARD_MAX; ++i) {
            if (board[i].addr != owner && board[i].addr != rs485_getaddr() && board[i].addr != RS485_INVALID) { // don't forward to owner, ourselves or invalid
                bool forward = false;
                if (board[i].addr >= RS485_PCM1 && board[i].addr <= RS485_PCM4) {
                    forward = pcms;
                } else {
                    forward = controllers;
                }
                if (forward) {
                    LOGD(TAG, "%d -- %s(): board[%d].mac/addr/type=%02x:%02x:%02x/%x/%x\r\n", 
                        millis(), __FUNCTION__, i, board[i].mac[0], board[i].mac[1], board[i].mac[2], board[i].addr, board[i].board_type);
                    pkt_put(board[i].mac, ptype, data, leng, 2);
                }
            }
        }
    }
}