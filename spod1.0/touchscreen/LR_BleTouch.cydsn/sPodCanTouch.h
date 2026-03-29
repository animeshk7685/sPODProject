// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _sPodCanTouch_H_
#define _sPodCanTouch_H_
//add your includes for the project sPodCanTouch here

#include <project.h>

    
#include "simplehsm.h"
#include "pubSub.h"
#include "simpleTimer.h"
#include "sPodImageData.h"

#include "common.h"

#include "mcpCan.h"    
    
//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();

extern tCalibrate calibrate;



#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project sPodCanTouch here

unsigned int getConfigCrc(void);
void loadSavedValues(void);
void loadDefaults(void);
void loadConfig(void);
void saveConfig(void);


//Do not add code below this line
#endif /* _sPodCanTouch_H_ */
