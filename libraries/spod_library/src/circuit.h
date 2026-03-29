//-----------------------------------------------------------------------------
//  circuit.h
//
//-----------------------------------------------------------------------------

#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__

#if defined(__cplusplus)
extern "C" {
#endif


#define MAX_FREQ_HZ      100

#define CIRCUIT_OFF      0x00
#define CIRCUIT_NOM      0x01
#define CIRCUIT_FLASH    0x02
#define CIRCUIT_STROBE   0x03
#define CIRCUIt_BLINK    0x04
#define CIRCUIT_FAULT    0x20
#define CIRCUIT_OPEN     0x40
#define CIRCUIT_SHORT    0x80
    

typedef enum __attribute__((__packed__)) {
    OUTPUT_UNUSED = 0,
    OUTPUT_TOGGLE = 1,
    OUTPUT_STROBE = 2,
    OUTPUT_WIG = 3,
    OUTPUT_WAG = 4,
    OUTPUT_PRIMARY_MASK = 0x07,
    OUTPUT_SECONDARY_MASK = 0x70,
    PRIMARY_MOMENTARY = 0x08,
    SECONDARY_DOUBLE = 0x80
} circuit_output_type_t;


typedef struct __attribute__((__packed__)) {
    uint8_t  index;      // PCMS * CIRCUITS (0..79)
    uint8_t  output;     // circuit_output_type_t -- OUTPUT_UNUSED .. OUTPUT_WAG
    uint8_t  dim_level;  // starting dim level
    uint8_t  frequency;  // if OUTPUT_STROBE or OUTPUT_WIG or OUTPUT_WAG this is the frequency in Hz (max 100Hz)
    uint8_t  alwaysOn:1;   // 1-bit -- output always on (used to turn outputs back on when returning from deep sleep mode)
    uint8_t  isIgnCtrl:1;  // 1-bit -- only turn output on if ignSense
    uint8_t  isLockout:1;  // 1-bit -- don't modify output if set
    uint8_t  isCurrentRestart:1; // 1-bit -- true if output is auto re-enabled after current subsides when SHORT was detected
    uint8_t  avail1:4;
    uint8_t  currentLimit;  // can specify the current limit, otherwise hard-coded at 45.0
} circuit_config_t;


typedef struct __attribute__((__packed__)) {
    uint8_t   index;   // PCMS * CIRCUITS (0..79)
    uint8_t   status;  // CIRCUIT_OFF .. CIRCUIT_SHORT
    uint8_t   outCmd;  // the pwm value (dim_level)
    uint8_t   output;  // the actual pwm value to write out
    uint16_t  blinkOn; // msecs output should be on
    uint16_t  blinkOff;// msecs output should be off
    adcVals_t current; // in amps
} circuit_status_t;


typedef struct __attribute__((__packed__)) {
    circuit_status_t lastOut;     // used if needsRestart[x] is true these values are used to restart the output once current is okay
    uint8_t  lastSwVal; // 8-bits -- previous outCmd value (used if alwaysOn[x] is true)
    uint8_t  needsRestart:1; // 1-bit -- true if output should now restart because current is now okay (ephemeral)
    uint8_t  avail:7;
    uint32_t curTime;   // 32-bits -- used with proMode_t.timers[x] -- this is current millis() snapshot for the timer
    uint16_t timers;    // 16-bits -- only turn output on for this many minutes
} circuit_local_status_t;


extern circuit_status_t circuit_status[PCMS*CIRCUITS];
extern circuit_local_status_t circuit_local[CIRCUITS];


extern void circuit_off(circuit_status_t* c);
extern void circuit_toggle(circuit_status_t* c);
extern void circuit_strobe(circuit_status_t* c);
extern void circuit_wig(circuit_status_t* c);
extern void circuit_wag(circuit_status_t* c);
extern void send_circuit_status_pkt(uint8_t index);

#if defined(__cplusplus)
}
#endif

#endif
