#ifndef _OSC_H_
#define _OSC_H_

#include "debug.h"
#include "global.h"

void osc_start (global_state_t * context,
                char           * port_number);

char * osc_get_url (global_state_t * context);

void osc_end (global_state_t * context);

#define OSC_METHOD(a, b, c) \
  lo_server_thread_add_method(context->osc_server, a, b, c, context)

#define OSC_CALLBACK(name) \
  static int name (const char  * path,        \
                   const char  * types,       \
                   lo_arg     ** argv,        \
                   int           argc,        \
                   lo_message    msg,         \
                   void        * user_data) { \
    global_state_t * context = (global_state_t *) user_data;

#define OSC_NOTIFY(addr, types, ...)                            \
  if (context->listen_address != NULL) {                        \
    lo_send(context->listen_address, addr, types, __VA_ARGS__); \
  }
  

#endif
