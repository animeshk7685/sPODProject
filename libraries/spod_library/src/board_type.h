#ifndef __BOARD_TYPE_H__
#define __BOARD_TYPE_H__

typedef enum {
    UNSPECIFIED_BOARD_TYPE  = 0x00,
    PDB_BOARD_TYPE          = 0x10,
    SDB_BOARD_TYPE          = 0x20,
    TSB_BOARD_TYPE          = 0x30,
    MASK_BOARD_TYPE         = 0x30
} board_type_t;

#endif
