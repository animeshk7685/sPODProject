/* ========================================
 *
 * Copyright Star Technologies, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF STAR TECHNOLOGIES.
 *
 *  Author: Luke Williams
 *
 *  Version 1.00    12-16-2017
 *  Version 1.01    12-19-2017
 *  Version 1.02    12-26-2017
 *  Version 1.03    01-11-2018
 *  Version 1.04    01-11-2018
 *  Version 1.05    01-15-2018
 *  Version 1.06    01-15-2018
 *
 * ========================================
*/

#include "spod.h"
#include "crc32.h"
#include "main.h"
#include "stdDrivers.h"

static const char* TAG = "RXCURR";


static double approxRollingAverage (double avg, double new_sample, double window)
{
    double sub = avg / window;
    double add = new_sample / window;

    avg = avg - sub;
    avg = avg + add;
    return avg;
}


// return true if this circuit is either off or on solid (not blinking)
static bool circuit_notpwm(uint8_t index)
{
    return circuit_status[index].blinkOff == 0 && ((circuit_status[index].outCmd == 0) || (circuit_status[index].outCmd == 0xFF));
}


// return true if circuit is on and pulsing (blinking)
static bool circuit_blink(uint8_t index)
{
    return (circuit_status[index].blinkOff != 0) && (circuit_status[index].outCmd != 0);
}


