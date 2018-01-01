/* Run time dynamic linker.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ldsodefs.h>
#include <_itoa.h>
#include <entry.h>
#include <fpu_control.h>
#include <hp-timing.h>
#include <libc-lock.h>
#include "dynamic-link.h"
#include <dl-librecon.h>
#include <unsecvars.h>
#include <dl-cache.h>
#include <dl-osinfo.h>
#include <dl-procinfo.h>
#include <tls.h>
#include <stap-probe.h>
#include <stackinfo.h>

#include <assert.h>

/* Avoid PLT use for our local calls at startup.  */
extern __typeof (__mempcpy) __mempcpy attribute_hidden;

/* GCC has mental blocks about _exit.  */
extern __typeof (_exit) exit_internal asm ("_exit") attribute_hidden;
#define _exit exit_internal

/* Helper function to handle errors while resolving symbols.  */
static void print_unresolved (int errcode, const char *objname,
			      const char *errsting);

/* Helper function to handle errors when a version is missing.  */
static void print_missing_version (int errcode, const char *objname,
				   const char *errsting);

/* Print the various times we collected.  */
static void print_statistics (hp_timing_t *total_timep);

/* Add audit objects.  */
static void process_dl_audit (char *str);

/* This is a list of all the modes the dynamic loader can be in.  */
enum mode { normal, list, verify, trace };

/* Process all environments variables the dynamic linker must recognize.
   Since all of them start with `LD_' we are a bit smarter while finding
   all the entries.  */
static void process_envvars (enum mode *modep);

#ifdef DL_ARGV_NOT_RELRO
int _dl_argc attribute_hidden;
char **_dl_argv = NULL;
/* Nonzero if we were run directly.  */
unsigned int _dl_skip_args attribute_hidden;
#else
int _dl_argc attribute_relro attribute_hidden;
char **_dl_argv attribute_relro = NULL;
unsigned int _dl_skip_args attribute_relro attribute_hidden;
#endif
rtld_hidden_data_def (_dl_argv)

#ifndef THREAD_SET_STACK_GUARD
/* Only exported for architectures that don't store the stack guard canary
   in thread local area.  */
uintptr_t __stack_chk_guard attribute_relro;
#endif

/* Only exported for architectures that don't store the pointer guard
   value in thread local area.  */
uintptr_t __pointer_chk_guard_local
     attribute_relro attribute_hidden __attribute__ ((nocommon));
#ifndef THREAD_SET_POINTER_GUARD
strong_alias (__pointer_chk_guard_local, __pointer_chk_guard)
#endif

/* Length limits for names and paths, to protect the dynamic linker,
   particularly when __libc_enable_secure is active.  */
#ifdef NAME_MAX
# define SECURE_NAME_LIMIT NAME_MAX
#else
# define SECURE_NAME_LIMIT 255
#endif
#ifdef PATH_MAX
# define SECURE_PATH_LIMIT PATH_MAX
#else
# define SECURE_PATH_LIMIT 1024
#endif

/* Check that AT_SECURE=0, or that the passed name does not contain
   directories and is not overly long.  Reject empty names
   unconditionally.  */
static bool
dso_name_valid_for_suid (const char *p)
{
  if (__glibc_unlikely (__libc_enable_secure))
    {
      /* Ignore pathnames with directories for AT_SECURE=1
	 programs, and also skip overlong names.  */
      size_t len = strlen (p);
      if (len >= SECURE_NAME_LIMIT || memchr (p, '/', len) != NULL)
	return false;
    }
  return *p != '\0';
}

/* LD_AUDIT variable contents.  Must be processed before the
   audit_list below.  */
const char *audit_list_string;

/* Cyclic list of auditing DSOs.  audit_list->next is the first
   element.  */
static struct audit_list
{
  const char *name;
  struct audit_list *next;
} *audit_list;

/* Iterator for audit_list_string followed by audit_list.  */
struct audit_list_iter
{
  /* Tail of audit_list_string still needing processing, or NULL.  */
  const char *audit_list_tail;

  /* The list element returned in the previous iteration.  NULL before
     the first element.  */
  struct audit_list *previous;

  /* Scratch buffer for returning a name which is part of
     audit_list_string.  */
  char fname[SECURE_NAME_LIMIT];
};

/* Initialize an audit list iterator.  */
static void
audit_list_iter_init (struct audit_list_iter *iter)
{
  iter->audit_list_tail = audit_list_string;
  iter->previous = NULL;
}

/* Iterate through both audit_list_string and audit_list.  */
static const char *
audit_list_iter_next (struct audit_list_iter *iter)
{
  if (iter->audit_list_tail != NULL)
    {
      /* First iterate over audit_list_string.  */
      while (*iter->audit_list_tail != '\0')
	{
	  /* Split audit list at colon.  */
	  size_t len = strcspn (iter->audit_list_tail, ":");
	  if (len > 0 && len < sizeof (iter->fname))
	    {
	      memcpy (iter->fname, iter->audit_list_tail, len);
	      iter->fname[len] = '\0';
	    }
	  else
	    /* Do not return this name to the caller.  */
	    iter->fname[0] = '\0';

	  /* Skip over the substring and the following delimiter.  */
	  iter->audit_list_tail += len;
	  if (*iter->audit_list_tail == ':')
	    ++iter->audit_list_tail;

	  /* If the name is valid, return it.  */
	  if (dso_name_valid_for_suid (iter->fname))
	    return iter->fname;
	  /* Otherwise, wrap around and try the next name.  */
	}
      /* Fall through to the procesing of audit_list.  */
    }

  if (iter->previous == NULL)
    {
      if (audit_list == NULL)
	/* No pre-parsed audit list.  */
	return NULL;
      /* Start of audit list.  The first list element is at
	 audit_list->next (cyclic list).  */
      iter->previous = audit_list->next;
      return iter->previous->name;
    }
  if (iter->previous == audit_list)
    /* Cyclic list wrap-around.  */
    return NULL;
  iter->previous = iter->previous->next;
  return iter->previous->name;
}

#ifndef HAVE_INLINED_SYSCALLS
/* Set nonzero during loading and initialization of executable and
   libraries, cleared before the executable's entry point runs.  This
   must not be initialized to nonzero, because the unused dynamic
   linker loaded in for libc.so's "ld.so.1" dep will provide the
   definition seen by libc.so's initializer; that value must be zero,
   and will be since that dynamic linker's _dl_start and dl_main will
   never be called.  */
int _dl_starting_up = 0;
rtld_hidden_def (_dl_starting_up)
#endif

/* This is the structure which defines all variables global to ld.so
   (except those which cannot be added for some reason).  */
struct rtld_global _rtld_global =
  {
    /* Generally the default presumption without further information is an
     * executable stack but this is not true for all platforms.  */
    ._dl_stack_flags = DEFAULT_STACK_PERMS,
#ifdef _LIBC_REENTRANT
    ._dl_load_lock = _RTLD_LOCK_RECURSIVE_INITIALIZER,
    ._dl_load_write_lock = _RTLD_LOCK_RECURSIVE_INITIALIZER,
#endif
    ._dl_nns = 1,
    ._dl_ns =
    {
#ifdef _LIBC_REENTRANT
      [LM_ID_BASE] = { ._ns_unique_sym_table
		       = { .lock = _RTLD_LOCK_RECURSIVE_INITIALIZER } }
#endif
    }
  };
/* If we would use strong_alias here the compiler would see a
   non-hidden definition.  This would undo the effect of the previous
   declaration.  So spell out was strong_alias does plus add the
   visibility attribute.  */
extern struct rtld_global _rtld_local
    __attribute__ ((alias ("_rtld_global"), visibility ("hidden")));


/* This variable is similar to _rtld_local, but all values are
   read-only after relocation.  */
struct rtld_global_ro _rtld_global_ro attribute_relro =
  {
    /* Get architecture specific initializer.  */
#include <dl-procinfo.c>
#ifdef NEED_DL_SYSINFO
    ._dl_sysinfo = DL_SYSINFO_DEFAULT,
#endif
    ._dl_debug_fd = STDERR_FILENO,
    ._dl_use_load_bias = -2,
    ._dl_correct_cache_id = _DL_CACHE_DEFAULT_ID,
#if !HAVE_TUNABLES
    ._dl_hwcap_mask = HWCAP_IMPORTANT,
#endif
    ._dl_lazy = 1,
    ._dl_fpu_control = _FPU_DEFAULT,
    ._dl_pagesize = EXEC_PAGESIZE,
    ._dl_inhibit_cache = 0,

    /* Function pointers.  */
    ._dl_debug_printf = _dl_debug_printf,
    ._dl_mcount = _dl_mcount,
    ._dl_lookup_symbol_x = _dl_lookup_symbol_x,
    ._dl_check_caller = _dl_check_caller,
    ._dl_open = _dl_open,
    ._dl_close = _dl_close,
    ._dl_tls_get_addr_soft = _dl_tls_get_addr_soft,
#ifdef HAVE_DL_DISCOVER_OSVERSION
    ._dl_discover_osversion = _dl_discover_osversion
#endif
  };
/* If we would use strong_alias here the compiler would see a
   non-hidden definition.  This would undo the effect of the previous
   declaration.  So spell out was strong_alias does plus add the
   visibility attribute.  */
extern struct rtld_global_ro _rtld_local_ro
    __attribute__ ((alias ("_rtld_global_ro"), visibility ("hidden")));


static void dl_main (const ElfW(Phdr) *phdr, ElfW(Word) phnum,
		     ElfW(Addr) *user_entry, ElfW(auxv_t) *auxv);

/* These two variables cannot be moved into .data.rel.ro.  */
static struct libname_list _dl_rtld_libname;
static struct libname_list _dl_rtld_libname2;

/* Variable for statistics.  */
#ifndef HP_TIMING_NONAVAIL
static hp_timing_t relocate_time;
static hp_timing_t load_time attribute_relro;
static hp_timing_t start_time attribute_relro;
#endif

/* Additional definitions needed by TLS initialization.  */
#ifdef TLS_INIT_HELPER
TLS_INIT_HELPER
#endif

/* Helper function for syscall implementation.  */
#ifdef DL_SYSINFO_IMPLEMENTATION
DL_SYSINFO_IMPLEMENTATION
#endif

/* Before ld.so is relocated we must not access variables which need
   relocations.  This means variables which are exported.  Variables
   declared as static are fine.  If we can mark a variable hidden this
   is fine, too.  The latter is important here.  We can avoid setting
   up a temporary link map for ld.so if we can mark _rtld_global as
   hidden.  */
#ifdef PI_STATIC_AND_HIDDEN
# define DONT_USE_BOOTSTRAP_MAP	1
#endif

#ifdef DONT_USE_BOOTSTRAP_MAP
static ElfW(Addr) _dl_start_final (void *arg);
#else
struct dl_start_final_info
{
  struct link_map l;
#if !defined HP_TIMING_NONAVAIL && HP_TIMING_INLINE
  hp_timing_t start_time;
#endif
};
static ElfW(Addr) _dl_start_final (void *arg,
				   struct dl_start_final_info *info);
#endif

/* These defined magically in the linker script.  */
extern char _begin[] attribute_hidden;
extern char _etext[] attribute_hidden;
extern char _end[] attribute_hidden;


#ifdef RTLD_START
RTLD_START
#else
# error "sysdeps/MACHINE/dl-machine.h fails to define RTLD_START"
#endif

/* This is the second half of _dl_start (below).  It can be inlined safely
   under DONT_USE_BOOTSTRAP_MAP, where it is careful not to make any GOT
   references.  When the tools don't permit us to avoid using a GOT entry
   for _dl_rtld_global (no attribute_hidden support), we must make sure
   this function is not inlined (see below).  */

