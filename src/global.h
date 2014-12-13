#ifndef _MAIN_H_
#define _MAIN_H_

#include "clip.h"

typedef struct GlobalState {
    audio_clip_t ** audio_clips;
    unsigned int    n_clips;
} global_state_t;

#endif
