#ifndef _CLIP_H_
#define _CLIP_H_

#include "global.h"

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <pthread.h>
#include <sndfile.h>

typedef enum {
    CLIP_READ_INIT = 0,
    CLIP_READ_STARTED,
    CLIP_READ_DONE
} clip_read_state_t;

typedef enum {
    CLIP_STOP = 0,
    CLIP_PLAY,
} clip_play_state_t;

typedef int clip_index_t;

typedef struct AudioClip {
    const char        * filename;
    SNDFILE           * sndfile;
    SF_INFO           * sfinfo;

    jack_ringbuffer_t * ringbuf;
    jack_nframes_t      position;
    clip_read_state_t   read_state;
    clip_play_state_t   play_state;

    pthread_t           thread;
    pthread_mutex_t     lock;
    pthread_cond_t      ready;
} audio_clip_t;

clip_index_t clip_add(global_state_t * context,
                      const char     * filename);

void clip_start(global_state_t * context,
                clip_index_t     index,
                jack_nframes_t   position);

#endif
