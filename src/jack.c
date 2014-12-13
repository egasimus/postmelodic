#include "debug.h"
#include "jack.h"

#include <stdlib.h>
#include <string.h>

static int process_callback (jack_nframes_t   nframes,
                             void           * arg) {
    MSG("%d", nframes);
    return 0;
}

jack_info_t * jack_start () {

    jack_info_t   * jack   = calloc(1, sizeof(jack_info_t));
    jack_status_t   status = 0;

    // try to connect
    jack->client = jack_client_open("foo", JackNullOption | JackNoStartServer, &status);

    // handle failure
    if (jack->client == 0) {
        if (status & JackServerFailed) {
            FATAL("Unable to connect to JACK server.");
        } else {
            FATAL("jack_client_open failed, status = 0x2.0x", status);
        }
        exit(1);
    }

    // notify success
    if (status & JackServerStarted) MSG("JACK server started.");

    // set callbacks
    jack_set_process_callback(jack->client, process_callback, 0);

    // return results
    return jack;


}

void jack_end (jack_info_t * jack) {

    jack_client_close(jack->client);

}
