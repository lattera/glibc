#ifndef _SYS_STATVFS_H
#include <io/sys/statvfs.h>

/* Now define the internal interfaces.  */
extern int __statvfs64 (__const char *__file, struct statvfs64 *__buf);
extern int __fstatvfs64 (int __fildes, struct statvfs64 *__buf);

libc_hidden_proto (statvfs)
libc_hidden_proto (fstatvfs)
#endif
