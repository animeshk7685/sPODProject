//-----------------------------------------------------------------------------
//  triggers.h
//
//-----------------------------------------------------------------------------

#ifndef __TRIGGERS_H__
#define __TRIGGERS_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define TRIGGER_LATCHED      0x40
#define TRIGGER_ACTIVE_LOW   0x80

#define TRIGGER1             0x01
#define TRIGGER2             0x02
#define TRIGGER3             0x04
#define TRIGGER4             0x08
#define TRIGGER5             0x10
#define TRIGGER6             0x20
#define TRIGGER_IGN          0x40
#define TRIGGER_LOV          0x80

#define TRIGGERS_APPLY_CMD   1
#define TRIGGERS_RESTORE_CMD 2


typedef struct {
    uint8_t bits;   // TRIGGER_LATCHED | TRIGGER_ACTIVE_LOW
} trigger_t;

#if defined(__cplusplus)
}
#endif

#endif
