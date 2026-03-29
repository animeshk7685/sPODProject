#include "lib.h"
#include "common.h"
#include "board_type.h"
#include "pkt.h"
#include "pkt_type.h"
#include "dab.h"
#include "main.h"
#include "board.h"


#ifdef __cplusplus
extern "C"
{
#endif


#define VERSION "1.0.0"


int verbosity = 4;
struct pollfd fds[2];
static char serial_name[64];
board_info_t board_info;  // not used, only here to satisfy linker (sniffer only listens)


void die(int error)
{
    exit(error);
}


void show_version(void)
{
    printf("SPOD-SNIFFER v%s\n", VERSION);
    fflush(stdout);
}


static void help(char *program_name)
{
    printf("--debug 6 (or -d 6)    -- debug level\n");
    printf("--serial    (or -s)    -- specify serial device (e.g. /dev/ttyUSB0)\n");

    die(__LINE__);
}


static void get_cmd_line_args(char *program_name, int argc, char **argv)
{
    static struct option options[] = {
        {"debug",     required_argument, 0, 'd'},
        {"help",      no_argument,       0, 'h'},
        {"serial",    required_argument, 0, 's'},
        {0,0,0,0}
    };

    while (1) {
        int index = 0, ch = getopt_long(argc, argv, "d:hs:", options, &index);
        char version[MAX_PATHNAME];
        char* period;

        switch (ch) {
        case -1:  return;
        case 'd': verbosity = atoi(optarg); break;
        case 's': printf("%s(): serial_name was '%s', now '%s\n", __FUNCTION__, serial_name, optarg);
                  local_strncpy(serial_name, optarg, sizeof(serial_name)); break;
        case 'h':
        default:  help(program_name); exit(__LINE__);
        }
    }
}


static void serial_init(char *devname)
{
    fds[1].fd = serial_open(devname, SERIAL_DATA_RATE);
    if (fds[1].fd < 0) {
        fprintf(stderr, "%s(): failed to open serial_open('%s'), error %m\n", 
                __FUNCTION__, devname);
        die(__LINE__);
    }
    fprintf(stdout, "serial '%s' initialized, fd=%d\n", devname, fds[1].fd);
}


static void setup(void)
{
    fds[0].fd = 0;
    fds[1].fd = -1;
    serial_init(serial_name);

    fds[0].events = POLLIN;
    fds[1].events = POLLIN;

    fds[0].revents = 0;
    fds[1].revents = 0;
}


static void tear_down(void)
{
    if (fds[1].fd >= 0) close(fds[1].fd);
    fds[1].fd = -1;
}


int main(int argc, char **argv)
{
    uint8_t our_mac[3] = {0x11, 0x22,0x33};
    openlog("spod_sniffer", LOG_PERROR, LOG_DAEMON);
    local_strncpy(serial_name, "/dev/ttyUSB0", sizeof(serial_name));
    get_cmd_line_args(argv[0], argc, argv);

    setup();
    dab_init(fds[1].fd, our_mac, false);

    for (;;) {
        fds[0].revents = 0;
        fds[1].revents = 0;

        int rc = poll(fds, ARRAY_SIZE(fds), dab_queued()? 2 : 250);
        if (rc <= 0) {
            dab_process();
        } else {
            if (fds[1].revents) dab_process();
        }
    }

    tear_down();
    return 0;
}

#ifdef __cplusplus
}
#endif
