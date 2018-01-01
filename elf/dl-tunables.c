/* The tunable framework.  See the README.tunables to know how to use the
   tunable in a glibc module.

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

#include <startup.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sysdep.h>
#include <fcntl.h>
#include <ldsodefs.h>

#define TUNABLES_INTERNAL 1
#include "dl-tunables.h"

#include <not-errno.h>

#if TUNABLES_FRONTEND == TUNABLES_FRONTEND_valstring
# define GLIBC_TUNABLES "GLIBC_TUNABLES"
#endif

#if TUNABLES_FRONTEND == TUNABLES_FRONTEND_valstring
static char *
tunables_strdup (const char *in)
{
  size_t i = 0;

  while (in[i++] != '\0');
  char *out = __sbrk (i);

  /* FIXME: In reality if the allocation fails, __sbrk will crash attempting to
     set the thread-local errno since the TCB has not yet been set up.  This
     needs to be fixed with an __sbrk implementation that does not set
     errno.  */
  if (out == (void *)-1)
    return NULL;

  i--;

  while (i-- > 0)
    out[i] = in[i];

  return out;
}
#endif

static char **
get_next_env (char **envp, char **name, size_t *namelen, char **val,
	      char ***prev_envp)
{
  while (envp != NULL && *envp != NULL)
    {
      char **prev = envp;
      char *envline = *envp++;
      int len = 0;

      while (envline[len] != '\0' && envline[len] != '=')
	len++;

      /* Just the name and no value, go to the next one.  */
      if (envline[len] == '\0')
	continue;

      *name = envline;
      *namelen = len;
      *val = &envline[len + 1];
      *prev_envp = prev;

      return envp;
    }

  return NULL;
}

#define TUNABLE_SET_VAL_IF_VALID_RANGE(__cur, __val, __type)		      \
({									      \
  __type min = (__cur)->type.min;					      \
  __type max = (__cur)->type.max;					      \
									      \
  if ((__type) (__val) >= min && (__type) (val) <= max)			      \
    {									      \
      (__cur)->val.numval = val;					      \
      (__cur)->initialized = true;					      \
    }									      \
})

static void
do_tunable_update_val (tunable_t *cur, const void *valp)
{
  uint64_t val;

  if (cur->type.type_code != TUNABLE_TYPE_STRING)
    val = *((int64_t *) valp);

  switch (cur->type.type_code)
    {
    case TUNABLE_TYPE_INT_32:
	{
	  TUNABLE_SET_VAL_IF_VALID_RANGE (cur, val, int64_t);
	  break;
	}
    case TUNABLE_TYPE_UINT_64:
	{
	  TUNABLE_SET_VAL_IF_VALID_RANGE (cur, val, uint64_t);
	  break;
	}
    case TUNABLE_TYPE_SIZE_T:
	{
	  TUNABLE_SET_VAL_IF_VALID_RANGE (cur, val, uint64_t);
	  break;
	}
    case TUNABLE_TYPE_STRING:
	{
	  cur->val.strval = valp;
	  break;
	}
    default:
      __builtin_unreachable ();
    }
}

/* Validate range of the input value and initialize the tunable CUR if it looks
   good.  */
static void
tunable_initialize (tunable_t *cur, const char *strval)
{
  uint64_t val;
  const void *valp;

  if (cur->type.type_code != TUNABLE_TYPE_STRING)
    {
      val = _dl_strtoul (strval, NULL);
      valp = &val;
    }
  else
    {
      cur->initialized = true;
      valp = strval;
    }
  do_tunable_update_val (cur, valp);
}

void
__tunable_set_val (tunable_id_t id, void *valp)
{
  tunable_t *cur = &tunable_list[id];

  do_tunable_update_val (cur, valp);
}

#if TUNABLES_FRONTEND == TUNABLES_FRONTEND_valstring
/* Parse the tunable string TUNESTR and adjust it to drop any tunables that may
   be unsafe for AT_SECURE processes so that it can be used as the new
   environment variable value for GLIBC_TUNABLES.  VALSTRING is the original
   environment variable string which we use to make NULL terminated values so
   that we don't have to allocate memory again for it.  */
