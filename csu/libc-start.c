/* Copyright (C) 1998-2016 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <exit-thread.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int __libc_multiple_libcs;

#include <tls.h>
#ifndef SHARED
# include <dl-osinfo.h>
extern void __pthread_initialize_minimal (void);
# ifndef THREAD_SET_STACK_GUARD
/* Only exported for architectures that don't store the stack guard canary
   in thread local area.  */
uintptr_t __stack_chk_guard attribute_relro;
# endif
# ifndef  THREAD_SET_POINTER_GUARD
/* Only exported for architectures that don't store the pointer guard
   value in thread local area.  */
uintptr_t __pointer_chk_guard_local
	attribute_relro attribute_hidden __attribute__ ((nocommon));
# endif
#endif

#ifdef HAVE_PTR_NTHREADS
/* We need atomic operations.  */
# include <atomic.h>
#endif


#ifndef SHARED
# include <link.h>
# include <dl-irel.h>

# ifdef ELF_MACHINE_IRELA
#  define IREL_T	ElfW(Rela)
#  define IPLT_START	__rela_iplt_start
#  define IPLT_END	__rela_iplt_end
#  define IREL		elf_irela
# elif defined ELF_MACHINE_IREL
#  define IREL_T	ElfW(Rel)
#  define IPLT_START	__rel_iplt_start
#  define IPLT_END	__rel_iplt_end
#  define IREL		elf_irel
# endif

static void
apply_irel (void)
{
# ifdef IREL
  /* We use weak references for these so that we'll still work with a linker
     that doesn't define them.  Such a linker doesn't support IFUNC at all
     and so uses won't work, but a statically-linked program that doesn't
     use any IFUNC symbols won't have a problem.  */
  extern const IREL_T IPLT_START[] __attribute__ ((weak));
  extern const IREL_T IPLT_END[] __attribute__ ((weak));
  for (const IREL_T *ipltent = IPLT_START; ipltent < IPLT_END; ++ipltent)
    IREL (ipltent);
# endif
}
#endif


#ifdef LIBC_START_MAIN
# ifdef LIBC_START_DISABLE_INLINE
#  define STATIC static
# else
#  define STATIC static inline __attribute__ ((always_inline))
# endif
#else
# define STATIC
# define LIBC_START_MAIN __libc_start_main
#endif

#ifdef MAIN_AUXVEC_ARG
/* main gets passed a pointer to the auxiliary.  */
# define MAIN_AUXVEC_DECL	, void *
# define MAIN_AUXVEC_PARAM	, auxvec
#else
# define MAIN_AUXVEC_DECL
# define MAIN_AUXVEC_PARAM
#endif

STATIC int LIBC_START_MAIN (int (*main) (int, char **, char **
					 MAIN_AUXVEC_DECL),
			    int argc,
			    char **argv,
#ifdef LIBC_START_MAIN_AUXVEC_ARG
			    ElfW(auxv_t) *auxvec,
#endif
			    __typeof (main) init,
			    void (*fini) (void),
			    void (*rtld_fini) (void),
			    void *stack_end)
     __attribute__ ((noreturn));


/* Note: the fini parameter is ignored here for shared library.  It
   is registered with __cxa_atexit.  This had the disadvantage that
   finalizers were called in more than one place.  */
