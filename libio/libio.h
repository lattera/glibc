/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Per Bothner <bothner@cygnus.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#ifndef _LIBIO_H
#define _LIBIO_H 1

#ifndef _LIBC
# error "libio.h should only be included when building glibc itself"
#endif
#ifdef _ISOMAC
# error "libio.h should not be included under _ISOMAC"
#endif

#include <stdio.h>

#if defined _IO_MTSAFE_IO && !defined _IO_lock_t_defined
# error "Someone forgot to include stdio-lock.h"
#endif

#define __need_wchar_t
#include <stddef.h>

#include <bits/types/__mbstate_t.h>
#include <bits/types/wint_t.h>
#include <gconv.h>

typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _IO_iconv_t;

/* Map the names used in libio to the names used in libc generally.  */
#define _IO_fpos_t __fpos_t
#define _IO_fpos64_t __fpos64_t
#define _IO_size_t size_t
#define _IO_ssize_t __ssize_t
#define _IO_off_t __off_t
#define _IO_off64_t __off64_t
#define _IO_pid_t __pid_t
#define _IO_uid_t __uid_t
#define _IO_BUFSIZ BUFSIZ
#define _IO_wint_t wint_t
#define _IO_va_list __gnuc_va_list

#include <shlib-compat.h>

/* compatibility defines */
#define _STDIO_USES_IOSTREAM
#define _IO_UNIFIED_JUMPTABLES 1
#define __HAVE_COLUMN
#define _IO_file_flags _flags

/* open modes */
#define _IOS_INPUT	1
#define _IOS_OUTPUT	2
#define _IOS_ATEND	4
#define _IOS_APPEND	8
#define _IOS_TRUNC	16
#define _IOS_NOCREATE	32
#define _IOS_NOREPLACE	64
#define _IOS_BIN	128

/* Magic numbers and bits for the _flags field.
   The magic numbers use the high-order bits of _flags;
   the remaining bits are available for variable flags.
   Note: The magic numbers must all be negative if stdio
   emulation is desired. */

#define _IO_MAGIC 0xFBAD0000 /* Magic number */
#define _OLD_STDIO_MAGIC 0xFABC0000 /* Emulate old stdio. */
#define _IO_MAGIC_MASK 0xFFFF0000
#define _IO_USER_BUF 1 /* User owns buffer; don't delete it on close. */
#define _IO_UNBUFFERED 2
#define _IO_NO_READS 4 /* Reading not allowed */
#define _IO_NO_WRITES 8 /* Writing not allowd */
#define _IO_EOF_SEEN 0x10
#define _IO_ERR_SEEN 0x20
#define _IO_DELETE_DONT_CLOSE 0x40 /* Don't call close(_fileno) on cleanup. */
#define _IO_LINKED 0x80 /* Set if linked (using _chain) to streambuf::_list_all.*/
#define _IO_IN_BACKUP 0x100
#define _IO_LINE_BUF 0x200
#define _IO_TIED_PUT_GET 0x400 /* Set if put and get pointer logicly tied. */
#define _IO_CURRENTLY_PUTTING 0x800
#define _IO_IS_APPENDING 0x1000
#define _IO_IS_FILEBUF 0x2000
#define _IO_BAD_SEEN 0x4000
#define _IO_USER_LOCK 0x8000

#define _IO_FLAGS2_MMAP 1
#define _IO_FLAGS2_NOTCANCEL 2
#define _IO_FLAGS2_FORTIFY 4
#define _IO_FLAGS2_USER_WBUF 8
#define _IO_FLAGS2_SCANF_STD 16
#define _IO_FLAGS2_NOCLOSE 32
#define _IO_FLAGS2_CLOEXEC 64
#define _IO_FLAGS2_NEED_LOCK 128

/* These are "formatting flags" matching the iostream fmtflags enum values. */
#define _IO_SKIPWS 01
#define _IO_LEFT 02
#define _IO_RIGHT 04
#define _IO_INTERNAL 010
#define _IO_DEC 020
#define _IO_OCT 040
#define _IO_HEX 0100
#define _IO_SHOWBASE 0200
#define _IO_SHOWPOINT 0400
#define _IO_UPPERCASE 01000
#define _IO_SHOWPOS 02000
#define _IO_SCIENTIFIC 04000
#define _IO_FIXED 010000
#define _IO_UNITBUF 020000
#define _IO_STDIO 040000
#define _IO_DONT_CLOSE 0100000
#define _IO_BOOLALPHA 0200000


struct _IO_jump_t;

/* A streammarker remembers a position in a buffer. */

struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;
  /* If _pos >= 0
 it points to _buf->Gbase()+_pos. FIXME comment */
  /* if _pos < 0, it points to _buf->eBptr()+_pos. FIXME comment */
  int _pos;
};

/* This is the structure from the libstdc++ codecvt class.  */
enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};

/* The order of the elements in the following struct must match the order
   of the virtual functions in the libstdc++ codecvt class.  */
