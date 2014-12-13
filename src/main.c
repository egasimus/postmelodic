#include "config.h"
#include "debug.h"
#include "reader.h"
#include "jack.h"
#include "osc.h"

int
main (int    argc,
      char * argv [])
{
    const char * filename;

    MSG("Hello :(");
    
    reader_preload_file(filename);

    jack_start_thread();

    osc_start_thread();

    jack_end_thread();

    osc_end_thread();
}
