#ifndef _STRING_H

#include <sys/types.h>

extern void *__memccpy (void *__dest, __const void *__src,
			  int __c, size_t __n);

extern size_t __strnlen (__const char *__string, size_t __maxlen)
     __attribute_pure__;

extern char *__strsep (char **__stringp, __const char *__delim);

extern int __strverscmp (__const char *__s1, __const char *__s2)
     __attribute_pure__;

extern int __strncasecmp (__const char *__s1, __const char *__s2,
			  size_t __n)
     __attribute_pure__;

extern int __strcasecmp (__const char *__s1, __const char *__s2)
     __attribute_pure__;

extern char *__strcasestr (__const char *__haystack, __const char *__needle)
     __attribute_pure__;

extern char *__strdup (__const char *__string)
     __attribute_malloc__;
extern char *__strndup (__const char *__string, size_t __n)
     __attribute_malloc__;

extern void *__rawmemchr (__const void *__s, int __c)
     __attribute_pure__;

extern char *__strchrnul (__const char *__s, int __c)
     __attribute_pure__;

extern void *__memrchr (__const void *__s, int __c, size_t __n)
     __attribute_pure__;

extern void *__memchr (__const void *__s, int __c, size_t __n)
     __attribute_pure__;

extern int __ffs (int __i) __attribute__ ((const));

extern char *__strerror_r (int __errnum, char *__buf, size_t __buflen);

/* Now the real definitions.  We do this here since some of the functions
   above are defined as macros in the headers.  */
#include <string/string.h>

/* Alternative version which doesn't pollute glibc's namespace.  */
#undef strndupa
#define strndupa(s, n)							      \
  (__extension__							      \
    ({									      \
      __const char *__old = (s);					      \
      size_t __len = __strnlen (__old, (n));				      \
      char *__new = (char *) __builtin_alloca (__len + 1);		      \
      __new[__len] = '\0';						      \
      (char *) memcpy (__new, __old, __len);				      \
    }))
#endif
