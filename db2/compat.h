/* Compatibility gunk for the db library.  */

#include <sys/types.h>
#include <errno.h>

#ifndef EFTYPE
# define EFTYPE EINVAL
#endif

/* Emulate Solaris llseek().  */
typedef loff_t offset_t;

extern int llseek (int fd, loff_t offset, int whence);
