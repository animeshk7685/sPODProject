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
#include "pcm_pins.h"
#include "xi.h"
#include "xo.h"
#include "status.h"

static const char* TAG = "STDD";


#define CS_ALL_OFF          0xFF
    

static int16_t adc_off;  // reading when all outputs are turned off


void turnLedsOn(bool on)
{
    // Shreel did not give me any LEDs!
}


//  Initilizes and enables adc module, call before polling adc pins
void adc_c_calibrate(void)
{
    // calibrate to be able to read lower currents
    c_sense_sel(CS_ALL_OFF);
    adc_off = analogRead(C_SENSE_IN_PIN);
    if (adc_off & 0x8000) adc_off = -1;
}


// Pass channel (0 - 7) and duty (0x00 - 0xFF) to change desired output, also
// supports OUT_ON (100% -- 0xFF) or OUT_OFF (0% -- 0x00) to fully turn off or on
void c_pwm(uint8_t chan, uint16_t duty, int lineno)
{
    static uint8_t last_output[CIRCUITS] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (last_output[chan] != duty) {
        LOGD(TAG, "%d -- %s(chan=%d, duty=0x%x, lineno=%d)\r\n", millis(), __FUNCTION__, chan, duty, lineno);
        xo_set_ledout(chan, duty? XO_LEDOUT_ON : XO_LEDOUT_OFF);
        last_output[chan] = duty;
    }
}


// returns the battery voltage
adcVals_t vbat_read(void)
{
    static uint32_t last = 0;
    bool show_vbat = false;
    if (millis() - last > 10000) {
        last = millis();
        //show_vbat = true;
    }

    adcVals_t temp = {.hex=0, .dec=0.0};
    int16_t raw, hexBat; 
    float raw2, bat; 
    
    raw = analogRead(LV_DETECT_PIN);
    if (show_vbat) LOGD(TAG, "%d -- %s(): raw=0x%x\r\n", millis(), __FUNCTION__, raw);
    if (raw & 0x8000) {
        LOGE(TAG, "%d -- %s(): Negative ADC val on bat: %i\r\n", millis(), __FUNCTION__, raw);
        raw = 0;
        return temp;
    }

    raw2 = raw * 3.3 / 0x7FF;
    bat = raw2 * 160.0 / 10.0 + 0.4;       // Scale according to voltage divider
    if (bat > 15.7) {
        hexBat = (bat/2 - 0.7) * 17;
    } else {
        hexBat = (bat - 0.7) * 17; 
    }
    if (hexBat > 0xFF) hexBat = 0xFF;
    
    temp.hex = hexBat;
    temp.dec = bat;          // compensate for diodes voltage drop
    if (show_vbat) LOGD(TAG, "%d -- %s(): hex=0x%x\r\n", millis(), __FUNCTION__, hexBat);

    return temp;
}


static uint16_t temp_tab[] = {
    1966, 1940, 1906, 1865, 1816, 1757, 1689, 1611, 
    1525, 1432, 1333, 1230, 1126, 1024,  924,  828, 
     739,  656,  581,  513,  452,  397,  349,  307, 
     270,  238,  210,  185,  164,  145,  128,  114, 
     102,   91,   81,   72,   65,   58,   53};

