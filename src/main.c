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

    char path[1024];

    if (getcwd(path, sizeof(path)) != NULL) {
        MSG("Current working dir: %s", path);
    } else {
        FATAL("No working directory.");
    }

    strcat(path, "/");
    strcat(path, argv[1]);

    global_state_t * context = calloc(1, sizeof(global_state_t));

    context->clips   = calloc(INITIAL_CLIP_SLOTS, sizeof(audio_clip_t));
    context->n_clips = 0;
    
    jack_start(context);

    osc_start(context);

    if (argv[1] == NULL) {
        MSG("No sample path specified.");
    } else {
        audio_clip_t * clip = context->clips[clip_add(context, path)];
    }

    while (1) usleep(10000);

    jack_end(context);

    osc_end(context);

    free(context->clips);
    free(context);
}
