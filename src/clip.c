#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"

#include <lo/lo.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void * clip_read (void * arg) {

    audio_clip_t * clip = (audio_clip_t*) arg;
    cue_index_t    i;
    cue_point_t  * cue;
    int            cued = 0;

    sf_count_t             buf_avail;
    sf_count_t             read_frames;
    jack_ringbuffer_data_t write_vector[2];

    size_t bytes_per_frame = FRAME_SIZE;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_mutex_lock(&clip->lock);

    while (1) {

        // read next chunk of data into the ringbuffer.
        // if the jack process_callback is currently reading
        // from a cuepoint buffer, preload the data after
        // the cue buffer's end into the ringbuffer.
        if (clip->cue > -1) {
            if (cued == 0) {

                cued = 1;

                // at this point jack callback should be
                // reading from cue buffer, so it's safe
                // to reset the ringbuffer in the meantime
                jack_ringbuffer_reset(clip->ringbuf);

                // seek file to first frame after end of
                // cue buffer data, so that the ringbuffer
                // continues from where the cue buffer left off
                cue = clip->cues[clip->cue];
                sf_seek(clip->sndfile,
                        cue->position + cue->length,
                        SEEK_SET);

            }
        } else cued = 0;

        // see if there's some free space for writing in the ringbuffer
        // a bit of the ringbuffer is made writable every time the jack
        // callback consumes its data; the entire ringbuffer is emptied
        // as soon as the callback starts reading from a cue buffer.
        jack_ringbuffer_get_write_vector(clip->ringbuf, write_vector);

        read_frames = 0;

        if (write_vector[0].len) {

            buf_avail = write_vector[0].len / bytes_per_frame;

            read_frames = sf_readf_float(
                clip->sndfile,
                (float*) write_vector[0].buf,
                buf_avail);

        }

        if (read_frames > 0) {

            // if we've read some data from the file, write it
            // into the ringbuffer, advance the write pointer,
            // and let the callback know we've read some data.
            jack_ringbuffer_write_advance(clip->ringbuf, read_frames * bytes_per_frame);

            clip->read_state = CLIP_READ_STARTED;

        } else {

            // on the other hand, if there was no data left to
            // read in the file, then we're done for now.
            clip->read_state = CLIP_READ_DONE;

        }

        // and then we wait for the jack callback
        // to consume some data from the ringbuffer
        pthread_cond_wait(&clip->ready, &clip->lock);

    }

}

static void * clip_read_cues (void * arg) {

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    audio_clip_t * clip = (audio_clip_t*) arg;
    cue_index_t    i;
    cue_point_t  * cue;
    sf_count_t     read_frames;

    // the loop runs once when the thread is started,
    // then blocks until clip_cue_set unlocks the mutex

    while (1) {

        pthread_mutex_lock(&clip->cue_lock);

        for (i = 0; i < INITIAL_CUE_SLOTS; i++) {

            cue = clip->cues[i];

            if (cue != NULL && cue->buffer == NULL) {

                cue->buffer = calloc(1, BUFFER_SIZE);

                sf_seek(clip->cue_sndfile, cue->position, SEEK_SET);
                read_frames = sf_readf_float(clip->cue_sndfile, cue->buffer, BUFFER_FRAMES);

                cue->length = read_frames / clip->sfinfo->channels;

            }

        }

    }

}

void clip_cue_set (audio_clip_t * clip,
                   cue_index_t    index,
                   jack_nframes_t position) {

    cue_point_t * cue = calloc(1, sizeof(cue_point_t));

    cue->position = position;
    cue->buffer   = NULL;

    clip->cues[index] = cue;

    pthread_mutex_unlock(&clip->cue_lock);

    MSG("cue %d %d %d", clip, index, position);

}

void clip_cue_jump (audio_clip_t * clip,
                    cue_index_t    index) {

    clip->position = clip->cues[index]->position;
    clip->cue      = index;

}

static void * clip_osc_notify (void * arg) {

    // sending osc notifications that playback has reached its end
    // separately from the jack process callback where it happens
    // since lo_send blocks and can therefore cause xruns galore.
    // thanks to @falktx for helping me make this work.

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    context_and_clip_t *    args = (context_and_clip_t *) arg;
    global_state_t     * context = args->context;
    clip_index_t           index = args->clip_index;
    audio_clip_t       *    clip = context->clips[index];

    // mutex starts locked. when unlocked (by jack process callback),
    // thread sends a "/stopped" notification and locks itself again

    while (1) {

        pthread_mutex_lock(&clip->osc_lock);

        if (context->listen_address) {
            int err = lo_send(
                context->listen_address, "/stopped", "is", 1, context->osc_port
            );
        }

    }

}

clip_index_t clip_load (global_state_t * context,
                        clip_index_t     index,
                        const char     * filename) {

    char sample_path [1025];
    realpath(filename, sample_path);
    if (access(sample_path, R_OK) != -1) {
        MSG("Loading sample %s into slot %d", sample_path, index);
    } else {
        FATAL("Sample %s does not exist or is not readable.", sample_path);
    }

    audio_clip_t * clip = calloc(1, sizeof(audio_clip_t));
    clip->filename    = sample_path;
    clip->read_state  = CLIP_READ_INIT;
    clip->play_state  = CLIP_STOP;
    clip->sfinfo      = calloc(1, sizeof(SF_INFO));
    clip->sndfile     = sf_open(filename, SFM_READ, clip->sfinfo);
    clip->cue_sfinfo  = calloc(1, sizeof(SF_INFO));
    clip->cue_sndfile = sf_open(filename, SFM_READ, clip->cue_sfinfo);

    if (clip->sndfile == NULL) {
        FATAL("Could not open %s", filename);
        exit(1);
    }

    // initialize cues
    clip->cue  = -1;
    clip->cues = calloc(INITIAL_CUE_SLOTS, sizeof(cue_point_t*));
    clip_cue_set(clip, 0, 0);

    // initialize ringbuffer
    clip->ringbuf = jack_ringbuffer_create(BUFFER_SIZE);
    memset(clip->ringbuf->buf, 0, clip->ringbuf->size);

    // log clip info
    MSG("%s: %d channels   %d kHz   %d frames   read %d   play %d   cue %d",
        clip->filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames,
        clip->read_state,
        clip->play_state,
        clip->cue);

    // initialize cue reader thread
    pthread_mutex_init(&clip->cue_lock, NULL);
    pthread_create(&clip->cue_thread, NULL, clip_read_cues, clip);

    // initialize ringbuffer reader thread
    pthread_mutex_init(&clip->lock, NULL);
    pthread_cond_init(&clip->ready, NULL);
    pthread_create(&clip->thread, NULL, clip_read, clip);

    // add clip to global list of clips
    context->clips[index] = clip;

    // initialize osc notification thread
    pthread_mutex_init(&clip->osc_lock, NULL);
    context_and_clip_t arg = { context, index };
    pthread_create(&clip->osc_thread, NULL, clip_osc_notify, &arg);
    pthread_mutex_lock(&clip->osc_lock);

    return 0;

}

void clip_start (global_state_t * context,
                 clip_index_t     clip_index,
                 cue_index_t      cue_index) {

    audio_clip_t * clip = context->clips[clip_index];
    context->now_playing = clip_index;
    clip_cue_jump(clip, cue_index);
    clip->play_state = CLIP_PLAY;

}


void clip_stop (global_state_t * context,
                clip_index_t     clip_index) {

    audio_clip_t * clip = context->clips[clip_index];
    clip->play_state = CLIP_STOP;
    pthread_mutex_unlock(&clip->osc_lock);

}
