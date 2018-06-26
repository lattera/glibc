/* Internal declarations for malloc, for use within libc.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _MALLOC_INTERNAL_H
#define _MALLOC_INTERNAL_H

#include <malloc-machine.h>
#include <malloc-sysdep.h>

/* INTERNAL_SIZE_T is the word-size used for internal bookkeeping of
   chunk sizes.

   The default version is the same as size_t.

   While not strictly necessary, it is best to define this as an
   unsigned type, even if size_t is a signed type. This may avoid some
   artificial size limitations on some systems.

   On a 64-bit machine, you may be able to reduce malloc overhead by
   defining INTERNAL_SIZE_T to be a 32 bit `unsigned int' at the
   expense of not being able to handle more than 2^32 of malloced
   space. If this limitation is acceptable, you are encouraged to set
   this unless you are on a platform requiring 16byte alignments. In
   this case the alignment requirements turn out to negate any
   potential advantages of decreasing size_t word size.

   Implementors: Beware of the possible combinations of:
     - INTERNAL_SIZE_T might be signed or unsigned, might be 32 or 64 bits,
       and might be the same width as int or as long
     - size_t might have different width and signedness as INTERNAL_SIZE_T
     - int and long might be 32 or 64 bits, and might be the same width

   To deal with this, most comparisons and difference computations
   among INTERNAL_SIZE_Ts should cast them to unsigned long, being
   aware of the fact that casting an unsigned int to a wider long does
   not sign-extend. (This also makes checking for negative numbers
   awkward.) Some of these casts result in harmless compiler warnings
   on some systems.  */
#ifndef INTERNAL_SIZE_T
# define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size.  */
#define SIZE_SZ (sizeof (INTERNAL_SIZE_T))

/* The corresponding bit mask value.  */
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)


/* Called in the parent process before a fork.  */
void __malloc_fork_lock_parent (void) attribute_hidden;

/* Called in the parent process after a fork.  */
void __malloc_fork_unlock_parent (void) attribute_hidden;

/* Called in the child process after a fork.  */
void __malloc_fork_unlock_child (void) attribute_hidden;

/* Called as part of the thread shutdown sequence.  */
void __malloc_arena_thread_freeres (void) attribute_hidden;

/* Set *RESULT to LEFT * RIGHT.  Return true if the multiplication
   overflowed.  */
static inline bool
check_mul_overflow_size_t (size_t left, size_t right, size_t *result)
{
#if __GNUC__ >= 5
  return __builtin_mul_overflow (left, right, result);
#else
  /* size_t is unsigned so the behavior on overflow is defined.  */
  *result = left * right;
  size_t half_size_t = ((size_t) 1) << (8 * sizeof (size_t) / 2);
  if (__glibc_unlikely ((left | right) >= half_size_t))
    {
      if (__glibc_unlikely (right != 0 && *result / right != left))
        return true;
    }
  return false;
#endif
}

#endif /* _MALLOC_INTERNAL_H */
