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


static int on_load (const char  * path,
                    const char  * types,
                    lo_arg     ** argv,
                    int           argc,
                    void        * data,
                    void        * user_data) {

    global_state_t * context = (global_state_t *) user_data;

    MSG("Load %s in slot %d", &argv[1]->s, argv[0]->h);

    clip_load(
        context,
        argv[0]->h,
        &argv[1]->s);

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


static int on_stop (const char  * path,
                    const char  * types,
                    lo_arg     ** argv,
                    int           argc,
                    void        * data,
                    void        * user_data) {

    global_state_t * context = (global_state_t *) user_data;

    clip_stop(
        context,
        argv[0]->h);

}


void osc_start (global_state_t * context,
                char           * port_number) {

    context->osc_server = lo_server_thread_new(port_number, on_error);

    lo_server_thread_add_method(context->osc_server, "/load", "is",  on_load, context);
    lo_server_thread_add_method(context->osc_server, "/play", "ii",  on_play, context);
    lo_server_thread_add_method(context->osc_server, "/cue",  "iii", on_cue,  context);
    lo_server_thread_add_method(context->osc_server, "/stop", "i",   on_stop, context);

    lo_server_thread_start(context->osc_server);

    return;

}


char * osc_get_url (global_state_t * context) {

    return lo_server_get_url(context->osc_server);

}


void osc_end (global_state_t * context) {

    lo_server_thread_free(context->osc_server);

}
