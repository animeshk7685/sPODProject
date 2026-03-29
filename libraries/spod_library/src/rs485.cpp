#include "spod_library.h"


static const char* TAG = "RS485";

#define DEBUG_ENABLED           (0)
#define RS485_ENUMERATION       (0)
#define RS485_ARBITRATION       (1)

#define TASK_STACK_SIZE         8192

#define MAX_PRIORITY            (configMAX_PRIORITIES-1)
#define IDLE_PRIORITY           0
#define LOOP_PRIORITY           1
#define PKT_PRIORITY            2
#define RS485_PWM_PRIORITY      3
#define RS485_ARBITER_PRIORITY  MAX_PRIORITY

#define ATTN_PIN                rs485_attn_pin

#define TIMEOUT_US              (US_PER_BYTE*40)
#define BACKOFF_US              (0x800-1) // *MUST* be a power of 2 - 1

#define LOCK_RETRIES            5
#define ARB_RETRIES             3


static uint8_t rs485_ctrl_pin;
static uint8_t rs485_attn_pin;
static uint8_t rs485_addr = RS485_INVALID;
static bool pcm_active = true;

#if RS485_ENUMERATION
static bool upstream() {return pcm_int1();}
static void downstream(bool on) {pcm_int2(on);}
#endif

#if RS485_ARBITRATION
static TaskHandle_t task_handle;
static uint8_t macs[BOARD_MAX][3];
static uint8_t arbaddr;

static inline void set_attn_high() {digitalWrite(ATTN_PIN, HIGH);}
static inline void set_attn_low() {digitalWrite(ATTN_PIN, LOW);}
static inline bool attn_is_high() {return digitalRead(ATTN_PIN) == HIGH;}
static inline bool attn_is_low() {return digitalRead(ATTN_PIN) == LOW;}

static void IRAM_ATTR attn_isr()
{
    BaseType_t yield = pdFALSE;
    uint32_t status = attn_is_high();
    xTaskNotifyFromISR(task_handle, status, eSetBits, &yield);
    if (yield) portYIELD_FROM_ISR();
}

static inline void make_attn_an_input()
{
    static bool virgin = true;
    if (virgin && rs485_ctrl_pin != 0xFF) {
        virgin = false;
        pinMode(rs485_ctrl_pin, OUTPUT);
        digitalWrite(rs485_ctrl_pin, HIGH);
    }
    pinMode(ATTN_PIN, INPUT_PULLUP);
    attachInterrupt(ATTN_PIN, attn_isr, FALLING);
}

static inline void make_attn_an_output()
{
    detachInterrupt(ATTN_PIN);
    pinMode(ATTN_PIN, OUTPUT);
}


static void client_write_addr()
{
    if (rs485_addr != RS485_INVALID) {
        serial_write(&rs485_addr, sizeof(rs485_addr));
    } else {
        // controller doesn't have an RS485 address, ask the arbiter for one
        uint8_t data[4];
        data[0] = DLE;
        memcpy(data+1, pkt_ourmac(), PKT_MACLENG);
        serial_write(data, sizeof(data));
    }
}


// return address or RS485_ARBITER on timeout
static uint8_t client_arbitration()
{
    uint64_t start_time;
    for (int retry = 0; retry < ARB_RETRIES; ++retry) {
        start_time = get_microseconds();
        while (get_microseconds() - start_time < TIMEOUT_US) {
            uint8_t data;
            if (serial_get_singleton(&data)) {
                if (data == ENQ) {
                    client_write_addr();
                    start_time = get_microseconds();
                } else {
                    if (data == DLE || (data >= RS485_PCM1 && data <= RS485_CONTROLLERn)) {
                        #if DEBUG_ENABLED
                        dbg_printf(-1, "%s(): data=0x%x (%lldus)\n", __FUNCTION__, data, get_microseconds() - start_time);
                        #endif
                        return data;
                    }
                }
            }
        }
    }

    dbg_printf(-1, "%s(): TIMEOUT (%lldus)\n", __FUNCTION__, get_microseconds() - start_time);
    return RS485_ARBITER;
}
#endif


