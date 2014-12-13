#ifndef _JACK_H_
#define _JACK_H_

#include <jack/jack.h>

typedef struct JackInfo {
    jack_client_t *  client;
    jack_port_t   ** output_ports;
} jack_info_t;

jack_info_t * jack_start ();

void jack_end ();

#endif
