#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

extern char * program_invocation_short_name;

void warnf(warning_t    level,
           const char * module,
           const char * file,
           const char * function,
           int          line,
           const char * fmt,
           ...) {

    va_list             args;
    static const char * level_tab[] = {
        "message",   "\033[1;32m",
        "warning",   "\033[1;33m",
        "assertion", "\033[1;31m"
    };

    module = program_invocation_short_name;

    if (module) fprintf(stderr, "[\033[1;30m%s\033[0m] ", module);

#ifdef DEBUG
    if (file)     fprintf( stderr, "%s",    file );
    if (line)     fprintf( stderr, ":%i",   line );
    if (function) fprintf( stderr, " %s()", function );
    fprintf( stderr, ": " );
#endif

    unsigned l = (unsigned)((level << 1) + 1);
    unsigned t = sizeof(level_tab) / sizeof(level_tab[0]);
    if (l < t) {
        fprintf(stderr, "%s", level_tab[(level << 1) + 1]);
        if (fmt) {
            va_start(args,   fmt);
            vfprintf(stderr, fmt, args);
            va_end(args );
        }
        fprintf(stderr, "\033[0m\n");
    }

}
