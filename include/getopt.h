#ifndef _GETOPT_H

#include <features.h>		/* Get __GNU_LIBRARY__ defined now.  */
#include <posix/getopt.h>

# ifdef _GETOPT_H

/* Now define the internal interfaces.  */
extern void __getopt_clean_environment (char **__env);

# endif

#endif
