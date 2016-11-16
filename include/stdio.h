#ifndef _STDIO_H
# if defined __need_FILE || defined __need___FILE || defined _ISOMAC
#  include <libio/stdio.h>
# else
#  include <libio/stdio.h>

/* Now define the internal interfaces.  */
__BEGIN_DECLS

extern int __fcloseall (void);
extern int __snprintf (char *__restrict __s, size_t __maxlen,
		       const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));
libc_hidden_proto (__snprintf)
extern int __vsnprintf (char *__restrict __s, size_t __maxlen,
			const char *__restrict __format, _G_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));
extern int __vfscanf (FILE *__restrict __s,
		      const char *__restrict __format,
		      _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0)));
libc_hidden_proto (__vfscanf)
extern int __vscanf (const char *__restrict __format,
		     _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0)));
extern _IO_ssize_t __getline (char **__lineptr, size_t *__n,
			      FILE *__stream);
extern int __vsscanf (const char *__restrict __s,
		      const char *__restrict __format,
		      _G_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0)));

#  ifndef __cplusplus
extern int __sprintf_chk (char *, int, size_t, const char *, ...) __THROW;
extern int __snprintf_chk (char *, size_t, int, size_t, const char *, ...)
     __THROW;
extern int __vsprintf_chk (char *, int, size_t, const char *,
			   _G_va_list) __THROW;
extern int __vsnprintf_chk (char *, size_t, int, size_t, const char *,
			    _G_va_list) __THROW;
extern int __printf_chk (int, const char *, ...);
extern int __fprintf_chk (FILE *, int, const char *, ...);
extern int __vprintf_chk (int, const char *, _G_va_list);
extern int __vfprintf_chk (FILE *, int, const char *, _G_va_list);
extern char *__fgets_unlocked_chk (char *buf, size_t size, int n, FILE *fp);
extern char *__fgets_chk (char *buf, size_t size, int n, FILE *fp);
extern int __asprintf_chk (char **, int, const char *, ...) __THROW;
extern int __vasprintf_chk (char **, int, const char *, _G_va_list) __THROW;
extern int __dprintf_chk (int, int, const char *, ...);
extern int __vdprintf_chk (int, int, const char *, _G_va_list);
extern int __obstack_printf_chk (struct obstack *, int, const char *, ...)
     __THROW;
extern int __obstack_vprintf_chk (struct obstack *, int, const char *,
				  _G_va_list) __THROW;
#  endif

extern int __isoc99_fscanf (FILE *__restrict __stream,
			    const char *__restrict __format, ...) __wur;
extern int __isoc99_scanf (const char *__restrict __format, ...) __wur;
extern int __isoc99_sscanf (const char *__restrict __s,
			    const char *__restrict __format, ...) __THROW;
extern int __isoc99_vfscanf (FILE *__restrict __s,
			     const char *__restrict __format,
			     _G_va_list __arg) __wur;
extern int __isoc99_vscanf (const char *__restrict __format,
			    _G_va_list __arg) __wur;
extern int __isoc99_vsscanf (const char *__restrict __s,
			     const char *__restrict __format,
			     _G_va_list __arg) __THROW;
libc_hidden_proto (__isoc99_vsscanf)
libc_hidden_proto (__isoc99_vfscanf)

/* Prototypes for compatibility functions.  */
extern FILE *__new_tmpfile (void);
extern FILE *__old_tmpfile (void);



#  define __need_size_t
#  define __need_wint_t
#  include <stddef.h>
/* Generate a unique file name (and possibly open it).  */
extern int __path_search (char *__tmpl, size_t __tmpl_len,
			  const char *__dir, const char *__pfx,
			  int __try_tempdir);

extern int __gen_tempname (char *__tmpl, int __suffixlen, int __flags,
			   int __kind);
/* The __kind argument to __gen_tempname may be one of: */
#  define __GT_FILE	0	/* create a file */
#  define __GT_DIR	1	/* create a directory */
#  define __GT_NOCREATE	2	/* just find a name not currently in use */

/* Print out MESSAGE on the error output and abort.  */
extern void __libc_fatal (const char *__message)
     __attribute__ ((__noreturn__));
extern void __libc_message (int do_abort, const char *__fnt, ...);
extern void __fortify_fail (const char *msg)
     __attribute__ ((__noreturn__)) internal_function;
libc_hidden_proto (__fortify_fail)

/* Acquire ownership of STREAM.  */
extern void __flockfile (FILE *__stream);

/* Relinquish the ownership granted for STREAM.  */
extern void __funlockfile (FILE *__stream);

