#include "debug.h"
#include "osc.h"

#include <lo/lo.h>

static void on_error (int num,
                      const char * msg,
                      const char * path) {

    FATAL("liblo server error %d in path %s: %s", num, path, msg);

}

static int on_any (const char  * path,
                   const char  * types,
                   lo_arg     ** argv,
                   int           argc,
                   void        * data,
                   void        * user_data) {

    int i;

    MSG("OSC message received at %s", path);

    for (i = 0; i < argc; i++) {
        MSG("arg #%d [%c]", i, types[i]);
        lo_arg_pp((lo_type) types[i],
                  argv[i]);
        MSG("");
    }

    return 1;

}

void osc_start (global_state_t * context) {

    context->osc_server = lo_server_thread_new("7770", on_error);

    lo_server_thread_add_method(context->osc_server, NULL, NULL, on_any, NULL);

    lo_server_thread_start(context->osc_server);

    return;

}

void osc_end (global_state_t * context) {

    lo_server_thread_free(context->osc_server);

}
