/*
 * pub_sub.h
 *
 *  Created on: Aug 26, 2013
 *      Author: Jason
 */

#ifndef pub_sub_H_
#define pub_sub_H_

#include "simple_hsm.h"
#include <stdbool.h>

#ifndef MAX_SUBSCRIBERS
#define MAX_SUBSCRIBERS 2
#endif

struct sub_t {
	simple_hsm_t *hsm;
	int signalMask;
};

typedef struct sub_t subscriber;

void subscribe(simple_hsm_t *hsm, int signal);
void publish(int signal, void* param);

#endif /* pub_sub_H_ */