#ifdef DONT_USE_BOOTSTRAP_MAP
static inline ElfW(Addr) __attribute__ ((always_inline))
_dl_start_final (void *arg)
#else
static ElfW(Addr) __attribute__ ((noinline))
_dl_start_final (void *arg, struct dl_start_final_info *info)
#endif
{
  ElfW(Addr) start_addr;

  if (HP_SMALL_TIMING_AVAIL)
    {
      /* If it hasn't happen yet record the startup time.  */
      if (! HP_TIMING_INLINE)
	HP_TIMING_NOW (start_time);
#if !defined DONT_USE_BOOTSTRAP_MAP && !defined HP_TIMING_NONAVAIL
      else
	start_time = info->start_time;
#endif
    }

  /* Transfer data about ourselves to the permanent link_map structure.  */
#ifndef DONT_USE_BOOTSTRAP_MAP
  GL(dl_rtld_map).l_addr = info->l.l_addr;
  GL(dl_rtld_map).l_ld = info->l.l_ld;
  memcpy (GL(dl_rtld_map).l_info, info->l.l_info,
	  sizeof GL(dl_rtld_map).l_info);
  GL(dl_rtld_map).l_mach = info->l.l_mach;
  GL(dl_rtld_map).l_relocated = 1;
#endif
  _dl_setup_hash (&GL(dl_rtld_map));
  GL(dl_rtld_map).l_real = &GL(dl_rtld_map);
  GL(dl_rtld_map).l_map_start = (ElfW(Addr)) _begin;
  GL(dl_rtld_map).l_map_end = (ElfW(Addr)) _end;
  GL(dl_rtld_map).l_text_end = (ElfW(Addr)) _etext;
  /* Copy the TLS related data if necessary.  */
#ifndef DONT_USE_BOOTSTRAP_MAP
# if NO_TLS_OFFSET != 0
  GL(dl_rtld_map).l_tls_offset = NO_TLS_OFFSET;
# endif
#endif

  HP_TIMING_NOW (GL(dl_cpuclock_offset));

  /* Initialize the stack end variable.  */
  __libc_stack_end = __builtin_frame_address (0);

  /* Call the OS-dependent function to set up life so we can do things like
     file access.  It will call `dl_main' (below) to do all the real work
     of the dynamic linker, and then unwind our frame and run the user
     entry point on the same stack we entered on.  */
  start_addr = _dl_sysdep_start (arg, &dl_main);

#ifndef HP_TIMING_NONAVAIL
  hp_timing_t rtld_total_time;
  if (HP_SMALL_TIMING_AVAIL)
    {
      hp_timing_t end_time;

      /* Get the current time.  */
      HP_TIMING_NOW (end_time);

      /* Compute the difference.  */
      HP_TIMING_DIFF (rtld_total_time, start_time, end_time);
    }
#endif

  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_STATISTICS))
    {
#ifndef HP_TIMING_NONAVAIL
      print_statistics (&rtld_total_time);
#else
      print_statistics (NULL);
#endif
    }

  return start_addr;
}

static ElfW(Addr) __attribute_used__
_dl_start (void *arg)
{
#ifdef DONT_USE_BOOTSTRAP_MAP
# define bootstrap_map GL(dl_rtld_map)
#else
  struct dl_start_final_info info;
# define bootstrap_map info.l
#endif

  /* This #define produces dynamic linking inline functions for
     bootstrap relocation instead of general-purpose relocation.
     Since ld.so must not have any undefined symbols the result
     is trivial: always the map of ld.so itself.  */
#define RTLD_BOOTSTRAP
#define BOOTSTRAP_MAP (&bootstrap_map)
#define RESOLVE_MAP(sym, version, flags) BOOTSTRAP_MAP
#include "dynamic-link.h"

  if (HP_TIMING_INLINE && HP_SMALL_TIMING_AVAIL)
#ifdef DONT_USE_BOOTSTRAP_MAP
    HP_TIMING_NOW (start_time);
#else
    HP_TIMING_NOW (info.start_time);
#endif

  /* Partly clean the `bootstrap_map' structure up.  Don't use
     `memset' since it might not be built in or inlined and we cannot
     make function calls at this point.  Use '__builtin_memset' if we
     know it is available.  We do not have to clear the memory if we
     do not have to use the temporary bootstrap_map.  Global variables
     are initialized to zero by default.  */
#ifndef DONT_USE_BOOTSTRAP_MAP
# ifdef HAVE_BUILTIN_MEMSET
  __builtin_memset (bootstrap_map.l_info, '\0', sizeof (bootstrap_map.l_info));
# else
  for (size_t cnt = 0;
       cnt < sizeof (bootstrap_map.l_info) / sizeof (bootstrap_map.l_info[0]);
       ++cnt)
    bootstrap_map.l_info[cnt] = 0;
# endif
#endif

  /* Figure out the run-time load address of the dynamic linker itself.  */
  bootstrap_map.l_addr = elf_machine_load_address ();

  /* Read our own dynamic section and fill in the info array.  */
  bootstrap_map.l_ld = (void *) bootstrap_map.l_addr + elf_machine_dynamic ();
  elf_get_dynamic_info (&bootstrap_map, NULL);

#if NO_TLS_OFFSET != 0
  bootstrap_map.l_tls_offset = NO_TLS_OFFSET;
#endif

#ifdef ELF_MACHINE_BEFORE_RTLD_RELOC
  ELF_MACHINE_BEFORE_RTLD_RELOC (bootstrap_map.l_info);
#endif

  if (bootstrap_map.l_addr || ! bootstrap_map.l_info[VALIDX(DT_GNU_PRELINKED)])
    {
      /* Relocate ourselves so we can do normal function calls and
	 data access using the global offset table.  */

      ELF_DYNAMIC_RELOCATE (&bootstrap_map, 0, 0, 0);
    }
  bootstrap_map.l_relocated = 1;

  /* Please note that we don't allow profiling of this object and
     therefore need not test whether we have to allocate the array
     for the relocation results (as done in dl-reloc.c).  */

  /* Now life is sane; we can call functions and access global data.
     Set up to use the operating system facilities, and find out from
     the operating system's program loader where to find the program
     header table in core.  Put the rest of _dl_start into a separate
     function, that way the compiler cannot put accesses to the GOT
     before ELF_DYNAMIC_RELOCATE.  */
  {
#ifdef DONT_USE_BOOTSTRAP_MAP
    ElfW(Addr) entry = _dl_start_final (arg);
#else
    ElfW(Addr) entry = _dl_start_final (arg, &info);
#endif

#ifndef ELF_MACHINE_START_ADDRESS
# define ELF_MACHINE_START_ADDRESS(map, start) (start)
#endif

    return ELF_MACHINE_START_ADDRESS (GL(dl_ns)[LM_ID_BASE]._ns_loaded, entry);
  }
}



/* Now life is peachy; we can do all normal operations.
   On to the real work.  */

/* Some helper functions.  */

/* Arguments to relocate_doit.  */
struct relocate_args
{
  struct link_map *l;
  int reloc_mode;
};

struct map_args
{
  /* Argument to map_doit.  */
  const char *str;
  struct link_map *loader;
  int mode;
  /* Return value of map_doit.  */
  struct link_map *map;
};

struct dlmopen_args
{
  const char *fname;
  struct link_map *map;
};

struct lookup_args
{
  const char *name;
  struct link_map *map;
  void *result;
};

/* Arguments to version_check_doit.  */
struct version_check_args
{
  int doexit;
  int dotrace;
};

static void
relocate_doit (void *a)
{
  struct relocate_args *args = (struct relocate_args *) a;

  _dl_relocate_object (args->l, args->l->l_scope, args->reloc_mode, 0);
}

static void
map_doit (void *a)
{
  struct map_args *args = (struct map_args *) a;
  int type = (args->mode == __RTLD_OPENEXEC) ? lt_executable : lt_library;
  args->map = _dl_map_object (args->loader, args->str, type, 0,
			      args->mode, LM_ID_BASE);
}

static void
dlmopen_doit (void *a)
{
  struct dlmopen_args *args = (struct dlmopen_args *) a;
  args->map = _dl_open (args->fname,
			(RTLD_LAZY | __RTLD_DLOPEN | __RTLD_AUDIT
			 | __RTLD_SECURE),
			dl_main, LM_ID_NEWLM, _dl_argc, _dl_argv,
			__environ);
}

static void
lookup_doit (void *a)
{
  struct lookup_args *args = (struct lookup_args *) a;
  const ElfW(Sym) *ref = NULL;
  args->result = NULL;
  lookup_t l = _dl_lookup_symbol_x (args->name, args->map, &ref,
				    args->map->l_local_scope, NULL, 0,
				    DL_LOOKUP_RETURN_NEWEST, NULL);
  if (ref != NULL)
    args->result = DL_SYMBOL_ADDRESS (l, ref);
}

static void
version_check_doit (void *a)
{
  struct version_check_args *args = (struct version_check_args *) a;
  if (_dl_check_all_versions (GL(dl_ns)[LM_ID_BASE]._ns_loaded, 1,
			      args->dotrace) && args->doexit)
    /* We cannot start the application.  Abort now.  */
    _exit (1);
}


static inline struct link_map *
find_needed (const char *name)
{
  struct r_scope_elem *scope = &GL(dl_ns)[LM_ID_BASE]._ns_loaded->l_searchlist;
  unsigned int n = scope->r_nlist;

  while (n-- > 0)
    if (_dl_name_match_p (name, scope->r_list[n]))
      return scope->r_list[n];

  /* Should never happen.  */
  return NULL;
}

static int
match_version (const char *string, struct link_map *map)
{
  const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
  ElfW(Verdef) *def;

#define VERDEFTAG (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (DT_VERDEF))
  if (map->l_info[VERDEFTAG] == NULL)
    /* The file has no symbol versioning.  */
    return 0;

  def = (ElfW(Verdef) *) ((char *) map->l_addr
			  + map->l_info[VERDEFTAG]->d_un.d_ptr);
  while (1)
    {
      ElfW(Verdaux) *aux = (ElfW(Verdaux) *) ((char *) def + def->vd_aux);

      /* Compare the version strings.  */
      if (strcmp (string, strtab + aux->vda_name) == 0)
	/* Bingo!  */
	return 1;

      /* If no more definitions we failed to find what we want.  */
      if (def->vd_next == 0)
	break;

      /* Next definition.  */
      def = (ElfW(Verdef) *) ((char *) def + def->vd_next);
    }

  return 0;
}

static bool tls_init_tp_called;

static void *
init_tls (void)
{
  /* Number of elements in the static TLS block.  */
  GL(dl_tls_static_nelem) = GL(dl_tls_max_dtv_idx);

  /* Do not do this twice.  The audit interface might have required
     the DTV interfaces to be set up early.  */
  if (GL(dl_initial_dtv) != NULL)
    return NULL;

  /* Allocate the array which contains the information about the
     dtv slots.  We allocate a few entries more than needed to
     avoid the need for reallocation.  */
  size_t nelem = GL(dl_tls_max_dtv_idx) + 1 + TLS_SLOTINFO_SURPLUS;

  /* Allocate.  */
  GL(dl_tls_dtv_slotinfo_list) = (struct dtv_slotinfo_list *)
    calloc (sizeof (struct dtv_slotinfo_list)
	    + nelem * sizeof (struct dtv_slotinfo), 1);
  /* No need to check the return value.  If memory allocation failed
     the program would have been terminated.  */

  struct dtv_slotinfo *slotinfo = GL(dl_tls_dtv_slotinfo_list)->slotinfo;
  GL(dl_tls_dtv_slotinfo_list)->len = nelem;
  GL(dl_tls_dtv_slotinfo_list)->next = NULL;

  /* Fill in the information from the loaded modules.  No namespace
     but the base one can be filled at this time.  */
  assert (GL(dl_ns)[LM_ID_BASE + 1]._ns_loaded == NULL);
  int i = 0;
  for (struct link_map *l = GL(dl_ns)[LM_ID_BASE]._ns_loaded; l != NULL;
       l = l->l_next)
    if (l->l_tls_blocksize != 0)
      {
	/* This is a module with TLS data.  Store the map reference.
	   The generation counter is zero.  */
	slotinfo[i].map = l;
	/* slotinfo[i].gen = 0; */
	++i;
      }
  assert (i == GL(dl_tls_max_dtv_idx));

  /* Compute the TLS offsets for the various blocks.  */
  _dl_determine_tlsoffset ();

  /* Construct the static TLS block and the dtv for the initial
     thread.  For some platforms this will include allocating memory
     for the thread descriptor.  The memory for the TLS block will
     never be freed.  It should be allocated accordingly.  The dtv
     array can be changed if dynamic loading requires it.  */
  void *tcbp = _dl_allocate_tls_storage ();
  if (tcbp == NULL)
    _dl_fatal_printf ("\
cannot allocate TLS data structures for initial thread\n");

  /* Store for detection of the special case by __tls_get_addr
     so it knows not to pass this dtv to the normal realloc.  */
  GL(dl_initial_dtv) = GET_DTV (tcbp);

  /* And finally install it for the main thread.  */
  const char *lossage = TLS_INIT_TP (tcbp);
  if (__glibc_unlikely (lossage != NULL))
    _dl_fatal_printf ("cannot set up thread-local storage: %s\n", lossage);
  tls_init_tp_called = true;

  return tcbp;
}

