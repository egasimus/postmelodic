#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"

#include <jack/ringbuffer.h>
#include <pthread.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

static void * clip_read (void * arg) {

    audio_clip_t * clip = (audio_clip_t*) arg;

    sf_count_t             buf_avail;
    sf_count_t             read_frames;
    jack_ringbuffer_data_t write_vector[2];

    size_t bytes_per_frame = FRAME_SIZE;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_mutex_lock(&clip->lock);

    while (1) {

        jack_ringbuffer_get_write_vector(clip->ringbuf, write_vector);

        read_frames = 0;

        if (write_vector[0].len) {

            buf_avail = write_vector[0].len / bytes_per_frame;

            read_frames = sf_readf_float(
                clip->sndfile,
                (float*) write_vector[0].buf,
                buf_avail);

        }

        if (read_frames == 0) break;

        jack_ringbuffer_write_advance(clip->ringbuf, read_frames * bytes_per_frame);

        clip->read_state = CLIP_READ_STARTED;

        pthread_cond_wait(&clip->ready, &clip->lock);

    }

    clip->read_state = CLIP_READ_DONE;

}

void clip_cue_add(audio_clip_t * clip,
                  cue_index_t    index,
                  jack_nframes_t position) {

    cue_point_t * cue = calloc(1, sizeof(cue_point_t));

    cue->position = position;
    cue->buffer   = jack_ringbuffer_create(
            sizeof(jack_default_audio_sample_t) * RINGBUFFER_SIZE);

    memset(cue->buffer->buf, 0, cue->buffer->size);

    clip->cues[index] = cue;

}

void clip_cue_jump(audio_clip_t * clip,
                   cue_index_t    index) {

    clip->cue      = index;
    clip->ringbuf  = clip->cues[index]->buffer;
    clip->position = clip->cues[index]->position;

}

clip_index_t clip_add(global_state_t * context,
                      const char     * filename) {

    audio_clip_t * clip = calloc(1, sizeof(audio_clip_t));

    clip->filename   = filename;
    clip->read_state = CLIP_READ_INIT;
    clip->play_state = CLIP_STOP;
    clip->sfinfo     = calloc(1, sizeof(SF_INFO));
    clip->sndfile    = sf_open(filename, SFM_READ, clip->sfinfo);

    if (clip->sndfile == NULL) {
        FATAL("Could not open %s", filename);
        exit(1);
    }

    MSG("%s: %d channels, %d kHz, %d frames, read %d, play %d",
        clip->filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames,
        clip->read_state,
        clip->play_state);

    // initialize cues and ringbuffer
    clip->next_cue = -1;
    clip->cues     = calloc(INITIAL_CUE_SLOTS, sizeof(cue_point_t*));
    clip_cue_add(clip, 0, 0);
    clip_cue_jump(clip, 0);

    // initialize reader thread
    pthread_mutex_init(&clip->lock, NULL);
    pthread_cond_init(&clip->ready, NULL);
    pthread_create(&clip->thread, NULL, clip_read, clip);

    // add clip to global list of clips
    context->clips[0] = clip;

    return 0;

}

void clip_start(global_state_t * context,
                clip_index_t     clip_index,
                cue_index_t      cue_index) {

    audio_clip_t * clip = context->clips[clip_index];

    if (clip->play_state == CLIP_PLAY) {
        clip->next_cue = cue_index;
    } else {
        clip_cue_jump(clip, cue_index);
        clip->play_state = CLIP_PLAY;
    }

}
