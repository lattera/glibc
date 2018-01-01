/* The tunable framework.  See the README to know how to use the tunable in
   a glibc module.

   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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
  bool initialized;			/* Flag to indicate that the tunable is
					   initialized.  */
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

# define TUNABLE_ENUM_NAME(__top,__ns,__id) TUNABLE_ENUM_NAME1 (__top,__ns,__id)
# define TUNABLE_ENUM_NAME1(__top,__ns,__id) __top ## _ ## __ns ## _ ## __id

# include "dl-tunable-list.h"

extern void __tunables_init (char **);
extern void __tunable_get_val (tunable_id_t, void *, tunable_callback_t);
extern void __tunable_set_val (tunable_id_t, void *);
rtld_hidden_proto (__tunables_init)
rtld_hidden_proto (__tunable_get_val)

/* Define TUNABLE_GET and TUNABLE_SET in short form if TOP_NAMESPACE and
   TUNABLE_NAMESPACE are defined.  This is useful shorthand to get and set
   tunables within a module.  */
#if defined TOP_NAMESPACE && defined TUNABLE_NAMESPACE
# define TUNABLE_GET(__id, __type, __cb) \
  TUNABLE_GET_FULL (TOP_NAMESPACE, TUNABLE_NAMESPACE, __id, __type, __cb)
# define TUNABLE_SET(__id, __type, __val) \
  TUNABLE_SET_FULL (TOP_NAMESPACE, TUNABLE_NAMESPACE, __id, __type, __val)
#else
# define TUNABLE_GET(__top, __ns, __id, __type, __cb) \
  TUNABLE_GET_FULL (__top, __ns, __id, __type, __cb)
# define TUNABLE_SET(__top, __ns, __id, __type, __val) \
  TUNABLE_SET_FULL (__top, __ns, __id, __type, __val)
#endif

/* Get and return a tunable value.  If the tunable was set externally and __CB
   is defined then call __CB before returning the value.  */
# define TUNABLE_GET_FULL(__top, __ns, __id, __type, __cb) \
({									      \
  tunable_id_t id = TUNABLE_ENUM_NAME (__top, __ns, __id);		      \
  __type ret;								      \
  __tunable_get_val (id, &ret, __cb);					      \
  ret;									      \
})

/* Set a tunable value.  */
# define TUNABLE_SET_FULL(__top, __ns, __id, __type, __val) \
({									      \
  __tunable_set_val (TUNABLE_ENUM_NAME (__top, __ns, __id),		      \
			& (__type) {__val});				      \
})

/* Namespace sanity for callback functions.  Use this macro to keep the
   namespace of the modules clean.  */
# define TUNABLE_CALLBACK(__name) _dl_tunable_ ## __name

# define TUNABLES_FRONTEND_valstring 1
/* The default value for TUNABLES_FRONTEND.  */
# define TUNABLES_FRONTEND_yes TUNABLES_FRONTEND_valstring

/* Compare two name strings, bounded by the name hardcoded in glibc.  */
static inline bool
__always_inline
tunable_is_name (const char *orig, const char *envname)
{
  for (;*orig != '\0' && *envname != '\0'; envname++, orig++)
    if (*orig != *envname)
      break;

  /* The ENVNAME is immediately followed by a value.  */
  if (*orig == '\0' && *envname == '=')
    return true;
  else
    return false;
}

#endif
#endif
