#ifndef _ARGP_H
#include <argp/argp.h>

/* This hack to allow programs that know what's going on to call argp
   recursively.  If someday argp is changed not to use the non-reentrant
   getopt interface, we can get rid of this shit.  XXX */
extern void _argp_unlock_xxx (void);

#endif
