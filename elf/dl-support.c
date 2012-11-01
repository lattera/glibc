/* Support for dynamic linking code in static libc.
   Copyright (C) 1996-2012 Free Software Foundation, Inc.
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

/* This file defines some things that for the dynamic linker are defined in
   rtld.c and dl-sysdep.c in ways appropriate to bootstrap dynamic linking.  */

#include <errno.h>
#include <libintl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <ldsodefs.h>
#include <dl-machine.h>
#include <bits/libc-lock.h>
#include <dl-cache.h>
#include <dl-librecon.h>
#include <dl-procinfo.h>
#include <unsecvars.h>
#include <hp-timing.h>
#include <stackinfo.h>

extern char *__progname;
char **_dl_argv = &__progname;	/* This is checked for some error messages.  */

/* Name of the architecture.  */
const char *_dl_platform;
size_t _dl_platformlen;

int _dl_debug_mask;
int _dl_lazy;
ElfW(Addr) _dl_use_load_bias = -2;
int _dl_dynamic_weak;

/* If nonzero print warnings about problematic situations.  */
int _dl_verbose;

/* We never do profiling.  */
const char *_dl_profile;
const char *_dl_profile_output;

/* Names of shared object for which the RUNPATHs and RPATHs should be
   ignored.  */
const char *_dl_inhibit_rpath;

/* The map for the object we will profile.  */
struct link_map *_dl_profile_map;

/* This is the address of the last stack address ever used.  */
void *__libc_stack_end;

/* Path where the binary is found.  */
const char *_dl_origin_path;

/* Nonzero if runtime lookup should not update the .got/.plt.  */
int _dl_bind_not;

/* Namespace information.  */
struct link_namespaces _dl_ns[DL_NNS];
size_t _dl_nns;

/* Incremented whenever something may have been added to dl_loaded. */
unsigned long long _dl_load_adds;

/* Fake scope.  In dynamically linked binaries this is the scope of the
   main application but here we don't have something like this.  So
   create a fake scope containing nothing.  */
struct r_scope_elem _dl_initial_searchlist;

#ifndef HAVE_INLINED_SYSCALLS
/* Nonzero during startup.  */
int _dl_starting_up = 1;
#endif

/* Random data provided by the kernel.  */
void *_dl_random;

/* Get architecture specific initializer.  */
#include <dl-procinfo.c>

/* We expect less than a second for relocation.  */
#ifdef HP_SMALL_TIMING_AVAIL
# undef HP_TIMING_AVAIL
# define HP_TIMING_AVAIL HP_SMALL_TIMING_AVAIL
#endif

/* Initial value of the CPU clock.  */
#ifndef HP_TIMING_NONAVAIL
hp_timing_t _dl_cpuclock_offset;
#endif

void (*_dl_init_static_tls) (struct link_map *) = &_dl_nothread_init_static_tls;

size_t _dl_pagesize = EXEC_PAGESIZE;

int _dl_inhibit_cache;

unsigned int _dl_osversion;

/* All known directories in sorted order.  */
struct r_search_path_elem *_dl_all_dirs;

/* All directories after startup.  */
struct r_search_path_elem *_dl_init_all_dirs;

/* The object to be initialized first.  */
struct link_map *_dl_initfirst;

/* Descriptor to write debug messages to.  */
int _dl_debug_fd = STDERR_FILENO;

int _dl_correct_cache_id = _DL_CACHE_DEFAULT_ID;

ElfW(auxv_t) *_dl_auxv;
ElfW(Phdr) *_dl_phdr;
size_t _dl_phnum;
uint64_t _dl_hwcap __attribute__ ((nocommon));

/* This is not initialized to HWCAP_IMPORTANT, matching the definition
   of _dl_important_hwcaps, below, where no hwcap strings are ever
   used.  This mask is still used to mediate the lookups in the cache
   file.  Since there is no way to set this nonzero (we don't grok the
   LD_HWCAP_MASK environment variable here), there is no real point in
   setting _dl_hwcap nonzero below, but we do anyway.  */
uint64_t _dl_hwcap_mask __attribute__ ((nocommon));

/* Prevailing state of the stack.  Generally this includes PF_X, indicating it's
 * executable but this isn't true for all platforms.  */
ElfW(Word) _dl_stack_flags = DEFAULT_STACK_PERMS;

/* If loading a shared object requires that we make the stack executable
   when it was not, we do it by calling this function.
   It returns an errno code or zero on success.  */
int (*_dl_make_stack_executable_hook) (void **) internal_function
  = _dl_make_stack_executable;


/* Function in libpthread to wait for termination of lookups.  */
void (*_dl_wait_lookup_done) (void);

struct dl_scope_free_list *_dl_scope_free_list;

#ifdef NEED_DL_SYSINFO
/* Needed for improved syscall handling on at least x86/Linux.  */
uintptr_t _dl_sysinfo = DL_SYSINFO_DEFAULT;
#endif
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
/* Address of the ELF headers in the vsyscall page.  */
const ElfW(Ehdr) *_dl_sysinfo_dso;

struct link_map *_dl_sysinfo_map;

# include "get-dynamic-info.h"
#endif
#include "setup-vdso.h"

