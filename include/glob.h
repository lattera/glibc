#ifndef	_GLOB_H
#include <posix/glob.h>

/* Now define the internal interfaces.  */
extern int __glob_pattern_p (__const char *__pattern, int __quote);
extern int __glob64 (__const char *__pattern, int __flags,
		     int (*__errfunc) (__const char *, int),
		     glob64_t *__pglob);

#endif