STATIC int
LIBC_START_MAIN (int (*main) (int, char **, char ** MAIN_AUXVEC_DECL),
		 int argc, char **argv,
#ifdef LIBC_START_MAIN_AUXVEC_ARG
		 ElfW(auxv_t) *auxvec,
#endif
		 __typeof (main) init,
		 void (*fini) (void),
		 void (*rtld_fini) (void), void *stack_end)
{
  /* Result of the 'main' function.  */
  int result;

  __libc_multiple_libcs = &_dl_starting_up && !_dl_starting_up;

#ifndef SHARED
  char **ev = &argv[argc + 1];

  __environ = ev;

  /* Store the lowest stack address.  This is done in ld.so if this is
     the code for the DSO.  */
  __libc_stack_end = stack_end;

# ifdef HAVE_AUX_VECTOR
  /* First process the auxiliary vector since we need to find the
     program header to locate an eventually present PT_TLS entry.  */
#  ifndef LIBC_START_MAIN_AUXVEC_ARG
  ElfW(auxv_t) *auxvec;
  {
    char **evp = ev;
    while (*evp++ != NULL)
      ;
    auxvec = (ElfW(auxv_t) *) evp;
  }
#  endif
  _dl_aux_init (auxvec);
  if (GL(dl_phdr) == NULL)
# endif
    {
      /* Starting from binutils-2.23, the linker will define the
         magic symbol __ehdr_start to point to our own ELF header
         if it is visible in a segment that also includes the phdrs.
         So we can set up _dl_phdr and _dl_phnum even without any
         information from auxv.  */

      extern const ElfW(Ehdr) __ehdr_start
	__attribute__ ((weak, visibility ("hidden")));
      if (&__ehdr_start != NULL)
        {
          assert (__ehdr_start.e_phentsize == sizeof *GL(dl_phdr));
          GL(dl_phdr) = (const void *) &__ehdr_start + __ehdr_start.e_phoff;
          GL(dl_phnum) = __ehdr_start.e_phnum;
        }
    }

# ifdef DL_SYSDEP_OSCHECK
  if (!__libc_multiple_libcs)
    {
      /* This needs to run to initiliaze _dl_osversion before TLS
	 setup might check it.  */
      DL_SYSDEP_OSCHECK (__libc_fatal);
    }
# endif

  /* Perform IREL{,A} relocations.  */
  apply_irel ();

  /* Initialize the thread library at least a bit since the libgcc
     functions are using thread functions if these are available and
     we need to setup errno.  */
  __pthread_initialize_minimal ();

  /* Set up the stack checker's canary.  */
  uintptr_t stack_chk_guard = _dl_setup_stack_chk_guard (_dl_random);
# ifdef THREAD_SET_STACK_GUARD
  THREAD_SET_STACK_GUARD (stack_chk_guard);
# else
  __stack_chk_guard = stack_chk_guard;
# endif

  /* Set up the pointer guard value.  */
  uintptr_t pointer_chk_guard = _dl_setup_pointer_guard (_dl_random,
							 stack_chk_guard);
# ifdef THREAD_SET_POINTER_GUARD
  THREAD_SET_POINTER_GUARD (pointer_chk_guard);
# else
  __pointer_chk_guard_local = pointer_chk_guard;
# endif

#endif

  /* Register the destructor of the dynamic linker if there is any.  */
  if (__glibc_likely (rtld_fini != NULL))
    __cxa_atexit ((void (*) (void *)) rtld_fini, NULL, NULL);

#ifndef SHARED
  /* Call the initializer of the libc.  This is only needed here if we
     are compiling for the static library in which case we haven't
     run the constructors in `_dl_start_user'.  */
  __libc_init_first (argc, argv, __environ);

  /* Register the destructor of the program, if any.  */
  if (fini)
    __cxa_atexit ((void (*) (void *)) fini, NULL, NULL);

  /* Some security at this point.  Prevent starting a SUID binary where
     the standard file descriptors are not opened.  We have to do this
     only for statically linked applications since otherwise the dynamic
     loader did the work already.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    __libc_check_standard_fds ();
#endif

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    GLRO(dl_debug_printf) ("\ninitialize program: %s\n\n", argv[0]);
#endif
  if (init)
    (*init) (argc, argv, __environ MAIN_AUXVEC_PARAM);

#ifdef SHARED
  /* Auditing checkpoint: we have a new object.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0))
    {
      struct audit_ifaces *afct = GLRO(dl_audit);
      struct link_map *head = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->preinit != NULL)
	    afct->preinit (&head->l_audit[cnt].cookie);

	  afct = afct->next;
	}
    }
#endif

#ifdef SHARED
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_IMPCALLS))
    GLRO(dl_debug_printf) ("\ntransferring control: %s\n\n", argv[0]);
#endif

#ifndef SHARED
  _dl_debug_initialize (0, LM_ID_BASE);
#endif
#ifdef HAVE_CLEANUP_JMP_BUF
  /* Memory for the cancellation buffer.  */
  struct pthread_unwind_buf unwind_buf;

  int not_first_call;
  not_first_call = setjmp ((struct __jmp_buf_tag *) unwind_buf.cancel_jmp_buf);
  if (__glibc_likely (! not_first_call))
    {
      struct pthread *self = THREAD_SELF;

      /* Store old info.  */
      unwind_buf.priv.data.prev = THREAD_GETMEM (self, cleanup_jmp_buf);
      unwind_buf.priv.data.cleanup = THREAD_GETMEM (self, cleanup);

      /* Store the new cleanup handler info.  */
      THREAD_SETMEM (self, cleanup_jmp_buf, &unwind_buf);

      /* Run the program.  */
      result = main (argc, argv, __environ MAIN_AUXVEC_PARAM);
    }
  else
    {
      /* Remove the thread-local data.  */
# ifdef SHARED
      PTHFCT_CALL (ptr__nptl_deallocate_tsd, ());
# else
      extern void __nptl_deallocate_tsd (void) __attribute ((weak));
      __nptl_deallocate_tsd ();
# endif

      /* One less thread.  Decrement the counter.  If it is zero we
	 terminate the entire process.  */
      result = 0;
# ifdef SHARED
      unsigned int *ptr = __libc_pthread_functions.ptr_nthreads;
#  ifdef PTR_DEMANGLE
      PTR_DEMANGLE (ptr);
#  endif
# else
      extern unsigned int __nptl_nthreads __attribute ((weak));
      unsigned int *const ptr = &__nptl_nthreads;
# endif

      if (! atomic_decrement_and_test (ptr))
	/* Not much left to do but to exit the thread, not the process.  */
	__exit_thread ();
    }
#else
  /* Nothing fancy, just call the function.  */
  result = main (argc, argv, __environ MAIN_AUXVEC_PARAM);
#endif

  exit (result);
}
