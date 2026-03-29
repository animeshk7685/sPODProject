#ifndef __STATUS_H__
#define __STATUS_H__

#define STATUS_MAGIC 0xF00D
#define STATUS_VERSION 1

#define PROMODE_MAGIC 0xFEED
#define PROMODE_VERSION 1


typedef struct __attribute__((__packed__)) {
    uint8_t dimVal; // dimVal -- duty cycle (output_t.outCmd is set to this)
    uint8_t onVal;  // on time -- output_t.blinkOn set to this
    uint8_t offVal; // off time -- output_t.blinkOff set to this
} lastVals_t;

typedef struct __attribute__((__packed__)) {
    uint16_t  blinkOn; // msecs output should be on
    uint16_t  blinkOff;// msecs output should be off
    adcVals_t current; // in amps
    uint8_t   outCmd;  // the pwm value
    uint8_t   output;  // the actual pwm value to write out
    uint8_t   status_flags;// OUTPUT_SHORT
                       // OUTPUT_OPEN
                       // OUTPUT_NOM
                       // OUTPUT_FLASH
                       // OUTPUT_FAULT
} output_t;


// these used to be part of proMode_t -- this information is either stored somewhere else or not used anymore
//uint16_t  isInputLatch:1;// 1-bit -- true if input is latched, otherwise edge
//uint16_t  isInputEnabled:1; // 1-bit per input -- if set input is enabled, otherwise disabled
//uint16_t  isInputLockout:1; // 1-bit per input -- if true input is disabled (and isInputLockActive[x] is set to true)
//uint16_t  isInputLockActive:1; // 1-bit per input -- true if isInputLockout[x] and input would otherwise be used

typedef struct __attribute__((__packed__)) {
    uint16_t  crc;
    uint16_t  magic;
    uint16_t  version;
    uint16_t  sizeof_this;

    uint8_t   alwaysOn:1;   // 1-bit -- output always on (used to turn outputs back on when returning from deep sleep mode)
    uint8_t   isIgnCtrl:1;  // 1-bit -- only turn output on if ignSense
    uint8_t   isLockout:1;  // 1-bit -- don't modify output if set
    uint8_t   isCurrentRestart:1; // 1-bit -- true if output is auto re-enabled after current subsides when SHORT was detected
    uint8_t   needsRestart:1; // 1-bit -- true if output should now restart because current is now okay (ephemeral)
    uint8_t   avail1:3;
    uint8_t   lastSwVal; // 8-bits -- previous outCmd value (used if alwaysOn[x] is true)
    uint8_t   currentLimit;  // can specify the current limit, otherwise hard-coded at 45.0
    uint8_t   avail2;
    uint32_t  curTime;   // 32-bits -- used with proMode_t.timers[x] -- this is current millis() snapshot for the timer
    uint16_t  timers;    // 16-bits -- only turn output on for this many minutes
    output_t  lastOut;     // used if needsRestart[x] is true these values are used to restart the output once current is okay
} proMode_t;

typedef struct __attribute__((__packed__)) {
    uint16_t crc;                       // CRC16 of remaining config_t structure, if 0 human generated file, assume good!
    uint16_t magic;                     // STATUS_MAGIC
    uint16_t version;                   // STATUS_VERSION
    uint16_t sizeof_this;             // sizeof(status_t)
    // don't add anything before this

    uint32_t    passkey;
    bool        debug;
    uint8_t     address;        // 0x80 - 0x83
    bool        ignSense;       // true if ignition is on
    bool        inputs[8];      // 8 1-bit values, non-zero if input triggered
    adcVals_t    batVolt;        // in volts
    adcVals_t    temp;
    bool        lvBypass;       // unused?
    bool        tempEdge;       // non-zero if edge temperature is too great
    bool        isUnsecured;    // BLE secured or unsecure
    output_t    out[8];
    proMode_t   pro[8];
    bool        isResetForSecUpdate; // 1-bit -- BLE security
    bool        isDeepSleepEnabled; // 1-bit -- if true deepsleep is enabled -- actually goes into deep sleep mode
    bool        isDisableSleep; // 1=bit -- don't go to sleep in PCM -- won't disable outputs (so different than status_t.isDeepSleepEnabled)
} status_t;

extern lastVals_t lastRecVals[PCMS*CIRCUITS]; // output set to these values when turned on -- last received values
extern uint32_t shortDelay[CIRCUITS]; // msecs -- used to timeout output when short occurs

#endif