/* Get `struct dirent' from the Linux kernel header file.  */

#ifndef _DIRENTRY_H
#define _DIRENTRY_H

#include <linux/dirent.h>

#define d_fileno	d_ino	/* backwards compatibility */

#undef  _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF

#endif /* _DIRENTRY_H */
