/* ========================================
 *
 * Copyright Star Technologies, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF STAR TECHNOLOGIES.
 *
 * ========================================
*/

#ifndef STDDRIVERS_H
#define STDDRIVERS_H

    #include "common.h"
//#include <project.h>    
//#include "myPrintf.h"   
    
//#define INPUTS_ADD          0x21        // Shift << 1 and | with r/w for full string
//#define OUTPUTS_ADD         0x42        // Shift << 1 and | with r/w for full string  
//    
//#define OUTPUTS_RESET_ADD   0x00
// 
//                            //Config Ports 0,1
//                            //1 = Input, 0 = Output
//#define IOI_INIT            0x06, 0xFF, 0xFF
//    
//                            //Invert Control Ports 0,1
//#define IOV_INIT            0x04, 0x4F, 0xFF
//    
//
//    
//                        //  | pin/software |    | presc  |  | enable |
//#define OUT_INIT            0x00, 0x30, 0x04,   0xFE, 0x40, 0x00, 0xA0
//                                                  //0x40 = 94 hz
//                                                  //0x98 = 40 hz
//  
#define INPUTS_MASK         0x00FF   
//    
//#define IN1_MASK            0x0080      // mask with inputs to check which pin is triggered
//#define IN2_MASK            0x0040
//#define IN3_MASK            0x0020
//#define IN4_MASK            0x0010
//#define IN5_MASK            0x0008
//#define IN6_MASK            0x0004
//#define IN7_MASK            0x0002
//#define IN8_MASK            0x0001    
//    
#define SW1_MASK            0x0100
//#define SW2_MASK            0x0200
#define SW3_MASK            0x0400
#define SW4_MASK            0x0800
#define SW5_MASK            0x1000
#define SW6_MASK            0x2000
#define IGN_CTRL_MASK       0x4000
#define TEMP_EDGE_MASK      0x8000

#define DEBUG_MASK          SW1_MASK
//#define LOWBAT_MASK         SW2_MASK
#define IGNCH0_MASK         SW3_MASK
#define IGNCH1_MASK         SW4_MASK
#define ADD_MASK            (SW5_MASK | SW6_MASK) 
//    
#define TEMP_CHAN           2  
#define VBAT_CHAN           1
#define CS_CHAN             0
//    
//#define CS_ON               0x00001000
////#define CS_OFF              0x00100000    
//    
////#define CS_ON               0x00110000
//#define CS_OFF              0x00000000
//#define CS_ALL_OFF          0
//    
//#define OUT_ON              0xFF
//#define OUT_OFF             0
//    
//#define OUT_1               0x06
//#define OUT_2               0x0A
//#define OUT_3               0x0E
//#define OUT_4               0x12    
//#define OUT_5               0x16
//#define OUT_6               0x1A
//#define OUT_7               0x1E
//#define OUT_8               0x22     

//typedef struct{
//    uint16   hex;
//    float32  dec;
//}adcVals;

//    extern int16 adc_off;  

//extern void write_millis(void);
//extern uint32 millis(void);

    //static uint8 check_reset_i2c_bus(void);

    void i2c_init(void);
    void configureCan(void);
    uint8 configureCan2(void);
    
    void c_pwm(uint8 chan, uint16 duty);
    uint16_t read_inputs(uint8 offset);
    adcVals_t vbat_read(void);
    adcVals_t temp_read(void);
    adcVals_t out_current();//uint8 chan);    
    
    void pre_read(uint8 chan);
    
    void c_sense_sel(uint8 chan);
    void adc_c_calibrate(void);
    
    void outputs_reset(void);
    
    void outputs_off(void);
    void outputs_sleep(void);
    void outputs_wake(void);
    void adc_pause(void);  

    
    
//    static void i2cWriteWait(uint32 address, uint8 * buffer, uint32 length, char comp[]);
//    static void i2cReadWait(uint32 address, uint8 * buffer, uint32 length, char comp[]);
    
uint8_t getAddr(uint16_t inputReg);


    
#endif

/* [] END OF FILE */
