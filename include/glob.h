#ifndef	_GLOB_H
#include <posix/glob.h>

libc_hidden_proto (glob)
libc_hidden_proto (glob64)
libc_hidden_proto (globfree)
libc_hidden_proto (globfree64)

/* Now define the internal interfaces.  */
extern int __glob_pattern_p (const char *__pattern, int __quote);
extern int __glob64 (const char *__pattern, int __flags,
		     int (*__errfunc) (const char *, int),
		     glob64_t *__pglob);

#endif