static void
parse_tunables (char *tunestr, char *valstring)
{
  if (tunestr == NULL || *tunestr == '\0')
    return;

  char *p = tunestr;

  while (true)
    {
      char *name = p;
      size_t len = 0;

      /* First, find where the name ends.  */
      while (p[len] != '=' && p[len] != ':' && p[len] != '\0')
	len++;

      /* If we reach the end of the string before getting a valid name-value
	 pair, bail out.  */
      if (p[len] == '\0')
	return;

      /* We did not find a valid name-value pair before encountering the
	 colon.  */
      if (p[len]== ':')
	{
	  p += len + 1;
	  continue;
	}

      p += len + 1;

      /* Take the value from the valstring since we need to NULL terminate it.  */
      char *value = &valstring[p - tunestr];
      len = 0;

      while (p[len] != ':' && p[len] != '\0')
	len++;

      /* Add the tunable if it exists.  */
      for (size_t i = 0; i < sizeof (tunable_list) / sizeof (tunable_t); i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  if (tunable_is_name (cur->name, name))
	    {
	      /* If we are in a secure context (AT_SECURE) then ignore the tunable
		 unless it is explicitly marked as secure.  Tunable values take
		 precendence over their envvar aliases.  */
	      if (__libc_enable_secure)
		{
		  if (cur->security_level == TUNABLE_SECLEVEL_SXID_ERASE)
		    {
		      if (p[len] == '\0')
			{
			  /* Last tunable in the valstring.  Null-terminate and
			     return.  */
			  *name = '\0';
			  return;
			}
		      else
			{
			  /* Remove the current tunable from the string.  We do
			     this by overwriting the string starting from NAME
			     (which is where the current tunable begins) with
			     the remainder of the string.  We then have P point
			     to NAME so that we continue in the correct
			     position in the valstring.  */
			  char *q = &p[len + 1];
			  p = name;
			  while (*q != '\0')
			    *name++ = *q++;
			  name[0] = '\0';
			  len = 0;
			}
		    }

		  if (cur->security_level != TUNABLE_SECLEVEL_NONE)
		    break;
		}

	      value[len] = '\0';
	      tunable_initialize (cur, value);
	      break;
	    }
	}

      if (p[len] == '\0')
	return;
      else
	p += len + 1;
    }
}
#endif

/* Enable the glibc.malloc.check tunable in SETUID/SETGID programs only when
   the system administrator has created the /etc/suid-debug file.  This is a
   special case where we want to conditionally enable/disable a tunable even
   for setuid binaries.  We use the special version of access() to avoid
   setting ERRNO, which is a TLS variable since TLS has not yet been set
   up.  */
static inline void
__always_inline
maybe_enable_malloc_check (void)
{
  tunable_id_t id = TUNABLE_ENUM_NAME (glibc, malloc, check);
  if (__libc_enable_secure && __access_noerrno ("/etc/suid-debug", F_OK) == 0)
    tunable_list[id].security_level = TUNABLE_SECLEVEL_NONE;
}

/* Initialize the tunables list from the environment.  For now we only use the
   ENV_ALIAS to find values.  Later we will also use the tunable names to find
   values.  */
void
__tunables_init (char **envp)
{
  char *envname = NULL;
  char *envval = NULL;
  size_t len = 0;
  char **prev_envp = envp;

  maybe_enable_malloc_check ();

  while ((envp = get_next_env (envp, &envname, &len, &envval,
			       &prev_envp)) != NULL)
    {
#if TUNABLES_FRONTEND == TUNABLES_FRONTEND_valstring
      if (tunable_is_name (GLIBC_TUNABLES, envname))
	{
	  char *new_env = tunables_strdup (envname);
	  if (new_env != NULL)
	    parse_tunables (new_env + len + 1, envval);
	  /* Put in the updated envval.  */
	  *prev_envp = new_env;
	  continue;
	}
#endif

      for (int i = 0; i < sizeof (tunable_list) / sizeof (tunable_t); i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  /* Skip over tunables that have either been set already or should be
	     skipped.  */
	  if (cur->initialized || cur->env_alias == NULL)
	    continue;

	  const char *name = cur->env_alias;

	  /* We have a match.  Initialize and move on to the next line.  */
	  if (tunable_is_name (name, envname))
	    {
	      /* For AT_SECURE binaries, we need to check the security settings of
		 the tunable and decide whether we read the value and also whether
		 we erase the value so that child processes don't inherit them in
		 the environment.  */
	      if (__libc_enable_secure)
		{
		  if (cur->security_level == TUNABLE_SECLEVEL_SXID_ERASE)
		    {
		      /* Erase the environment variable.  */
		      char **ep = prev_envp;

		      while (*ep != NULL)
			{
			  if (tunable_is_name (name, *ep))
			    {
			      char **dp = ep;

			      do
				dp[0] = dp[1];
			      while (*dp++);
			    }
			  else
			    ++ep;
			}
		      /* Reset the iterator so that we read the environment again
			 from the point we erased.  */
		      envp = prev_envp;
		    }

		  if (cur->security_level != TUNABLE_SECLEVEL_NONE)
		    continue;
		}

	      tunable_initialize (cur, envval);
	      break;
	    }
	}
    }
}

/* Set the tunable value.  This is called by the module that the tunable exists
   in. */
void
__tunable_get_val (tunable_id_t id, void *valp, tunable_callback_t callback)
{
  tunable_t *cur = &tunable_list[id];

  switch (cur->type.type_code)
    {
    case TUNABLE_TYPE_UINT_64:
	{
	  *((uint64_t *) valp) = (uint64_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_INT_32:
	{
	  *((int32_t *) valp) = (int32_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_SIZE_T:
	{
	  *((size_t *) valp) = (size_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_STRING:
	{
	  *((const char **)valp) = cur->val.strval;
	  break;
	}
    default:
      __builtin_unreachable ();
    }

  if (cur->initialized && callback != NULL)
    callback (&cur->val);
}

rtld_hidden_def (__tunable_get_val)
