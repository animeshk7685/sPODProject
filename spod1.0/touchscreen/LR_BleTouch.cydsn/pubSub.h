/*
 * pubSub.h
 *
 *  Created on: Aug 26, 2013
 *      Author: Jason
 */

#ifndef PUBSUB_H_
#define PUBSUB_H_

#include "simplehsm.h"
#include <stdbool.h>

#ifndef MAX_SUBSCRIBERS
#define MAX_SUBSCRIBERS 2
#endif

struct sub_t
{
	simplehsm_t *hsm;
	int signalMask;
};

typedef struct sub_t subscriber;

void subscribe(simplehsm_t *hsm, int signal);
void publish(int signal, void* param);

#endif /* PUBSUB_H_ */
