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

#include "stdDrivers.h"
//#include "mcpCan.h"


#define INPUTS_ADD          0x21        // Shift << 1 and | with r/w for full string
#define OUTPUTS_ADD         0x42        // Shift << 1 and | with r/w for full string  
    
#define OUTPUTS_RESET_ADD   0x00
 
                            //Config Ports 0,1
                            //1 = Input, 0 = Output
#define IOI_INIT            0x06, 0xFF, 0xFF
    
                            //Invert Control Ports 0,1
#define IOV_INIT            0x04, 0x4F, 0xFF
    
                        //  | pin/software |    | presc  |  | enable |
#define OUT_INIT            0x00, 0x30, 0x04,   0xFE, 0x40, 0x00, 0xA0
                                                  //0x40 = 94 hz
                                                  //0x98 = 40 hz
   
#define IN1_MASK            0x0080      // mask with inputs to check which pin is triggered
#define IN2_MASK            0x0040
#define IN3_MASK            0x0020
#define IN4_MASK            0x0010
#define IN5_MASK            0x0008
#define IN6_MASK            0x0004
#define IN7_MASK            0x0002
#define IN8_MASK            0x0001    
    
    
#define CS_ON               0x00001000
#define CS_OFF              0x00000000
#define CS_ALL_OFF          0
    
#define OUT_ON              0xFF
#define OUT_OFF             0
    
#define OUT_1               0x06
#define OUT_2               0x0A
#define OUT_3               0x0E
#define OUT_4               0x12    
#define OUT_5               0x16
#define OUT_6               0x1A
#define OUT_7               0x1E
#define OUT_8               0x22  


static int16_t adc_off;

void configureCan()
{
    
    STBY_Write(0);
    CyDelay(1);
    uint8 res;
    res = mcpCanInit(CAN_250KBPS);
    if(res != MCP2515_OK)
    {
        myPrintf("CAN Config Error...\n");
        while(1){};
    }
    
    //myPrintf("CAN configured \n");
}

uint8 configureCan2()
{
    
    STBY_Write(0);
    CyDelay(1);
    uint8 res;
    res = mcpCanInit(CAN_250KBPS);
    if(res != MCP2515_OK)
    {
        myPrintf("CAN Config Error...\n");
//        while(1){};
        return 1;
    }
    
    //myPrintf("CAN configured \n");
    return 0;
}

static void i2cWriteWait(uint32 address, uint8 * buffer, uint32 length, char comp[])
{
    I2C_I2CMasterClearStatus();
    I2C_I2CMasterWriteBuf(address, buffer, length, I2C_I2C_MODE_COMPLETE_XFER);   
    uint32 status = 0;
    uint32 timeout = millis();
    for(;;) 
    {    
        status = I2C_I2CMasterStatus();
        
//        myPrintf(".");
        
        if(0u != (status & I2C_I2C_MSTAT_WR_CMPLT))    
        {              
            /* Transfer complete. Check Master status to make sure that transfer completed without errors. */  
            myPrintf(comp);
            break;    
        } 
        
        if((millis() - timeout) > 500)
        {
            myPrintf("i2c Timeout \n ");
            timeout = millis();
//            if(!check_reset_i2c_bus())
//            {
//                I2C_Stop();
//                    CyDelay(10);
//                I2C_Start(); 
//            }
//            break;
        }
    }
}

static void i2cReadWait(uint32 address, uint8 * buffer, uint32 length, char comp[])
{
    uint32 timeout = millis();
    
    I2C_I2CMasterReadBuf(address, buffer, length, I2C_I2C_MODE_COMPLETE_XFER);   
    for(;;) 
    {    
        if(0u != (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))    
        {              
            /* Transfer complete. Check Master status to make sure that transfer completed without errors. */  
            myPrintf(comp);
            break;    
        } 
        
        if((millis() - timeout) > 500)
        {
            myPrintf("i2c Timeout \n ");
            timeout = millis();
//            if(!check_reset_i2c_bus())
//            {
//                I2C_Stop();
//                    CyDelay(10);
//                I2C_Start(); 
//            }
//            break;
        }
    }
}

