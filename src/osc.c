#include "clip.h"
#include "debug.h"
#include "global.h"
#include "osc.h"

#include <lo/lo.h>


static void on_error (int num,
                      const char * msg,
                      const char * path) {

    FATAL("liblo server error %d in path %s: %s", num, path, msg);

}


OSC_CALLBACK (on_load)
    MSG("Load %s in slot %d", &argv[1]->s, argv[0]->i);
    clip_load(
        context,
        argv[0]->i,
        &argv[1]->s);
}


OSC_CALLBACK (on_play)
    clip_start(
        context,
        argv[0]->i,
        argv[1]->i);
}


OSC_CALLBACK (on_cue)
    clip_cue_set(
        context->clips[argv[0]->i],
        argv[1]->i,
        argv[2]->i);
}


OSC_CALLBACK (on_stop)
    clip_stop(
        context,
        argv[0]->i);
}


OSC_CALLBACK (on_listen)
    if (argc == 1) {
      context->listen_address = lo_address_new_from_url(&argv[0]->s);
    } else if (argc == 2) {
      context->listen_address = lo_address_new(&argv[0]->s, &argv[1]->s);
    }
}


void osc_start (global_state_t * context,
                char           * port_number) {

    context->osc_server = lo_server_thread_new(port_number, on_error);

    OSC_METHOD("/load",   "is",  on_load);
    OSC_METHOD("/play",   "ii",  on_play);
    OSC_METHOD("/cue",    "iii", on_cue);
    OSC_METHOD("/stop",   "i",   on_stop);
    OSC_METHOD("/listen", "ss",  on_listen);
    OSC_METHOD("/listen", "s",   on_listen);

    lo_server_thread_start(context->osc_server);

    return;

}


char * osc_get_url (global_state_t * context) {

    return lo_server_get_url(context->osc_server);

}


void osc_end (global_state_t * context) {

    lo_server_thread_free(context->osc_server);

}
