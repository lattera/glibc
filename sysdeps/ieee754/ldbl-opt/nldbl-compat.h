/* Prototypes for compatibility double == long double entry points.
   Copyright (C) 2006-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@cygnus.com>, 2006.

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
   <http://www.gnu.org/licenses/>.  */

#ifndef __NLDBL_COMPAT_H
#define __NLDBL_COMPAT_H	1

/* Avoid long double prototypes.  */
#define __NO_LONG_DOUBLE_MATH	1
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <printf.h>
#include <wchar.h>
#include <math.h>
#include <monetary.h>
#include <sys/syslog.h>


/* Declare the __nldbl_NAME function the wrappers call that's in libc.so.  */
#define NLDBL_DECL(name) extern __typeof (name) __nldbl_##name

NLDBL_DECL (_IO_vfscanf);
NLDBL_DECL (vfscanf);
NLDBL_DECL (vfwscanf);
NLDBL_DECL (obstack_vprintf);
NLDBL_DECL (vasprintf);
NLDBL_DECL (dprintf);
NLDBL_DECL (vdprintf);
NLDBL_DECL (fprintf);
NLDBL_DECL (vfprintf);
NLDBL_DECL (vfwprintf);
NLDBL_DECL (vsnprintf);
NLDBL_DECL (vsprintf);
NLDBL_DECL (vsscanf);
NLDBL_DECL (vswprintf);
NLDBL_DECL (vswscanf);
NLDBL_DECL (__asprintf);
NLDBL_DECL (asprintf);
NLDBL_DECL (__printf_fp);
NLDBL_DECL (printf_size);
NLDBL_DECL (syslog);
NLDBL_DECL (vsyslog);
NLDBL_DECL (qecvt);
NLDBL_DECL (qfcvt);
NLDBL_DECL (qgcvt);
NLDBL_DECL (__vstrfmon_l);
NLDBL_DECL (__isoc99_scanf);
NLDBL_DECL (__isoc99_fscanf);
NLDBL_DECL (__isoc99_sscanf);
NLDBL_DECL (__isoc99_vscanf);
NLDBL_DECL (__isoc99_vfscanf);
NLDBL_DECL (__isoc99_vsscanf);
NLDBL_DECL (__isoc99_wscanf);
NLDBL_DECL (__isoc99_fwscanf);
NLDBL_DECL (__isoc99_swscanf);
NLDBL_DECL (__isoc99_vwscanf);
NLDBL_DECL (__isoc99_vfwscanf);
NLDBL_DECL (__isoc99_vswscanf);

/* This one does not exist in the normal interface, only
   __nldbl___vstrfmon really exists.  */
extern ssize_t __nldbl___vstrfmon (char *, size_t, const char *, va_list)
  __THROW;

/* These don't use __typeof because they were not declared by the headers,
   since we don't compile with _FORTIFY_SOURCE.  */
extern int __nldbl___vfprintf_chk (FILE *__restrict, int,
				   const char *__restrict, __gnuc_va_list);
extern int __nldbl___vfwprintf_chk (FILE *__restrict, int,
				    const wchar_t *__restrict, __gnuc_va_list);
extern int __nldbl___vsprintf_chk (char *__restrict, int, size_t,
				   const char *__restrict, __gnuc_va_list)
  __THROW;
extern int __nldbl___vsnprintf_chk (char *__restrict, size_t, int, size_t,
				    const char *__restrict, __gnuc_va_list)
  __THROW;
extern int __nldbl___vswprintf_chk (wchar_t *__restrict, size_t, int, size_t,
				    const wchar_t *__restrict, __gnuc_va_list)
  __THROW;
extern int __nldbl___vasprintf_chk (char **, int, const char *, __gnuc_va_list)
  __THROW;
extern int __nldbl___vdprintf_chk (int, int, const char *, __gnuc_va_list);
extern int __nldbl___obstack_vprintf_chk (struct obstack *, int, const char *,
					  __gnuc_va_list) __THROW;
extern void __nldbl___vsyslog_chk (int, int, const char *, va_list);

/* The original declarations of these were hidden by the including
   file.  */
extern double __nldbl_daddl (double, double) __THROW;
extern double __nldbl_ddivl (double, double) __THROW;
extern double __nldbl_dmull (double, double) __THROW;
extern double __nldbl_dsubl (double, double) __THROW;

#endif /* __NLDBL_COMPAT_H */
