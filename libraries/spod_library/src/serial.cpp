#include "spod_library.h"
#include <ArduinoRS485.h>
#include "HardwareSerial.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"


static const char* TAG = "SERIAL";


#define DEBUG_ENABLED 0

#define RX_PIN 18
#define TX_PIN 19
//#define DE_PIN 5
//#define RE_PIN -1

#define US_DELAY_BEFORE_TRANSMIT (US_PER_BYTE)
#define US_DELAY_AFTER_TRANSMIT  (0)


// refer to ~/.arduino15/packages/esp32/hardware/esp32/2.0.4/cores/esp32/HardwareSerial.h and HardwareSerial.cpp
//          esp-idf/components/driver/uart.c
//          esp-idf/components/include/driver/uart.h

// refer to Arduino/libraries/ArduinoRS485/src/RS485.cpp and RS485.h
//RS485Class rs485(Serial1, TX_PIN, DE_PIN, RE_PIN);

uint32_t serial_break_error = 0;
uint32_t serial_buffer_full_error = 0;
uint32_t serial_buffer_ovf_error = 0;
uint32_t serial_fifo_ovf_error = 0;
uint32_t serial_frame_error = 0;
uint32_t serial_parity_error = 0;
uint32_t serial_other_error = 0;
uint32_t serial_tx_verify_leng_error = 0;
uint32_t serial_tx_verify_data_error = 0;
uint32_t serial_tx_count = 0;

static uint8_t rx_buff[1024];
static ring_t rx_ring = {.size=sizeof(rx_buff), .head=0, .tail=0, .data=rx_buff};
static SemaphoreHandle_t lock;


void serial_lock()
{
    xSemaphoreTake(lock, portMAX_DELAY);
}


void serial_unlock()
{
    xSemaphoreGive(lock);
}


static const char* serial_rx_error_string(hardwareSerial_error_t error)
{
    switch (error) {
    case UART_BREAK_ERROR:       return "UART_BREAK_ERROR";
    case UART_BUFFER_FULL_ERROR: return "UART_BUFFER_FULL_ERROR";
    case UART_FIFO_OVF_ERROR:    return "UART_FIFO_OVF_ERROR";
    case UART_FRAME_ERROR:       return "UART_FRAME_ERROR";
    case UART_PARITY_ERROR:      return "UART_PARITY_ERROR";
    default:                     return "UART_ERROR_UNKNOWN";
    } 
}


static void serial_rx_error_cb(hardwareSerial_error_t error)
{
    switch (error) {
    case UART_BREAK_ERROR:       ++serial_break_error;       return;
    case UART_BUFFER_FULL_ERROR: ++serial_buffer_full_error; break;
    case UART_FIFO_OVF_ERROR:    ++serial_fifo_ovf_error;    break;
    case UART_FRAME_ERROR:       ++serial_frame_error;       break;
    case UART_PARITY_ERROR:      ++serial_parity_error;      break;
    default:
        ++serial_other_error;
        LOGE(TAG, "%s(): UNKNOWN ERROR=0x%x\r\n", __FUNCTION__, error);
        Serial.flush();
        return;
    }
    LOGD(TAG, "%s(): error=%d (%s)\r\n", __FUNCTION__, error, serial_rx_error_string(error));
}


static void serial_rx_cb(void)
{
    extern void pkt_notify();

    while (Serial1.available() > 0) {
        uint8_t data[128];
        size_t nread = Serial1.read(data, sizeof(data));
        #if DEBUG_ENABLED
        switch (nread) {
        case 0: break;
        case 1: dbg_printf(-1, "%s(nread=%d): 0x%x\r\n", __FUNCTION__, nread, data[0]); break;
        case 2: dbg_printf(-1, "%s(nread=%d): 0x%x 0x%x\r\n", __FUNCTION__, nread, data[0], data[1]); break;
        case 3: dbg_printf(-1, "%s(nread=%d): 0x%x 0x%x 0x%x\r\n", __FUNCTION__, nread, data[0], data[1], data[2]); break;
        default:dbg_printf(-1, "%s(nread=%d): 0x%x 0x%x 0x%x 0x%x\r\n", __FUNCTION__, nread, data[0], data[1], data[2], data[3]); break;
        }
        #endif
        if (!ring_write(&rx_ring, data, nread, NULL)) {
            ++serial_buffer_ovf_error;
            break;
        }
    }
    pkt_notify();
}