static uint8 check_reset_i2c_bus(void)
{
    if(I2C_sda_Read() == 0)     // if the i2c is hung up, toggle the scl pin until it's reset
    {
        I2C_Stop(); 
        
        uint8_t k = 0;
        
        myPrintf("toggle i2c bus: ");
        
        uint32_t regVal, regSave;
        
        regSave = CY_GET_REG32(I2C_scl__0__HSIOM);
        regVal = CY_GET_REG32(I2C_scl__0__HSIOM);
        regVal &= ~I2C_scl__0__HSIOM_MASK;
           
        /* Set Pin as GPIO controlled by firmware */
        regVal = CY_SET_REG32(I2C_scl__0__HSIOM, regVal |(I2C_scl__0__HSIOM_GPIO <<I2C_scl__0__HSIOM_SHIFT)); 
           
        /* Set Pin drive mode */    
        I2C_scl_SetDriveMode(I2C_scl_DM_STRONG);   
        
        while(k < 8)
        {
            if(I2C_sda_Read() == 0){
                k = 0;
                myPrintf(",");
            }
            else
            {
                myPrintf(".");
            }
            
            I2C_scl_Write(0);
            
            CyDelayUs(100);
            
            I2C_scl_Write(1);
            
            CyDelayUs(100);
            
            k++;
        }
        
        I2C_scl_SetDriveMode(I2C_scl_DM_OD_LO);  
        
        regVal = CY_SET_REG32(I2C_scl__0__HSIOM, regSave); 
        
        myPrintf("done \n");
        
        I2C_Start(); 
        
        return 1;
    }
    
    return 0;
}


//  Sets up external ICs on I2C bus, call to initilize or reset external ICs
void i2c_init(void)
{
    uint8 in_init[] = {IOV_INIT};
    uint8 in_init2[] = {IOI_INIT};
    
    uint8 out_init[] = {OUT_INIT};
    
    myPrintf("\nBegin I2C init \n");
    
    check_reset_i2c_bus();

    
    I2C_I2CMasterClearStatus();
    CyDelay(50);
    
    while(!I2C_CHECK_I2C_FSM_IDLE); 
    
    i2cWriteWait(INPUTS_ADD, in_init2, 3, ".");
    i2cWriteWait(INPUTS_ADD, in_init, 3, "Inputs initialized \n");
    
    
//    uint16 temp = read_inputs(0);
//    myPrintf("%x\n", temp);
//    
//    temp = read_inputs(2);
//    myPrintf("%x\n", temp);
//    
//    temp = read_inputs(4);
//    myPrintf("%x\n", temp);
//    
//    temp = read_inputs(6);
//    myPrintf("%x\n", temp);
    
    
    i2cWriteWait(OUTPUTS_ADD, out_init, 3, ".");
    CyDelayUs(500);
    i2cWriteWait(OUTPUTS_ADD, &out_init[3], 2, ".");
    CyDelayUs(500);
    i2cWriteWait(OUTPUTS_ADD, &out_init[5], 2, "Outputs initialized \n");
    CyDelayUs(500);
    
    outputs_off();
}


//  Initilizes and enables adc module, call before polling adc pins
void adc_c_calibrate(void)
{
//    return;
    
    // calibrate to be able to read lower currents
    c_sense_sel(CS_ALL_OFF);
    ADC_StartConvert();
//    CyDelay(1);
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    
    adc_off = ADC_GetResult16(CS_CHAN);
//    ADC_StopConvert();

    
    
    if(adc_off & 0x8000)
    {
//        myPrintf("-");
//        adc_off = ~adc_off + 1;
        adc_off = -1;
    }

    myPrintf("ADC csense offset: %i \n", adc_off);
}

// Pass channel (0 - 7) and duty (0x00 - 0xFF) to change desired output
// also supports OUT_ON (100%) or OUT_OFF (0%) to fully turn off or on
void c_pwm(uint8 chan, uint16 duty)
{   
    uint16 b_duty;
    uint8 out_buf[5] = {0};
   
    switch(chan){
        case 0:
        out_buf[0] = OUT_1;
        break;
        case 1:
        out_buf[0] = OUT_2;
        break;
        case 2:
        out_buf[0] = OUT_3;
        break;
        case 3:
        out_buf[0] = OUT_4;
        break;
        case 4:
        out_buf[0] = OUT_5;
        break;
        case 5:
        out_buf[0] = OUT_6;
        break;
        case 6:
        out_buf[0] = OUT_7;
        break;
        case 7:
        out_buf[0] = OUT_8;
        break;
        default:
        myPrintf("invalid channel %d\n", chan);
        return;
    }
    
    if(duty >= 0xFE)
    {
        //myPrintf("duty too large, set to ON \n");
        duty = OUT_ON;
    }
    
    if(duty == OUT_ON) {
        out_buf[2] = 0x10;
    }else if(duty == OUT_OFF) {
        out_buf[4] = 0x10;

    }else {
        b_duty = duty << 4;
        
        out_buf[3] = 0xFF & b_duty;
        out_buf[4] = 0x0F & (b_duty >> 8);
    }
    
    i2cWriteWait(OUTPUTS_ADD, out_buf, 5, "");
    
}

