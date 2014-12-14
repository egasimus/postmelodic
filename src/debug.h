#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef __MODULE__
#ifdef PACKAGE
#define __MODULE__ PACKAGE
#else
#define __MODULE__ 0
#endif
#endif

#ifndef __GNUC__
#define __FUNCTION__ 0
#endif

typedef enum {
    W_MESSAGE = 0,
    W_WARNING,
    W_FATAL
} warning_t;

void
warnf(warning_t    level,
      const char * module,
      const char * file,
      const char * function,
      int          line,
      const char * fmt,
      ...);

#ifdef DEBUG
  #define DMSG(  fmt,  args... ) warnf( W_MESSAGE, __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )
  #define DWARN( fmt,  args... ) warnf( W_WARNING, __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )
  #define ASSERT(pred, fmt, args... ) do { if ( ! (pred) ) { warnf( W_FATAL, __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args ); exit(1); } } while ( 0 )
#else
  #define DMSG(  fmt,  args... )
  #define DWARN( fmt,  args... )
  #define ASSERT(pred, fmt, args... ) (void)(pred)
#endif

#define MSG(  fmt, args... )  warnf( W_MESSAGE, __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )
#define WARN( fmt, args... )  warnf( W_WARNING, __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )
#define FATAL(fmt, args... ) (warnf( W_FATAL,   __MODULE__, __FILE__, __FUNCTION__, __LINE__, fmt, ## args ), exit(1))

#define RMSG( fmt, args... ) ( MSG(fmt, ## args), fprintf(stderr, "\033[1A\r") )

#endif
