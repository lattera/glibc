/* Support for dynamic linking code in static libc.
   Copyright (C) 1996-2002, 2003, 2004 Free Software Foundation, Inc.
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

/* This file defines some things that for the dynamic linker are defined in
   rtld.c and dl-sysdep.c in ways appropriate to bootstrap dynamic linking.  */

#include <errno.h>
#include <libintl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <dl-machine.h>
#include <bits/libc-lock.h>
#include <dl-cache.h>
#include <dl-librecon.h>
#include <unsecvars.h>
#include <hp-timing.h>

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

#ifdef USE_TLS
void (*_dl_init_static_tls) (struct link_map *) = &_dl_nothread_init_static_tls;
#endif

size_t _dl_pagesize;

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

ElfW(Phdr) *_dl_phdr;
size_t _dl_phnum;
unsigned long int _dl_hwcap __attribute__ ((nocommon));

/* Prevailing state of the stack, PF_X indicating it's executable.  */
ElfW(Word) _dl_stack_flags = PF_R|PF_W|PF_X;

/* If loading a shared object requires that we make the stack executable
   when it was not, we do it by calling this function.
   It returns an errno code or zero on success.  */
int (*_dl_make_stack_executable_hook) (void **) internal_function
  = _dl_make_stack_executable;


#ifdef NEED_DL_SYSINFO
/* Needed for improved syscall handling on at least x86/Linux.  */
uintptr_t _dl_sysinfo = DL_SYSINFO_DEFAULT;
#endif
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
/* Address of the ELF headers in the vsyscall page.  */
const ElfW(Ehdr) *_dl_sysinfo_dso;
#endif

/* During the program run we must not modify the global data of
   loaded shared object simultanously in two threads.  Therefore we
   protect `_dl_open' and `_dl_close' in dl-close.c.

   This must be a recursive lock since the initializer function of
   the loaded object might as well require a call to this function.
   At this time it is not anymore a problem to modify the tables.  */
__rtld_lock_define_initialized_recursive (, _dl_load_lock)


#ifdef HAVE_AUX_VECTOR
int _dl_clktck;

void
internal_function
_dl_aux_init (ElfW(auxv_t) *av)
{
  int seen = 0;
  uid_t uid = 0;
  gid_t gid = 0;

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
	GL(dl_phdr) = av->a_un.a_ptr;
	break;
      case AT_PHNUM:
	GL(dl_phnum) = av->a_un.a_val;
	break;
      case AT_HWCAP:
	GLRO(dl_hwcap) = av->a_un.a_val;
	break;
#ifdef NEED_DL_SYSINFO
      case AT_SYSINFO:
	GL(dl_sysinfo) = av->a_un.a_val;
	break;
#endif
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
      case AT_SYSINFO_EHDR:
	GL(dl_sysinfo_dso) = av->a_un.a_ptr;
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

  /* Initialize the data structures for the search paths for shared
     objects.  */
  _dl_init_paths (getenv ("LD_LIBRARY_PATH"));

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


const struct r_strlenpair *
internal_function
_dl_important_hwcaps (const char *platform, size_t platform_len, size_t *sz,
		      size_t *max_capstrlen)
{
  static struct r_strlenpair result;
  static char buf[1];

  result.str = buf;	/* Does not really matter.  */
  result.len = 0;

  *sz = 1;
  return &result;
}


#ifdef DL_SYSINFO_IMPLEMENTATION
DL_SYSINFO_IMPLEMENTATION
#endif