// release the rs485 lock, this includes making ATTN an input again
// and re-enabling attn_isr()
void rs485_unlock()
{
    #if RS485_ARBITRATION
    set_attn_high();
    make_attn_an_input();
    #endif
    serial_unlock();
}


// this routine returns with serial_lock() taken, rs485_unlock() must be called after a packet
// is written to the RS485 bus to rlease serial_lock()!
bool rs485_lock()
{
    #if RS485_ARBITRATION
    #if DEBUG_ENABLED
    dbg_printf(-1, "%s(): ATTN=%d\n", __FUNCTION__, attn_is_high());
    #endif
    for (int retry = 0; retry < LOCK_RETRIES; ++retry) {
        while (attn_is_low()) ms_delay(2);
        uint64_t start_time = get_microseconds();

        // when arbiter calls rs485_lock(), making ATTN an output will disable
        // attn_isr() so it will never activate and attempt to serial_lock()
        // as well (in case you were wondering)
        make_attn_an_output();
        set_attn_low();
        serial_lock();
        #if DEBUG_ENABLED
        dbg_printf(-1, "%s(): LOCKED\n", __FUNCTION__);
        #endif

        if (rs485_addr == RS485_ARBITER) {
            // arbiter is locking the bus, simply present arbiter owns bus
            uint8_t data[2] = {ENQ, RS485_ARBITER};
            serial_write(data, sizeof(data));
            #if DEBUG_ENABLED
            dbg_printf(-1, "%s(): ARBITER LOCKED BUS (%lldus)\n", __FUNCTION__, get_microseconds() - start_time);
            #endif
            arbaddr = RS485_ARBITER;
            dbg_drain();
            return true;
        }

        uint8_t addr = client_arbitration();
        if (addr == DLE) {
            // got back a DLE, if this is a controller the address is stale
            if (rs485_addr >= RS485_CONTROLLER1) rs485_addr = RS485_INVALID;
            dbg_printf(-1, "%s(): addr stale, setting to invalid and trying again...\n", __FUNCTION__);
        } else {
            if (rs485_addr == RS485_INVALID && addr >= RS485_CONTROLLER1) rs485_addr = addr; // set address if new
            if (addr == rs485_addr) {
                #if DEBUG_ENABLED
                dbg_printf(-1, "%s(): GRANTED addr=0x%x (%lld us)\n", __FUNCTION__, addr, get_microseconds() - start_time);
                #endif
                dbg_drain();
                return true; // RS485 bus granted and now locked for writing
            }
            dbg_printf(-1, "%s(): addr=0x%x NOT OURS!\n", __FUNCTION__, addr);
        }

        // failed to take ownership of RS485 bus, release lock, delay a random time and retry
        rs485_unlock();
        dbg_printf(-1, "%s(): OWNERSHIP FAILED (%lldus) -- retry %d\n", __FUNCTION__, get_microseconds() - start_time, retry);
        dbg_drain();
        if (retry != LOCK_RETRIES-1) delayMicroseconds(rand()&BACKOFF_US);
    }
    return false;
    #else
    serial_lock();
    #endif
    return true;
}


#if RS485_ARBITRATION
// slot 0 is returned if no empty slots
static uint8_t empty_slot()
{
    for (int slot = RS485_CONTROLLER1-RS485_BASE; slot <= RS485_CONTROLLERn-RS485_BASE; ++slot) {
        if (pkt_mac_zero(macs[slot])) {
            //dbg_printf(-1, "%s(): slot=%d\n", __FUNCTION__, slot);
            return slot;
        }
    }
    return 0;
}


// returning 0 means no controller found
static uint8_t find_controller(uint8_t* mac)
{
    for (int c = RS485_CONTROLLER1; c <= RS485_CONTROLLERn; ++c) {
        if (pkt_mac_match(macs[c-RS485_BASE], mac)) {
            #if DEBUG_ENABLED
            dbg_printf(-1, "%s(): controller=%x\n", __FUNCTION__, c);
            #endif
            return c;
        }
    }
    return 0;   
}