bool readCurrents(void* unused)
{
    if (isAwake) {
        static adcVals_t localCur;
        //static uint8_t faults;
        static int ci = 0, retry;
        static bool outOn = false;
        static bool wait_for_outon = false;
        static uint8_t avgF = 3;
        static double avgCurrentsDec[CIRCUITS];
        static double avgCurrentsHex[CIRCUITS];
        static uint32_t proShortTimer[CIRCUITS] = {0};
        static uint8_t lastWasErr[CIRCUITS] = {0};
        uint8_t index = current_pcm*CIRCUITS + ci;
        uint8_t prev_status = circuit_status[index].status;

        outOn = false;
        
        pre_read(ci);        // takes ~3ms
        
        if (circuit_blink(index)) {
            if (!wait_for_outon) retry = 0;
            wait_for_outon = true;
            
            if (circuit_status[index].output == 0) {  // if blink mode, wait for output to be on
                if (++retry > 50) {
                    // timedout, move on to next circuit -- waited .5 seconds for output to be on (50*10ms)
                    wait_for_outon = false;
                    ++ci;
                    return true;
                }
            }
            
            outOn = true;
            wait_for_outon = false;
        }

        localCur = out_current(ci);      // read current
        if (outOn && (circuit_status[index].output == 0)) {    // check blink didn't turn off while reading
            wait_for_outon = true;
            return true;
        }
        
        if (circuit_notpwm(index)) { // std (not blink or pwm)
            if ((offTimer[ci] == 0) || (millis() - offTimer[ci] > 6000)) {
                avgF = 3;
            } else {
                avgF = localCur.dec > 0.5? 100 : 3;   // filter out back emf fault
            }
        } else {
            // non-standard: flash, strobe, pwm
            if (circuit_status[index].outCmd != 0) {
                if (localCur.dec > 70.0f && circuit_status[index].outCmd < 0xFE) {
                    if (lastWasErr[ci] < 2) {
                        localCur.dec = 0;
                        localCur.hex = 0;
                        lastWasErr[ci]++;
                    }
                } else {
                    lastWasErr[ci] = 0;
                }
                
                if (circuit_status[index].outCmd < 0xFE) {        // scale pwm currents
                    uint16_t scaledCurr = (localCur.hex * circuit_status[index].outCmd) / 0xFF;
                    localCur.hex = approxRollingAverage(scaledCurr, localCur.hex, 4);   // 3:1
                }
                
                if (circuit_status[index].blinkOff != 0 && circuit_status[index].blinkOn > 0) { // scale blink mode currents/avoid /0
                    if ((circuit_status[index].blinkOff / circuit_status[index].blinkOn) == 4) {
                        localCur.hex /= 4;
                    } else {
                        localCur.hex /= 2;
                    }
                }
                
                avgF = localCur.dec > 0.5? 10 : 75; // filter out transient
                
                if(circuit_status[index].outCmd < 0x40) {   // ignore short protection on pwm < %25
                    localCur.dec = 0;
                }
            } else {
                avgF = 3;
            }
        }
        
        if (globalOffTimer[ci] > 0 && localCur.dec > 0.5) { // supress flyback current spike
            avgF = 100;
        }
        
        bool isCurrError = false;       // overCurrent/overTemperature/output overVoltage/etc...
        
        if (localCur.dec > 74.0f) {
            isCurrError = true;
            //error code...
        }
        
        if (globalOffTimer[ci] > 0 && isCurrError) {   // ignore flyback
            localCur.dec = 0.0f;
            localCur.hex = 0;
        } else {
            avgCurrentsDec[ci] = approxRollingAverage(avgCurrentsDec[ci], localCur.dec, avgF);
            avgCurrentsHex[ci] = approxRollingAverage(avgCurrentsHex[ci], localCur.hex, avgF);
        }
        circuit_status[index].current.dec = avgCurrentsDec[ci];
        circuit_status[index].current.hex = avgCurrentsHex[ci];
        
        bool needsHandleShort = false;
        bool needsShortRestart = false;
        
        float currLimit = 45.0f;
        #if 0 // TODO
        currLimit = ((float)pcm_config[current_pcm].circuits[ci].currentLimit);
        if (pcm_config[current_pcm].circuits[ci].currentLimit >= 30) currLimit = 45.0;
        #endif

        if (false && circuit_status[index].outCmd > 0 && circuit_status[index].outCmd < 0xFE) { // ignore pwm
        } else 
        if (globalOffTimer[ci] > 0) {  // ignore off (flyback) current for IGNORE_OFF_TIME_MS 
        } else {
            if (approxRollingAverage(localCur.dec, avgCurrentsDec[ci], 2) < currLimit) { //45.0)
                proShortTimer[ci] = millis();
            }
            
            if ((approxRollingAverage(localCur.dec, avgCurrentsDec[ci], 2) > 65.0) || 
                ((circuit_status[index].blinkOff == 0 && circuit_status[ci].outCmd >= 0xFE) && 
                ((offTimer[ci] == 0) || (millis() - offTimer[ci] > 2000) ) &&
                (approxRollingAverage(avgCurrentsDec[ci], localCur.dec, 3) > currLimit && (millis() - proShortTimer[ci] > 3000)))) {
                needsHandleShort = true;
                
                if (pcm_config[current_pcm].circuits[ci].isCurrentRestart && !circuit_local[ci].needsRestart) {
                    circuit_local[ci].lastOut = circuit_status[index];
                    circuit_local[ci].needsRestart = true;
                }
            }
        }
        
        if (needsHandleShort) {
            LOGE(TAG, "Fault/Short on SW%d: %f \r\n", ci, localCur.dec);
            circuit_status[index].outCmd = 0;
            circuit_status[index].blinkOff = 0;
            circuit_status[index].status = OUTPUT_SHORT;
                
            offTimer[ci] = 0;
            
            shortDelay[ci] = millis();
        } else 
        if ((shortDelay[ci] != 0) && (millis() - shortDelay[ci] > 3500)) {
            shortDelay[ci] = 0;
        } else 
        if (circuit_local[ci].needsRestart && approxRollingAverage(localCur.dec, avgCurrentsDec[ci], 2) < currLimit) {
            needsShortRestart = true;
        }
        
        if (needsShortRestart) {
            circuit_status[index].status   = circuit_local[ci].lastOut.status;
            circuit_status[index].outCmd   = circuit_local[ci].lastOut.outCmd;
            circuit_status[index].blinkOn  = circuit_local[ci].lastOut.blinkOn;
            circuit_status[index].blinkOff = circuit_local[ci].lastOut.blinkOff;
            circuit_local[ci].needsRestart = false;
        }
        
        if (circuit_status[index].current.dec < 0.05) { // Check open/short conditions
            if (circuit_status[index].current.hex <= 2) circuit_status[index].current.hex = 0;
            circuit_status[index].current.dec = 0;
            
            if (circuit_status[index].outCmd) {
                if (!shortDelay[ci]) circuit_status[index].status = OUTPUT_OPEN;      // no current & powered
            } else {
                if (!shortDelay[ci]) circuit_status[index].status = OUTPUT_OFF;     // not powered
            }
        } else 
        if (circuit_status[index].outCmd) {
            if (circuit_status[index].blinkOff != 0 && circuit_status[index].blinkOn != 0) { // check if strobe or flash
                if(!shortDelay[ci]) circuit_status[index].status = OUTPUT_FLASH;
            } else {
                if (circuit_status[index].current.dec > 35.0 && circuit_status[index].outCmd > 200) { // short
                    LOGI(TAG, "Over current on SW%d\r\n", ci);
                } else { // nominal
                    if (!shortDelay[ci]) circuit_status[index].status = OUTPUT_NOM;           // nominal operation  
                }
            }
        }
#if 0
        else { // fault
            // TODO: this seems incorrect, but matches legacy code
            if (!shortDelay[ci]) {
                LOGE(TAG, "%d -- %s(): setting circuit_Status[%d].status to OUTPUT_FAULT against my better judgment\r\n", millis(), __FUNCTION__, index);
                circuit_status[index].status = OUTPUT_FAULT;
            }
            faults++;
        }  
        if (faults > 2) {
            adc_c_calibrate();
            faults = 0;
        }
#endif        

        if (prev_status != circuit_status[index].status) {
            send_circuit_status_pkt(index);         
        }

        ci = (ci + 1) % CIRCUITS;
    }
    return true;
}