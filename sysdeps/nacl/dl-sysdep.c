/* Operating system support for run-time dynamic linker.  NaCl version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#ifdef SHARED

# include <assert.h>
# include <ldsodefs.h>
# include <stdint.h>
# include <nacl-interfaces.h>

/* NaCl's elf32.h is incompatible with the real <elf.h>.  */
# define NATIVE_CLIENT_SRC_INCLUDE_ELF32_H_
# include <native_client/src/untrusted/nacl/nacl_startup.h>

/* The RTLD_START code sets up the pointer that gets to these
   macros as COOKIE to point to two words:
   [0] the argument to the entry point from the system (see nacl_startup.h)
   [1] the stack base
*/

# define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp)	\
  do {								\
    uint32_t *_info = ((void **) (cookie))[0];			\
    (argc) = nacl_startup_argc (_info);				\
    (argv) = nacl_startup_argv (_info);				\
    (envp) = nacl_startup_envp (_info);				\
    (auxp) = nacl_startup_auxv (_info);				\
  } while (0)

# define DL_STACK_END(cookie)	(((void **) (cookie))[1])

/* This is called from the entry point (_start), defined by the RTLD_START
   macro in the machine-specific dl-machine.h file.  At this point, dynamic
   linking has been completed and the first argument is the application's
   entry point.  */
attribute_hidden internal_function __attribute__ ((noreturn))
void
_dl_start_user (void (*user_entry) (uint32_t info[]), uint32_t info[])
{
  if (_dl_skip_args > 0)
    {
      /* There are some arguments that the user program should not see.
	 Just slide up the INFO pointer so its NACL_STARTUP_ARGV points
	 to what should now be argv[0], and copy back the earlier fields.  */
      assert (nacl_startup_argc (info) >= _dl_skip_args);
      assert (NACL_STARTUP_ARGV == 3);
      uint32_t envc = info[NACL_STARTUP_ENVC];
      uint32_t argc = info[NACL_STARTUP_ARGC];
      info += _dl_skip_args;
      info[NACL_STARTUP_ENVC] = envc;
      info[NACL_STARTUP_ARGC] = argc - _dl_skip_args;
    }

  /* Pass our finalizer function to the user.  */
  info[NACL_STARTUP_FINI] = (uintptr_t) &_dl_fini;

  /* Run initializers.  */
  _dl_init (GL(dl_ns)[0]._ns_loaded,
	    nacl_startup_argc (info),
	    nacl_startup_argv (info),
	    nacl_startup_envp (info));

  /* Call the user's entry point.  This should never return.  */
  (*user_entry) (info);

  /* Fail clearly just in case it did return.  */
  __builtin_trap ();
}

# define DL_SYSDEP_INIT	__nacl_initialize_interfaces ()

#endif  /* SHARED */

#include <elf/dl-sysdep.c>

#include <dl-sysdep-open.h>
#include <nacl-interfaces.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

char *
internal_function
_dl_sysdep_open_object (const char *name, size_t namelen, int *fd)
{
  int error = __nacl_irt_resource_open.open_resource (name, fd);
  if (error)
    return NULL;
  assert (*fd != -1);
  char *realname = __strdup (name);
  if (__glibc_unlikely (realname == NULL))
    {
      __close (*fd);
      *fd = -1;
    }
  return realname;
}