static unsigned int
do_preload (const char *fname, struct link_map *main_map, const char *where)
{
  const char *objname;
  const char *err_str = NULL;
  struct map_args args;
  bool malloced;

  args.str = fname;
  args.loader = main_map;
  args.mode = __RTLD_SECURE;

  unsigned int old_nloaded = GL(dl_ns)[LM_ID_BASE]._ns_nloaded;

  (void) _dl_catch_error (&objname, &err_str, &malloced, map_doit, &args);
  if (__glibc_unlikely (err_str != NULL))
    {
      _dl_error_printf ("\
ERROR: ld.so: object '%s' from %s cannot be preloaded (%s): ignored.\n",
			fname, where, err_str);
      /* No need to call free, this is still before
	 the libc's malloc is used.  */
    }
  else if (GL(dl_ns)[LM_ID_BASE]._ns_nloaded != old_nloaded)
    /* It is no duplicate.  */
    return 1;

  /* Nothing loaded.  */
  return 0;
}

#if defined SHARED && defined _LIBC_REENTRANT \
    && defined __rtld_lock_default_lock_recursive
static void
rtld_lock_default_lock_recursive (void *lock)
{
  __rtld_lock_default_lock_recursive (lock);
}

static void
rtld_lock_default_unlock_recursive (void *lock)
{
  __rtld_lock_default_unlock_recursive (lock);
}
#endif


static void
security_init (void)
{
  /* Set up the stack checker's canary.  */
  uintptr_t stack_chk_guard = _dl_setup_stack_chk_guard (_dl_random);
#ifdef THREAD_SET_STACK_GUARD
  THREAD_SET_STACK_GUARD (stack_chk_guard);
#else
  __stack_chk_guard = stack_chk_guard;
#endif

  /* Set up the pointer guard as well, if necessary.  */
  uintptr_t pointer_chk_guard
    = _dl_setup_pointer_guard (_dl_random, stack_chk_guard);
#ifdef THREAD_SET_POINTER_GUARD
  THREAD_SET_POINTER_GUARD (pointer_chk_guard);
#endif
  __pointer_chk_guard_local = pointer_chk_guard;

  /* We do not need the _dl_random value anymore.  The less
     information we leave behind, the better, so clear the
     variable.  */
  _dl_random = NULL;
}

#include "setup-vdso.h"

/* The library search path.  */
static const char *library_path attribute_relro;
/* The list preloaded objects.  */
static const char *preloadlist attribute_relro;
/* Nonzero if information about versions has to be printed.  */
static int version_info attribute_relro;

/* The LD_PRELOAD environment variable gives list of libraries
   separated by white space or colons that are loaded before the
   executable's dependencies and prepended to the global scope list.
   (If the binary is running setuid all elements containing a '/' are
   ignored since it is insecure.)  Return the number of preloads
   performed.  */
unsigned int
handle_ld_preload (const char *preloadlist, struct link_map *main_map)
{
  unsigned int npreloads = 0;
  const char *p = preloadlist;
  char fname[SECURE_PATH_LIMIT];

  while (*p != '\0')
    {
      /* Split preload list at space/colon.  */
      size_t len = strcspn (p, " :");
      if (len > 0 && len < sizeof (fname))
	{
	  memcpy (fname, p, len);
	  fname[len] = '\0';
	}
      else
	fname[0] = '\0';

      /* Skip over the substring and the following delimiter.  */
      p += len;
      if (*p != '\0')
	++p;

      if (dso_name_valid_for_suid (fname))
	npreloads += do_preload (fname, main_map, "LD_PRELOAD");
    }
  return npreloads;
}

