#ifndef _MAIN_H_
#define _MAIN_H_

#include "clip.h"

typedef struct GlobalState {
    audio_clip_t ** clips;
    unsigned int    n_clips;

    jack_client_t               *  jack_client;
    jack_port_t                 ** output_ports;
    jack_default_audio_sample_t ** output_buffers;
} global_state_t;

#define SAMPLE_SIZE (sizeof (jack_default_audio_sample_t))
#define FRAME_SIZE  (sizeof(jack_default_audio_sample_t) * clip->sfinfo->channels)

#endif
