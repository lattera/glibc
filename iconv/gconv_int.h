/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#ifndef _GCONV_INT_H
#define _GCONV_INT_H	1

#include "gconv.h"
#include <regex.h>

__BEGIN_DECLS


/* Structure for alias definition.  Simply to strings.  */
struct gconv_alias
{
  __const char *fromname;
  __const char *toname;
};


/* Default size of intermediate buffers.  */
#define GCONV_DEFAULT_BUFSIZE	8160


/* Description for an available conversion module.  */
struct gconv_module
{
  __const char *from_pattern;
  __const char *from_constpfx;
  size_t from_constpfx_len;
  __const regex_t *from_regex;

  __const char *to_string;

  int cost;

  __const char *module_name;
};


/* Global variables.  */

/* Database of alias names.  */
extern void *__gconv_alias_db;

/* Array with available modules.  */
extern size_t __gconv_nmodules;
extern struct gconv_module **__gconv_modules_db;


/* Return in *HANDLE decriptor for transformation from FROMSET to TOSET.  */
extern int __gconv_open __P ((__const char *__toset, __const char *__fromset,
			      gconv_t *__handle))
     internal_function;

/* Free resources associated with transformation descriptor CD.  */
extern int __gconv_close __P ((gconv_t cd))
     internal_function;

/* Transform at most *INBYTESLEFT bytes from buffer starting at *INBUF
   according to rules described by CD and place up to *OUTBYTESLEFT
   bytes in buffer starting at *OUTBUF.  Return number of written
   characters in *CONVERTED if this pointer is not null.  */
extern int __gconv __P ((gconv_t __cd,
			 __const char **__inbuf, size_t *__inbytesleft,
			 char **__outbuf, size_t *__outbytesleft,
			 size_t *__converted))
     internal_function;

/* Return in *HANDLE a pointer to an array with *NSTEPS elements describing
   the single steps necessary for transformation from FROMSET to TOSET.  */
extern int __gconv_find_transform __P ((__const char *__toset,
					__const char *__fromset,
					struct gconv_step **__handle,
					size_t *__nsteps))
     internal_function;

/* Read all the configuration data and cache it.  */
extern void __gconv_read_conf __P ((void))
     internal_function;

/* Comparison function to search alias.  */
extern int __gconv_alias_compare __P ((__const void *__p1,
				       __const void *__p2));

/* Clear reference to transformation step implementations which might
   cause the code to be unloaded.  */
extern int __gconv_close_transform __P ((struct gconv_step *__steps,
					 size_t __nsteps))
     internal_function;


/* Find in the shared object associated with HANDLE for a function with
   name NAME.  Return function pointer or NULL.  */
extern void *__gconv_find_func __P ((void *__handle, __const char *__name))
     internal_function;

/* Load shared object named by NAME.  If already loaded increment reference
   count.  */
extern void *__gconv_find_shlib __P ((__const char *__name))
     internal_function;

/* Release shared object.  If no further reference is available unload
   the object.  */
extern int __gconv_release_shlib __P ((void *__handle))
     internal_function;

/* Fill STEP with information about builtin module with NAME.  */
extern void __gconv_get_builtin_trans __P ((__const char *__name,
					    struct gconv_step *__step))
     internal_function;



/* Builtin transformations.  */
#ifdef _LIBC
# define __BUILTIN_TRANS(Name) \
  extern int Name __P ((struct gconv_step *__step,			      \
			struct gconv_step_data *__data, __const char *__inbuf,\
			size_t *__inlen, size_t *__written, int __do_flush))

__BUILTIN_TRANS (__gconv_transform_dummy);
__BUILTIN_TRANS (__gconv_transform_ucs4_utf8);
__BUILTIN_TRANS (__gconv_transform_utf8_ucs4);
__BUILTIN_TRANS (__gconv_transform_ucs2_ucs4);
__BUILTIN_TRANS (__gconv_transform_ucs4_ucs2);
# undef __BUITLIN_TRANS

extern int __gconv_transform_init_rstate __P ((struct gconv_step *__step,
					      struct gconv_step_data *__data));
extern void __gconv_transform_end_rstate __P ((struct gconv_step_data *__data));

#endif

__END_DECLS

#endif /* gconv_int.h */
