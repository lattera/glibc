/* Compatibility gunk for the db library.  */

#include <sys/types.h>

#define EFTYPE EINVAL

/* Emulate Solaris llseek().  */
typedef loff_t offset_t;

extern int llseek (int fd, loff_t offset, int whence);
