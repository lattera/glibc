/* dlinfo -- Get information from the dynamic linker.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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

#if 0
  if (args->handle == RTLD_SELF)
    {

      /* Find the highest-addressed object that CALLER is not below.  */
      for (l = GL(dl_loaded); l != NULL; l = l->l_next)
	if (caller >= l->l_map_start && caller < l->l_map_end)
	  /* There must be exactly one DSO for the range of the virtual
	     memory.  Otherwise something is really broken.  */
	  break;

      if (l == NULL)
	GLRO(dl_signal_error) (0, NULL, NULL, N_("\
RTLD_SELF used in code not dynamically loaded"));
    }
#endif

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
    }
}

int
dlinfo (void *handle, int request, void *arg)
{
  struct dlinfo_args args = { (ElfW(Addr)) RETURN_ADDRESS (0),
			      handle, request, arg };
  return _dlerror_run (&dlinfo_doit, &args) ? -1 : 0;
}
