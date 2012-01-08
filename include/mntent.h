#ifndef _MNTENT_H
#include <misc/mntent.h>

/* Now define the internal interfaces.  */
extern FILE *__setmntent (const char *__file, const char *__mode);
extern FILE *__setmntent_internal (const char *__file, const char *__mode);
extern struct mntent *__getmntent_r (FILE *__stream,
				     struct mntent *__result,
				     char *__buffer, int __bufsize);
extern struct mntent *__getmntent_r_internal (FILE *__stream,
					      struct mntent *__result,
					      char *__buffer, int __bufsize)
     attribute_hidden;
extern int __addmntent (FILE *__stream, const struct mntent *__mnt);
extern int __endmntent (FILE *__stream);
extern int __endmntent_internal (FILE *__stream) attribute_hidden;
extern char *__hasmntopt (const struct mntent *__mnt, const char *__opt);

#ifndef NOT_IN_libc
# define __setmntent(file, mode) INTUSE(__setmntent) (file, mode)
# define __endmntent(stream) INTUSE(__endmntent) (stream)
# define __getmntent_r(stream, result, buffer, bufsize) \
  INTUSE(__getmntent_r) (stream, result, buffer, bufsize)
#endif

#endif
