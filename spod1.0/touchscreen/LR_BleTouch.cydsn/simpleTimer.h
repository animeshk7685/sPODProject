/*
 * simpleTimer.h
 *
 *  Created on: Sep 26, 2014
 *      Author: Jason
 */

#ifndef SIMPLETIMER_H_
#define SIMPLETIMER_H_

#include <stdbool.h>
    
typedef struct {
	unsigned long rollover;
	unsigned long currentTime;
} tTimer;

bool timerDidFire(tTimer *timer, unsigned long elapsedTime);


#endif /* SIMPLETIMER_H_ */
