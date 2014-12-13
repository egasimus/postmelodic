#include "debug.h"
#include "jack.h"

#include <stdlib.h>
#include <string.h>

static int process_callback (jack_nframes_t   nframes,
                             void           * arg) {
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

    // open outputs
    jack->output_ports = calloc(1, sizeof(jack_port_t*)); 
    jack->output_ports[0] = jack_port_register(jack->client,
                                               "foo",
                                               JACK_DEFAULT_AUDIO_TYPE,
                                               JackPortIsOutput,
                                               0);

    // set callbacks
    jack_set_process_callback(jack->client, process_callback, 0);

    // activate client
    if (jack_activate(jack->client)) {
        FATAL("Can't activate client :(");
        exit(1);
    }

    // return results
    return jack;

}

void jack_end (jack_info_t * jack) {

    jack_client_close(jack->client);

    free(jack->output_ports);

}
