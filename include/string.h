#ifndef _STRING_H

#ifndef _ISOMAC
#include <sys/types.h>

extern void *__memccpy (void *__dest, const void *__src,
			int __c, size_t __n);

extern size_t __strnlen (const char *__string, size_t __maxlen)
     __attribute_pure__;

extern char *__strsep (char **__stringp, const char *__delim);

extern int __strverscmp (const char *__s1, const char *__s2)
     __attribute_pure__;

extern int __strncasecmp (const char *__s1, const char *__s2,
			  size_t __n)
     __attribute_pure__;

extern int __strcasecmp (const char *__s1, const char *__s2)
     __attribute_pure__;

extern char *__strcasestr (const char *__haystack, const char *__needle)
     __attribute_pure__;

extern char *__strdup (const char *__string)
     __attribute_malloc__;
extern char *__strndup (const char *__string, size_t __n)
     __attribute_malloc__;

extern void *__rawmemchr (const void *__s, int __c)
     __attribute_pure__;

extern char *__strchrnul (const char *__s, int __c)
     __attribute_pure__;

extern void *__memrchr (const void *__s, int __c, size_t __n)
     __attribute_pure__;

extern void *__memchr (const void *__s, int __c, size_t __n)
     __attribute_pure__;

extern int __ffs (int __i) __attribute__ ((const));

extern char *__strerror_r (int __errnum, char *__buf, size_t __buflen);
#endif

/* Now the real definitions.  We do this here since some of the functions
   above are defined as macros in the headers.  */
#include <string/string.h>

#ifndef _ISOMAC
extern __typeof (strcoll_l) __strcoll_l;
extern __typeof (strxfrm_l) __strxfrm_l;
extern __typeof (strcasecmp_l) __strcasecmp_l;
extern __typeof (strncasecmp_l) __strncasecmp_l;

/* Alternative version which doesn't pollute glibc's namespace.  */
#ifndef NOT_IN_libc
# undef strndupa
# define strndupa(s, n)							      \
  (__extension__							      \
    ({									      \
      const char *__old = (s);						      \
      size_t __len = __strnlen (__old, (n));				      \
      char *__new = (char *) __builtin_alloca (__len + 1);		      \
      __new[__len] = '\0';						      \
      (char *) memcpy (__new, __old, __len);				      \
    }))
#endif

libc_hidden_proto (__mempcpy)
libc_hidden_proto (__stpcpy)
libc_hidden_proto (__stpncpy)
libc_hidden_proto (__rawmemchr)
libc_hidden_proto (__strcasecmp)
libc_hidden_proto (__strcasecmp_l)
libc_hidden_proto (__strncasecmp_l)
libc_hidden_proto (__strdup)
libc_hidden_proto (__strndup)
libc_hidden_proto (__strerror_r)
libc_hidden_proto (__strverscmp)
libc_hidden_proto (basename)
libc_hidden_proto (strcoll)
libc_hidden_proto (__strcoll_l)
libc_hidden_proto (__strxfrm_l)
libc_hidden_proto (__strtok_r)
extern char *__strsep_g (char **__stringp, const char *__delim);
libc_hidden_proto (__strsep_g)
libc_hidden_proto (strnlen)

libc_hidden_builtin_proto (memchr)
libc_hidden_builtin_proto (memcpy)
libc_hidden_builtin_proto (mempcpy)
libc_hidden_builtin_proto (memcmp)
libc_hidden_builtin_proto (memmove)
libc_hidden_builtin_proto (memset)
libc_hidden_builtin_proto (strcat)
libc_hidden_builtin_proto (strchr)
libc_hidden_builtin_proto (strcmp)
libc_hidden_builtin_proto (strcpy)
libc_hidden_builtin_proto (strcspn)
libc_hidden_builtin_proto (strlen)
libc_hidden_builtin_proto (strncmp)
libc_hidden_builtin_proto (strncpy)
libc_hidden_builtin_proto (strpbrk)
libc_hidden_builtin_proto (stpcpy)
libc_hidden_builtin_proto (strrchr)
libc_hidden_builtin_proto (strspn)
libc_hidden_builtin_proto (strstr)
libc_hidden_builtin_proto (ffs)

# ifndef _ISOMAC
#  ifndef index
#   define index(s, c)	(strchr ((s), (c)))
#  endif
#  ifndef rindex
#   define rindex(s, c)	(strrchr ((s), (c)))
#  endif
# endif

extern void *__memcpy_chk (void *__restrict __dest,
			   const void *__restrict __src, size_t __len,
			   size_t __destlen) __THROW;
extern void *__memmove_chk (void *__dest, const void *__src, size_t __len,
			    size_t __destlen) __THROW;
extern void *__mempcpy_chk (void *__restrict __dest,
			    const void *__restrict __src, size_t __len,
			    size_t __destlen) __THROW;
extern void *__memset_chk (void *__dest, int __ch, size_t __len,
			   size_t __destlen) __THROW;
extern char *__strcpy_chk (char *__restrict __dest,
			   const char *__restrict __src,
			   size_t __destlen) __THROW;
extern char *__stpcpy_chk (char *__restrict __dest,
			   const char *__restrict __src,
			   size_t __destlen) __THROW;
extern char *__strncpy_chk (char *__restrict __dest,
			    const char *__restrict __src,
			    size_t __len, size_t __destlen) __THROW;
extern char *__strcat_chk (char *__restrict __dest,
			   const char *__restrict __src,
			   size_t __destlen) __THROW;
extern char *__strncat_chk (char *__restrict __dest,
			    const char *__restrict __src,
			    size_t __len, size_t __destlen) __THROW;
#endif

#endif
