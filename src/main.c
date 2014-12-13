#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"
#include "jack.h"
#include "osc.h"

#include <stdlib.h>
#include <unistd.h>

int
main (int    argc,
      char * argv [])
{
    const char     * filename = "data/breakbeat-140bpm.wav";
    global_state_t * context  = calloc(1, sizeof(global_state_t));
    jack_info_t    * jack     = calloc(1, sizeof(jack_info_t));

    context->audio_clips = calloc(INITIAL_CLIP_SLOTS, sizeof(audio_clip_t));
    context->n_clips     = 0;
    
    clip_add(context, filename);

    jack = jack_start(context);

    osc_start();

    sleep(3);

    jack_end(jack);

    free(jack);

    osc_end();

    free(context->audio_clips);
    free(context);
}