// returns the battery voltage
adcVals_t vbat_read(void)
{
    int16 raw, hexBat; 
    float32 raw2, bat; 
    
    adcVals_t temp;
    
    temp.hex = 0;
    temp.dec = 0.0;
    
//    CyDelay(1);
    ADC_StartConvert();
    
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);  
    raw = ADC_GetResult16(VBAT_CHAN);

    
//    raw = raw + adc_off;
    
    if(raw & 0x8000)
    {
        myPrintf("Negative ADC val on bat: %i\n", raw);
        raw = 0;
        return temp;
    }

//    raw2 = ADC_CountsTo_Volts(VBAT_CHAN, raw);
    
    raw2 = raw * 3.3 / 0x7FF;
    
//    bat = raw2 * 61.0 / 10.0;       // Scale according to voltage divider
    bat = raw2 * 160.0 / 10.0 + 0.4;       // Scale according to voltage divider
    
//    hexBat = (raw - 27) * 83 / 328;
    
    if(bat > 15.7)
    {
        hexBat = (bat/2 - 0.7) * 17;
    }
    else
    {
        hexBat = (bat - 0.7) * 17; 
    }
    
    if(hexBat > 0xFF) {  
        hexBat = 0xFF;
    }
    
    temp.hex = hexBat;
    temp.dec = bat;          // compensation for diodes voltage drop

 //   temp.hex = raw;
  //  temp.dec = raw2;
    
    return temp;
}

static uint16 temp_tab[] = {
    1966, 1940, 1906, 1865, 1816, 1757, 1689, 1611, 
    1525, 1432, 1333, 1230, 1126, 1024,  924,  828, 
     739,  656,  581,  513,  452,  397,  349,  307, 
     270,  238,  210,  185,  164,  145,  128,  114, 
     102,   91,   81,   72,   65,   58,   53};

// returns the temperature
adcVals_t temp_read(void)
{
    uint8 i;
    int16 raw;
    adcVals_t temp;
    
    temp.hex = 0;
    temp.dec = 0.0;
    
    ADC_StartConvert();
    
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);

    
    raw = ADC_GetResult16(TEMP_CHAN);
  
    
    
    if(raw & 0x8000)
    {
        myPrintf("Negative ADC val on therm %i\n", raw);
        raw = 0;
        return temp;
    }
      
    for(i=0 ; i<39 ; i++)                                      
    {
        if(raw >=  temp_tab[i])
        {
            break;
        }
    }
    
    if(i <= 0)
        temp.dec = -40.0;
    else if(i >= 38)
        temp.dec = 150.0;
    else
    {
        temp.dec = ((float)(temp_tab[i-1] - raw)/(float)(temp_tab[i-1] - temp_tab[i]) + (i-1.0)) * 5.0 - 40.0;
    }

    temp.hex = (uint8)(temp.dec);
    
//    temp.dec = ADC_CountsTo_Volts(TEMP_CHAN, raw);
    
    return temp;
}

void pre_read(uint8 chan)
{
    c_sense_sel(CS_ALL_OFF);
    
    CyDelayUs(10);
    
    c_sense_sel(chan + 1);
}

