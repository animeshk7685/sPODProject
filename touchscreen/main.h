// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _spod_touchscreen_H_
#define _spod_touchscreen_H_

#include "simple_hsm.h"
#include "pub_sub.h"
#include "simple_timer.h"
#include "image_data.h"

#include "common.h"
//#include "mcp_can.h" 
#include "config.h"  

#define I2C_CLK_PIN 3
#define I2C_DAT_PIN 4

//#ifdef __cplusplus
//extern "C" {
//#endif

extern switch_config_t saved_switch_config;
extern pcm_config_t saved_pcm_config[PCMS];
extern uint8_t current_switch;

//#ifdef __cplusplus
//} // extern "C"
//#endif

extern void main_setup();
extern void main_loop();
unsigned int getConfigCrc(void);
void loadSavedValues(void);
void loadDefaults(void);
void loadConfig(void);
void saveConfig(void);

#endif /* _spod_touchscreen_H_ */