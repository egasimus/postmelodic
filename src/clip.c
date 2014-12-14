#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"

#include <jack/ringbuffer.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static void * clip_read (void * arg) {

    audio_clip_t * clip = (audio_clip_t*) clip;

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
                clip->sndfile, (float*) write_vector[0].buf, buf_avail);

        }

        if (read_frames == 0) break;

        jack_ringbuffer_write_advance(clip->ringbuf, read_frames * bytes_per_frame);

        if (clip->state == INIT) clip->state = READING;

        pthread_cond_wait(&clip->ready, &clip->lock);

    }

    if (clip->state == INIT)           clip->state = ENDED;
    else if (clip->state == READING)   clip->state = READY;
    else if (clip->state == READ_PLAY) clip->state = PLAY;

}

void clip_add(global_state_t * context,
              const char     * filename) {

    audio_clip_t * clip = calloc(1, sizeof(audio_clip_t));

    clip->state   = INIT;
    clip->sfinfo  = calloc(1, sizeof(SF_INFO));
    clip->sndfile = sf_open(filename, SFM_READ, clip->sfinfo);

    if (clip->sndfile == NULL) {
        FATAL("Could not open %s", filename);
        exit(1);
    }

    clip->filename = filename;

    MSG("%s: %d channels, %d kHz, %d frames, state %s",
        clip->filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames);

    clip->ringbuf = jack_ringbuffer_create(
        sizeof(jack_default_audio_sample_t) * RINGBUFFER_SIZE);
    memset(clip->ringbuf->buf, 0, clip->ringbuf->size);

    context->clips[0] = clip;

    pthread_create(&clip->thread, NULL, clip_read, clip);

}
