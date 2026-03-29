#ifndef __RS485_H__
#define __RS485_H__

#include "spod_library.h"

#define ENQ 0x05
#define DLE 0x10

#define RS485_BASE          0x80
#define RS485_PCM1          (RS485_BASE+0)
#define RS485_PCM2          (RS485_BASE+1)
#define RS485_PCM3          (RS485_BASE+2)
#define RS485_PCM4          (RS485_BASE+3)
#define RS485_CONTROLLER1   (RS485_BASE+4)
#define RS485_CONTROLLERn   (RS485_BASE+15)
#define RS485_ARBITER       RS485_PCM1
#define RS485_MAXADDR       RS485_CONTROLLERn
#define RS485_INVALID       0x00

extern bool    rs485_lock();
extern void    rs485_unlock();
extern void    rs485_setup(board_type_t board_type, uint8_t ctrl_pin, uint8_t attn_pin);
extern uint8_t rs485_enumerate(board_type_t board_type, uint8_t ctrl_pin, uint8_t attn_pin);
extern uint8_t rs485_getaddr();
extern uint8_t rs485_arbaddr();
extern void    rs485_arbiter_lost();
extern void    rs485_arbiter_duplicate();

#endif