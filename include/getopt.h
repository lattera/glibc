#ifndef _GETOPT_H

#include <features.h>		/* Get __GNU_LIBRARY__ defined now.  */
#include <posix/getopt.h>

# if defined _GETOPT_H && !defined _ISOMAC

/* Now define the internal interfaces.  */
extern void __getopt_clean_environment (char **__env);

# endif /* _GETOPT_H && !_ISOMAC */
#endif
