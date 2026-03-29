#ifndef _SERIAL_H_
#define _SERIAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define MAIN_SERIAL_DEV "/dev/ttyUSB0"
#define ALT_SERIAL_DEV  "/dev/ttyMSM1"

#define MAX_DATARATE 460800
#define MIN_DATARATE 9600
//#define SERIAL_DATA_RATE 115200
//#define SERIAL_DATA_RATE MAX_DATARATE
#define SERIAL_DATA_RATE MAX_DATARATE
#define DATA_RATE_SCALE (MAX_DATARATE/SERIAL_DATA_RATE)

extern int serial_setbaud(int fd, unsigned baud_rate);
extern int serial_config(int fd, unsigned baud_rate);
extern int serial_open(char* devname, unsigned baud_rate);

#ifdef __cplusplus
}
#endif

#endif
