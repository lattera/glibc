#ifndef _SYS_STATFS_H
#include <io/sys/statfs.h>

/* Now define the internal interfaces.  */
extern int __statfs (__const char *__file, struct statfs *__buf);
extern int __fstatfs (int __fildes, struct statfs *__buf);
#endif
