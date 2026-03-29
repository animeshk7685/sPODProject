#define YIELD()

class GDTransport {
private:
  byte model;
public:
  void ios() {
    pinMode(CS, OUTPUT);       // pin 10
    digitalWrite(CS, HIGH);
    //pinMode(SD_PIN, OUTPUT);   // pin 3
    //digitalWrite(SD_PIN, HIGH);
  }
  void begin0() {
    Serial.printf("%s/%s()\r\n", __FILE__, __FUNCTION__);
    ios();
    SPI.begin(6, 2, 7, 10);  // LCE-GRH: ADDED sck, miso, mosi, ss -- used to be line commented above

    hostcmd(0x42);    // SLEEP
    hostcmd(0x61);    // CLKSEL default
    hostcmd(0x00);    // ACTIVE
    hostcmd(0x48);    // CLKINT
    hostcmd(0x49);    // PD_ROMS all up
    hostcmd(0x68);    // RST_PULSE
  }
  void begin1() {
    uint16_t data;
    while ((__rd16(0xc0000UL) & 0xff) != 0x08) continue;
    data = __rd16(0x0c0000);
    ft8xx_model = data >> 12;  // non-zero for FT81x, zero for FT80x (the '0' or '1' of FT8xX)
    Serial.printf("%s/%s(): *0xc0000=0x%x (ft8xx_model=%d)\r\n", __FILE__, __FUNCTION__, data, ft8xx_model); Serial.flush();
  
    wp = 0;
    freespace = 4096 - 4;

    stream();
  }

  void cmd32(uint32_t x) {
    if (freespace < 4) {
      getfree(4);
    }
    wp += 4;
    freespace -= 4;
    union {
      uint32_t c;
      uint8_t b[4];
    };
    c = x;
    SPI.transfer(b[0]);
    SPI.transfer(b[1]);
    SPI.transfer(b[2]);
    SPI.transfer(b[3]);
  }
  void cmdbyte(byte x) {
    if (freespace == 0) {
      getfree(1);
    }
    wp++;
    freespace--;
    SPI.transfer(x);
  }
  void cmd_n(byte *s, uint16_t n) {
    if (freespace < n) {
      getfree(n);
    }
    wp += n;
    freespace -= n;
    while (n > 8) {
      n -= 8;
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
      SPI.transfer(*s++);
    }
    while (n--)
      SPI.transfer(*s++);
  }

  void flush() {
    YIELD();
    getfree(0);
  }
  uint16_t rp() {
    uint16_t r = __rd16(REG_CMD_READ);
    if (r == 0xfff) {
      GD.alert("COPROCESSOR EXCEPTION");
    }
    return r;
  }
  void finish() {
    wp &= 0xffc;
    __end();
    __wr16(REG_CMD_WRITE, wp);
    while (rp() != wp)
      YIELD();
    stream();
  }

  byte rd(uint32_t addr)
  {
    __end(); // stop streaming
    __start(addr);
    SPI.transfer(0);  // dummy
    byte r = SPI.transfer(0);
    stream();
    return r;
  }

  void wr(uint32_t addr, byte v)
  {
    __end(); // stop streaming
    __wstart(addr);
    SPI.transfer(v);
    stream();
  }

  uint16_t rd16(uint32_t addr)
  {
    uint16_t r = 0;
    __end(); // stop streaming
    __start(addr);
    SPI.transfer(0);
    r = SPI.transfer(0);
    r |= (SPI.transfer(0) << 8);
    stream();
    return r;
  }

  void wr16(uint32_t addr, uint32_t v)
  {
    __end(); // stop streaming
    __wstart(addr);
    SPI.transfer(v);
    SPI.transfer(v >> 8);
    stream();
  }

  uint32_t rd32(uint32_t addr)
  {
    __end(); // stop streaming
    __start(addr);
    SPI.transfer(0);
    union {
      uint32_t c;
      uint8_t b[4];
    };
    b[0] = SPI.transfer(0);
    b[1] = SPI.transfer(0);
    b[2] = SPI.transfer(0);
    b[3] = SPI.transfer(0);
    stream();
    return c;
  }
  void rd_n(byte *dst, uint32_t addr, uint16_t n)
  {
    __end(); // stop streaming
    __start(addr);
    SPI.transfer(0);
    while (n--)
      *dst++ = SPI.transfer(0);
    stream();
  }
  void wr_n(uint32_t addr, byte *src, uint16_t n)
  {
    __end(); // stop streaming
    __wstart(addr);
    while (n--)
      SPI.transfer(*src++);
    stream();
  }

  void wr32(uint32_t addr, unsigned long v)
  {
    __end(); // stop streaming
    __wstart(addr);
    SPI.transfer(v);
    SPI.transfer(v >> 8);
    SPI.transfer(v >> 16);
    SPI.transfer(v >> 24);
    stream();
  }

  uint32_t getwp(void) {
    return RAM_CMD + (wp & 0xffc);
  }

  void bulk(uint32_t addr) {
    __end(); // stop streaming
    __start(addr);
  }
  void resume(void) {
    stream();
  }

  static void __start(uint32_t addr) // start an SPI read transaction to addr
  {
    digitalWrite(CS, LOW);
    SPI.transfer(addr >> 16);
    SPI.transfer(highByte(addr));
    SPI.transfer(lowByte(addr));  
  }

  static void __wstart(uint32_t addr) // start an SPI write transaction to addr
  {
    digitalWrite(CS, LOW);
    SPI.transfer(0x80 | (addr >> 16));
    SPI.transfer(highByte(addr));
    SPI.transfer(lowByte(addr));  
  }

  static void __end() // end the SPI transaction
  {
    digitalWrite(CS, HIGH);
  }

  void stop() // end the SPI transaction
  {
    wp &= 0xffc;
    __end();
    __wr16(REG_CMD_WRITE, wp);
  }

  void stream(void) {
    __end();
    __wstart(RAM_CMD + (wp & 0xfff));
  }

  static unsigned int __rd16(uint32_t addr)
  {
    unsigned int r;

    __start(addr);
    SPI.transfer(0);  // dummy
    r = SPI.transfer(0);
    r |= (SPI.transfer(0) << 8);
    __end();
    return r;
  }

  static void __wr16(uint32_t addr, unsigned int v)
  {
    __wstart(addr);
    SPI.transfer(lowByte(v));
    SPI.transfer(highByte(v));
    __end();
  }

  void hostcmd(byte a)
  {
    digitalWrite(CS, LOW);
    SPI.transfer(a);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(CS, HIGH);
  }

  void getfree(uint16_t n)
  {
    wp &= 0xfff;
    __end();
    __wr16(REG_CMD_WRITE, wp & 0xffc);
    do {
      uint16_t fullness = (wp - rp()) & 4095;
      freespace = (4096 - 4) - fullness;
    } while (freespace < n);
    stream();
  }

  byte streaming;
  uint16_t wp;
  uint16_t freespace;
};
