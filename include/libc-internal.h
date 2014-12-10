/* This file contains a number of internal prototype declarations that
   don't fit anywhere else.  */

#ifndef _LIBC_INTERNAL
# define _LIBC_INTERNAL 1

#include <hp-timing.h>

/* Initialize the `__libc_enable_secure' flag.  */
extern void __libc_init_secure (void);

/* This function will be called from _init in init-first.c.  */
extern void __libc_global_ctors (void);

/* Discover the tick frequency of the machine if something goes wrong,
   we return 0, an impossible hertz.  */
extern int __profile_frequency (void);
libc_hidden_proto (__profile_frequency)

/* Hooks for the instrumenting functions.  */
extern void __cyg_profile_func_enter (void *this_fn, void *call_site);
extern void __cyg_profile_func_exit (void *this_fn, void *call_site);

/* Get frequency of the system processor.  */
extern hp_timing_t __get_clockfreq (void);

/* Free all allocated resources.  */
extern void __libc_freeres (void);
libc_hidden_proto (__libc_freeres)

/* Free resources stored in thread-local variables on thread exit.  */
extern void __libc_thread_freeres (void);

/* Define and initialize `__progname' et. al.  */
extern void __init_misc (int, char **, char **);

/* 1 if 'type' is a pointer type, 0 otherwise.  */
# define __pointer_type(type) (__builtin_classify_type ((type) 0) == 5)

/* __intptr_t if P is true, or T if P is false.  */
# define __integer_if_pointer_type_sub(T, P) \
  __typeof__ (*(0 ? (__typeof__ (0 ? (T *) 0 : (void *) (P))) 0 \
		  : (__typeof__ (0 ? (__intptr_t *) 0 : (void *) (!(P)))) 0))

/* __intptr_t if EXPR has a pointer type, or the type of EXPR otherwise.  */
# define __integer_if_pointer_type(expr) \
  __integer_if_pointer_type_sub(__typeof__ ((__typeof__ (expr)) 0), \
				__pointer_type (__typeof__ (expr)))

/* Cast an integer or a pointer VAL to integer with proper type.  */
# define cast_to_integer(val) ((__integer_if_pointer_type (val)) (val))

/* Align a value by rounding down to closest size.
   e.g. Using size of 4096, we get this behavior:
	{4095, 4096, 4097} = {0, 4096, 4096}.  */
#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))

/* Align a value by rounding up to closest size.
   e.g. Using size of 4096, we get this behavior:
	{4095, 4096, 4097} = {4096, 4096, 8192}.

  Note: The size argument has side effects (expanded multiple times).  */
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

/* Same as ALIGN_DOWN(), but automatically casts when base is a pointer.  */
#define PTR_ALIGN_DOWN(base, size) \
  ((__typeof__ (base)) ALIGN_DOWN ((uintptr_t) (base), (size)))

/* Same as ALIGN_UP(), but automatically casts when base is a pointer.  */
#define PTR_ALIGN_UP(base, size) \
  ((__typeof__ (base)) ALIGN_UP ((uintptr_t) (base), (size)))

/* Ignore the value of an expression when a cast to void does not
   suffice (in particular, for a call to a function declared with
   attribute warn_unused_result).  */
#define ignore_value(x) \
  ({ __typeof__ (x) __ignored_value = (x); (void) __ignored_value; })

/* The macros to control diagnostics are structured like this, rather
   than a single macro that both pushes and pops diagnostic state and
   takes the affected code as an argument, because the GCC pragmas
   work by disabling the diagnostic for a range of source locations
   and do not work when all the pragmas and the affected code are in a
   single macro expansion.  */

/* Push diagnostic state.  */
#define DIAG_PUSH_NEEDS_COMMENT _Pragma ("GCC diagnostic push")

/* Pop diagnostic state.  */
#define DIAG_POP_NEEDS_COMMENT _Pragma ("GCC diagnostic pop")

#define _DIAG_STR1(s) #s
#define _DIAG_STR(s) _DIAG_STR1(s)

/* Ignore the diagnostic OPTION.  VERSION is the most recent GCC
   version for which the diagnostic has been confirmed to appear in
   the absence of the pragma (in the form MAJOR.MINOR for GCC 4.x,
   just MAJOR for GCC 5 and later).  Uses of this pragma should be
   reviewed when the GCC version given is no longer supported for
   building glibc; the version number should always be on the same
   source line as the macro name, so such uses can be found with grep.
   Uses should come with a comment giving more details of the
   diagnostic, and an architecture on which it is seen if possibly
   optimization-related and not in architecture-specific code.  This
   macro should only be used if the diagnostic seems hard to fix (for
   example, optimization-related false positives).  */
#define DIAG_IGNORE_NEEDS_COMMENT(version, option)	\
  _Pragma (_DIAG_STR (GCC diagnostic ignored option))

#endif /* _LIBC_INTERNAL  */