// returns the temperature
adcVals_t temp_read(void)
{
    static uint32_t last = 0;
    bool show_temp = false;
    if (millis() - last > 10000) {
        last = millis();
        //show_temp = true;
    }
    adcVals_t temp = {.hex=0, .dec=0.0};
    int16_t raw = analogRead(TEMP_DETECT_PIN/*TEMP_TRIGGER_PIN*/); // TODO: TEMP_TRIGGER_PIN (10) is not an analog pin
    if (show_temp) LOGD(TAG, "%d -- %s(): raw=0x%x\r\n", millis(), __FUNCTION__, raw);
    if (raw & 0x8000) {
        LOGE(TAG, "%d -- %s(): Negative ADC val on therm %i\r\n", millis(), __FUNCTION__, raw);
        raw = 0;
        return temp;
    }
    
    uint8_t i;
    for (i=0 ; i<39 ; i++) {
        if (raw >= temp_tab[i]) break;
    }
    
    if (i <= 0)
        temp.dec = -40.0;
    else 
    if(i >= 38)
        temp.dec = 150.0;
    else
        temp.dec = ((float)(temp_tab[i-1] - raw)/(float)(temp_tab[i-1] - temp_tab[i]) + (i-1.0)) * 5.0 - 40.0;
    temp.hex = (uint8_t)(temp.dec);

    if (show_temp) LOGD(TAG, "%d -- %s(): hex=0x%x\r\n", millis(), __FUNCTION__, temp.hex);

    return temp;
}


void pre_read(uint8_t chan)
{
    c_sense_sel(CS_ALL_OFF);
    us_delay(10); // TODO: don't know if we need this delay
    c_sense_sel(chan);
}


// Returns the current in amps for the desired channel (0-7)
adcVals_t out_current(uint8_t chan)
{
    static uint32_t last = 0;
    bool show_current = false;
    if (millis() - last > 10000) {
        last = millis();
        //show_current = true;
    }

    adcVals_t temp = {.hex=0, .dec=0.0};
    int16_t raw, rawt;
    float raw2, current;
        
    raw = analogRead(C_SENSE_IN_PIN);
    CyDelay(1);
    rawt = analogRead(C_SENSE_IN_PIN);
    c_sense_sel(CS_ALL_OFF);
    
    if (show_current) LOGD(TAG, "%d -- %s(chan=%d): raw=0x%x, rawt=0x%x\r\n", millis(), __FUNCTION__, raw, rawt);

    if (rawt - raw > 0x20)     raw = raw;
    else if(raw - rawt > 0x20) raw = rawt;
    else                       raw = (raw + rawt)/2;
    
    raw -= adc_off;
    if (raw & 0x8000) {
        if (adc_off > 0) adc_off--;
        raw = 0;
    }    
    if (raw == 0) return temp;
    
    raw2 = (raw * 5.00)/1023.00; // convert raw analog value to volts
    
    //current = ((float32)(raw)) * 5.0 / 0x07FF / 2000.0 * (7000.0);
    //current = raw2 / 2000 * 6300;  // Convert to current then scale  << 7007IC
    current = raw2 / 680 * 17000;  // Convert to current then scale   << 7004IC
    
    // scale factor ~17000 for 7004, and ~7000 for 7007
    raw = (uint16_t) (current * (2.0 / 17.0) * (2047.0 / 5.0) + 0.5);
    temp.hex = raw;
    temp.dec = current;

    if (show_current) LOGD(TAG, "%d -- %s(): hex=0x%x\r\n", millis(), __FUNCTION__, temp.hex);

    return temp;
}


// Will read inputs and return bitfield, '&' with appropriate mask to check individual inputs
uint16_t read_inputs(uint8_t offset)
{   
    return get_inputs();
}


// Pass channel to read current from (1 - 8), CS_ALL_OFF (0xFF) will turn all off
void c_sense_sel(uint8_t chan)
{
    if (chan == CS_ALL_OFF) {
        for (uint8_t port=0; port < 12; ++port) {
            xo_set_se(port, XO_LEDOUT_OFF);
        }
    } else {
        xo_set_se(chan, XO_LEDOUT_ON);
    }
}


// disables all outputs and current selects
void outputs_off(void)
{
    for (uint8_t port=0; port < 12; ++port) {
        xo_set_se(port, XO_LEDOUT_OFF);
    }
}


// sleeps the outputs IC and disables all outputs
void outputs_sleep(void)
{
    xo_set_alloff();
}


// wakes the outputs IC, but does not turn any outputs back on
void outputs_wake(void)
{
}


//  Disables adc module, call function to reduce power consumption
void adc_pause(void)
{
}    


void outputs_reset(void)
{
    xo_init();
}