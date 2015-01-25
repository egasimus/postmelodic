#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"
#include "jack.h"
#include "osc.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main (int    argc,
          char * argv []) {

    char sample_path [1024];

    if (argv[1] == NULL) FATAL("No sample path specified.");
    
    realpath(argv[1], sample_path);
    
    if (access(sample_path, R_OK) != -1) {
        MSG("Loading sample %s", sample_path);
    } else {
        FATAL("Sample %s does not exist or is not readable.", sample_path);
    }

    global_state_t * context = calloc(1, sizeof(global_state_t));

    context->clips   = calloc(INITIAL_CLIP_SLOTS, sizeof(audio_clip_t));
    context->n_clips = 0;

    jack_start(context);
    osc_start(context);
    
    audio_clip_t * clip = context->clips[clip_add(context, sample_path)];

    while (1) {

      if (!(clip == NULL ||
            clip->read_state == CLIP_READ_INIT ||
            clip->play_state == CLIP_STOP)) {

        MSG("%s: %d channels   %d kHz   %d/%d frames   read %d   play %d   cue %d   %s",
             clip->filename,
             clip->sfinfo->channels,
             clip->sfinfo->samplerate,
             clip->position,
             clip->sfinfo->frames,
             clip->read_state,
             clip->play_state,
             clip->cue,
             (clip->cue > -1) ? "cued" : "ring");

      }

      usleep(100000);

    }

    osc_end(context);
    jack_end(context);

    free(context->clips);
    free(context);
}