void serial_clear_errors()
{
    serial_break_error = 0;
    serial_buffer_full_error = 0;
    serial_buffer_ovf_error = 0;
    serial_fifo_ovf_error = 0;
    serial_frame_error = 0;
    serial_parity_error = 0;
    serial_other_error = 0;
}


void serial_init(int dePin)
{
    LOGD(TAG, "%s(): DATA_RATE=%d, dePin=%d\r\n", __FUNCTION__, DATA_RATE, dePin);

    // refer to Arduino/libraries/ArduinoRS485/src/RS485.cpp
    RS485.setPins(RS485_DEFAULT_TX_PIN, dePin, RS485_DEFAULT_RE_PIN);
    RS485.begin(DATA_RATE);  
    Serial1.begin(DATA_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

    Serial1.onReceive(serial_rx_cb, false);
    Serial1.onReceiveError(serial_rx_error_cb);
    if (!Serial1.setRxFIFOFull(4)) { // Size of ACK/NAK packet
        LOGE(TAG, "%s(): Serial1.setRxFIFOFULL(4) FAILED!\r\n", __FUNCTION__);
    }
    if (!Serial1.setRxTimeout(1)) { // This waits a millisecond before timing out -- not okay! (refer to serial_get_singleton() routine below...)
        LOGE(TAG, "%s(): Serial1.setRxTimeout(1) FAILED!\r\n", __FUNCTION__);
    }
    RS485.setDelays(US_DELAY_BEFORE_TRANSMIT /*usec to wait before transmit after setting DE high*/,
                    US_DELAY_AFTER_TRANSMIT /*usec to wait after flushing UART before setting DE low*/);
    RS485.endTransmission();  // puts RS485 in receive mode

    lock = xSemaphoreCreateMutex();
}


uint16_t serial_available()
{
    return ring_valid(&rx_ring);
}


size_t serial_read(uint8_t* data, size_t leng)
{
    size_t nread = ring_valid(&rx_ring);
    if (nread > leng) nread = leng;
    ring_read(&rx_ring, data, nread, true);
    return nread; 
}


bool serial_get(uint8_t* data)
{
    return ring_get(&rx_ring, data);
}


// why??? because the provided esp32 uart driver refuses to
// give up the last byte in the uart hardware rxfifo without
// a long timeout and we want this to be quick for rs485
// bus arbitration -- so we force the issue here...
bool serial_get_singleton(uint8_t* data)
{
    bool singleton = false;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    if (serial_get(data)) {
        // data available in our ring buffer
        singleton = true;
    } else
    if (Serial1.available()) {
        // data available in uart ring buffer
        singleton = Serial1.read(data, 1) == 1;
    }
    else
    if (uart_ll_get_rxfifo_len(UART_LL_GET_HW(UART_NUM_1)) > 0) {
        // data available in uart hardware rxfifo
        uart_ll_read_rxfifo(UART_LL_GET_HW(UART_NUM_1), data, 1);
        singleton = true;
    }
    #if DEBUG_ENABLED
    if (singleton) dbg_printf(-1, "%s(): 0x%x\n", __FUNCTION__, *data);
    #endif
    portEXIT_CRITICAL(&mux);
    return singleton;
}


bool serial_write(uint8_t* data, size_t leng)
{
    #if DEBUG_ENABLED
    switch (leng) {
    case 0: break;
    case 1: dbg_printf(-1, "%s(leng=%d): 0x%x\n", __FUNCTION__, leng, data[0]); break;
    case 2: dbg_printf(-1, "%s(leng=%d): 0x%x 0x%x\n", __FUNCTION__, leng, data[0], data[1]); break;
    case 3: dbg_printf(-1, "%s(leng=%d): 0x%x 0x%x 0x%x\n", __FUNCTION__, leng, data[0], data[1], data[2]); break;
    default:dbg_printf(-1, "%s(leng=%d): 0x%x 0x%x 0x%x 0x%x\n", __FUNCTION__, leng, data[0], data[1], data[2], data[3]); break;
    }
    #endif

    RS485.beginTransmission();
    RS485.write((const char*)data, leng);
    serial_wait_for_txidle(); // wait until all bytes have been transmitted out UART
    RS485.endTransmission();

    ++serial_tx_count;
    return true;
}


void serial_acked()
{
}


void serial_wait_for_txidle()
{
    uart_wait_tx_idle_polling(UART_NUM_1); // wait until all bytes have been transmitted out UART
}
