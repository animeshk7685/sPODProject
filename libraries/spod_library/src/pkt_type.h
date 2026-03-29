//-----------------------------------------------------------------------------
//  pkt_type.h
//
//-----------------------------------------------------------------------------

#ifndef __PKT_TYPE_H__
#define __PKT_TYPE_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    HEARTBEAT_PKT_TYPE      = 0x00,  // every device on the RS485 bus sends a heartbeat packet every 5 seconds
    SYS_PKT_TYPE            = 0x01,  // first byte of payload is sys_pkt_type_t...
    CONFIG_PKT_TYPE         = 0x02,  // config_t
    PCM_CONFIG_PKT_TYPE     = 0x03,  // pcm_config_t (pcm configuration)
    PCM_STATUS_PKT_TYPE     = 0x04,  // pcm_status_t (pcm status)
    SWITCH_CONFIG_PKT_TYPE  = 0x05,  // switch_config_t (switch configuration)
    SWITCH_STATUS_PKT_TYPE  = 0x06,  // switch_status_t (button press/release)
    AVAIL07_PKT_TYPE        = 0x07,
    OTA_CTRL_PKT_TYPE       = 0x08,
    OTA_CTRL_PKT_ACK_TYPE   = 0x09,
    OTA_DATA_PKT_TYPE       = 0x0A,
    RS485_PKT_TYPE          = 0x0B,  // first byte of payload is rs485_pkt_type_t...
    CAN_PKT_TYPE            = 0x0C,  // TODO: deprecated and this becomes AVAIL0C_PKT_TYPE
    DEBUG_PKT_TYPE          = 0x0D,
    CIRCUIT_STATUS_PKT_TYPE = 0x0E,  // circuit_status_t
    TRIGGER_PKT_TYPE        = 0x0F,
    MASK_PKT_TYPE           = 0x0F
} pkt_type_t;


typedef enum {
    ABORT_PKT_TYPE          = 0x01,
    FGET_PKT_TYPE           = 0x02,
    FPUT_PKT_TYPE           = 0x03,
    FDAT_PKT_TYPE           = 0x04,
    FACK_PKT_TYPE           = 0x05,
    FRM_PKT_TYPE            = 0x06,
    FLS_PKT_TYPE            = 0x07,
} sys_pkt_type_t;


typedef enum {
    ARBITER_REBOOTED_PKT_TYPE = 0x01,
    GET_BOARD_INFO_PKT_TYPE   = 0x02,
    ACK_BOARD_INFO_PKT_TYPE   = 0x03,
    PCM_INFO_PKT_TYPE         = 0x04,  // PCM broadcasts at beginning of the world, 8 or 16
} rs485_pkt_type_t;


// TODO: should this be somewhere else?
#define SWITCH_PACKET   0x80
#define DEBUG_PACKET    0x90
#define SYSTEM_PACKET   0xA0
#define STATUS_PACKET   0xB0
#define PRO_PACKET      0xC0


#if defined(__cplusplus)
}
#endif

#endif