/* During the program run we must not modify the global data of
   loaded shared object simultanously in two threads.  Therefore we
   protect `_dl_open' and `_dl_close' in dl-close.c.

   This must be a recursive lock since the initializer function of
   the loaded object might as well require a call to this function.
   At this time it is not anymore a problem to modify the tables.  */
__rtld_lock_define_initialized_recursive (, _dl_load_lock)
/* This lock is used to keep __dl_iterate_phdr from inspecting the
   list of loaded objects while an object is added to or removed from
   that list.  */
__rtld_lock_define_initialized_recursive (, _dl_load_write_lock)


#ifdef HAVE_AUX_VECTOR
int _dl_clktck;

void
internal_function
_dl_aux_init (ElfW(auxv_t) *av)
{
  int seen = 0;
  uid_t uid = 0;
  gid_t gid = 0;

  _dl_auxv = av;
  for (; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_PAGESZ:
	GLRO(dl_pagesize) = av->a_un.a_val;
	break;
      case AT_CLKTCK:
	GLRO(dl_clktck) = av->a_un.a_val;
	break;
      case AT_PHDR:
	GL(dl_phdr) = (void *) av->a_un.a_val;
	break;
      case AT_PHNUM:
	GL(dl_phnum) = av->a_un.a_val;
	break;
      case AT_HWCAP:
	GLRO(dl_hwcap) = (unsigned long int) av->a_un.a_val;
	break;
#ifdef NEED_DL_SYSINFO
      case AT_SYSINFO:
	GL(dl_sysinfo) = av->a_un.a_val;
	break;
#endif
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
      case AT_SYSINFO_EHDR:
	GL(dl_sysinfo_dso) = (void *) av->a_un.a_val;
	break;
#endif
      case AT_UID:
	uid ^= av->a_un.a_val;
	seen |= 1;
	break;
      case AT_EUID:
	uid ^= av->a_un.a_val;
	seen |= 2;
	break;
      case AT_GID:
	gid ^= av->a_un.a_val;
	seen |= 4;
	break;
      case AT_EGID:
	gid ^= av->a_un.a_val;
	seen |= 8;
	break;
      case AT_SECURE:
	seen = -1;
	__libc_enable_secure = av->a_un.a_val;
	__libc_enable_secure_decided = 1;
	break;
      case AT_RANDOM:
	_dl_random = (void *) av->a_un.a_val;
	break;
# ifdef DL_PLATFORM_AUXV
      DL_PLATFORM_AUXV
# endif
      }
  if (seen == 0xf)
    {
      __libc_enable_secure = uid != 0 || gid != 0;
      __libc_enable_secure_decided = 1;
    }
}
#endif


void
internal_function
_dl_non_dynamic_init (void)
{
  if (HP_TIMING_AVAIL)
    HP_TIMING_NOW (_dl_cpuclock_offset);

  if (!_dl_pagesize)
    _dl_pagesize = __getpagesize ();

  _dl_verbose = *(getenv ("LD_WARN") ?: "") == '\0' ? 0 : 1;

  /* Set up the data structures for the system-supplied DSO early,
     so they can influence _dl_init_paths.  */
  setup_vdso (NULL, NULL);

  /* Initialize the data structures for the search paths for shared
     objects.  */
  _dl_init_paths (getenv ("LD_LIBRARY_PATH"));

  /* Remember the last search directory added at startup.  */
  _dl_init_all_dirs = GL(dl_all_dirs);

  _dl_lazy = *(getenv ("LD_BIND_NOW") ?: "") == '\0';

  _dl_bind_not = *(getenv ("LD_BIND_NOT") ?: "") != '\0';

  _dl_dynamic_weak = *(getenv ("LD_DYNAMIC_WEAK") ?: "") == '\0';

  _dl_profile_output = getenv ("LD_PROFILE_OUTPUT");
  if (_dl_profile_output == NULL || _dl_profile_output[0] == '\0')
    _dl_profile_output
      = &"/var/tmp\0/var/profile"[__libc_enable_secure ? 9 : 0];

  if (__libc_enable_secure)
    {
      static const char unsecure_envvars[] =
	UNSECURE_ENVVARS
#ifdef EXTRA_UNSECURE_ENVVARS
	EXTRA_UNSECURE_ENVVARS
#endif
	;
      const char *cp = unsecure_envvars;

      while (cp < unsecure_envvars + sizeof (unsecure_envvars))
	{
	  __unsetenv (cp);
	  cp = (const char *) __rawmemchr (cp, '\0') + 1;
	}

      if (__access ("/etc/suid-debug", F_OK) != 0)
	__unsetenv ("MALLOC_CHECK_");
    }

#ifdef DL_PLATFORM_INIT
  DL_PLATFORM_INIT;
#endif

#ifdef DL_OSVERSION_INIT
  DL_OSVERSION_INIT;
#endif

  /* Now determine the length of the platform string.  */
  if (_dl_platform != NULL)
    _dl_platformlen = strlen (_dl_platform);

  /* Scan for a program header telling us the stack is nonexecutable.  */
  if (_dl_phdr != NULL)
    for (uint_fast16_t i = 0; i < _dl_phnum; ++i)
      if (_dl_phdr[i].p_type == PT_GNU_STACK)
	{
	  _dl_stack_flags = _dl_phdr[i].p_flags;
	  break;
	}
}

#ifdef DL_SYSINFO_IMPLEMENTATION
DL_SYSINFO_IMPLEMENTATION
#endif
