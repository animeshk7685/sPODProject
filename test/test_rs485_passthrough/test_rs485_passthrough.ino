#include <ArduinoRS485.h>
#include "HardwareSerial.h"
#include "driver/uart.h"

#define RX_PIN 18
#define TX_PIN 19
#define DE_PIN 5
#define RE_PIN -1

//RS485Class rs485(Serial1, TX_PIN, DE_PIN, RE_PIN);


void receive_error_cb(hardwareSerial_error_t error)
{
  switch (error) {
    case UART_BREAK_ERROR:
      Serial.print("\n***BREAK***\n");
      break;
    case UART_BUFFER_FULL_ERROR:
      Serial.print("\n***BUFFER_FULL_ERROR***\n");
      break;    
    case UART_FIFO_OVF_ERROR:
      Serial.print("\n***FIFO_OVF_ERROR***\n");
      break;       
    case UART_FRAME_ERROR:
      Serial.print("\n***FRAME_ERROR***\n");
      break;         
    case UART_PARITY_ERROR:
      Serial.print("\n***PARITY_ERROR***\n");
      break;            
    default:
      Serial.printf("%s(): error=0x%x\n", __FUNCTION__, error);
      break;
  }
  Serial.flush();
}


void receive_cb(void)
{
  int count = 0;

  while (Serial1.available() > 0) {
    char data = Serial1.read();
    if (data == '\0') {
      Serial.printf("%d: NULL\n", ++count);
    } else {
      Serial.printf("%d: '%c'\n", ++count, data);
    }
  }
  Serial.flush();
}


void setup() {
  Serial.begin(115200);
  RS485.begin(115200, SERIAL_8N1);
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial1.onReceive(receive_cb, false);
  Serial1.onReceiveError(receive_error_cb);
  uart_set_rx_full_threshold(1,1);
  RS485.setDelays(50,0);

  RS485.endTransmission();
  Serial.println("setup()");
  Serial.flush();
}

void loop() {
  static uint32_t last = 0;
  uint32_t now = millis();

  if (now - last > 5000) {
    last = now;
    RS485.beginTransmission();
    RS485.printf("%d\r\n", now/1000);
    RS485.endTransmission();
  }
}
