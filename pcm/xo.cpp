#include "spod_library.h"
#include "pcm_pins.h"
#include "xo.h"
#include <PCA9956B.h>


static const char* TAG = "XO";


#if 0
CIRCUIT 1   IREF0   LEDOUT0 0:1  (select 2:3)
CIRCUIT 2   IREF2   LEDOUT0 4:5  (select 6:7)
CIRCUIT 3   IREF4   LEDOUT1 0:1  (select 2:3)
CIRCUIT 4   IREF6   LEDOUT1 4:5  (select 6:7)
CIRCUIT X+  IREF8   LEDOUT2 0:1  (select 2:3)
CIRCUIT X-  IREF10  LEDOUT2 4:5  (select 6:7)
CIRCUIT 5   IREF23  LEDOUT5 2:3  (select 0:1)
CIRCUIT 6   IREF21  LEDOUT5 6:7  (select 4:5)
CIRCUIT 7   IREF19  LEDOUT4 2:3  (select 0:1)
CIRCUIT 8   IREF17  LEDOUT4 6:7  (select 4:5)
CIRCUIT Y+  IREF15  LEDOUT3 2:3  (select 0:1)
CIRCUIT Y-  IREF13  LEDOUT3 6:7  (select 4:5)
#endif

// PCA9956B is the port expander used to provide the ouputs,
// Refer to the spod/libraries/LEDDrivers_NXP_Arduino for specifics


static PCA9956B xo(Wire, I2C_PCA9956_ADDR);

static uint8_t pwm_reg[12]  = {PCA9956B::PWM0, PCA9956B::PWM2, PCA9956B::PWM4, PCA9956B::PWM6, PCA9956B::PWM8, PCA9956B::PWM10,   
                               PCA9956B::PWM23, PCA9956B::PWM21, PCA9956B::PWM19, PCA9956B::PWM17, PCA9956B::PWM15, PCA9956B::PWM13};

static uint8_t iref_reg[12]  =  {/*1*/  PCA9956B::IREF0, PCA9956B::IREF2, PCA9956B::IREF4, PCA9956B::IREF6, 
                                 /*X+*/ PCA9956B::IREF8, 
                                 /*X-*/ PCA9956B::IREF10,   
                                 /*5*/  PCA9956B::IREF23, PCA9956B::IREF21, PCA9956B::IREF19, PCA9956B::IREF17, 
                                 /*Y+*/ PCA9956B::IREF15, 
                                 /*Y-*/ PCA9956B::IREF13};
static uint8_t ledout_reg[12] = {/*1*/  PCA9956B::LEDOUT0, PCA9956B::LEDOUT0, PCA9956B::LEDOUT1, PCA9956B::LEDOUT1, 
                                 /*X+*/ PCA9956B::LEDOUT2,
                                 /*X-*/ PCA9956B::LEDOUT2,   
                                 /*5*/  PCA9956B::LEDOUT5, PCA9956B::LEDOUT5, PCA9956B::LEDOUT4, PCA9956B::LEDOUT4,
                                 /*Y+*/ PCA9956B::LEDOUT3,
                                 /*Y-*/ PCA9956B::LEDOUT3};
static uint8_t ctrl_ledout_shift[12] = {0, 4, 0, 4, 0, 4,   6, 2, 6, 2, 6, 2};
static uint8_t se_ledout_shift[12]   = {2, 6, 2, 6, 2, 6,   0, 4, 0, 4, 0, 4};
static uint8_t chan2idx[12] = {0, 1, 2, 3, 6, 7, 8, 9, 4, 5, 10, 11};


