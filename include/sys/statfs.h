#ifndef _SYS_STATFS_H
#include <io/sys/statfs.h>

/* Now define the internal interfaces.  */
extern int __statfs __P ((__const char *__file, struct statfs *__buf));
extern int __fstatfs __P ((int __fildes, struct statfs *__buf));
#endif
