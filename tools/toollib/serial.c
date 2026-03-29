#include "lib.h"


#ifdef __cplusplus
extern "C"
{
#endif

static struct termios saved_termios;

int serial_setbaud(int fd, unsigned baud_rate)
{
    extern void setbaud(int fd, int speed);
    setbaud(fd, baud_rate);
    return 0;
}


int serial_config(int fd, unsigned baud_rate)
{
    struct termios options; 
    int rc;

    if ((rc=serial_setbaud(fd, baud_rate)) != 0)
        return rc;

    memset(&options, 0, sizeof(options));
    if ((rc=tcgetattr(fd, &options)) != 0)
        return rc;

    options.c_cflag |= (CLOCAL | CREAD);            // enable receiver, local mode
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);  // no parity 8N1
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;                    // no hardware flow control

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input, no echo
    options.c_iflag &= ~(IXON | IXOFF | IXANY);     // disable software flow control
    options.c_oflag &= ~(OPOST | OCRNL | ONLCR);    // raw output

    cfmakeraw(&options);

    options.c_cc[VMIN] = 0;                         // non-blocking mode VMIN = VTIME = 0
    options.c_cc[VTIME] = 0;                        //

    rc = tcsetattr(fd, TCSANOW, &options);
    return rc;
}


// open the specified serial port nd set the baud_rate appropriately,
// values for baud_rate are of the form B38400, B115200, etc.
//
// returns <0 on error >=0 on success
//
int serial_open(char* devname, unsigned baud_rate)
{
    int fd = open(devname, O_RDWR|O_NOCTTY|O_NDELAY|O_NONBLOCK);
    if (fd >= 0) {
        int rc = serial_config(fd, baud_rate);
        if (rc) {
            return rc;
        }
        fcntl(fd, F_SETFL, O_NONBLOCK); // non-blocking I/O (we use poll())
    }
    return fd;
}


static void shell_exit(void)
{
    tcsetattr(0, TCSANOW, &saved_termios);
}


/**
 * Initialize the shell module
 */
void shell_init(void)
{
    struct termios raw;
    char token[MAXSTRING];

    tcgetattr(0, &saved_termios);
    atexit(shell_exit);

    tcgetattr(0, &raw);
    raw.c_lflag &= ~(ICANON|ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &raw);
}

#ifdef __cplusplus
}
#endif
