#ifndef _MAIN_H_
#define _MAIN_H_

#include "clip.h"

#include <jack/jack.h>
#include <lo/lo.h>
#include <pthread.h>

typedef struct AudioClip audio_clip_t;

typedef struct GlobalState {

    audio_clip_t  ** clips;
    clip_index_t     now_playing;

    jack_client_t  * jack_client;
    jack_port_t   ** output_ports;

    char           * osc_port;
    lo_server_thread osc_server;
    lo_address       listen_address;

} global_state_t;

#define SAMPLE_SIZE (sizeof (jack_default_audio_sample_t))
#define FRAME_SIZE  (SAMPLE_SIZE * clip->sfinfo->channels)
#define BUFFER_SIZE (FRAME_SIZE * BUFFER_FRAMES)

#endif
