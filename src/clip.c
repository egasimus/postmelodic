#include "clip.h"
#include "debug.h"
#include "global.h"

#include <stdlib.h>

void clip_add(global_state_t * context,
              const char     * filename) {

    audio_clip_t * clip = calloc(1, sizeof(audio_clip_t));

    clip->sfinfo = calloc(1, sizeof(SF_INFO));

    clip->sndfile = sf_open(filename, SFM_READ, clip->sfinfo);

    if (clip->sndfile == NULL) {
        FATAL("Could not open %s", filename);
        exit(1);
    }

    MSG("%s: %d channels, %d kHz, %d frames",
        filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames);

}
