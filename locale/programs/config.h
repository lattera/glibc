#ifndef _LD_CONFIG_H
#define _LD_CONFIG_H

/* Use the internal textdomain used for libc messages.  */
#define PACKAGE _libc_intl_domainname
#ifndef VERSION
/* Get libc version number.  */
#include "../../version.h"
#endif

#define DEFAULT_CHARMAP "POSIX"

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif



#define HAVE_VPRINTF 1
#define HAVE_STRING_H 1


#include_next <config.h>

#endif
