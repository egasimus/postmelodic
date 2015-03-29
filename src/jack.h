#ifndef _JACK_H_
#define _JACK_H_

#include "global.h"

#include <jack/jack.h>

void jack_start (global_state_t * context,
                 char           * client_name,
                 char           * connect_to);

void jack_end (global_state_t * context);

#endif
