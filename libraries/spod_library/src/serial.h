#ifndef __SERIAL_H__
#define __SERIAL_H__

#define MAX_DATARATE 921600  // 10.85 us/byte
//#define MAX_DATARATE 460800  // 21.7 us/byte
#define MIN_DATARATE 115200  // 87 us/byte

#define DATA_RATE MAX_DATARATE
//#define DATA_RATE 115200    // 86.8 us/byte
//#define DATA_RATE 230400    // 43.4 us/byte

#define DATA_RATE_SCALE (MAX_DATARATE/DATA_RATE)

// US_PER_BYTE is used for RS-485 to determine how long to delay for RE/DE settle time
#if DATA_RATE == 115200
#define US_PER_BYTE 87
#elif DATA_RATE == 230400
#define US_PER_BYTE 44
#elif DATA_RATE == 460800
#define US_PER_BYTE 22
#elif DATA_RATE == 921600
#define US_PER_BYTE 11
#else
#error "UNKNOWN DATA RATE!"
#endif


extern uint32_t serial_break_error;
extern uint32_t serial_buffer_full_error;
extern uint32_t serial_fifo_ovf_error;
extern uint32_t serial_frame_error;
extern uint32_t serial_parity_error;
extern uint32_t serial_other_error;

extern void serial_lock();
extern void serial_unlock();
extern void serial_clear_errors();
extern void serial_init(int dePin);
extern uint16_t serial_available();
extern bool serial_get(uint8_t* data);
extern bool serial_get_singleton(uint8_t* data);
extern size_t serial_read(uint8_t* data, size_t leng);
extern bool serial_write(uint8_t* data, size_t leng);
extern void serial_acked();
extern void serial_wait_for_txidle();

#endif