static void
dl_main (const ElfW(Phdr) *phdr,
	 ElfW(Word) phnum,
	 ElfW(Addr) *user_entry,
	 ElfW(auxv_t) *auxv)
{
  const ElfW(Phdr) *ph;
  enum mode mode;
  struct link_map *main_map;
  size_t file_size;
  char *file;
  bool has_interp = false;
  unsigned int i;
  bool prelinked = false;
  bool rtld_is_main = false;
#ifndef HP_TIMING_NONAVAIL
  hp_timing_t start;
  hp_timing_t stop;
  hp_timing_t diff;
#endif
  void *tcbp = NULL;

  GL(dl_init_static_tls) = &_dl_nothread_init_static_tls;

#if defined SHARED && defined _LIBC_REENTRANT \
    && defined __rtld_lock_default_lock_recursive
  GL(dl_rtld_lock_recursive) = rtld_lock_default_lock_recursive;
  GL(dl_rtld_unlock_recursive) = rtld_lock_default_unlock_recursive;
#endif

  /* The explicit initialization here is cheaper than processing the reloc
     in the _rtld_local definition's initializer.  */
  GL(dl_make_stack_executable_hook) = &_dl_make_stack_executable;

  /* Process the environment variable which control the behaviour.  */
  process_envvars (&mode);

#ifndef HAVE_INLINED_SYSCALLS
  /* Set up a flag which tells we are just starting.  */
  _dl_starting_up = 1;
#endif

  if (*user_entry == (ElfW(Addr)) ENTRY_POINT)
    {
      /* Ho ho.  We are not the program interpreter!  We are the program
	 itself!  This means someone ran ld.so as a command.  Well, that
	 might be convenient to do sometimes.  We support it by
	 interpreting the args like this:

	 ld.so PROGRAM ARGS...

	 The first argument is the name of a file containing an ELF
	 executable we will load and run with the following arguments.
	 To simplify life here, PROGRAM is searched for using the
	 normal rules for shared objects, rather than $PATH or anything
	 like that.  We just load it and use its entry point; we don't
	 pay attention to its PT_INTERP command (we are the interpreter
	 ourselves).  This is an easy way to test a new ld.so before
	 installing it.  */
      rtld_is_main = true;

      /* Note the place where the dynamic linker actually came from.  */
      GL(dl_rtld_map).l_name = rtld_progname;

      while (_dl_argc > 1)
	if (! strcmp (_dl_argv[1], "--list"))
	  {
	    mode = list;
	    GLRO(dl_lazy) = -1;	/* This means do no dependency analysis.  */

	    ++_dl_skip_args;
	    --_dl_argc;
	    ++_dl_argv;
	  }
	else if (! strcmp (_dl_argv[1], "--verify"))
	  {
	    mode = verify;

	    ++_dl_skip_args;
	    --_dl_argc;
	    ++_dl_argv;
	  }
	else if (! strcmp (_dl_argv[1], "--inhibit-cache"))
	  {
	    GLRO(dl_inhibit_cache) = 1;
	    ++_dl_skip_args;
	    --_dl_argc;
	    ++_dl_argv;
	  }
	else if (! strcmp (_dl_argv[1], "--library-path")
		 && _dl_argc > 2)
	  {
	    library_path = _dl_argv[2];

	    _dl_skip_args += 2;
	    _dl_argc -= 2;
	    _dl_argv += 2;
	  }
	else if (! strcmp (_dl_argv[1], "--inhibit-rpath")
		 && _dl_argc > 2)
	  {
	    GLRO(dl_inhibit_rpath) = _dl_argv[2];

	    _dl_skip_args += 2;
	    _dl_argc -= 2;
	    _dl_argv += 2;
	  }
	else if (! strcmp (_dl_argv[1], "--audit") && _dl_argc > 2)
	  {
	    process_dl_audit (_dl_argv[2]);

	    _dl_skip_args += 2;
	    _dl_argc -= 2;
	    _dl_argv += 2;
	  }
	else
	  break;

      /* If we have no further argument the program was called incorrectly.
	 Grant the user some education.  */
      if (_dl_argc < 2)
	_dl_fatal_printf ("\
Usage: ld.so [OPTION]... EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]\n\
You have invoked `ld.so', the helper program for shared library executables.\n\
This program usually lives in the file `/lib/ld.so', and special directives\n\
in executable files using ELF shared libraries tell the system's program\n\
loader to load the helper program from this file.  This helper program loads\n\
the shared libraries needed by the program executable, prepares the program\n\
to run, and runs it.  You may invoke this helper program directly from the\n\
command line to load and run an ELF executable file; this is like executing\n\
that file itself, but always uses this helper program from the file you\n\
specified, instead of the helper program file specified in the executable\n\
file you run.  This is mostly of use for maintainers to test new versions\n\
of this helper program; chances are you did not intend to run this program.\n\
\n\
  --list                list all dependencies and how they are resolved\n\
  --verify              verify that given object really is a dynamically linked\n\
			object we can handle\n\
  --inhibit-cache       Do not use " LD_SO_CACHE "\n\
  --library-path PATH   use given PATH instead of content of the environment\n\
			variable LD_LIBRARY_PATH\n\
  --inhibit-rpath LIST  ignore RUNPATH and RPATH information in object names\n\
			in LIST\n\
  --audit LIST          use objects named in LIST as auditors\n");

      ++_dl_skip_args;
      --_dl_argc;
      ++_dl_argv;

      /* The initialization of _dl_stack_flags done below assumes the
	 executable's PT_GNU_STACK may have been honored by the kernel, and
	 so a PT_GNU_STACK with PF_X set means the stack started out with
	 execute permission.  However, this is not really true if the
	 dynamic linker is the executable the kernel loaded.  For this
	 case, we must reinitialize _dl_stack_flags to match the dynamic
	 linker itself.  If the dynamic linker was built with a
	 PT_GNU_STACK, then the kernel may have loaded us with a
	 nonexecutable stack that we will have to make executable when we
	 load the program below unless it has a PT_GNU_STACK indicating
	 nonexecutable stack is ok.  */

      for (ph = phdr; ph < &phdr[phnum]; ++ph)
	if (ph->p_type == PT_GNU_STACK)
	  {
	    GL(dl_stack_flags) = ph->p_flags;
	    break;
	  }

      if (__builtin_expect (mode, normal) == verify)
	{
	  const char *objname;
	  const char *err_str = NULL;
	  struct map_args args;
	  bool malloced;

	  args.str = rtld_progname;
	  args.loader = NULL;
	  args.mode = __RTLD_OPENEXEC;
	  (void) _dl_catch_error (&objname, &err_str, &malloced, map_doit,
				  &args);
	  if (__glibc_unlikely (err_str != NULL))
	    /* We don't free the returned string, the programs stops
	       anyway.  */
	    _exit (EXIT_FAILURE);
	}
      else
	{
	  HP_TIMING_NOW (start);
	  _dl_map_object (NULL, rtld_progname, lt_executable, 0,
			  __RTLD_OPENEXEC, LM_ID_BASE);
	  HP_TIMING_NOW (stop);

	  HP_TIMING_DIFF (load_time, start, stop);
	}

      /* Now the map for the main executable is available.  */
      main_map = GL(dl_ns)[LM_ID_BASE]._ns_loaded;

      if (__builtin_expect (mode, normal) == normal
	  && GL(dl_rtld_map).l_info[DT_SONAME] != NULL
	  && main_map->l_info[DT_SONAME] != NULL
	  && strcmp ((const char *) D_PTR (&GL(dl_rtld_map), l_info[DT_STRTAB])
		     + GL(dl_rtld_map).l_info[DT_SONAME]->d_un.d_val,
		     (const char *) D_PTR (main_map, l_info[DT_STRTAB])
		     + main_map->l_info[DT_SONAME]->d_un.d_val) == 0)
	_dl_fatal_printf ("loader cannot load itself\n");

      phdr = main_map->l_phdr;
      phnum = main_map->l_phnum;
      /* We overwrite here a pointer to a malloc()ed string.  But since
	 the malloc() implementation used at this point is the dummy
	 implementations which has no real free() function it does not
	 makes sense to free the old string first.  */
      main_map->l_name = (char *) "";
      *user_entry = main_map->l_entry;

#ifdef HAVE_AUX_VECTOR
      /* Adjust the on-stack auxiliary vector so that it looks like the
	 binary was executed directly.  */
      for (ElfW(auxv_t) *av = auxv; av->a_type != AT_NULL; av++)
	switch (av->a_type)
	  {
	  case AT_PHDR:
	    av->a_un.a_val = (uintptr_t) phdr;
	    break;
	  case AT_PHNUM:
	    av->a_un.a_val = phnum;
	    break;
	  case AT_ENTRY:
	    av->a_un.a_val = *user_entry;
	    break;
	  case AT_EXECFN:
	    av->a_un.a_val = (uintptr_t) _dl_argv[0];
	    break;
	  }
#endif
    }
  else
    {
      /* Create a link_map for the executable itself.
	 This will be what dlopen on "" returns.  */
      main_map = _dl_new_object ((char *) "", "", lt_executable, NULL,
				 __RTLD_OPENEXEC, LM_ID_BASE);
      assert (main_map != NULL);
      main_map->l_phdr = phdr;
      main_map->l_phnum = phnum;
      main_map->l_entry = *user_entry;

      /* Even though the link map is not yet fully initialized we can add
	 it to the map list since there are no possible users running yet.  */
      _dl_add_to_namespace_list (main_map, LM_ID_BASE);
      assert (main_map == GL(dl_ns)[LM_ID_BASE]._ns_loaded);

      /* At this point we are in a bit of trouble.  We would have to
	 fill in the values for l_dev and l_ino.  But in general we
	 do not know where the file is.  We also do not handle AT_EXECFD
	 even if it would be passed up.

	 We leave the values here defined to 0.  This is normally no
	 problem as the program code itself is normally no shared
	 object and therefore cannot be loaded dynamically.  Nothing
	 prevent the use of dynamic binaries and in these situations
	 we might get problems.  We might not be able to find out
	 whether the object is already loaded.  But since there is no
	 easy way out and because the dynamic binary must also not
	 have an SONAME we ignore this program for now.  If it becomes
	 a problem we can force people using SONAMEs.  */

      /* We delay initializing the path structure until we got the dynamic
	 information for the program.  */
    }

  main_map->l_map_end = 0;
  main_map->l_text_end = 0;
  /* Perhaps the executable has no PT_LOAD header entries at all.  */
  main_map->l_map_start = ~0;
  /* And it was opened directly.  */
  ++main_map->l_direct_opencount;

  /* Scan the program header table for the dynamic section.  */
  for (ph = phdr; ph < &phdr[phnum]; ++ph)
    switch (ph->p_type)
      {
      case PT_PHDR:
	/* Find out the load address.  */
	main_map->l_addr = (ElfW(Addr)) phdr - ph->p_vaddr;
	break;
      case PT_DYNAMIC:
	/* This tells us where to find the dynamic section,
	   which tells us everything we need to do.  */
	main_map->l_ld = (void *) main_map->l_addr + ph->p_vaddr;
	break;
      case PT_INTERP:
	/* This "interpreter segment" was used by the program loader to
	   find the program interpreter, which is this program itself, the
	   dynamic linker.  We note what name finds us, so that a future
	   dlopen call or DT_NEEDED entry, for something that wants to link
	   against the dynamic linker as a shared library, will know that
	   the shared object is already loaded.  */
	_dl_rtld_libname.name = ((const char *) main_map->l_addr
				 + ph->p_vaddr);
	/* _dl_rtld_libname.next = NULL;	Already zero.  */
	GL(dl_rtld_map).l_libname = &_dl_rtld_libname;

	/* Ordinarilly, we would get additional names for the loader from
	   our DT_SONAME.  This can't happen if we were actually linked as
	   a static executable (detect this case when we have no DYNAMIC).
	   If so, assume the filename component of the interpreter path to
	   be our SONAME, and add it to our name list.  */
	if (GL(dl_rtld_map).l_ld == NULL)
	  {
	    const char *p = NULL;
	    const char *cp = _dl_rtld_libname.name;

	    /* Find the filename part of the path.  */
	    while (*cp != '\0')
	      if (*cp++ == '/')
		p = cp;

	    if (p != NULL)
	      {
		_dl_rtld_libname2.name = p;
		/* _dl_rtld_libname2.next = NULL;  Already zero.  */
		_dl_rtld_libname.next = &_dl_rtld_libname2;
	      }
	  }

	has_interp = true;
	break;
      case PT_LOAD:
	{
	  ElfW(Addr) mapstart;
	  ElfW(Addr) allocend;

	  /* Remember where the main program starts in memory.  */
	  mapstart = (main_map->l_addr
		      + (ph->p_vaddr & ~(GLRO(dl_pagesize) - 1)));
	  if (main_map->l_map_start > mapstart)
	    main_map->l_map_start = mapstart;

	  /* Also where it ends.  */
	  allocend = main_map->l_addr + ph->p_vaddr + ph->p_memsz;
	  if (main_map->l_map_end < allocend)
	    main_map->l_map_end = allocend;
	  if ((ph->p_flags & PF_X) && allocend > main_map->l_text_end)
	    main_map->l_text_end = allocend;
	}
	break;

      case PT_TLS:
	if (ph->p_memsz > 0)
	  {
	    /* Note that in the case the dynamic linker we duplicate work
	       here since we read the PT_TLS entry already in
	       _dl_start_final.  But the result is repeatable so do not
	       check for this special but unimportant case.  */
	    main_map->l_tls_blocksize = ph->p_memsz;
	    main_map->l_tls_align = ph->p_align;
	    if (ph->p_align == 0)
	      main_map->l_tls_firstbyte_offset = 0;
	    else
	      main_map->l_tls_firstbyte_offset = (ph->p_vaddr
						  & (ph->p_align - 1));
	    main_map->l_tls_initimage_size = ph->p_filesz;
	    main_map->l_tls_initimage = (void *) ph->p_vaddr;

	    /* This image gets the ID one.  */
	    GL(dl_tls_max_dtv_idx) = main_map->l_tls_modid = 1;
	  }
	break;

      case PT_GNU_STACK:
	GL(dl_stack_flags) = ph->p_flags;
	break;

      case PT_GNU_RELRO:
	main_map->l_relro_addr = ph->p_vaddr;
	main_map->l_relro_size = ph->p_memsz;
	break;
      }

  /* Adjust the address of the TLS initialization image in case
     the executable is actually an ET_DYN object.  */
  if (main_map->l_tls_initimage != NULL)
    main_map->l_tls_initimage
      = (char *) main_map->l_tls_initimage + main_map->l_addr;
  if (! main_map->l_map_end)
    main_map->l_map_end = ~0;
  if (! main_map->l_text_end)
    main_map->l_text_end = ~0;
  if (! GL(dl_rtld_map).l_libname && GL(dl_rtld_map).l_name)
    {
      /* We were invoked directly, so the program might not have a
	 PT_INTERP.  */
      _dl_rtld_libname.name = GL(dl_rtld_map).l_name;
      /* _dl_rtld_libname.next = NULL;	Already zero.  */
      GL(dl_rtld_map).l_libname =  &_dl_rtld_libname;
    }
  else
    assert (GL(dl_rtld_map).l_libname); /* How else did we get here?  */

  /* If the current libname is different from the SONAME, add the
     latter as well.  */
  if (GL(dl_rtld_map).l_info[DT_SONAME] != NULL
      && strcmp (GL(dl_rtld_map).l_libname->name,
		 (const char *) D_PTR (&GL(dl_rtld_map), l_info[DT_STRTAB])
		 + GL(dl_rtld_map).l_info[DT_SONAME]->d_un.d_val) != 0)
    {
      static struct libname_list newname;
      newname.name = ((char *) D_PTR (&GL(dl_rtld_map), l_info[DT_STRTAB])
		      + GL(dl_rtld_map).l_info[DT_SONAME]->d_un.d_ptr);
      newname.next = NULL;
      newname.dont_free = 1;

      assert (GL(dl_rtld_map).l_libname->next == NULL);
      GL(dl_rtld_map).l_libname->next = &newname;
    }
  /* The ld.so must be relocated since otherwise loading audit modules
     will fail since they reuse the very same ld.so.  */
  assert (GL(dl_rtld_map).l_relocated);

  if (! rtld_is_main)
    {
      /* Extract the contents of the dynamic section for easy access.  */
      elf_get_dynamic_info (main_map, NULL);
      /* Set up our cache of pointers into the hash table.  */
      _dl_setup_hash (main_map);
    }

  if (__builtin_expect (mode, normal) == verify)
    {
      /* We were called just to verify that this is a dynamic
	 executable using us as the program interpreter.  Exit with an
	 error if we were not able to load the binary or no interpreter
	 is specified (i.e., this is no dynamically linked binary.  */
      if (main_map->l_ld == NULL)
	_exit (1);

      /* We allow here some platform specific code.  */
#ifdef DISTINGUISH_LIB_VERSIONS
      DISTINGUISH_LIB_VERSIONS;
#endif
      _exit (has_interp ? 0 : 2);
    }

  struct link_map **first_preload = &GL(dl_rtld_map).l_next;
  /* Set up the data structures for the system-supplied DSO early,
     so they can influence _dl_init_paths.  */
  setup_vdso (main_map, &first_preload);

#ifdef DL_SYSDEP_OSCHECK
  DL_SYSDEP_OSCHECK (_dl_fatal_printf);
#endif

  /* Initialize the data structures for the search paths for shared
     objects.  */
  _dl_init_paths (library_path);

  /* Initialize _r_debug.  */
  struct r_debug *r = _dl_debug_initialize (GL(dl_rtld_map).l_addr,
					    LM_ID_BASE);
  r->r_state = RT_CONSISTENT;

  /* Put the link_map for ourselves on the chain so it can be found by
     name.  Note that at this point the global chain of link maps contains
     exactly one element, which is pointed to by dl_loaded.  */
  if (! GL(dl_rtld_map).l_name)
    /* If not invoked directly, the dynamic linker shared object file was
       found by the PT_INTERP name.  */
    GL(dl_rtld_map).l_name = (char *) GL(dl_rtld_map).l_libname->name;
  GL(dl_rtld_map).l_type = lt_library;
  main_map->l_next = &GL(dl_rtld_map);
  GL(dl_rtld_map).l_prev = main_map;
  ++GL(dl_ns)[LM_ID_BASE]._ns_nloaded;
  ++GL(dl_load_adds);

  /* If LD_USE_LOAD_BIAS env variable has not been seen, default
     to not using bias for non-prelinked PIEs and libraries
     and using it for executables or prelinked PIEs or libraries.  */
  if (GLRO(dl_use_load_bias) == (ElfW(Addr)) -2)
    GLRO(dl_use_load_bias) = main_map->l_addr == 0 ? -1 : 0;

  /* Set up the program header information for the dynamic linker
     itself.  It is needed in the dl_iterate_phdr callbacks.  */
  const ElfW(Ehdr) *rtld_ehdr;

  /* Starting from binutils-2.23, the linker will define the magic symbol
     __ehdr_start to point to our own ELF header if it is visible in a
     segment that also includes the phdrs.  If that's not available, we use
     the old method that assumes the beginning of the file is part of the
     lowest-addressed PT_LOAD segment.  */
#ifdef HAVE_EHDR_START
  extern const ElfW(Ehdr) __ehdr_start __attribute__ ((visibility ("hidden")));
  rtld_ehdr = &__ehdr_start;
#else
  rtld_ehdr = (void *) GL(dl_rtld_map).l_map_start;
#endif
  assert (rtld_ehdr->e_ehsize == sizeof *rtld_ehdr);
  assert (rtld_ehdr->e_phentsize == sizeof (ElfW(Phdr)));

  const ElfW(Phdr) *rtld_phdr = (const void *) rtld_ehdr + rtld_ehdr->e_phoff;

  GL(dl_rtld_map).l_phdr = rtld_phdr;
  GL(dl_rtld_map).l_phnum = rtld_ehdr->e_phnum;


  /* PT_GNU_RELRO is usually the last phdr.  */
  size_t cnt = rtld_ehdr->e_phnum;
  while (cnt-- > 0)
    if (rtld_phdr[cnt].p_type == PT_GNU_RELRO)
      {
	GL(dl_rtld_map).l_relro_addr = rtld_phdr[cnt].p_vaddr;
	GL(dl_rtld_map).l_relro_size = rtld_phdr[cnt].p_memsz;
	break;
      }

  /* Add the dynamic linker to the TLS list if it also uses TLS.  */
  if (GL(dl_rtld_map).l_tls_blocksize != 0)
    /* Assign a module ID.  Do this before loading any audit modules.  */
    GL(dl_rtld_map).l_tls_modid = _dl_next_tls_modid ();

  /* If we have auditing DSOs to load, do it now.  */
  bool need_security_init = true;
  if (__glibc_unlikely (audit_list != NULL)
      || __glibc_unlikely (audit_list_string != NULL))
    {
      struct audit_ifaces *last_audit = NULL;
      struct audit_list_iter al_iter;
      audit_list_iter_init (&al_iter);

      /* Since we start using the auditing DSOs right away we need to
	 initialize the data structures now.  */
      tcbp = init_tls ();

      /* Initialize security features.  We need to do it this early
	 since otherwise the constructors of the audit libraries will
	 use different values (especially the pointer guard) and will
	 fail later on.  */
      security_init ();
      need_security_init = false;

      while (true)
	{
	  const char *name = audit_list_iter_next (&al_iter);
	  if (name == NULL)
	    break;

	  int tls_idx = GL(dl_tls_max_dtv_idx);

	  /* Now it is time to determine the layout of the static TLS
	     block and allocate it for the initial thread.  Note that we
	     always allocate the static block, we never defer it even if
	     no DF_STATIC_TLS bit is set.  The reason is that we know
	     glibc will use the static model.  */
	  struct dlmopen_args dlmargs;
	  dlmargs.fname = name;
	  dlmargs.map = NULL;

	  const char *objname;
	  const char *err_str = NULL;
	  bool malloced;
	  (void) _dl_catch_error (&objname, &err_str, &malloced, dlmopen_doit,
				  &dlmargs);
	  if (__glibc_unlikely (err_str != NULL))
	    {
	    not_loaded:
	      _dl_error_printf ("\
ERROR: ld.so: object '%s' cannot be loaded as audit interface: %s; ignored.\n",
				name, err_str);
	      if (malloced)
		free ((char *) err_str);
	    }
	  else
	    {
	      struct lookup_args largs;
	      largs.name = "la_version";
	      largs.map = dlmargs.map;

	      /* Check whether the interface version matches.  */
	      (void) _dl_catch_error (&objname, &err_str, &malloced,
				      lookup_doit, &largs);

	      unsigned int (*laversion) (unsigned int);
	      unsigned int lav;
	      if  (err_str == NULL
		   && (laversion = largs.result) != NULL
		   && (lav = laversion (LAV_CURRENT)) > 0
		   && lav <= LAV_CURRENT)
		{
		  /* Allocate structure for the callback function pointers.
		     This call can never fail.  */
		  union
		  {
		    struct audit_ifaces ifaces;
#define naudit_ifaces 8
		    void (*fptr[naudit_ifaces]) (void);
		  } *newp = malloc (sizeof (*newp));

		  /* Names of the auditing interfaces.  All in one
		     long string.  */
		  static const char audit_iface_names[] =
		    "la_activity\0"
		    "la_objsearch\0"
		    "la_objopen\0"
		    "la_preinit\0"
#if __ELF_NATIVE_CLASS == 32
		    "la_symbind32\0"
#elif __ELF_NATIVE_CLASS == 64
		    "la_symbind64\0"
#else
# error "__ELF_NATIVE_CLASS must be defined"
#endif
#define STRING(s) __STRING (s)
		    "la_" STRING (ARCH_LA_PLTENTER) "\0"
		    "la_" STRING (ARCH_LA_PLTEXIT) "\0"
		    "la_objclose\0";
		  unsigned int cnt = 0;
		  const char *cp = audit_iface_names;
		  do
		    {
		      largs.name = cp;
		      (void) _dl_catch_error (&objname, &err_str, &malloced,
					      lookup_doit, &largs);

		      /* Store the pointer.  */
		      if (err_str == NULL && largs.result != NULL)
			{
			  newp->fptr[cnt] = largs.result;

			  /* The dynamic linker link map is statically
			     allocated, initialize the data now.   */
			  GL(dl_rtld_map).l_audit[cnt].cookie
			    = (intptr_t) &GL(dl_rtld_map);
			}
		      else
			newp->fptr[cnt] = NULL;
		      ++cnt;

		      cp = (char *) rawmemchr (cp, '\0') + 1;
		    }
		  while (*cp != '\0');
		  assert (cnt == naudit_ifaces);

		  /* Now append the new auditing interface to the list.  */
		  newp->ifaces.next = NULL;
		  if (last_audit == NULL)
		    last_audit = GLRO(dl_audit) = &newp->ifaces;
		  else
		    last_audit = last_audit->next = &newp->ifaces;
		  ++GLRO(dl_naudit);

		  /* Mark the DSO as being used for auditing.  */
		  dlmargs.map->l_auditing = 1;
		}
	      else
		{
		  /* We cannot use the DSO, it does not have the
		     appropriate interfaces or it expects something
		     more recent.  */
#ifndef NDEBUG
		  Lmid_t ns = dlmargs.map->l_ns;
#endif
		  _dl_close (dlmargs.map);

		  /* Make sure the namespace has been cleared entirely.  */
		  assert (GL(dl_ns)[ns]._ns_loaded == NULL);
		  assert (GL(dl_ns)[ns]._ns_nloaded == 0);

		  GL(dl_tls_max_dtv_idx) = tls_idx;
		  goto not_loaded;
		}
	    }
	}

      /* If we have any auditing modules, announce that we already
	 have two objects loaded.  */
      if (__glibc_unlikely (GLRO(dl_naudit) > 0))
	{
	  struct link_map *ls[2] = { main_map, &GL(dl_rtld_map) };

	  for (unsigned int outer = 0; outer < 2; ++outer)
	    {
	      struct audit_ifaces *afct = GLRO(dl_audit);
	      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
		{
		  if (afct->objopen != NULL)
		    {
		      ls[outer]->l_audit[cnt].bindflags
			= afct->objopen (ls[outer], LM_ID_BASE,
					 &ls[outer]->l_audit[cnt].cookie);

		      ls[outer]->l_audit_any_plt
			|= ls[outer]->l_audit[cnt].bindflags != 0;
		    }

		  afct = afct->next;
		}
	    }
	}
    }

  /* Keep track of the currently loaded modules to count how many
     non-audit modules which use TLS are loaded.  */
  size_t count_modids = _dl_count_modids ();

  /* Set up debugging before the debugger is notified for the first time.  */
#ifdef ELF_MACHINE_DEBUG_SETUP
  /* Some machines (e.g. MIPS) don't use DT_DEBUG in this way.  */
  ELF_MACHINE_DEBUG_SETUP (main_map, r);
  ELF_MACHINE_DEBUG_SETUP (&GL(dl_rtld_map), r);
#else
  if (main_map->l_info[DT_DEBUG] != NULL)
    /* There is a DT_DEBUG entry in the dynamic section.  Fill it in
       with the run-time address of the r_debug structure  */
    main_map->l_info[DT_DEBUG]->d_un.d_ptr = (ElfW(Addr)) r;

  /* Fill in the pointer in the dynamic linker's own dynamic section, in
     case you run gdb on the dynamic linker directly.  */
  if (GL(dl_rtld_map).l_info[DT_DEBUG] != NULL)
    GL(dl_rtld_map).l_info[DT_DEBUG]->d_un.d_ptr = (ElfW(Addr)) r;
#endif

  /* We start adding objects.  */
  r->r_state = RT_ADD;
  _dl_debug_state ();
  LIBC_PROBE (init_start, 2, LM_ID_BASE, r);

  /* Auditing checkpoint: we are ready to signal that the initial map
     is being constructed.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0))
    {
      struct audit_ifaces *afct = GLRO(dl_audit);
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->activity != NULL)
	    afct->activity (&main_map->l_audit[cnt].cookie, LA_ACT_ADD);

	  afct = afct->next;
	}
    }

  /* We have two ways to specify objects to preload: via environment
     variable and via the file /etc/ld.so.preload.  The latter can also
     be used when security is enabled.  */
  assert (*first_preload == NULL);
  struct link_map **preloads = NULL;
  unsigned int npreloads = 0;

  if (__glibc_unlikely (preloadlist != NULL))
    {
      HP_TIMING_NOW (start);
      npreloads += handle_ld_preload (preloadlist, main_map);
      HP_TIMING_NOW (stop);
      HP_TIMING_DIFF (diff, start, stop);
      HP_TIMING_ACCUM_NT (load_time, diff);
    }

  /* There usually is no ld.so.preload file, it should only be used
     for emergencies and testing.  So the open call etc should usually
     fail.  Using access() on a non-existing file is faster than using
     open().  So we do this first.  If it succeeds we do almost twice
     the work but this does not matter, since it is not for production
     use.  */
  static const char preload_file[] = "/etc/ld.so.preload";
  if (__glibc_unlikely (__access (preload_file, R_OK) == 0))
    {
      /* Read the contents of the file.  */
      file = _dl_sysdep_read_whole_file (preload_file, &file_size,
					 PROT_READ | PROT_WRITE);
      if (__glibc_unlikely (file != MAP_FAILED))
	{
	  /* Parse the file.  It contains names of libraries to be loaded,
	     separated by white spaces or `:'.  It may also contain
	     comments introduced by `#'.  */
	  char *problem;
	  char *runp;
	  size_t rest;

	  /* Eliminate comments.  */
	  runp = file;
	  rest = file_size;
	  while (rest > 0)
	    {
	      char *comment = memchr (runp, '#', rest);
	      if (comment == NULL)
		break;

	      rest -= comment - runp;
	      do
		*comment = ' ';
	      while (--rest > 0 && *++comment != '\n');
	    }

	  /* We have one problematic case: if we have a name at the end of
	     the file without a trailing terminating characters, we cannot
	     place the \0.  Handle the case separately.  */
	  if (file[file_size - 1] != ' ' && file[file_size - 1] != '\t'
	      && file[file_size - 1] != '\n' && file[file_size - 1] != ':')
	    {
	      problem = &file[file_size];
	      while (problem > file && problem[-1] != ' '
		     && problem[-1] != '\t'
		     && problem[-1] != '\n' && problem[-1] != ':')
		--problem;

	      if (problem > file)
		problem[-1] = '\0';
	    }
	  else
	    {
	      problem = NULL;
	      file[file_size - 1] = '\0';
	    }

	  HP_TIMING_NOW (start);

	  if (file != problem)
	    {
	      char *p;
	      runp = file;
	      while ((p = strsep (&runp, ": \t\n")) != NULL)
		if (p[0] != '\0')
		  npreloads += do_preload (p, main_map, preload_file);
	    }

	  if (problem != NULL)
	    {
	      char *p = strndupa (problem, file_size - (problem - file));

	      npreloads += do_preload (p, main_map, preload_file);
	    }

	  HP_TIMING_NOW (stop);
	  HP_TIMING_DIFF (diff, start, stop);
	  HP_TIMING_ACCUM_NT (load_time, diff);

	  /* We don't need the file anymore.  */
	  __munmap (file, file_size);
	}
    }

  if (__glibc_unlikely (*first_preload != NULL))
    {
      /* Set up PRELOADS with a vector of the preloaded libraries.  */
      struct link_map *l = *first_preload;
      preloads = __alloca (npreloads * sizeof preloads[0]);
      i = 0;
      do
	{
	  preloads[i++] = l;
	  l = l->l_next;
	} while (l);
      assert (i == npreloads);
    }

  /* Load all the libraries specified by DT_NEEDED entries.  If LD_PRELOAD
     specified some libraries to load, these are inserted before the actual
     dependencies in the executable's searchlist for symbol resolution.  */
  HP_TIMING_NOW (start);
  _dl_map_object_deps (main_map, preloads, npreloads, mode == trace, 0);
  HP_TIMING_NOW (stop);
  HP_TIMING_DIFF (diff, start, stop);
  HP_TIMING_ACCUM_NT (load_time, diff);

  /* Mark all objects as being in the global scope.  */
  for (i = main_map->l_searchlist.r_nlist; i > 0; )
    main_map->l_searchlist.r_list[--i]->l_global = 1;

  /* Remove _dl_rtld_map from the chain.  */
  GL(dl_rtld_map).l_prev->l_next = GL(dl_rtld_map).l_next;
  if (GL(dl_rtld_map).l_next != NULL)
    GL(dl_rtld_map).l_next->l_prev = GL(dl_rtld_map).l_prev;

  for (i = 1; i < main_map->l_searchlist.r_nlist; ++i)
    if (main_map->l_searchlist.r_list[i] == &GL(dl_rtld_map))
      break;

  bool rtld_multiple_ref = false;
  if (__glibc_likely (i < main_map->l_searchlist.r_nlist))
    {
      /* Some DT_NEEDED entry referred to the interpreter object itself, so
	 put it back in the list of visible objects.  We insert it into the
	 chain in symbol search order because gdb uses the chain's order as
	 its symbol search order.  */
      rtld_multiple_ref = true;

      GL(dl_rtld_map).l_prev = main_map->l_searchlist.r_list[i - 1];
      if (__builtin_expect (mode, normal) == normal)
	{
	  GL(dl_rtld_map).l_next = (i + 1 < main_map->l_searchlist.r_nlist
				    ? main_map->l_searchlist.r_list[i + 1]
				    : NULL);
#ifdef NEED_DL_SYSINFO_DSO
	  if (GLRO(dl_sysinfo_map) != NULL
	      && GL(dl_rtld_map).l_prev->l_next == GLRO(dl_sysinfo_map)
	      && GL(dl_rtld_map).l_next != GLRO(dl_sysinfo_map))
	    GL(dl_rtld_map).l_prev = GLRO(dl_sysinfo_map);
#endif
	}
      else
	/* In trace mode there might be an invisible object (which we
	   could not find) after the previous one in the search list.
	   In this case it doesn't matter much where we put the
	   interpreter object, so we just initialize the list pointer so
	   that the assertion below holds.  */
	GL(dl_rtld_map).l_next = GL(dl_rtld_map).l_prev->l_next;

      assert (GL(dl_rtld_map).l_prev->l_next == GL(dl_rtld_map).l_next);
      GL(dl_rtld_map).l_prev->l_next = &GL(dl_rtld_map);
      if (GL(dl_rtld_map).l_next != NULL)
	{
	  assert (GL(dl_rtld_map).l_next->l_prev == GL(dl_rtld_map).l_prev);
	  GL(dl_rtld_map).l_next->l_prev = &GL(dl_rtld_map);
	}
    }

  /* Now let us see whether all libraries are available in the
     versions we need.  */
  {
    struct version_check_args args;
    args.doexit = mode == normal;
    args.dotrace = mode == trace;
    _dl_receive_error (print_missing_version, version_check_doit, &args);
  }

  /* We do not initialize any of the TLS functionality unless any of the
     initial modules uses TLS.  This makes dynamic loading of modules with
     TLS impossible, but to support it requires either eagerly doing setup
     now or lazily doing it later.  Doing it now makes us incompatible with
     an old kernel that can't perform TLS_INIT_TP, even if no TLS is ever
     used.  Trying to do it lazily is too hairy to try when there could be
     multiple threads (from a non-TLS-using libpthread).  */
  bool was_tls_init_tp_called = tls_init_tp_called;
  if (tcbp == NULL)
    tcbp = init_tls ();

  if (__glibc_likely (need_security_init))
    /* Initialize security features.  But only if we have not done it
       earlier.  */
    security_init ();

  if (__builtin_expect (mode, normal) != normal)
    {
      /* We were run just to list the shared libraries.  It is
	 important that we do this before real relocation, because the
	 functions we call below for output may no longer work properly
	 after relocation.  */
      struct link_map *l;

      if (GLRO(dl_debug_mask) & DL_DEBUG_PRELINK)
	{
	  struct r_scope_elem *scope = &main_map->l_searchlist;

	  for (i = 0; i < scope->r_nlist; i++)
	    {
	      l = scope->r_list [i];
	      if (l->l_faked)
		{
		  _dl_printf ("\t%s => not found\n", l->l_libname->name);
		  continue;
		}
	      if (_dl_name_match_p (GLRO(dl_trace_prelink), l))
		GLRO(dl_trace_prelink_map) = l;
	      _dl_printf ("\t%s => %s (0x%0*Zx, 0x%0*Zx)",
			  DSO_FILENAME (l->l_libname->name),
			  DSO_FILENAME (l->l_name),
			  (int) sizeof l->l_map_start * 2,
			  (size_t) l->l_map_start,
			  (int) sizeof l->l_addr * 2,
			  (size_t) l->l_addr);

	      if (l->l_tls_modid)
		_dl_printf (" TLS(0x%Zx, 0x%0*Zx)\n", l->l_tls_modid,
			    (int) sizeof l->l_tls_offset * 2,
			    (size_t) l->l_tls_offset);
	      else
		_dl_printf ("\n");
	    }
	}
      else if (GLRO(dl_debug_mask) & DL_DEBUG_UNUSED)
	{
	  /* Look through the dependencies of the main executable
	     and determine which of them is not actually
	     required.  */
	  struct link_map *l = main_map;

	  /* Relocate the main executable.  */
	  struct relocate_args args = { .l = l,
					.reloc_mode = ((GLRO(dl_lazy)
						       ? RTLD_LAZY : 0)
						       | __RTLD_NOIFUNC) };
	  _dl_receive_error (print_unresolved, relocate_doit, &args);

	  /* This loop depends on the dependencies of the executable to
	     correspond in number and order to the DT_NEEDED entries.  */
	  ElfW(Dyn) *dyn = main_map->l_ld;
	  bool first = true;
	  while (dyn->d_tag != DT_NULL)
	    {
	      if (dyn->d_tag == DT_NEEDED)
		{
		  l = l->l_next;
#ifdef NEED_DL_SYSINFO_DSO
		  /* Skip the VDSO since it's not part of the list
		     of objects we brought in via DT_NEEDED entries.  */
		  if (l == GLRO(dl_sysinfo_map))
		    l = l->l_next;
#endif
		  if (!l->l_used)
		    {
		      if (first)
			{
			  _dl_printf ("Unused direct dependencies:\n");
			  first = false;
			}

		      _dl_printf ("\t%s\n", l->l_name);
		    }
		}

	      ++dyn;
	    }

	  _exit (first != true);
	}
      else if (! main_map->l_info[DT_NEEDED])
	_dl_printf ("\tstatically linked\n");
      else
	{
	  for (l = main_map->l_next; l; l = l->l_next)
	    if (l->l_faked)
	      /* The library was not found.  */
	      _dl_printf ("\t%s => not found\n", l->l_libname->name);
	    else if (strcmp (l->l_libname->name, l->l_name) == 0)
	      _dl_printf ("\t%s (0x%0*Zx)\n", l->l_libname->name,
			  (int) sizeof l->l_map_start * 2,
			  (size_t) l->l_map_start);
	    else
	      _dl_printf ("\t%s => %s (0x%0*Zx)\n", l->l_libname->name,
			  l->l_name, (int) sizeof l->l_map_start * 2,
			  (size_t) l->l_map_start);
	}

      if (__builtin_expect (mode, trace) != trace)
	for (i = 1; i < (unsigned int) _dl_argc; ++i)
	  {
	    const ElfW(Sym) *ref = NULL;
	    ElfW(Addr) loadbase;
	    lookup_t result;

	    result = _dl_lookup_symbol_x (_dl_argv[i], main_map,
					  &ref, main_map->l_scope,
					  NULL, ELF_RTYPE_CLASS_PLT,
					  DL_LOOKUP_ADD_DEPENDENCY, NULL);

	    loadbase = LOOKUP_VALUE_ADDRESS (result);

	    _dl_printf ("%s found at 0x%0*Zd in object at 0x%0*Zd\n",
			_dl_argv[i],
			(int) sizeof ref->st_value * 2,
			(size_t) ref->st_value,
			(int) sizeof loadbase * 2, (size_t) loadbase);
	  }
      else
	{
	  /* If LD_WARN is set, warn about undefined symbols.  */
	  if (GLRO(dl_lazy) >= 0 && GLRO(dl_verbose))
	    {
	      /* We have to do symbol dependency testing.  */
	      struct relocate_args args;
	      unsigned int i;

	      args.reloc_mode = ((GLRO(dl_lazy) ? RTLD_LAZY : 0)
				 | __RTLD_NOIFUNC);

	      i = main_map->l_searchlist.r_nlist;
	      while (i-- > 0)
		{
		  struct link_map *l = main_map->l_initfini[i];
		  if (l != &GL(dl_rtld_map) && ! l->l_faked)
		    {
		      args.l = l;
		      _dl_receive_error (print_unresolved, relocate_doit,
					 &args);
		    }
		}

	      if ((GLRO(dl_debug_mask) & DL_DEBUG_PRELINK)
		  && rtld_multiple_ref)
		{
		  /* Mark the link map as not yet relocated again.  */
		  GL(dl_rtld_map).l_relocated = 0;
		  _dl_relocate_object (&GL(dl_rtld_map),
				       main_map->l_scope, __RTLD_NOIFUNC, 0);
		}
	    }
#define VERNEEDTAG (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (DT_VERNEED))
	  if (version_info)
	    {
	      /* Print more information.  This means here, print information
		 about the versions needed.  */
	      int first = 1;
	      struct link_map *map;

	      for (map = main_map; map != NULL; map = map->l_next)
		{
		  const char *strtab;
		  ElfW(Dyn) *dyn = map->l_info[VERNEEDTAG];
		  ElfW(Verneed) *ent;

		  if (dyn == NULL)
		    continue;

		  strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
		  ent = (ElfW(Verneed) *) (map->l_addr + dyn->d_un.d_ptr);

		  if (first)
		    {
		      _dl_printf ("\n\tVersion information:\n");
		      first = 0;
		    }

		  _dl_printf ("\t%s:\n", DSO_FILENAME (map->l_name));

		  while (1)
		    {
		      ElfW(Vernaux) *aux;
		      struct link_map *needed;

		      needed = find_needed (strtab + ent->vn_file);
		      aux = (ElfW(Vernaux) *) ((char *) ent + ent->vn_aux);

		      while (1)
			{
			  const char *fname = NULL;

			  if (needed != NULL
			      && match_version (strtab + aux->vna_name,
						needed))
			    fname = needed->l_name;

			  _dl_printf ("\t\t%s (%s) %s=> %s\n",
				      strtab + ent->vn_file,
				      strtab + aux->vna_name,
				      aux->vna_flags & VER_FLG_WEAK
				      ? "[WEAK] " : "",
				      fname ?: "not found");

			  if (aux->vna_next == 0)
			    /* No more symbols.  */
			    break;

			  /* Next symbol.  */
			  aux = (ElfW(Vernaux) *) ((char *) aux
						   + aux->vna_next);
			}

		      if (ent->vn_next == 0)
			/* No more dependencies.  */
			break;

		      /* Next dependency.  */
		      ent = (ElfW(Verneed) *) ((char *) ent + ent->vn_next);
		    }
		}
	    }
	}

      _exit (0);
    }

  if (main_map->l_info[ADDRIDX (DT_GNU_LIBLIST)]
      && ! __builtin_expect (GLRO(dl_profile) != NULL, 0)
      && ! __builtin_expect (GLRO(dl_dynamic_weak), 0))
    {
      ElfW(Lib) *liblist, *liblistend;
      struct link_map **r_list, **r_listend, *l;
      const char *strtab = (const void *) D_PTR (main_map, l_info[DT_STRTAB]);

      assert (main_map->l_info[VALIDX (DT_GNU_LIBLISTSZ)] != NULL);
      liblist = (ElfW(Lib) *)
		main_map->l_info[ADDRIDX (DT_GNU_LIBLIST)]->d_un.d_ptr;
      liblistend = (ElfW(Lib) *)
		   ((char *) liblist +
		    main_map->l_info[VALIDX (DT_GNU_LIBLISTSZ)]->d_un.d_val);
      r_list = main_map->l_searchlist.r_list;
      r_listend = r_list + main_map->l_searchlist.r_nlist;

      for (; r_list < r_listend && liblist < liblistend; r_list++)
	{
	  l = *r_list;

	  if (l == main_map)
	    continue;

	  /* If the library is not mapped where it should, fail.  */
	  if (l->l_addr)
	    break;

	  /* Next, check if checksum matches.  */
	  if (l->l_info [VALIDX(DT_CHECKSUM)] == NULL
	      || l->l_info [VALIDX(DT_CHECKSUM)]->d_un.d_val
		 != liblist->l_checksum)
	    break;

	  if (l->l_info [VALIDX(DT_GNU_PRELINKED)] == NULL
	      || l->l_info [VALIDX(DT_GNU_PRELINKED)]->d_un.d_val
		 != liblist->l_time_stamp)
	    break;

	  if (! _dl_name_match_p (strtab + liblist->l_name, l))
	    break;

	  ++liblist;
	}


      if (r_list == r_listend && liblist == liblistend)
	prelinked = true;

      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
	_dl_debug_printf ("\nprelink checking: %s\n",
			  prelinked ? "ok" : "failed");
    }


  /* Now set up the variable which helps the assembler startup code.  */
  GL(dl_ns)[LM_ID_BASE]._ns_main_searchlist = &main_map->l_searchlist;

  /* Save the information about the original global scope list since
     we need it in the memory handling later.  */
  GLRO(dl_initial_searchlist) = *GL(dl_ns)[LM_ID_BASE]._ns_main_searchlist;

  /* Remember the last search directory added at startup, now that
     malloc will no longer be the one from dl-minimal.c.  As a side
     effect, this marks ld.so as initialized, so that the rtld_active
     function returns true from now on.  */
  GLRO(dl_init_all_dirs) = GL(dl_all_dirs);

  /* Print scope information.  */
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_SCOPES))
    {
      _dl_debug_printf ("\nInitial object scopes\n");

      for (struct link_map *l = main_map; l != NULL; l = l->l_next)
	_dl_show_scope (l, 0);
    }

  if (prelinked)
    {
      if (main_map->l_info [ADDRIDX (DT_GNU_CONFLICT)] != NULL)
	{
	  ElfW(Rela) *conflict, *conflictend;
#ifndef HP_TIMING_NONAVAIL
	  hp_timing_t start;
	  hp_timing_t stop;
#endif

	  HP_TIMING_NOW (start);
	  assert (main_map->l_info [VALIDX (DT_GNU_CONFLICTSZ)] != NULL);
	  conflict = (ElfW(Rela) *)
	    main_map->l_info [ADDRIDX (DT_GNU_CONFLICT)]->d_un.d_ptr;
	  conflictend = (ElfW(Rela) *)
	    ((char *) conflict
	     + main_map->l_info [VALIDX (DT_GNU_CONFLICTSZ)]->d_un.d_val);
	  _dl_resolve_conflicts (main_map, conflict, conflictend);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_DIFF (relocate_time, start, stop);
	}


      /* Mark all the objects so we know they have been already relocated.  */
      for (struct link_map *l = main_map; l != NULL; l = l->l_next)
	{
	  l->l_relocated = 1;
	  if (l->l_relro_size)
	    _dl_protect_relro (l);

	  /* Add object to slot information data if necessasy.  */
	  if (l->l_tls_blocksize != 0 && tls_init_tp_called)
	    _dl_add_to_slotinfo (l);
	}
    }
  else
    {
      /* Now we have all the objects loaded.  Relocate them all except for
	 the dynamic linker itself.  We do this in reverse order so that copy
	 relocs of earlier objects overwrite the data written by later
	 objects.  We do not re-relocate the dynamic linker itself in this
	 loop because that could result in the GOT entries for functions we
	 call being changed, and that would break us.  It is safe to relocate
	 the dynamic linker out of order because it has no copy relocs (we
	 know that because it is self-contained).  */

      int consider_profiling = GLRO(dl_profile) != NULL;
#ifndef HP_TIMING_NONAVAIL
      hp_timing_t start;
      hp_timing_t stop;
#endif

      /* If we are profiling we also must do lazy reloaction.  */
      GLRO(dl_lazy) |= consider_profiling;

      HP_TIMING_NOW (start);
      unsigned i = main_map->l_searchlist.r_nlist;
      while (i-- > 0)
	{
	  struct link_map *l = main_map->l_initfini[i];

	  /* While we are at it, help the memory handling a bit.  We have to
	     mark some data structures as allocated with the fake malloc()
	     implementation in ld.so.  */
	  struct libname_list *lnp = l->l_libname->next;

	  while (__builtin_expect (lnp != NULL, 0))
	    {
	      lnp->dont_free = 1;
	      lnp = lnp->next;
	    }
	  /* Also allocated with the fake malloc().  */
	  l->l_free_initfini = 0;

	  if (l != &GL(dl_rtld_map))
	    _dl_relocate_object (l, l->l_scope, GLRO(dl_lazy) ? RTLD_LAZY : 0,
				 consider_profiling);

	  /* Add object to slot information data if necessasy.  */
	  if (l->l_tls_blocksize != 0 && tls_init_tp_called)
	    _dl_add_to_slotinfo (l);
	}
      HP_TIMING_NOW (stop);

      HP_TIMING_DIFF (relocate_time, start, stop);

      /* Now enable profiling if needed.  Like the previous call,
	 this has to go here because the calls it makes should use the
	 rtld versions of the functions (particularly calloc()), but it
	 needs to have _dl_profile_map set up by the relocator.  */
      if (__glibc_unlikely (GL(dl_profile_map) != NULL))
	/* We must prepare the profiling.  */
	_dl_start_profile ();
    }

  if ((!was_tls_init_tp_called && GL(dl_tls_max_dtv_idx) > 0)
      || count_modids != _dl_count_modids ())
    ++GL(dl_tls_generation);

  /* Now that we have completed relocation, the initializer data
     for the TLS blocks has its final values and we can copy them
     into the main thread's TLS area, which we allocated above.
     Note: thread-local variables must only be accessed after completing
     the next step.  */
  _dl_allocate_tls_init (tcbp);

  /* And finally install it for the main thread.  */
  if (! tls_init_tp_called)
    {
      const char *lossage = TLS_INIT_TP (tcbp);
      if (__glibc_unlikely (lossage != NULL))
	_dl_fatal_printf ("cannot set up thread-local storage: %s\n",
			  lossage);
    }

  /* Make sure no new search directories have been added.  */
  assert (GLRO(dl_init_all_dirs) == GL(dl_all_dirs));

  if (! prelinked && rtld_multiple_ref)
    {
      /* There was an explicit ref to the dynamic linker as a shared lib.
	 Re-relocate ourselves with user-controlled symbol definitions.

	 We must do this after TLS initialization in case after this
	 re-relocation, we might call a user-supplied function
	 (e.g. calloc from _dl_relocate_object) that uses TLS data.  */

#ifndef HP_TIMING_NONAVAIL
      hp_timing_t start;
      hp_timing_t stop;
      hp_timing_t add;
#endif

      HP_TIMING_NOW (start);
      /* Mark the link map as not yet relocated again.  */
      GL(dl_rtld_map).l_relocated = 0;
      _dl_relocate_object (&GL(dl_rtld_map), main_map->l_scope, 0, 0);
      HP_TIMING_NOW (stop);
      HP_TIMING_DIFF (add, start, stop);
      HP_TIMING_ACCUM_NT (relocate_time, add);
    }

  /* Do any necessary cleanups for the startup OS interface code.
     We do these now so that no calls are made after rtld re-relocation
     which might be resolved to different functions than we expect.
     We cannot do this before relocating the other objects because
     _dl_relocate_object might need to call `mprotect' for DT_TEXTREL.  */
  _dl_sysdep_start_cleanup ();

