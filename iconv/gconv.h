/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _GCONV_H
#define _GCONV_H	1

#include <features.h>
#include <sys/types.h>
#include <regex.h>

__BEGIN_DECLS

/* Error codes for gconv functions.  */
enum
{
  GCONV_OK = 0,
  GCONV_NOCONV,
  GCONV_NODB,
  GCONV_NOMEM,

  GCONV_EMPTY_INPUT,
  GCONV_FULL_OUTPUT,
  GCONV_ILLEGAL_INPUT,
  GCONV_INCOMPLETE_INPUT,

  GCONV_ILLEGAL_DESCRIPTOR,
  GCONV_INTERNAL_ERROR
};


/* Structure for alias definition.  Simply to strings.  */
struct gconv_alias
{
  __const char *fromname;
  __const char *toname;
};


/* Default size of intermediate buffers.  */
#define GCONV_DEFAULT_BUFSIZE	8160


/* Forward declarations.  */
struct gconv_step;
struct gconv_step_data;


/* Type of a conversion function.  */
typedef int (*gconv_fct) __P ((struct gconv_step *,
			       struct gconv_step_data *,
			       __const char *, size_t *, size_t *, int));

/* Constructor and destructor for local data for conversion step.  */
typedef int (*gconv_init_fct) __P ((struct gconv_step *,
				    struct gconv_step_data *));
typedef void (*gconv_end_fct) __P ((struct gconv_step_data *));


/* Description of a conversion step.  */
struct gconv_step
{
  void *shlib_handle;

  __const char *from_name;
  __const char *to_name;

  gconv_fct fct;
  gconv_init_fct init_fct;
  gconv_end_fct end_fct;
};

/* Additional data for steps in use of conversion descriptor.  This is
   allocated by the `init' function.  */
struct gconv_step_data
{
  char *outbuf;		/* Output buffer for this step.  */
  size_t outbufavail;	/* Bytes already available in output buffer.  */
  size_t outbufsize;	/* Size of output buffer.  */

  int is_last;

  void *data;		/* Pointer to step-local data.  */
};

/* Combine conversion step description with data.  */
typedef struct gconv_info
{
  size_t nsteps;
  struct gconv_step *steps;
  struct gconv_step_data *data;
} *gconv_t;


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
			      gconv_t *__handle));

/* Free resources associated with transformation descriptor CD.  */
extern int __gconv_close __P ((gconv_t cd));

/* Transform at most *INBYTESLEFT bytes from buffer starting at *INBUF
   according to rules described by CD and place up to *OUTBYTESLEFT
   bytes in buffer starting at *OUTBUF.  Return number of written
   characters in *CONVERTED if this pointer is not null.  */
extern int __gconv __P ((gconv_t __cd,
			 __const char **__inbuf, size_t *__inbytesleft,
			 char **__outbuf, size_t *__outbytesleft,
			 size_t *__converted));

/* Return in *HANDLE a pointer to an array with *NSTEPS elements describing
   the single steps necessary for transformation from FROMSET to TOSET.  */
extern int __gconv_find_transform __P ((__const char *__toset,
					__const char *__fromset,
					struct gconv_step **__handle,
					size_t *__nsteps));

/* Read all the configuration data and cache it.  */
extern void __gconv_read_conf __P ((void));

/* Comparison function to search alias.  */
extern int __gconv_alias_compare __P ((__const void *__p1,
				       __const void *__p2));

/* Clear reference to transformation step implementations which might
   cause the code to be unloaded.  */
extern int __gconv_close_transform __P ((struct gconv_step *__steps,
					 size_t __nsteps));


/* Find in the shared object associated with HANDLE for a function with
   name NAME.  Return function pointer or NULL.  */
extern void *__gconv_find_func __P ((void *__handle, __const char *__name));

/* Load shared object named by NAME.  If already loaded increment reference
   count.  */
extern void *__gconv_find_shlib __P ((__const char *__name));

/* Release shared object.  If no further reference is available unload
   the object.  */
extern int __gconv_release_shlib __P ((void *__handle));

/* Fill STEP with information about builtin module with NAME.  */
extern void __gconv_get_builtin_trans __P ((__const char *__name,
					    struct gconv_step *__step));



/* Builtin transformations.  */
#ifdef _LIBC
# define __BUILTIN_TRANS(Name) \
  extern int Name __P ((struct gconv_step *__step,			      \
			struct gconv_step_data *__data, __const char *__inbuf,\
			size_t *__inlen, size_t *__written, int __do_flush))

__BUILTIN_TRANS (__gconv_transform_dummy);
__BUILTIN_TRANS (__gconv_transform_ucs4_utf8);
__BUILTIN_TRANS (__gconv_transform_utf8_ucs4);
# undef __BUITLIN_TRANS

extern int __gconv_transform_init_rstate __P ((struct gconv_step *__step,
					      struct gconv_step_data *__data));
extern void __gconv_transform_end_rstate __P ((struct gconv_step_data *__data));

#endif

__END_DECLS

#endif /* gconv.h */
