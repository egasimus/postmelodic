#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"
#include "jack.h"
#include "osc.h"

#include <stdlib.h>
#include <unistd.h>

int main (int    argc,
          char * argv []) {

    const char     * filename = "data/breakbeat-140bpm.wav";
    global_state_t * context  = calloc(1, sizeof(global_state_t));

    context->clips   = calloc(INITIAL_CLIP_SLOTS, sizeof(audio_clip_t));
    context->n_clips = 0;
    
    jack_start(context);

    osc_start(context);

    audio_clip_t * clip = context->clips[clip_add(context, filename)];

    clip_cue_set(clip, 1, 18914);

    /*printf("Press <ENTER> key to start playing...");*/
    /*getchar();*/

    /*clip_start(context, 0);*/

    while (1) usleep(10000);

    jack_end(context);

    osc_end(context);

    free(context->clips);
    free(context);
}
