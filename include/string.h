#ifndef _STRING_H
#include <string/string.h>

/* Now define the internal interfaces.  */
extern __ptr_t __memccpy __P ((__ptr_t __dest, __const __ptr_t __src,
			       int __c, size_t __n));

extern size_t __strnlen __P ((__const char *__string, size_t __maxlen));

extern char *__strsep __P ((char **__stringp, __const char *__delim));

extern int __strverscmp __P ((__const char *__s1, __const char *__s2));

extern int __strncasecmp __P ((__const char *__s1, __const char *__s2,
			       size_t __n));

extern char *__strndup __P ((__const char *__string, size_t __n));

#endif
