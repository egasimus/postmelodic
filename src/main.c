#include "clip.h"
#include "config.h"
#include "debug.h"
#include "global.h"
#include "jack.h"
#include "osc.h"

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

global_state_t * context;

void alright_stop (int s) {
    osc_end(context);
    jack_end(context);

    free(context->clips);
    free(context);

    exit(s);
}

int main (int    argc,
          char * argv []) {

    char sample_path [1025];
    char client_name [257] = "Postmelodic";
    char connect_to  [257] = "\0";
    char port_number [6]   = "\0";
    int  opt;
    int  verbose           = 0;

    // parse command line
    opterr = 0;
    while ((opt = getopt (argc, argv, "n:p:c:v")) != -1) {

        switch (opt) {
            case 'n':
                strncpy(client_name, optarg, 256);
                break;
            case 'p':
                strncpy(port_number, optarg, 5);
                MSG("Requested OSC control port: %s", port_number);
                break;
            case 'c':
                strncpy(connect_to, optarg, 256);
                MSG("Will try to connect to: %s", connect_to);
            case 'v':
                verbose = 1;
                MSG("Verbose mode on");
            case '?':
                if (isprint(optopt)) {
                    MSG("Unknown option '-%c'.", optopt);
                } else {
                    MSG("Unknown option character '\\x%x'.", optopt);
                } 
        }

    }

    MSG("Requested JACK client name: %s", client_name);

    if (argv[optind] == NULL) FATAL("No sample path specified.");
    
    realpath(argv[optind], sample_path);
    
    if (access(sample_path, R_OK) != -1) {
        MSG("Loading sample %s", sample_path);
    } else {
        FATAL("Sample %s does not exist or is not readable.", sample_path);
    }

    context          = calloc(1, sizeof(global_state_t));
    context->clips   = calloc(INITIAL_CLIP_SLOTS, sizeof(audio_clip_t));
    context->n_clips = 0;

    jack_start(context, client_name, connect_to);
    osc_start(context, port_number);

    struct sigaction act;
    act.sa_handler = alright_stop;
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    
    audio_clip_t * clip = context->clips[clip_add(context, sample_path)];

    while (1) {

      if (verbose &&
          !(clip == NULL                       ||
            clip->read_state == CLIP_READ_INIT ||
            clip->play_state == CLIP_STOP      )) {

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

      usleep(UPDATE_EVERY);

    }

    alright_stop(0);
}