#ifdef SHARED
  /* Auditing checkpoint: we have added all objects.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0))
    {
      struct link_map *head = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
      /* Do not call the functions for any auditing object.  */
      if (head->l_auditing == 0)
	{
	  struct audit_ifaces *afct = GLRO(dl_audit);
	  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	    {
	      if (afct->activity != NULL)
		afct->activity (&head->l_audit[cnt].cookie, LA_ACT_CONSISTENT);

	      afct = afct->next;
	    }
	}
    }
#endif

  /* Notify the debugger all new objects are now ready to go.  We must re-get
     the address since by now the variable might be in another object.  */
  r = _dl_debug_initialize (0, LM_ID_BASE);
  r->r_state = RT_CONSISTENT;
  _dl_debug_state ();
  LIBC_PROBE (init_complete, 2, LM_ID_BASE, r);

#if defined USE_LDCONFIG && !defined MAP_COPY
  /* We must munmap() the cache file.  */
  _dl_unload_cache ();
#endif

  /* Once we return, _dl_sysdep_start will invoke
     the DT_INIT functions and then *USER_ENTRY.  */
}

/* This is a little helper function for resolving symbols while
   tracing the binary.  */
static void
print_unresolved (int errcode __attribute__ ((unused)), const char *objname,
		  const char *errstring)
{
  if (objname[0] == '\0')
    objname = RTLD_PROGNAME;
  _dl_error_printf ("%s	(%s)\n", errstring, objname);
}