static void init( float current )
{
	xo.reg_w( PCA9956B::LEDOUT0, 0x55 ); // b01 turns circuit off in current hardware, b00 turns circuit on!
	xo.reg_w( PCA9956B::LEDOUT1, 0x55 );
	xo.reg_w( PCA9956B::LEDOUT2, 0x55 );
	xo.reg_w( PCA9956B::LEDOUT3, 0x55 );
	xo.reg_w( PCA9956B::LEDOUT4, 0x55 );
	xo.reg_w( PCA9956B::LEDOUT5, 0x55 );
	xo.write_r8( PCA9956B::PWMALL, 0x00 );
	xo.irefall( (uint8_t)(current * 255.0) ); // want all IREF set to 0xFF in current hardware
}


void xo_init()
{
    Wire.begin();
    xo.begin(1.0, PCA9956B::NONE, false);
    init(1.0);
}


void xo_set_se(uint8_t port, uint8_t mode /* XO_LEDOUT_ON or XO_LEDOUT_OFF */)
{
    uint8_t index = chan2idx[port];
    uint8_t ledout = xo.read_r8(ledout_reg[index]);
    uint8_t shift = se_ledout_shift[index];
    ledout &= ~(0x3<<shift);
    ledout |= (mode<<shift);
    xo.write_r8(ledout_reg[index], ledout);
}


void xo_set_alloff()
{
	xo.reg_w( PCA9956B::LEDOUT0, 0x00 );
	xo.reg_w( PCA9956B::LEDOUT1, 0x00 );
	xo.reg_w( PCA9956B::LEDOUT2, 0x00 );
	xo.reg_w( PCA9956B::LEDOUT3, 0x00 );
	xo.reg_w( PCA9956B::LEDOUT4, 0x00 );
	xo.reg_w( PCA9956B::LEDOUT5, 0x00 );
}


void xo_set_ledout(uint8_t port, uint8_t mode /* XO_LEDOUT_xx */)
{
    uint8_t index = chan2idx[port];
    uint8_t ledout = xo.read_r8(ledout_reg[index]);
    uint8_t shift = ctrl_ledout_shift[index];

    //LOGD(TAG, "%d -- %s(port=%d, mode=%x): ledout_reg[%d]=%x, shift=%d, ledout=%x\r\n", 
    //    millis(), __FUNCTION__, port, mode, port, ledout_reg[index], shift, ledout);

    ledout &= ~(0x3<<shift);
    ledout |= (mode<<shift);
    xo.write_r8(ledout_reg[index], ledout);

    //LOGD(TAG, "%d -- %s(): new ledout=%x (read %x)\r\n", 
    //    millis(), __FUNCTION__, ledout, xo.read_r8(ledout_reg[index]));
}


void xo_set_iref(uint8_t port, uint8_t iref)
{
    //LOGD(TAG, "%d -- %s(port=%d, iref=%x): iref_reg[%d]=%x\r\n", millis(), __FUNCTION__, port, iref, port, iref_reg[port]);
    //xo.write_r8(iref_reg[port], iref);
}


void xo_set_pwm(uint8_t port, uint8_t duty_cycle)
{
    //xo.write_r8(pwm_reg[port], duty_cycle);
}


void xo_dump_regs()
{
 	LOGD(TAG, "%d -- LEDOUT0=0x%x\r\n", millis(), xo.read_r8( PCA9956B::LEDOUT0 ));
 	LOGD(TAG, "%d -- LEDOUT1=0x%x\r\n", millis(), xo.read_r8( PCA9956B::LEDOUT1 ));
 	LOGD(TAG, "%d -- LEDOUT2=0x%x\r\n", millis(), xo.read_r8( PCA9956B::LEDOUT2 ));
 	LOGD(TAG, "%d -- LEDOUT3=0x%x\r\n", millis(), xo.read_r8( PCA9956B::LEDOUT3 ));
 	LOGD(TAG, "%d -- LEDOUT4=0x%x\r\n", millis(), xo.read_r8( PCA9956B::LEDOUT4 ));
    LOGD(TAG, "%d -- IREF0=0x%x\r\n", millis(), xo.read_r8(PCA9956B::IREF0));
    LOGD(TAG, "%d -- PWM0=0x%x\r\n", millis(), xo.read_r8(PCA9956B::PWM0));
}