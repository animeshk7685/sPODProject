/*
 * pubSub.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Jason
 */

#include "pubSub.h"
#include <stddef.h>

static bool isInit = false;

static int subIndex = 0;

static subscriber subscribers[MAX_SUBSCRIBERS];

void subscribe(simplehsm_t *hsm, int signal)
{
	int i = 0;
	if(!isInit)
	{
		for(i = 0; i < MAX_SUBSCRIBERS; i++)
		{
			subscribers[i].hsm = NULL;
			subscribers[i].signalMask = 0;
		}

		isInit = true;

	}

	int currentSubIndex = -1;

	for(i = 0; i < subIndex; i++)
	{
		if(subscribers[i].hsm == hsm)
		{
			currentSubIndex = i;
			break;
		}
	}

	if(currentSubIndex == -1)
	{
		currentSubIndex = subIndex;
		subscribers[subIndex].hsm = hsm;
		if(subIndex < MAX_SUBSCRIBERS)
					subIndex++;
	}

	subscribers[currentSubIndex].signalMask |= (1 << signal);


}

void publish(int signal, void* param)
{
	int mask = (1 << signal);
	int i = 0;
	for(i = 0; i < MAX_SUBSCRIBERS; i++)
	{
		if(subscribers[i].signalMask & mask)
		{
			simplehsm_signal_current_state(subscribers[i].hsm, signal, param);
		}
	}
}
