#include "config.h"
#include "debug.h"
#include "reader.h"
#include "jack.h"
#include "osc.h"

int
main (int    argc,
      char * argv [])
{
    const char  * filename;
    jack_info_t * jack;
    
    reader_preload_file(filename);

    jack = jack_start();

    osc_start();

    sleep(5);

    jack_end(jack);

    osc_end();
}