struct _IO_codecvt
{
  void (*__codecvt_destr) (struct _IO_codecvt *);
  enum __codecvt_result (*__codecvt_do_out) (struct _IO_codecvt *,
					     __mbstate_t *,
					     const wchar_t *,
					     const wchar_t *,
					     const wchar_t **, char *,
					     char *, char **);
  enum __codecvt_result (*__codecvt_do_unshift) (struct _IO_codecvt *,
						 __mbstate_t *, char *,
						 char *, char **);
  enum __codecvt_result (*__codecvt_do_in) (struct _IO_codecvt *,
					    __mbstate_t *,
					    const char *, const char *,
					    const char **, wchar_t *,
					    wchar_t *, wchar_t **);
  int (*__codecvt_do_encoding) (struct _IO_codecvt *);
  int (*__codecvt_do_always_noconv) (struct _IO_codecvt *);
  int (*__codecvt_do_length) (struct _IO_codecvt *, __mbstate_t *,
			      const char *, const char *, _IO_size_t);
  int (*__codecvt_do_max_length) (struct _IO_codecvt *);

  _IO_iconv_t __cd_in;
  _IO_iconv_t __cd_out;
};

/* Extra data for wide character streams.  */
struct _IO_wide_data
{
  wchar_t *_IO_read_ptr;	/* Current read pointer */
  wchar_t *_IO_read_end;	/* End of get area. */
  wchar_t *_IO_read_base;	/* Start of putback+get area. */
  wchar_t *_IO_write_base;	/* Start of put area. */
  wchar_t *_IO_write_ptr;	/* Current put pointer. */
  wchar_t *_IO_write_end;	/* End of put area. */
  wchar_t *_IO_buf_base;	/* Start of reserve area. */
  wchar_t *_IO_buf_end;		/* End of reserve area. */
  /* The following fields are used to support backing up and undo. */
  wchar_t *_IO_save_base;	/* Pointer to start of non-current get area. */
  wchar_t *_IO_backup_base;	/* Pointer to first valid character of
				   backup area */
  wchar_t *_IO_save_end;	/* Pointer to end of non-current get area. */

  __mbstate_t _IO_state;
  __mbstate_t _IO_last_state;
  struct _IO_codecvt _codecvt;

  wchar_t _shortbuf[1];

  const struct _IO_jump_t *_wide_vtable;
};

typedef struct _IO_FILE _IO_FILE;

struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
extern _IO_FILE *_IO_stdin attribute_hidden;
extern _IO_FILE *_IO_stdout attribute_hidden;
extern _IO_FILE *_IO_stderr attribute_hidden;

/* Compatibility names for cookie I/O functions.  */
typedef cookie_read_function_t __io_read_fn;
typedef cookie_write_function_t __io_write_fn;
typedef cookie_seek_function_t __io_seek_fn;
typedef cookie_close_function_t __io_close_fn;
typedef cookie_io_functions_t _IO_cookie_io_functions_t;

struct _IO_cookie_file;

/* Initialize one of those.  */
extern void _IO_cookie_init (struct _IO_cookie_file *__cfile, int __read_write,
			     void *__cookie, _IO_cookie_io_functions_t __fns);

extern int __underflow (_IO_FILE *);
extern _IO_wint_t __wunderflow (_IO_FILE *);
extern _IO_wint_t __wuflow (_IO_FILE *);
extern _IO_wint_t __woverflow (_IO_FILE *, _IO_wint_t);

#if  __GNUC__ >= 3
# define _IO_BE(expr, res) __builtin_expect ((expr), res)
#else
# define _IO_BE(expr, res) (expr)
#endif

#define _IO_getc_unlocked(_fp) __getc_unlocked_body (_fp)
#define _IO_peekc_unlocked(_fp) \
       (_IO_BE ((_fp)->_IO_read_ptr >= (_fp)->_IO_read_end, 0) \
	  && __underflow (_fp) == EOF ? EOF \
	: *(unsigned char *) (_fp)->_IO_read_ptr)
#define _IO_putc_unlocked(_ch, _fp) __putc_unlocked_body (_ch, _fp)

# define _IO_getwc_unlocked(_fp) \
  (_IO_BE ((_fp)->_wide_data == NULL					\
	   || ((_fp)->_wide_data->_IO_read_ptr				\
	       >= (_fp)->_wide_data->_IO_read_end), 0)			\
   ? __wuflow (_fp) : (_IO_wint_t) *(_fp)->_wide_data->_IO_read_ptr++)
# define _IO_putwc_unlocked(_wch, _fp) \
  (_IO_BE ((_fp)->_wide_data == NULL					\
	   || ((_fp)->_wide_data->_IO_write_ptr				\
	       >= (_fp)->_wide_data->_IO_write_end), 0)			\
   ? __woverflow (_fp, _wch)						\
   : (_IO_wint_t) (*(_fp)->_wide_data->_IO_write_ptr++ = (_wch)))

#define _IO_feof_unlocked(_fp) __feof_unlocked_body (_fp)
#define _IO_ferror_unlocked(_fp) __ferror_unlocked_body (_fp)

extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __THROW;
extern int _IO_ferror (_IO_FILE *__fp) __THROW;

extern int _IO_peekc_locked (_IO_FILE *__fp);

/* This one is for Emacs. */
#define _IO_PENDING_OUTPUT_COUNT(_fp)	\
	((_fp)->_IO_write_ptr - (_fp)->_IO_write_base)

extern void _IO_flockfile (_IO_FILE *) __THROW;
extern void _IO_funlockfile (_IO_FILE *) __THROW;
extern int _IO_ftrylockfile (_IO_FILE *) __THROW;

#define _IO_peekc(_fp) _IO_peekc_unlocked (_fp)
#define _IO_flockfile(_fp) /**/
#define _IO_funlockfile(_fp) /**/
#define _IO_ftrylockfile(_fp) /**/
#ifndef _IO_cleanup_region_start
#define _IO_cleanup_region_start(_fct, _fp) /**/
#endif
#ifndef _IO_cleanup_region_end
#define _IO_cleanup_region_end(_Doit) /**/
#endif

#define _IO_need_lock(_fp) \
  (((_fp)->_flags2 & _IO_FLAGS2_NEED_LOCK) != 0)

extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
			_IO_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
			 _IO_va_list);
extern _IO_ssize_t _IO_padn (_IO_FILE *, int, _IO_ssize_t);
extern _IO_size_t _IO_sgetn (_IO_FILE *, void *, _IO_size_t);

extern _IO_off64_t _IO_seekoff (_IO_FILE *, _IO_off64_t, int, int);
extern _IO_off64_t _IO_seekpos (_IO_FILE *, _IO_off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __THROW;


extern _IO_wint_t _IO_getwc (_IO_FILE *__fp);
extern _IO_wint_t _IO_putwc (wchar_t __wc, _IO_FILE *__fp);
extern int _IO_fwide (_IO_FILE *__fp, int __mode) __THROW;

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
#  define _IO_fwide_maybe_incompatible \
  (__builtin_expect (&_IO_stdin_used == NULL, 0))
extern const int _IO_stdin_used;
weak_extern (_IO_stdin_used);
#else
# define _IO_fwide_maybe_incompatible (0)
#endif

/* A special optimized version of the function above.  It optimizes the
   case of initializing an unoriented byte stream.  */
#define _IO_fwide(__fp, __mode) \
  ({ int __result = (__mode);						      \
     if (__result < 0 && ! _IO_fwide_maybe_incompatible)		      \
       {								      \
	 if ((__fp)->_mode == 0)					      \
	   /* We know that all we have to do is to set the flag.  */	      \
	   (__fp)->_mode = -1;						      \
	 __result = (__fp)->_mode;					      \
       }								      \
     else if (__builtin_constant_p (__mode) && (__mode) == 0)		      \
       __result = _IO_fwide_maybe_incompatible ? -1 : (__fp)->_mode;	      \
     else								      \
       __result = _IO_fwide (__fp, __result);				      \
     __result; })

extern int _IO_vfwscanf (_IO_FILE * __restrict, const wchar_t * __restrict,
			 _IO_va_list, int *__restrict);
extern int _IO_vfwprintf (_IO_FILE *__restrict, const wchar_t *__restrict,
			  _IO_va_list);
extern _IO_ssize_t _IO_wpadn (_IO_FILE *, wint_t, _IO_ssize_t);
extern void _IO_free_wbackup_area (_IO_FILE *) __THROW;

#ifdef __LDBL_COMPAT
__LDBL_REDIR_DECL (_IO_vfscanf)
__LDBL_REDIR_DECL (_IO_vfprintf)
#endif

libc_hidden_proto (__overflow)
libc_hidden_proto (__underflow)
libc_hidden_proto (__uflow)
libc_hidden_proto (__woverflow)
libc_hidden_proto (__wunderflow)
libc_hidden_proto (__wuflow)
libc_hidden_proto (_IO_free_backup_area)
libc_hidden_proto (_IO_free_wbackup_area)
libc_hidden_proto (_IO_padn)
libc_hidden_proto (_IO_putc)
libc_hidden_proto (_IO_sgetn)
libc_hidden_proto (_IO_vfprintf)
libc_hidden_proto (_IO_vfscanf)

#ifdef _IO_MTSAFE_IO
# undef _IO_peekc
# undef _IO_flockfile
# undef _IO_funlockfile
# undef _IO_ftrylockfile

# define _IO_peekc(_fp) _IO_peekc_locked (_fp)
# if _IO_lock_inexpensive
#  define _IO_flockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_lock_lock (*(_fp)->_lock)
#  define _IO_funlockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_lock_unlock (*(_fp)->_lock)
# else
#  define _IO_flockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_flockfile (_fp)
#  define _IO_funlockfile(_fp) \
  if (((_fp)->_flags & _IO_USER_LOCK) == 0) _IO_funlockfile (_fp)
# endif
#endif /* _IO_MTSAFE_IO */

#endif /* _LIBIO_H */
