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


static int on_play (const char  * path,
                    const char  * types,
                    lo_arg     ** argv,
                    int           argc,
                    void        * data,
                    void        * user_data) {

    global_state_t * context = (global_state_t *) user_data;

    clip_start(
        context,
        argv[0]->h,
        argv[1]->h);

}


static int on_cue (const char  * path,
                   const char  * types,
                   lo_arg     ** argv,
                   int           argc,
                   void        * data,
                   void        * user_data) {

    global_state_t * context = (global_state_t *) user_data;

    clip_cue_set(
        context->clips[argv[0]->h],
        argv[1]->h,
        argv[2]->h);

}


void osc_start (global_state_t * context) {

    context->osc_server = lo_server_thread_new("7770", on_error);

    lo_server_thread_add_method(context->osc_server, "/play", "hh", on_play, context);
    lo_server_thread_add_method(context->osc_server, "/cue", "hhh", on_cue, context);

    lo_server_thread_start(context->osc_server);

    return;

}


void osc_end (global_state_t * context) {

    lo_server_thread_free(context->osc_server);

}
