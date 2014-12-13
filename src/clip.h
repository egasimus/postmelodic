#ifndef _CLIP_H_
#define _CLIP_H_

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <sndfile.h>

typedef struct GlobalState global_state_t;

typedef struct AudioClip {
    SNDFILE           * sndfile;
    SF_INFO           * sfinfo;
    jack_ringbuffer_t * ringbuf;
    jack_nframes_t      pos;
} audio_clip_t;

void clip_add(global_state_t * context,
              const char     * filename);

#endif
