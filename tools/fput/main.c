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
    DO_PUT,
    WRITE_CHUNK,
    WAIT_FOR_ACK,
    COMPLETE
} FPUT_STATE;


int verbosity = 4;
struct pollfd fds[2];
static char serial_name[64];
static char file_name[128];
static FILE* file_desc;

static FPUT_STATE fput_state = COMATOSE;

static char  board_name[64];
board_info_t board_info;
board_desc_t board_desc;


void die(int error)
{
    exit(error);
}


static void show_version(void)
{
    printf("\nFPUT v%s\n", VERSION);
    fflush(stdout);
}


static void help(char *program_name)
{
    show_version();
    printf("--debug 6            (or -d)  -- debug level (default is %d)\n", verbosity);
    printf("--serial <dev name>  (or -s)  -- specify serial device (e.g. /dev/ttyUSB0)\n");
    printf("--board <board_name> (or -b)  -- specify SPOD board name or MAC (e.g. 'spod1', 'pdb2', 'touchscreen', 'switch', '03:af:bc')\n");
    printf("--file <file_name>   (or -f)  -- specify name of file to download\n");
    die(__LINE__);
}


static void get_cmd_line_args(char *program_name, int argc, char **argv)
{
    static struct option options[] = {
        {"debug",     required_argument, 0, 'd'},
        {"help",      no_argument,       0, 'h'},
        {"serial",    required_argument, 0, 's'},
        {"board",     required_argument, 0, 'b'},
        {"file",      required_argument, 0, 'f'},
        {0,0,0,0}
    };

    if (argc == 1) {
        printf("Must specify at least a filename to PUT\n");
        help(program_name);
    }

    while (1) {
        int index = 0, ch = getopt_long(argc, argv, "b:d:f:hs:", options, &index);
        char version[MAX_PATHNAME];
        char* period;

        switch (ch) {
        case -1:  return;
        case 'b': local_strncpy(board_name, optarg, sizeof(board_name)); break;
        case 'd': verbosity = atoi(optarg); break;
        case 'f': local_strncpy(file_name, optarg, sizeof(file_name)); break;
        case 's': printf("%s(): serial_name was '%s', now '%s\n", __FUNCTION__, serial_name, optarg);
                  local_strncpy(serial_name, optarg, sizeof(serial_name)); break;
        case 'h':
        default:  help(program_name); exit(__LINE__);
        }
    }
}


static const char* state_string(FPUT_STATE state)
{
    switch (state) {
        case COMATOSE: return "COMATOSE";
        case DISCOVERY: return "DISCOVERY";
        case DO_PUT: return "DO_PUT";
        case WRITE_CHUNK: return "WRITE_CHUNK";
        case WAIT_FOR_ACK: return "WAIT_FOR_ACK";
        case COMPLETE: return "COMPLETE";
        default: return "UNKNOWN";
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
}


static void process_fack_packet(uint8_t ptype, uint8_t* pdata, uint8_t pleng)
{
    if (pleng > 128) pleng = 128;
    pdata[pleng] = '\0';
    dbg_printf(-1, "FACK(%s-%02x:%02x:%02x '%s')\n", 
        board_type_string((board_type_t)(ptype&MASK_BOARD_TYPE)), pdata[0], pdata[1], pdata[2], pdata+3);
    /*if (fput_state == WAIT_FOR_ACK)*/ fput_state = COMPLETE;
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


static void do_put(void)
{
    dab_setmac(board_desc.mac);

    file_desc = fopen(file_name, "r");
    uint32_t size = file_size(file_name);
    if (size == 0 || file_desc == NULL) {
        printf("File '%s' does not exist!\n", file_name);
        fput_state = COMPLETE;
    } else {
        char base_name[MAXSTRING];
        uint32_t crc = file_crc32(file_name);
        uint32_t file_name_length = strlen(file_name)+1;
        uint8_t payload[128];
        uint8_t nbytes = 0;

        base_filename(file_name, base_name);
        file_name_length = strlen(base_name)+1;
        if (file_name_length >= sizeof(payload)-sizeof(size)-sizeof(crc)) {
            file_name_length = sizeof(payload) - sizeof(size) - sizeof(crc) - 1;
        }
        payload[0] = FPUT_PKT_TYPE; nbytes += 1;
        memcpy(payload + nbytes, &size, sizeof(size)); nbytes += sizeof(size);
        memcpy(payload + nbytes, &crc, sizeof(crc)); nbytes += sizeof(crc);
        memcpy(payload + nbytes, base_name, file_name_length); nbytes += file_name_length;
        payload[sizeof(payload)-1] = '\0';

        if (dab_output(SYS_PKT_TYPE, payload, nbytes, NULL) == 0) {
            fput_state = WRITE_CHUNK;
        } else {
            printf("Unable to output FPUT_PKT!!!\n");
            fput_state = COMPLETE;
        }
    }
}


static void write_chunk(void)
{
    if (!dab_txq_full()) {
        uint8_t data[128];
        long offset = ftell(file_desc);
        uint8_t nbytes = fread(data, 1, sizeof(data), file_desc);
        if (nbytes > 0) { 
            if (dab_output(FDAT_PKT_TYPE, data, nbytes, NULL) != 0) {
                fseek(file_desc, offset, SEEK_SET);
                usleep(1000*250); // out of buffer space -- sleep 250ms to let system catch up
            }
            usleep(100000);
        } else {
            fput_state = WAIT_FOR_ACK;
        }
    }
}


int main(int argc, char **argv)
{
    uint8_t our_mac[3] = {0x11, 0x22,0x33};
    FPUT_STATE previous_state = COMATOSE;

    openlog("spod_fput", LOG_PERROR, LOG_DAEMON);
    const char* spod_tools_serial_device = getenv("SPOD_TOOLS_SERIAL_DEVICE");
    const char* spod_tools_board_name = getenv("SPOD_TOOLS_BOARD_NAME");
    local_strncpy(serial_name, spod_tools_serial_device != NULL? spod_tools_serial_device : "/dev/ttyUSB0", sizeof(serial_name));
    local_strncpy(board_name, spod_tools_board_name != NULL? spod_tools_board_name : "spod", sizeof(board_name));

    get_cmd_line_args(argv[0], argc, argv);

    if (strlen(file_name) == 0) {
        printf("Must specify a file to PUT!\n");
        help(argv[0]);
    }

    srandom(now());
    setup();
    dab_init(fds[1].fd, our_mac, false);
    dab_register_input(process_input);

    fput_state = determine_board_type(board_name, &board_desc)? DO_PUT : DISCOVERY;
    dbg_printf(-1, "%s(): fput_state=%d, board_name='%s'\r\n", __FUNCTION__, fput_state, board_name);

    while (fput_state != COMPLETE) {
        fds[0].revents = 0;
        fds[1].revents = 0;

        int rc = poll(fds, ARRAY_SIZE(fds), dab_queued()? 2 : 250);
        if (rc <= 0) {
            dab_process();
        } else {
            if (fds[1].revents) dab_process();
        }

        if (previous_state != fput_state) {
            dbg_printf(0, "%s --> %s\n", state_string(previous_state), state_string(fput_state));
            previous_state = fput_state;
        }
        switch (fput_state) {
        case COMATOSE:
            fput_state = DISCOVERY;
            break;
        case DISCOVERY:
            if (board_tad_search(board_desc.type, board_desc.addr, &board_info)) {
                memcpy(board_desc.mac, board_info.mac, sizeof(board_desc.mac));
                fput_state = DO_PUT;
            }
            break;
        case DO_PUT:
            do_put();
            break;
        case WRITE_CHUNK:
            write_chunk();
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