/* This is a little helper function for resolving symbols while
   tracing the binary.  */
static void
print_missing_version (int errcode __attribute__ ((unused)),
		       const char *objname, const char *errstring)
{
  _dl_error_printf ("%s: %s: %s\n", RTLD_PROGNAME,
		    objname, errstring);
}

/* Nonzero if any of the debugging options is enabled.  */
static int any_debug attribute_relro;

/* Process the string given as the parameter which explains which debugging
   options are enabled.  */
static void
process_dl_debug (const char *dl_debug)
{
  /* When adding new entries make sure that the maximal length of a name
     is correctly handled in the LD_DEBUG_HELP code below.  */
  static const struct
  {
    unsigned char len;
    const char name[10];
    const char helptext[41];
    unsigned short int mask;
  } debopts[] =
    {
#define LEN_AND_STR(str) sizeof (str) - 1, str
      { LEN_AND_STR ("libs"), "display library search paths",
	DL_DEBUG_LIBS | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("reloc"), "display relocation processing",
	DL_DEBUG_RELOC | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("files"), "display progress for input file",
	DL_DEBUG_FILES | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("symbols"), "display symbol table processing",
	DL_DEBUG_SYMBOLS | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("bindings"), "display information about symbol binding",
	DL_DEBUG_BINDINGS | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("versions"), "display version dependencies",
	DL_DEBUG_VERSIONS | DL_DEBUG_IMPCALLS },
      { LEN_AND_STR ("scopes"), "display scope information",
	DL_DEBUG_SCOPES },
      { LEN_AND_STR ("all"), "all previous options combined",
	DL_DEBUG_LIBS | DL_DEBUG_RELOC | DL_DEBUG_FILES | DL_DEBUG_SYMBOLS
	| DL_DEBUG_BINDINGS | DL_DEBUG_VERSIONS | DL_DEBUG_IMPCALLS
	| DL_DEBUG_SCOPES },
      { LEN_AND_STR ("statistics"), "display relocation statistics",
	DL_DEBUG_STATISTICS },
      { LEN_AND_STR ("unused"), "determined unused DSOs",
	DL_DEBUG_UNUSED },
      { LEN_AND_STR ("help"), "display this help message and exit",
	DL_DEBUG_HELP },
    };
#define ndebopts (sizeof (debopts) / sizeof (debopts[0]))

  /* Skip separating white spaces and commas.  */
  while (*dl_debug != '\0')
    {
      if (*dl_debug != ' ' && *dl_debug != ',' && *dl_debug != ':')
	{
	  size_t cnt;
	  size_t len = 1;

	  while (dl_debug[len] != '\0' && dl_debug[len] != ' '
		 && dl_debug[len] != ',' && dl_debug[len] != ':')
	    ++len;

	  for (cnt = 0; cnt < ndebopts; ++cnt)
	    if (debopts[cnt].len == len
		&& memcmp (dl_debug, debopts[cnt].name, len) == 0)
	      {
		GLRO(dl_debug_mask) |= debopts[cnt].mask;
		any_debug = 1;
		break;
	      }

	  if (cnt == ndebopts)
	    {
	      /* Display a warning and skip everything until next
		 separator.  */
	      char *copy = strndupa (dl_debug, len);
	      _dl_error_printf ("\
warning: debug option `%s' unknown; try LD_DEBUG=help\n", copy);
	    }

	  dl_debug += len;
	  continue;
	}

      ++dl_debug;
    }

  if (GLRO(dl_debug_mask) & DL_DEBUG_UNUSED)
    {
      /* In order to get an accurate picture of whether a particular
	 DT_NEEDED entry is actually used we have to process both
	 the PLT and non-PLT relocation entries.  */
      GLRO(dl_lazy) = 0;
    }

  if (GLRO(dl_debug_mask) & DL_DEBUG_HELP)
    {
      size_t cnt;

      _dl_printf ("\
Valid options for the LD_DEBUG environment variable are:\n\n");

      for (cnt = 0; cnt < ndebopts; ++cnt)
	_dl_printf ("  %.*s%s%s\n", debopts[cnt].len, debopts[cnt].name,
		    "         " + debopts[cnt].len - 3,
		    debopts[cnt].helptext);

      _dl_printf ("\n\
To direct the debugging output into a file instead of standard output\n\
a filename can be specified using the LD_DEBUG_OUTPUT environment variable.\n");
      _exit (0);
    }
}

