#include "clip.h"
#include "debug.h"
#include "global.h"
#include "jack.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <jack/jack.h>

static int process_callback (jack_nframes_t   nframes,
                             void           * arg) {

    global_state_t * context    = (global_state_t*) arg;
    clip_index_t     clip_index = context->now_playing;
    audio_clip_t   * clip       = context->clips[clip_index];

    jack_default_audio_sample_t  * readbuf;
    jack_default_audio_sample_t ** output_buffers =
        calloc(1, sizeof(jack_default_audio_sample_t*));

    jack_nframes_t i;

    // get output buffer
    output_buffers[0] = jack_port_get_buffer(
        context->output_ports[0],
        nframes);

    // clear output buffer so output defaults to silence,
    // rather than looping any previous data in the buffer
    for (i = 0; i < nframes; i++) {
        output_buffers[0][i] = 0;
    }

    // give up if there's nothing to play
    if (clip_index == -1) return 0;

    // do nothing if the clip does not exist,
    // if it is stopped, or if it is not ready
    if (clip == NULL ||
        clip->read_state == CLIP_READ_INIT ||
        clip->play_state == CLIP_STOP) return 0;

    // fill output buffer with frames
    // from either cue or ring buffer
    for (i = 0; i < nframes; i++) {

        if (clip->cue > -1) {

            cue_point_t * cue = clip->cues[clip->cue];

            // write frame from cue buffer into output buffer
            output_buffers[0][i] =
                cue->buffer[(clip->position - cue->position) * clip->sfinfo->channels];

            // advance playback position
            clip->position++;

            // if we've passed the end of the cue buffer,
            // return to reading the ringbuffer instead.
            if (clip->position > cue->position + cue->length) {
                clip->cue = -1;
            }

        } else {

            // allocate intermediary read buffer
            readbuf = calloc(
                clip->sfinfo->channels,
                sizeof(jack_default_audio_sample_t*));

            // read one frame's worth of data from ringbuffer
            size_t read_count = jack_ringbuffer_read(
                clip->ringbuf,
                (void*)readbuf,
                FRAME_SIZE);

            // if there was nothing left to read, stop the clip.
            if (read_count == 0 && clip->play_state == CLIP_PLAY) {
                clip_stop(context, clip_index);
                return 0;
            }

            // advance playback position
            clip->position += read_count / FRAME_SIZE;

            // write frame into output buffer
            output_buffers[0][i] = readbuf[0]; 

        }

    }

    // if the reader thread is done, tell it to
    // read the next chunks into the ringbuffer
    if (pthread_mutex_trylock(&clip->lock) == 0) {
        pthread_cond_signal(&clip->ready);
        pthread_mutex_unlock(&clip->lock);
    };
 
    return 0;
}

void jack_start (global_state_t * context,
                 char           * client_name,
                 char           * connect_to) {

    jack_status_t status = 0;

    // try to open client
    context->jack_client = jack_client_open(
        client_name,
        JackNullOption | JackNoStartServer,
        &status);

    // handle failure
    if (context->jack_client == 0) {
        if (status & JackServerFailed) {
            FATAL("Unable to connect to JACK server.");
        } else {
            FATAL("jack_client_open failed, status = 0x2.0x", status);
        }
        exit(1);
    }

    // notify success
    if (status & JackServerStarted) MSG("JACK server started.");

    // open outputs
    context->output_ports    = calloc(1, sizeof(jack_port_t*)); 
    context->output_ports[0] = jack_port_register(context->jack_client,
                                                  "output",
                                                  JACK_DEFAULT_AUDIO_TYPE,
                                                  JackPortIsOutput,
                                                  0);

    // set callbacks
    jack_set_process_callback(context->jack_client, process_callback, context);

    // activate client
    if (jack_activate(context->jack_client)) {
        FATAL("Can't activate client :(");
        exit(1);
    }

    // connect to target client
    if (connect_to[0] != '\0') {
      MSG("Trying to connect to: %s", connect_to);
      jack_connect(
        context->jack_client,
        jack_port_name(context->output_ports[0]),
        connect_to);
    }

}

void jack_end (global_state_t * context) {

    jack_client_close(context->jack_client);

    free(context->output_ports);

}
