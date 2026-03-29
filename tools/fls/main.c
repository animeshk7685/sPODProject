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


typedef enum {
    COMATOSE,
    DISCOVERY,
    DO_LS,
    READ_DATA,
    WAIT_FOR_ACK,
    COMPLETE
} FLS_STATE;


int verbosity = 4;
struct pollfd fds[2];
static char serial_name[64];

static FLS_STATE fls_state = COMATOSE;

static char  board_name[64];
board_info_t board_info;
board_desc_t board_desc;


void die(int error)
{
    exit(error);
}


static void show_version(void)
{
    printf("\nFLS v%s\n", VERSION);
    fflush(stdout);
}


static void help(char *program_name)
{
    show_version();
    printf("--debug 6            (or -d)  -- debug level (default is %d)\n", verbosity);
    printf("--serial <dev name>  (or -s)  -- specify serial device (e.g. /dev/ttyUSB0)\n");
    printf("--board <board_name> (or -b)  -- specify SPOD board name or MAC (e.g. 'spod1', 'pdb2', 'touchscreen', 'switch', '03:af:bc')\n");
    die(__LINE__);
}


static void get_cmd_line_args(char *program_name, int argc, char **argv)
{
    static struct option options[] = {
        {"debug",     required_argument, 0, 'd'},
        {"help",      no_argument,       0, 'h'},
        {"serial",    required_argument, 0, 's'},
        {"board",     required_argument, 0, 'b'},
        {0,0,0,0}
    };

    while (1) {
        int index = 0, ch = getopt_long(argc, argv, "b:d:f:hs:", options, &index);
        char version[MAX_PATHNAME];
        char* period;

        switch (ch) {
        case -1:  return;
        case 'b': local_strncpy(board_name, optarg, sizeof(board_name)); break;
        case 'd': verbosity = atoi(optarg); break;
        case 's': printf("%s(): serial_name was '%s', now '%s\n", __FUNCTION__, serial_name, optarg);
                  local_strncpy(serial_name, optarg, sizeof(serial_name)); break;
        case 'h':
        default:  help(program_name); exit(__LINE__);
        }
    }
}


static const char* state_string(FLS_STATE state)
{
    switch (state) {
        case COMATOSE:     return "COMATOSE";
        case DISCOVERY:    return "DISCOVERY";
        case DO_LS:        return "DO_LS";
        case READ_DATA:    return "READ_DATA";
        case WAIT_FOR_ACK: return "WAIT_FOR_ACK";
        case COMPLETE:     return "COMPLETE";
        default:           return "UNKNOWN";
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


static void process_abort_packet(uint8_t ptype, uint8_t* pdata, uint8_t pleng)
{
    if (pleng > 128) pleng = 128;
    pdata[pleng] = '\0';
    dbg_printf(-1, "ABORT(%s-%02x:%02x:%02x '%s')\n", 
        board_type_string((board_type_t)(ptype&MASK_BOARD_TYPE)), pdata[0], pdata[1], pdata[2], pdata+3);
    exit(__LINE__);
}


static void process_fdat_packet(uint8_t ptype, uint8_t* pdata, uint8_t pleng)
{
    dbg_printf(-1, "FDAT(%s-%02x:%02x:%02x '%s')\n", 
        board_type_string((board_type_t)(ptype&MASK_BOARD_TYPE)), pdata[0], pdata[1], pdata[2], pdata+3);
}


static void process_fack_packet(uint8_t ptype, uint8_t* pdata, uint8_t pleng)
{
    uint8_t mac[3];

    if (pleng < 4) {
        dbg_printf(-1, "Expected FACK packet length too short -- %d\n", pleng);
    } else {
        memcpy(mac, pdata, sizeof(mac)); pdata += sizeof(mac);
        pleng -= sizeof(mac);

        if (pleng > 128) pleng = 128;
        pdata[pleng] = '\0';
        dbg_printf(-1, "FACK(%s-%02x:%02x:%02x '%s')\n", 
            board_type_string((board_type_t)(ptype&MASK_BOARD_TYPE)), mac[0], mac[1], mac[2], pdata);
    }
    if (fls_state == READ_DATA) fls_state = COMPLETE;
}


int process_input(uint8_t seqno, uint8_t ptype, uint8_t* pdata, uint8_t pleng)
{
    switch (ptype & MASK_PKT_TYPE) {
    case HEARTBEAT_PKT_TYPE: board_process_heartbeat_packet(pdata, pdata+3, ptype, pleng-3); break;
    case ABORT_PKT_TYPE:     process_abort_packet(ptype, pdata, pleng); break;
    case FDAT_PKT_TYPE:      process_fdat_packet(ptype, pdata, pleng); break;
    case FACK_PKT_TYPE:      process_fack_packet(ptype, pdata, pleng); break;
    default: break;
    }
    return 0;
}


static void do_ls(void)
{
    char base_name[MAXSTRING];
    uint8_t payload[1] = {FLS_PKT_TYPE};
    if (dab_output(SYS_PKT_TYPE, payload, 1, NULL) == 0) {
        fls_state = READ_DATA;
    } else {
        printf("Unable to output FLS_PKT!!!\n");
        fls_state = COMPLETE;
    }
}


int main(int argc, char **argv)
{
    uint8_t our_mac[3] = {0x11, 0x22,0x33};
    FLS_STATE previous_state = COMATOSE;

    openlog("spod_fls", LOG_PERROR, LOG_DAEMON);
    const char* spod_tools_serial_device = getenv("SPOD_TOOLS_SERIAL_DEVICE");
    const char* spod_tools_board_name = getenv("SPOD_TOOLS_BOARD_NAME");
    local_strncpy(serial_name, spod_tools_serial_device != NULL? spod_tools_serial_device : "/dev/ttyUSB0", sizeof(serial_name));
    local_strncpy(board_name, spod_tools_board_name != NULL? spod_tools_board_name : "spod", sizeof(board_name));

    get_cmd_line_args(argv[0], argc, argv);

    srandom(now());
    setup();
    dab_init(fds[1].fd, our_mac, false);
    dab_register_input(process_input);

    fls_state = determine_board_type(board_name, &board_desc)? DO_LS : COMATOSE;
    while (fls_state != COMPLETE) {
        fds[0].revents = 0;
        fds[1].revents = 0;

        int rc = poll(fds, ARRAY_SIZE(fds), dab_queued()? 2 : 250);
        if (rc <= 0) {
            dab_process();
        } else {
            if (fds[1].revents) dab_process();
        }

        if (previous_state != fls_state) {
            dbg_printf(0, "%s --> %s\n", state_string(previous_state), state_string(fls_state));
            previous_state = fls_state;
        }
        switch (fls_state) {
        case COMATOSE:
            fls_state = DISCOVERY;
            break;
        case DISCOVERY:
            if (board_tad_search(board_desc.type, board_desc.addr, &board_info)) {
                memcpy(board_desc.mac, board_info.mac, sizeof(board_desc.mac));
                fls_state = DO_LS;
            }
            break;
        case DO_LS:
            do_ls();
            break;
        case READ_DATA:
            break;
        case WAIT_FOR_ACK:
        case COMPLETE:
        default:
            break;
        }
    }

    tear_down();
    return 0;
}

#ifdef __cplusplus
}
#endif