/* The tunable framework.  See the README to know how to use the tunable in
   a glibc module.

   Copyright (C) 2016-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _TUNABLES_H_
#define _TUNABLES_H_

#if !HAVE_TUNABLES
static inline void
__always_inline
__tunables_init (char **unused __attribute__ ((unused)))
{
  /* This is optimized out if tunables are not enabled.  */
}
#else

# include <stddef.h>
# include "dl-tunable-types.h"

/* A tunable.  */
struct _tunable
{
  const char *name;			/* Internal name of the tunable.  */
  tunable_type_t type;			/* Data type of the tunable.  */
  tunable_val_t val;			/* The value.  */
  const char *strval;			/* The string containing the value,
					   points into envp.  */
  tunable_seclevel_t security_level;	/* Specify the security level for the
					   tunable with respect to AT_SECURE
					   programs.  See description of
					   tunable_seclevel_t to see a
					   description of the values.

					   Note that even if the tunable is
					   read, it may not get used by the
					   target module if the value is
					   considered unsafe.  */
  /* Compatibility elements.  */
  const char *env_alias;		/* The compatibility environment
					   variable name.  */
};

typedef struct _tunable tunable_t;

/* Full name for a tunable is top_ns.tunable_ns.id.  */
# define TUNABLE_NAME_S(top,ns,id) #top "." #ns "." #id

# define TUNABLE_ENUM_NAME(top,ns,id) TUNABLE_ENUM_NAME1 (top,ns,id)
# define TUNABLE_ENUM_NAME1(top,ns,id) top ## _ ## ns ## _ ## id

# include "dl-tunable-list.h"

extern void __tunables_init (char **);
extern void __tunable_set_val (tunable_id_t, void *, tunable_callback_t);

/* Check if the tunable has been set to a non-default value and if it is, copy
   it over into __VAL.  */
# define TUNABLE_SET_VAL(__id,__val) \
({									      \
  __tunable_set_val							      \
   (TUNABLE_ENUM_NAME (TOP_NAMESPACE, TUNABLE_NAMESPACE, __id), (__val),      \
    NULL);								      \
})

/* Same as TUNABLE_SET_VAL, but also call the callback function __CB.  */
# define TUNABLE_SET_VAL_WITH_CALLBACK(__id,__val,__cb) \
({									      \
  __tunable_set_val							      \
   (TUNABLE_ENUM_NAME (TOP_NAMESPACE, TUNABLE_NAMESPACE, __id), (__val),      \
    DL_TUNABLE_CALLBACK (__cb));					      \
})

/* Namespace sanity for callback functions.  Use this macro to keep the
   namespace of the modules clean.  */
# define DL_TUNABLE_CALLBACK(__name) _dl_tunable_ ## __name

# define TUNABLES_FRONTEND_valstring 1
/* The default value for TUNABLES_FRONTEND.  */
# define TUNABLES_FRONTEND_yes TUNABLES_FRONTEND_valstring
#endif
#endif
