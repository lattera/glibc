#ifndef _STRING_H

#include <sys/types.h>

extern void *__memccpy (void *__dest, __const void *__src,
			  int __c, size_t __n);

extern size_t __strnlen (__const char *__string, size_t __maxlen);

extern char *__strsep (char **__stringp, __const char *__delim);

extern int __strverscmp (__const char *__s1, __const char *__s2);

extern int __strncasecmp (__const char *__s1, __const char *__s2,
			  size_t __n);

extern char *__strndup (__const char *__string, size_t __n);

extern void *__rawmemchr (__const void *__s, int __c);

extern char *__strchrnul (__const char *__s, int __c);

extern void *__memrchr (__const void *__s, int __c, size_t __n);

/* Now the real definitions.  We do this here since some of the functions
   above are defined as macros in the headers.  */
#include <string/string.h>
#endif
