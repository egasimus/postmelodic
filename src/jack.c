#include "clip.h"
#include "debug.h"
#include "global.h"
#include "jack.h"

#include <stdlib.h>
#include <string.h>
#include <jack/jack.h>

static int process_callback (jack_nframes_t   nframes,
                             void           * arg) {

    global_state_t * context = (global_state_t*) arg;
    audio_clip_t   * clip    = context->clips[0];
    jack_nframes_t   i;

    jack_default_audio_sample_t buf [clip->sfinfo->channels];

    MSG("%s: %d channels, %d kHz, %d frames, state: %d",
        clip->filename,
        clip->sfinfo->channels,
        clip->sfinfo->samplerate,
        clip->sfinfo->frames,
        clip->state);

    if (clip->state == INIT) return 0;

    context->output_buffers[0] = jack_port_get_buffer(
        context->output_ports[0],
        nframes);
   
    for (i = 0; i < nframes; i++) {
        size_t read_count = jack_ringbuffer_read(
            clip->ringbuf, (void*)buf, FRAME_SIZE);
        if (read_count == 0 && clip->state == PLAY) {
            clip->state = ENDED;
            return 0;
        }
        clip->position += read_count / FRAME_SIZE;
        context->output_buffers[0][i] = buf[0]; 
    }
 
    return 0;
}

void jack_start (global_state_t * context) {

    jack_status_t status = 0;

    // try to connect
    context->jack_client = jack_client_open(
        "foo",
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
    context->output_buffers  = calloc(1, sizeof(jack_default_audio_sample_t*));
    context->output_ports[0] = jack_port_register(context->jack_client,
                                                  "foo",
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

}

void jack_end (global_state_t * context) {

    jack_client_close(context->jack_client);

    free(context->output_ports);
    free(context->output_buffers);

}