// if no empty slots RS485_BASE will be returned, which basically gives control back to arbiter
static uint8_t new_controller(uint8_t* mac)
{
    uint8_t slot = empty_slot();
    memcpy(macs[slot], mac, PKT_MACLENG);
    #if DEBUG_ENABLED
    dbg_printf(-1, "%s(): controller=%x\n", __FUNCTION__, slot|RS485_BASE);
    #endif
    return slot | RS485_BASE;
}


// return address or RS485_ARBITER on timeout
static uint8_t arbitration()
{
    uint64_t start_time;
    uint8_t enq = ENQ;
    for (int retry = 0; retry < ARB_RETRIES; ++retry) {
        serial_write(&enq, sizeof(enq));
        start_time = get_microseconds();
        while (get_microseconds() - start_time < TIMEOUT_US) {
            uint8_t data;
            if (serial_get_singleton(&data)) {
                if (data == DLE || (data >= RS485_PCM1 && data <= RS485_CONTROLLERn)) {
                    #if DEBUG_ENABLED
                    dbg_printf(-1, "%s(): data=0x%x (%lldus)\n", __FUNCTION__, data, get_microseconds() - start_time);
                    #endif
                    return data;
                }
            }
        }
    }

    dbg_printf(-1, "%s(): TIMEOUT (%lldus)\n", __FUNCTION__, get_microseconds() - start_time);
    return RS485_ARBITER;
}


// return true if three bytes recieved, false on timeout
static bool get_mac(uint8_t mac[PKT_MACLENG])
{
    uint64_t start_time = get_microseconds();
    int count = 0;

    while (get_microseconds() - start_time < TIMEOUT_US) {
        if (serial_get_singleton(mac + count)) {
            if (++count == PKT_MACLENG) {
                //dbg_printf(-1, "%s(): SUCCESS (%lldus)\n", __FUNCTION__, get_microseconds() - start_time);
                return true;
            }
        }
    }

    dbg_printf(-1, "%s(): TIMEOUT (%lldus)\n", __FUNCTION__, get_microseconds() - start_time);
    return false;
}


// if the addr does not have an associated MAC write DLE, otherwise write addr
static void grant_bus(uint8_t addr)
{
    if (addr >= RS485_PCM1 && addr <= RS485_CONTROLLERn) {
        if (addr >= RS485_CONTROLLER1 && pkt_mac_zero(macs[addr-RS485_BASE])) {
            dbg_printf(-1, "%s(): macs[%d] ZERO!!!\n", __FUNCTION__, addr-RS485_BASE);
            addr = DLE;
        }
        arbaddr = addr;
        serial_write(&addr, sizeof(addr));
    }
}


// arbitration _task() handles all rs485 bus arbitration
//
// rs485 arbitration, note that when the arbiter is writing packets
// to the bus, when it calls rs485_lock() it will change the ATTN
// line from an input to an output which will disable the attn_isr()
// so the arbitration task will not wake up and try to serial_lock()

static void arbitration_task(void* unused)
{
    while (pcm_active) {
        uint32_t status;
        if (xTaskNotifyWait(0x00, 0x00, &status, portMAX_DELAY/*ms_to_ticks(2000)*/)) {
            uint64_t start_time = get_microseconds();
            //dbg_printf(-1, "%s(): ATTN=%d\n", __FUNCTION__, attn_is_high());
            if (attn_is_low()) {
                serial_lock();
                uint8_t addr = arbitration();
                if (addr == DLE) {
                    uint8_t mac[3];
                    if (!get_mac(mac)) {
                        dbg_printf(-1, "%s(): wait_for_mac() failed!\n", __FUNCTION__);
                    } else {
                        addr = find_controller(mac);
                        if (addr != RS485_INVALID) {
                            //dbg_printf(-1, "%s(): controller 0x%x found\n", __FUNCTION__, addr);
                        } else {
                            addr = new_controller(mac);
                            //dbg_printf(-1, "%s(): new_controller 0x%x\n", __FUNCTION__, addr);
                        }
                    }
                }
                grant_bus(addr);
                serial_unlock();
                //dbg_printf(-1, "%s(): grant_bus(%x) (%lldus)\n", __FUNCTION__, addr, get_microseconds() - start_time);
                dbg_drain();
            }
        }
    }
}


