#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>
#include <jack/ringbuffer.h>

void clip_add(global_state_t * context,
              const char     * filename) {

    audio_clip_t * clip = calloc(1, sizeof(audio_clip_t));

    clip->sfinfo = calloc(1, sizeof(SF_INFO));

    clip->sndfile = sf_open(filename, SFM_READ, clip->sfinfo);

    if (clip->sndfile == NULL) {
        FATAL("Could not open %s", filename);
        exit(1);
    }

    clip->filename = filename;

    MSG("%s: %d channels, %d kHz, %d frames",
        filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames);

    clip->ringbuf = jack_ringbuffer_create(
        sizeof(jack_default_audio_sample_t) * RINGBUFFER_SIZE);
    memset(clip->ringbuf->buf, 0, clip->ringbuf->size);

    context->clips[0] = clip;

}
