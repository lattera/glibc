/* dlinfo -- Get information from the dynamic linker.
   Copyright (C) 2003, 2004, 2006, 2007, 2008 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <dlfcn.h>
#include <link.h>
#include <ldsodefs.h>
#include <libintl.h>

#if !defined SHARED && defined IS_IN_libdl

int
dlinfo (void *handle, int request, void *arg)
{
  return __dlinfo (handle, request, arg, RETURN_ADDRESS (0));
}

#else

# include <dl-tls.h>

struct dlinfo_args
{
  ElfW(Addr) caller;
  void *handle;
  int request;
  void *arg;
};

static void
dlinfo_doit (void *argsblock)
{
  struct dlinfo_args *const args = argsblock;
  struct link_map *l = args->handle;

# if 0
  if (args->handle == RTLD_SELF)
    {
      Lmid_t nsid;

      /* Find the highest-addressed object that CALLER is not below.  */
      for (nsid = 0; nsid < DL_NNS; ++nsid)
	for (l = GL(dl_ns)[nsid]._ns_loaded; l != NULL; l = l->l_next)
	  if (caller >= l->l_map_start && caller < l->l_map_end
	      && (l->l_contiguous || _dl_addr_inside_object (l, caller)))
	    break;

      if (l == NULL)
	GLRO(dl_signal_error) (0, NULL, NULL, N_("\
RTLD_SELF used in code not dynamically loaded"));
    }
# endif

  switch (args->request)
    {
    case RTLD_DI_CONFIGADDR:
    default:
      GLRO(dl_signal_error) (0, NULL, NULL, N_("unsupported dlinfo request"));
      break;

    case RTLD_DI_LMID:
      *(Lmid_t *) args->arg = l->l_ns;
      break;

    case RTLD_DI_LINKMAP:
      *(struct link_map **) args->arg = l;
      break;

    case RTLD_DI_SERINFO:
      _dl_rtld_di_serinfo (l, args->arg, false);
      break;
    case RTLD_DI_SERINFOSIZE:
      _dl_rtld_di_serinfo (l, args->arg, true);
      break;

    case RTLD_DI_ORIGIN:
      strcpy (args->arg, l->l_origin);
      break;

    case RTLD_DI_TLS_MODID:
      *(size_t *) args->arg = 0;
      *(size_t *) args->arg = l->l_tls_modid;
      break;

    case RTLD_DI_TLS_DATA:
      {
	void *data = NULL;
	if (l->l_tls_modid != 0)
	  data = GLRO(dl_tls_get_addr_soft) (l);
	*(void **) args->arg = data;
	break;
      }
    }
}

int
__dlinfo (void *handle, int request, void *arg DL_CALLER_DECL)
{
# ifdef SHARED
  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dlinfo (handle, request, arg,
				DL_CALLER);
# endif

  struct dlinfo_args args = { (ElfW(Addr)) DL_CALLER,
			      handle, request, arg };
  return _dlerror_run (&dlinfo_doit, &args) ? -1 : 0;
}
# ifdef SHARED
strong_alias (__dlinfo, dlinfo)
# endif
#endif
