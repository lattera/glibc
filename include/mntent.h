#ifndef _MNTENT_H
#include <misc/mntent.h>

/* Now define the internal interfaces.  */
extern FILE *__setmntent (__const char *__file, __const char *__mode) __THROW;
extern struct mntent *__getmntent_r (FILE *__stream,
				     struct mntent *__result,
				     char *__buffer, int __bufsize) __THROW;
extern int __addmntent (FILE *__stream, __const struct mntent *__mnt) __THROW;
extern int __endmntent (FILE *__stream) __THROW;
extern char *__hasmntopt (__const struct mntent *__mnt,
			  __const char *__opt) __THROW;
#endif