/* Try to acquire ownership of STREAM but do not block if it is not
   possible.  */
extern int __ftrylockfile (FILE *__stream);

extern int __getc_unlocked (FILE *__fp);
extern wint_t __getwc_unlocked (FILE *__fp);

extern int __fxprintf (FILE *__fp, const char *__fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));

extern const char *const _sys_errlist_internal[] attribute_hidden;
extern int _sys_nerr_internal attribute_hidden;

libc_hidden_proto (__asprintf)
#  if IS_IN (libc)
extern _IO_FILE *_IO_new_fopen (const char*, const char*);
#   define fopen(fname, mode) _IO_new_fopen (fname, mode)
extern _IO_FILE *_IO_new_fdopen (int, const char*);
#   define fdopen(fd, mode) _IO_new_fdopen (fd, mode)
extern int _IO_new_fclose (_IO_FILE*);
#   define fclose(fp) _IO_new_fclose (fp)
extern int _IO_fputs (const char*, _IO_FILE*);
libc_hidden_proto (_IO_fputs)
#   define fputs(str, fp) _IO_fputs (str, fp)
extern int _IO_new_fsetpos (_IO_FILE *, const _IO_fpos_t *);
#   define fsetpos(fp, posp) _IO_new_fsetpos (fp, posp)
extern int _IO_new_fgetpos (_IO_FILE *, _IO_fpos_t *);
#   define fgetpos(fp, posp) _IO_new_fgetpos (fp, posp)
#  endif

libc_hidden_proto (dprintf)
extern __typeof (dprintf) __dprintf
     __attribute__ ((__format__ (__printf__, 2, 3)));
libc_hidden_proto (__dprintf)
libc_hidden_proto (fprintf)
libc_hidden_proto (vfprintf)
libc_hidden_proto (sprintf)
libc_hidden_proto (sscanf)
libc_hidden_proto (fwrite)
libc_hidden_proto (perror)
libc_hidden_proto (remove)
libc_hidden_proto (rewind)
libc_hidden_proto (fileno)
extern __typeof (fileno) __fileno;
libc_hidden_proto (__fileno)
libc_hidden_proto (fwrite)
libc_hidden_proto (fseek)
extern __typeof (ftello) __ftello;
libc_hidden_proto (__ftello)
libc_hidden_proto (fflush)
libc_hidden_proto (fflush_unlocked)
extern __typeof (fflush_unlocked) __fflush_unlocked;
libc_hidden_proto (__fflush_unlocked)
extern __typeof (fread_unlocked) __fread_unlocked;
libc_hidden_proto (__fread_unlocked)
libc_hidden_proto (fwrite_unlocked)
libc_hidden_proto (fgets_unlocked)
extern __typeof (fgets_unlocked) __fgets_unlocked;
libc_hidden_proto (__fgets_unlocked)
libc_hidden_proto (fputs_unlocked)
extern __typeof (fputs_unlocked) __fputs_unlocked;
libc_hidden_proto (__fputs_unlocked)
libc_hidden_proto (fmemopen)
/* The prototype needs repeating instead of using __typeof to use
   __THROW in C++ tests.  */
extern FILE *__open_memstream (char **, size_t *) __THROW __wur;
libc_hidden_proto (__open_memstream)
libc_hidden_proto (__libc_fatal)
rtld_hidden_proto (__libc_fatal)
libc_hidden_proto (__vsprintf_chk)
libc_hidden_proto (__vsnprintf_chk)
libc_hidden_proto (__vfprintf_chk)
libc_hidden_proto (__vasprintf_chk)
libc_hidden_proto (__vdprintf_chk)
libc_hidden_proto (__obstack_vprintf_chk)

/* The <stdio.h> header does not include the declaration for gets
   anymore when compiling with _GNU_SOURCE.  Provide a copy here.  */
extern char *gets (char *__s);
#  if __USE_FORTIFY_LEVEL > 0
extern char *__gets_chk (char *__str, size_t) __wur;
extern char *__REDIRECT (__gets_warn, (char *__str), gets)
     __wur __warnattr ("please use fgets or getline instead, gets can't "
		       "specify buffer size");

__fortify_function __wur char *
gets (char *__str)
{
  if (__bos (__str) != (size_t) -1)
    return __gets_chk (__str, __bos (__str));
  return __gets_warn (__str);
}
#  endif

extern FILE * __fmemopen (void *buf, size_t len, const char *mode);
libc_hidden_proto (__fmemopen)

__END_DECLS
# endif

#endif