// Returns the current in amps for the desired channel (0-7)
adcVals_t out_current()//uint8 chan)
{
    int16 raw, rawt;//, offset;
    float32 raw2, current;
    
    adcVals_t temp;
    
    temp.hex = 0;
    temp.dec = 0.0;
    
    ADC_StartConvert();
    
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    raw = ADC_GetResult16(CS_CHAN);
    
    CyDelay(1);
    
    ADC_StartConvert();
    
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    rawt = ADC_GetResult16(CS_CHAN);
    
    c_sense_sel(CS_ALL_OFF);
    
//    raw++;
//    rawt++;
    
    if(rawt - raw > 0x20)
    {
        raw = raw;
        
    }
    else if(raw - rawt > 0x20)
    {
        raw = rawt;
    }
    else
    {
        raw = (raw + rawt)/2;
    }
    
//    myPrintf("offset: %d \n", offset);
    //myPrintf("avg off: %d \n", adc_off);
    //myPrintf("raw: %d \n", raw);
    
    raw = raw - adc_off;
    
    if(raw & 0x8000)
    {
//        myPrintf("Negative ADC val on csense, chan: %d\n", (chan + 1));
        //return raw;
        
        if(adc_off > 0){
            adc_off--;
        }
        raw = 0;
    }    

    
    if(raw == 0) {
        return temp;
    }
    
    raw2 = ADC_CountsTo_Volts(CS_CHAN, raw);
    
    //current = ((float32)(raw)) * 5.0 / 0x07FF / 2000.0 * (7000.0);
    //current = raw2 / 2000 * 6300;  // Convert to current then scale  << 7007IC
    
    current = raw2 / 680 * 17000;  // Convert to current then scale   << 7004IC
    
    // scale factor ~17000 for 7004, and ~7000 for 7007
    
    raw = (uint16) (current * (2.0 / 17.0) * (2047.0 / 5.0) + 0.5);
    
    temp.hex = raw;
    temp.dec = current;
    
    return temp;
}

// Will read inputs and return bitfield, '&' with appropriate mask to check individual inputs
uint16_t read_inputs(uint8 offset)
{   
    uint8 i2c_read[2] = {offset, 0};
    uint16 inputs = 0;
    
    i2cWriteWait(INPUTS_ADD, i2c_read, 1, "");
    i2cReadWait(INPUTS_ADD, i2c_read, 2, "");
    CyDelayUs(300);
    
    inputs = (i2c_read[0] << 8) | (i2c_read[1]);
    
    //myPrintf("%d \n", inputs);
    
//    I2C_I2CMasterClearStatus();
    
    return inputs;
}

// Pass channel to read current from (1 - 8), CS_ALL_OFF (0) will turn all off
void c_sense_sel(uint8 chan)
{
    int i;
    uint32 chan_sel[8];
    uint8 b_c_sel[33];
    
    uint8 *temp;
    
    if(chan > 8)
    {
        myPrintf("invalid channel number, all current sense outputs disabled \n");
        chan = 0;
    }
    
    chan = 8 - chan;
    
    for(i = 7; i >= 0; i--)      // reorder to map to pins
    {
        chan_sel[i] = CS_OFF;
        
        if(i == chan)
            chan_sel[i] = CS_ON;    
    }
    
    temp = (uint8*)chan_sel;
    
    for(i = 1; i < 33; i++)
    {
        b_c_sel[i] = temp[i-1];
    }
    
    b_c_sel[0] = 0x26;
    
    i2cWriteWait(OUTPUTS_ADD, b_c_sel, 33, "");
    
    CyDelayUs(200);
    
}

// disables all outputs and current selects
void outputs_off(void)
{
    uint8 temp_buf[] = {0xFD, 0x10};

    i2cWriteWait(OUTPUTS_ADD, temp_buf, 2, ""); //All outputs OFF \n");
}

// sleeps the outputs IC and disables all outputs
void outputs_sleep(void)
{
    uint8 sleep_buf[] = {0x00, 0x30};
    
//    outputs_off();
    
    i2cWriteWait(OUTPUTS_ADD, sleep_buf, 2, "PWM gen sleep \n");
}

// wakes the outputs IC, but does not turn any outputs back on
void outputs_wake(void)
{
    uint8 sleep_buf[] = {0x00, 0x20};
    
    i2cWriteWait(OUTPUTS_ADD, sleep_buf, 2, "PWM gen wake \n");
}

//  Disables adc module, call function to reduce power consumption
void adc_pause(void)
{
    ADC_StopConvert();
   
    ADC_Stop();
}    

void outputs_reset(void)
{
    uint8 buffer[] = {0x06};
    
    check_reset_i2c_bus(); // clear bus jam
    
    i2cWriteWait(OUTPUTS_RESET_ADD, buffer, 1, "PWM reset\n");
}



uint8_t getAddr(uint16_t inputReg)
{
    uint8_t addr;
    
    addr = ((uint16_t)inputReg & ADD_MASK) >> 12;
    
    if(addr == 1){
        addr = 2;
    } else if(addr == 2){
        addr = 1;
    }
    
    return addr;
}



/* [] END OF FILE */
