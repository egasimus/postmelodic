#ifndef _OSC_H_
#define _OSC_H_

#include "debug.h"
#include "global.h"

void osc_start (global_state_t * context,
                char           * port_number);

char * osc_get_url (global_state_t * context);

void osc_end (global_state_t * context);

#endif