static void
process_dl_audit (char *str)
{
  /* The parameter is a colon separated list of DSO names.  */
  char *p;

  while ((p = (strsep) (&str, ":")) != NULL)
    if (dso_name_valid_for_suid (p))
      {
	/* This is using the local malloc, not the system malloc.  The
	   memory can never be freed.  */
	struct audit_list *newp = malloc (sizeof (*newp));
	newp->name = p;

	if (audit_list == NULL)
	  audit_list = newp->next = newp;
	else
	  {
	    newp->next = audit_list->next;
	    audit_list = audit_list->next = newp;
	  }
      }
}

/* Process all environments variables the dynamic linker must recognize.
   Since all of them start with `LD_' we are a bit smarter while finding
   all the entries.  */
extern char **_environ attribute_hidden;


static void
process_envvars (enum mode *modep)
{
  char **runp = _environ;
  char *envline;
  enum mode mode = normal;
  char *debug_output = NULL;

  /* This is the default place for profiling data file.  */
  GLRO(dl_profile_output)
    = &"/var/tmp\0/var/profile"[__libc_enable_secure ? 9 : 0];

  while ((envline = _dl_next_ld_env_entry (&runp)) != NULL)
    {
      size_t len = 0;

      while (envline[len] != '\0' && envline[len] != '=')
	++len;

      if (envline[len] != '=')
	/* This is a "LD_" variable at the end of the string without
	   a '=' character.  Ignore it since otherwise we will access
	   invalid memory below.  */
	continue;

      switch (len)
	{
	case 4:
	  /* Warning level, verbose or not.  */
	  if (memcmp (envline, "WARN", 4) == 0)
	    GLRO(dl_verbose) = envline[5] != '\0';
	  break;

	case 5:
	  /* Debugging of the dynamic linker?  */
	  if (memcmp (envline, "DEBUG", 5) == 0)
	    {
	      process_dl_debug (&envline[6]);
	      break;
	    }
	  if (memcmp (envline, "AUDIT", 5) == 0)
	    audit_list_string = &envline[6];
	  break;

	case 7:
	  /* Print information about versions.  */
	  if (memcmp (envline, "VERBOSE", 7) == 0)
	    {
	      version_info = envline[8] != '\0';
	      break;
	    }

	  /* List of objects to be preloaded.  */
	  if (memcmp (envline, "PRELOAD", 7) == 0)
	    {
	      preloadlist = &envline[8];
	      break;
	    }

	  /* Which shared object shall be profiled.  */
	  if (memcmp (envline, "PROFILE", 7) == 0 && envline[8] != '\0')
	    GLRO(dl_profile) = &envline[8];
	  break;

	case 8:
	  /* Do we bind early?  */
	  if (memcmp (envline, "BIND_NOW", 8) == 0)
	    {
	      GLRO(dl_lazy) = envline[9] == '\0';
	      break;
	    }
	  if (memcmp (envline, "BIND_NOT", 8) == 0)
	    GLRO(dl_bind_not) = envline[9] != '\0';
	  break;

	case 9:
	  /* Test whether we want to see the content of the auxiliary
	     array passed up from the kernel.  */
	  if (!__libc_enable_secure
	      && memcmp (envline, "SHOW_AUXV", 9) == 0)
	    _dl_show_auxv ();
	  break;

#if !HAVE_TUNABLES
	case 10:
	  /* Mask for the important hardware capabilities.  */
	  if (!__libc_enable_secure
	      && memcmp (envline, "HWCAP_MASK", 10) == 0)
	    GLRO(dl_hwcap_mask) = _dl_strtoul (&envline[11], NULL);
	  break;
#endif

	case 11:
	  /* Path where the binary is found.  */
	  if (!__libc_enable_secure
	      && memcmp (envline, "ORIGIN_PATH", 11) == 0)
	    GLRO(dl_origin_path) = &envline[12];
	  break;

	case 12:
	  /* The library search path.  */
	  if (!__libc_enable_secure
	      && memcmp (envline, "LIBRARY_PATH", 12) == 0)
	    {
	      library_path = &envline[13];
	      break;
	    }

	  /* Where to place the profiling data file.  */
	  if (memcmp (envline, "DEBUG_OUTPUT", 12) == 0)
	    {
	      debug_output = &envline[13];
	      break;
	    }

	  if (!__libc_enable_secure
	      && memcmp (envline, "DYNAMIC_WEAK", 12) == 0)
	    GLRO(dl_dynamic_weak) = 1;
	  break;

	case 13:
	  /* We might have some extra environment variable with length 13
	     to handle.  */
#ifdef EXTRA_LD_ENVVARS_13
	  EXTRA_LD_ENVVARS_13
#endif
	  if (!__libc_enable_secure
	      && memcmp (envline, "USE_LOAD_BIAS", 13) == 0)
	    {
	      GLRO(dl_use_load_bias) = envline[14] == '1' ? -1 : 0;
	      break;
	    }
	  break;

	case 14:
	  /* Where to place the profiling data file.  */
	  if (!__libc_enable_secure
	      && memcmp (envline, "PROFILE_OUTPUT", 14) == 0
	      && envline[15] != '\0')
	    GLRO(dl_profile_output) = &envline[15];
	  break;

	case 16:
	  /* The mode of the dynamic linker can be set.  */
	  if (memcmp (envline, "TRACE_PRELINKING", 16) == 0)
	    {
	      mode = trace;
	      GLRO(dl_verbose) = 1;
	      GLRO(dl_debug_mask) |= DL_DEBUG_PRELINK;
	      GLRO(dl_trace_prelink) = &envline[17];
	    }
	  break;

	case 20:
	  /* The mode of the dynamic linker can be set.  */
	  if (memcmp (envline, "TRACE_LOADED_OBJECTS", 20) == 0)
	    mode = trace;
	  break;

	  /* We might have some extra environment variable to handle.  This
	     is tricky due to the pre-processing of the length of the name
	     in the switch statement here.  The code here assumes that added
	     environment variables have a different length.  */
#ifdef EXTRA_LD_ENVVARS
	  EXTRA_LD_ENVVARS
#endif
	}
    }

  /* The caller wants this information.  */
  *modep = mode;

  /* Extra security for SUID binaries.  Remove all dangerous environment
     variables.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    {
      static const char unsecure_envvars[] =
#ifdef EXTRA_UNSECURE_ENVVARS
	EXTRA_UNSECURE_ENVVARS
#endif
	UNSECURE_ENVVARS;
      const char *nextp;

      nextp = unsecure_envvars;
      do
	{
	  unsetenv (nextp);
	  /* We could use rawmemchr but this need not be fast.  */
	  nextp = (char *) (strchr) (nextp, '\0') + 1;
	}
      while (*nextp != '\0');

      if (__access ("/etc/suid-debug", F_OK) != 0)
	{
#if !HAVE_TUNABLES
	  unsetenv ("MALLOC_CHECK_");
#endif
	  GLRO(dl_debug_mask) = 0;
	}

      if (mode != normal)
	_exit (5);
    }
  /* If we have to run the dynamic linker in debugging mode and the
     LD_DEBUG_OUTPUT environment variable is given, we write the debug
     messages to this file.  */
  else if (any_debug && debug_output != NULL)
    {
      const int flags = O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW;
      size_t name_len = strlen (debug_output);
      char buf[name_len + 12];
      char *startp;

      buf[name_len + 11] = '\0';
      startp = _itoa (__getpid (), &buf[name_len + 11], 10, 0);
      *--startp = '.';
      startp = memcpy (startp - name_len, debug_output, name_len);

      GLRO(dl_debug_fd) = __open (startp, flags, DEFFILEMODE);
      if (GLRO(dl_debug_fd) == -1)
	/* We use standard output if opening the file failed.  */
	GLRO(dl_debug_fd) = STDOUT_FILENO;
    }
}


