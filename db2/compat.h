/* Compatibility gunk for the db library.  */

#include <sys/types.h>
#include <errno.h>

#include <sys/stat.h>
#ifdef _STATBUF_ST_BLKSIZE
# define HAVE_ST_BLKSIZE
#endif


#ifndef EFTYPE
# define EFTYPE EINVAL
#endif

/* Emulate Solaris llseek().  */
typedef loff_t offset_t;

extern int llseek (int fd, loff_t offset, int whence);
