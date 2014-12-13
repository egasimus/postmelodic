#ifndef _JACK_H_
#define _JACK_H_

#include <jack/jack.h>

typedef struct GlobalState global_state_t;

void jack_start (global_state_t * context);

void jack_end (global_state_t * context);

#endif