/* Print the various times we collected.  */
static void
__attribute ((noinline))
print_statistics (hp_timing_t *rtld_total_timep)
{
#ifndef HP_TIMING_NONAVAIL
  char buf[200];
  char *cp;
  char *wp;

  /* Total time rtld used.  */
  if (HP_SMALL_TIMING_AVAIL)
    {
      HP_TIMING_PRINT (buf, sizeof (buf), *rtld_total_timep);
      _dl_debug_printf ("\nruntime linker statistics:\n"
			"  total startup time in dynamic loader: %s\n", buf);

      /* Print relocation statistics.  */
      char pbuf[30];
      HP_TIMING_PRINT (buf, sizeof (buf), relocate_time);
      cp = _itoa ((1000ULL * relocate_time) / *rtld_total_timep,
		  pbuf + sizeof (pbuf), 10, 0);
      wp = pbuf;
      switch (pbuf + sizeof (pbuf) - cp)
	{
	case 3:
	  *wp++ = *cp++;
	case 2:
	  *wp++ = *cp++;
	case 1:
	  *wp++ = '.';
	  *wp++ = *cp++;
	}
      *wp = '\0';
      _dl_debug_printf ("\
	    time needed for relocation: %s (%s%%)\n", buf, pbuf);
    }
#endif

  unsigned long int num_relative_relocations = 0;
  for (Lmid_t ns = 0; ns < GL(dl_nns); ++ns)
    {
      if (GL(dl_ns)[ns]._ns_loaded == NULL)
	continue;

      struct r_scope_elem *scope = &GL(dl_ns)[ns]._ns_loaded->l_searchlist;

      for (unsigned int i = 0; i < scope->r_nlist; i++)
	{
	  struct link_map *l = scope->r_list [i];

	  if (l->l_addr != 0 && l->l_info[VERSYMIDX (DT_RELCOUNT)])
	    num_relative_relocations
	      += l->l_info[VERSYMIDX (DT_RELCOUNT)]->d_un.d_val;
#ifndef ELF_MACHINE_REL_RELATIVE
	  /* Relative relocations are processed on these architectures if
	     library is loaded to different address than p_vaddr or
	     if not prelinked.  */
	  if ((l->l_addr != 0 || !l->l_info[VALIDX(DT_GNU_PRELINKED)])
	      && l->l_info[VERSYMIDX (DT_RELACOUNT)])
#else
	  /* On e.g. IA-64 or Alpha, relative relocations are processed
	     only if library is loaded to different address than p_vaddr.  */
	  if (l->l_addr != 0 && l->l_info[VERSYMIDX (DT_RELACOUNT)])
#endif
	    num_relative_relocations
	      += l->l_info[VERSYMIDX (DT_RELACOUNT)]->d_un.d_val;
	}
    }

  _dl_debug_printf ("                 number of relocations: %lu\n"
		    "      number of relocations from cache: %lu\n"
		    "        number of relative relocations: %lu\n",
		    GL(dl_num_relocations),
		    GL(dl_num_cache_relocations),
		    num_relative_relocations);

#ifndef HP_TIMING_NONAVAIL
  /* Time spend while loading the object and the dependencies.  */
  if (HP_SMALL_TIMING_AVAIL)
    {
      char pbuf[30];
      HP_TIMING_PRINT (buf, sizeof (buf), load_time);
      cp = _itoa ((1000ULL * load_time) / *rtld_total_timep,
		  pbuf + sizeof (pbuf), 10, 0);
      wp = pbuf;
      switch (pbuf + sizeof (pbuf) - cp)
	{
	case 3:
	  *wp++ = *cp++;
	case 2:
	  *wp++ = *cp++;
	case 1:
	  *wp++ = '.';
	  *wp++ = *cp++;
	}
      *wp = '\0';
      _dl_debug_printf ("\
	   time needed to load objects: %s (%s%%)\n",
				buf, pbuf);
    }
#endif
}
