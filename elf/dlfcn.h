/* User functions for run-time dynamic loading.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef	_DLFCN_H
#define	_DLFCN_H 1

#include <features.h>
#define __need_NULL
#include <stddef.h>

/* Collect various system dependand definitions and declarations.  */
#include <bits/dlfcn.h>

/* If the first argument of `dlsym' or `dlvsym' is set to RTLD_NEXT
   the run-time address of the symbol called NAME in the next shared
   object is returned.  The "next" relation is defined by the order
   the shared objects were loaded.  */
#define RTLD_NEXT	((void *) -1l)

/* If the first argument to `dlsym' or `dlvsym' is set to RTLD_DEFAULT
   the run-time address of the symbol called NAME in the global scope
   is returned.  */
#define RTLD_DEFAULT	NULL

__BEGIN_DECLS

/* Open the shared object FILE and map it in; return a handle that can be
   passed to `dlsym' to get symbol values from it.  */
extern void *dlopen __P ((__const char *__file, int __mode));

/* Unmap and close a shared object opened by `dlopen'.
   The handle cannot be used again after calling `dlclose'.  */
extern int dlclose __P ((void *__handle));

/* Find the run-time address in the shared object HANDLE refers to
   of the symbol called NAME.  */
extern void *dlsym __P ((void *__handle, __const char *__name));

#ifdef __USE_GNU
/* Find the run-time address in the shared object HANDLE refers to
   of the symbol called NAME with VERSION.  */
extern void *dlvsym __P ((void *__handle, __const char *__name,
			  __const char *__version));
#endif

/* When any of the above functions fails, call this function
   to return a string describing the error.  Each call resets
   the error string so that a following call returns null.  */
extern char *dlerror __P ((void));

#ifdef __USE_GNU
/* Fill in *INFO with the following information about ADDRESS.
   Returns 0 iff no shared object's segments contain that address.  */
typedef struct
  {
    __const char *dli_fname;	/* File name of defining object.  */
    void *dli_fbase;		/* Load address of that object.  */
    __const char *dli_sname;	/* Name of nearest symbol.  */
    void *dli_saddr;		/* Exact value of nearest symbol.  */
  } Dl_info;
extern int dladdr __P ((const void *__address, Dl_info *__info));

/* To support profiling of shared objects it is a good idea to call
   the function found using `dlsym' using the following macro since
   these calls do not use the PLT.  But this would mean the dynamic
   loader has no chance to find out when the function is called.  The
   macro applies the necessary magic so that profiling is possible.
   Rewrite
	foo = (*fctp) (arg1, arg2);
   into
        foo = DL_CALL_FCT (fctp, (arg1, arg2));
*/
# define DL_CALL_FCT(fctp, args) \
  (_dl_mcount_wrapper_check (fctp), (*(fctp)) args)

/* This function calls the profiling functions.  */
extern void _dl_mcount_wrapper_check __P ((void *__selfpc));
#endif

__END_DECLS

#endif	/* dlfcn.h */
