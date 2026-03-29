/*
 * simpleTimer.c
 *
 *  Created on: Sep 26, 2014
 *      Author: Jason
 */
#include <stdint.h>
#include "simpleTimer.h"

bool timerDidFire(tTimer *timer, unsigned long elapsedTime) {
	if(elapsedTime >= timer->currentTime)
	{
		timer->currentTime = timer->rollover;
		return true;
	}

	timer->currentTime -= elapsedTime;
	if (timer->currentTime <= 0) {
		timer->currentTime = timer->rollover;
		return true;
	}
	return false;
}
