#ifndef _STDIO_H
# if defined __need_FILE || defined __need___FILE
#  include <libio/stdio.h>
# else
#  include <libio/stdio.h>

/* Now define the internal interfaces.  */
extern int __fcloseall (void);
extern int __snprintf (char *__restrict __s, size_t __maxlen,
		       __const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __vsnprintf (char *__restrict __s, size_t __maxlen,
			__const char *__restrict __format, _G_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));
extern int __vfscanf (FILE *__restrict __s,
		      __const char *__restrict __format,
		      _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int __vscanf (__const char *__restrict __format,
		     _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0)));
extern _IO_ssize_t __getline (char **__lineptr, size_t *__n,
			      FILE *__stream);
extern int __vsscanf (__const char *__restrict __s,
		      __const char *__restrict __format,
		      _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0)));

/* Prototypes for compatibility functions.  */
extern FILE *__new_tmpfile (void);
extern FILE *__old_tmpfile (void);



#  define __need_size_t
#  include <stddef.h>
/* Generate a unique file name (and possibly open it).  */
extern int __path_search (char *__tmpl, size_t __tmpl_len,
			  __const char *__dir, __const char *__pfx,
			  int __try_tempdir);

extern int __gen_tempname (char *__tmpl, int __kind);
/* The __kind argument to __gen_tempname may be one of: */
#  define __GT_FILE	0	/* create a file */
#  define __GT_BIGFILE	1	/* create a file, using open64 */
#  define __GT_DIR	2	/* create a directory */
#  define __GT_NOCREATE	3	/* just find a name not currently in use */

/* Print out MESSAGE on the error output and abort.  */
extern void __libc_fatal (__const char *__message)
     __attribute__ ((__noreturn__));

/* Acquire ownership of STREAM.  */
extern void __flockfile (FILE *__stream);

/* Relinquish the ownership granted for STREAM.  */
extern void __funlockfile (FILE *__stream);

/* Try to acquire ownership of STREAM but do not block if it is not
   possible.  */
extern int __ftrylockfile (FILE *__stream);

extern int __getc_unlocked (FILE *__fp);
#  ifdef USE_IN_LIBIO
extern wint_t __getwc_unlocked (FILE *__fp);
#  endif


extern __const char *__const _sys_errlist_internal[] attribute_hidden;
extern int _sys_nerr_internal attribute_hidden;

extern int __asprintf_internal (char **__restrict __ptr,
				__const char *__restrict __fmt, ...)
     attribute_hidden __attribute__ ((__format__ (__printf__, 2, 3)));
#  ifndef NOT_IN_libc
#    define __asprintf(ptr, fmt, args...) \
  INTUSE(__asprintf) (ptr, fmt, ##args)
#  endif

# endif

#endif
