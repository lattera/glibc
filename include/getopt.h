#ifndef _GETOPT_H

#include <posix/getopt.h>

# ifdef _GETOPT_H

libc_hidden_proto (getopt_long)
libc_hidden_proto (getopt_long_only)

/* Now define the internal interfaces.  */
extern void __getopt_clean_environment (char **__env);

# endif

#endif