static void start_arbitration_task()
{
    xTaskCreate(arbitration_task, "rs485_arbitration_task", TASK_STACK_SIZE, NULL, RS485_ARBITER_PRIORITY, &task_handle);
}
#else
static void start_arbitration_task() {}
#endif


#if RS485_ENUMERATION
static void pwm_task(void* _msecs)
{
    uint32_t msecs = (uint32_t)_msecs;

    ESP_LOGI(TAG, "%s(msecs=%d)", __FUNCTION__, msecs);
    for (bool on = true; pcm_active; on = !on) {
        downstream(on);
        ms_delay(msecs);
    }

    vTaskDelete(NULL);
}


static void start_pwm_task(uint32_t msecs)
{
    xTaskCreate(pwm_task, "rs485_pwm_task", TASK_STACK_SIZE, (void*)msecs, RS485_PWM_PRIORITY, NULL);
}


static uint32_t wait_for_falling_edge(void)
{
    uint32_t start = millis();
    while (millis() - start < 150) {
        if (!upstream()) return millis() - start;
    }
    return 0;
}


static uint32_t wait_for_rising_edge(void)
{
    uint32_t start = millis();
    while (millis() - start < 150) {
        if (upstream()) return millis() - start;
    }
    return 0;
}


static uint8_t upstream_neighbor(void)
{
    wait_for_falling_edge();
    for (;;) {
        uint32_t t1 = wait_for_rising_edge();
        uint32_t t2 = wait_for_falling_edge();
        if (t1 == t2 && t1 != 0) return (t1/30);
    }
}
#endif


void rs485_setup(board_type_t board_type, uint8_t ctrl_pin, uint8_t attn_pin)
{
    task_handle = xTaskGetCurrentTaskHandle();
    rs485_ctrl_pin = ctrl_pin;
    rs485_attn_pin = attn_pin;

    #if RS485_ARBITRATION
    make_attn_an_input();
    #endif

    #if RS485_ENUMERATION
    if (board_type == PDB_BOARD_TYPE) {
        downstream(true);
        delay(2);
    }
    #endif
}


uint8_t rs485_enumerate(board_type_t board_type, uint8_t ctrl_pin, uint8_t attn_pin)
{
    rs485_setup(board_type, ctrl_pin, attn_pin);
    #if RS485_ENUMERATION
    if (board_type == PDB_BOARD_TYPE) {
        uint32_t start = millis();
        while (millis() - start < 250) {
            if (upstream()) break;
        }
        rs485_addr = millis() - start >= 250? 0 : upstream_neighbor();
        if (rs485_addr < 3) {
            start_pwm_task((rs485_addr+1)*30);
        }
        rs485_addr |= RS485_BASE;

        if (rs485_addr == RS485_ARBITER) {
            start_arbitration_task();
        }
    }
    #else
    switch (board_type) {
    case PDB_BOARD_TYPE: rs485_addr = RS485_PCM1; start_arbitration_task(); break;
    case SDB_BOARD_TYPE: rs485_addr = RS485_CONTROLLER1; break;
    case TSB_BOARD_TYPE: rs485_addr = RS485_CONTROLLER1+1; break;
    default:             rs485_addr = RS485_INVALID; break;
    }
    #endif
    return rs485_addr;
}


uint8_t rs485_arbaddr()
{
    return arbaddr;
}


uint8_t rs485_getaddr()
{
    return rs485_addr;
}


// called when PCM1's heartbeat time goes backward signalling PCM1 rebooted,
// in this case this board's address is set to INVALID
void rs485_arbiter_lost()
{
    if (rs485_addr >= RS485_CONTROLLER1) rs485_addr = RS485_INVALID;
}


// we are the arbiter but just received a message that there is another
// arbiter in the system, shutdown this arbiter (TODO: including BLE?)
void rs485_arbiter_duplicate()
{
    #if RS485_ENUMERATION
    rs485_addr = RS485_INVALID;
    pcm_active = false; // this will cause the two tasks to terminate
    #endif
}