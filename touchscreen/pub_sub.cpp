/*
 * pub_sub.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Jason
 */

#include "pub_sub.h"
#include <stddef.h>


static subscriber subscribers[MAX_SUBSCRIBERS];


void subscribe(simple_hsm_t *hsm, int signal)
{
	static bool isInit = false;
	static int subIndex = 0;

	if (!isInit) {
		isInit = true;
		for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
			subscribers[i].hsm = NULL;
			subscribers[i].signalMask = 0;
		}
	}

	int currentSubIndex = -1;

	for (int i = 0; i < subIndex; i++) {
		if (subscribers[i].hsm == hsm) {
			currentSubIndex = i;
			break;
		}
	}

	if (currentSubIndex == -1) {
		currentSubIndex = subIndex;
		subscribers[subIndex].hsm = hsm;
		if (subIndex < MAX_SUBSCRIBERS) subIndex++;
	}

	subscribers[currentSubIndex].signalMask |= (1 << signal);
}


void publish(int signal, void* param)
{
	int mask = (1 << signal);
	int i = 0;
	for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
		if (subscribers[i].signalMask & mask) {
			simple_hsm_signal_current_state(subscribers[i].hsm, signal, param);
		}
	}
}
