#ifndef _CLIP_H_
#define _CLIP_H_

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <pthread.h>
#include <sndfile.h>

typedef struct GlobalState global_state_t;

typedef enum {
    INIT = 0,
    READING,
    READY,
    READ_PLAY,
    PLAY,
    ENDED
} clip_state_t;

typedef struct AudioClip {
    const char        * filename;
    SNDFILE           * sndfile;
    SF_INFO           * sfinfo;

    jack_ringbuffer_t * ringbuf;
    jack_nframes_t      position;
    clip_state_t        state;

    pthread_t           thread;
    pthread_mutex_t     lock;
    pthread_cond_t      ready;
} audio_clip_t;

void clip_add(global_state_t * context,
              const char     * filename);

#endif
