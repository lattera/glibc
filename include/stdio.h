#ifndef _STDIO_H
#ifdef USE_IN_LIBIO
#ifdef __need_FILE
# include <libio/stdio.h>
#else
# include <libio/stdio.h>

/* Now define the internal interfaces.  */
extern int __fcloseall __P ((void));
extern int __snprintf __P ((char *__restrict __s, size_t __maxlen,
			    __const char *__restrict __format, ...))
     __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __vfscanf __P ((FILE *__restrict __s,
			   __const char *__restrict __format,
			   _G_va_list __arg))
     __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int __vscanf __P ((__const char *__restrict __format,
			  _G_va_list __arg))
     __attribute__ ((__format__ (__scanf__, 1, 0)));
extern _IO_ssize_t __getline __P ((char **__lineptr, size_t *__n,
				   FILE *__stream));
extern int __vsscanf __P ((__const char *__restrict __s,
			   __const char *__restrict __format,
			   _G_va_list __arg))
     __attribute__ ((__format__ (__scanf__, 2, 0)));

#endif
#else
#include <stdio/stdio.h>
#endif

# define __need_size_t
# include <stddef.h>
/* Generate a unique file name (and possibly open it).  */
extern int __path_search __P ((char *__tmpl, size_t __tmpl_len,
			       __const char *__dir,
			       __const char *__pfx));

extern int __gen_tempname __P ((char *__tmpl, int __openit, int __large_file));

/* Print out MESSAGE on the error output and abort.  */
extern void __libc_fatal __P ((__const char *__message))
     __attribute__ ((__noreturn__));

   
#endif
