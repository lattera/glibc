/* Atomic operations.  PowerPC version.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

#include <stdint.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int16_t atomic16_t;
typedef uint16_t uatomic16_t;
typedef int_fast16_t atomic_fast16_t;
typedef uint_fast16_t uatomic_fast16_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;


#define __arch_compare_and_exchange_bool_8_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_acq(mem, newval, oldval) \
  (abort (), 0)

#ifdef UP
# define __ARCH_ACQ_INSTR	""
# define __ARCH_REL_INSTR	""
#else
# define __ARCH_ACQ_INSTR	"isync"
# define __ARCH_REL_INSTR	"sync"
#endif

/*
 * XXX At present these have both acquire and release semantics.
 * Ultimately we should do separate _acq and _rel versions.
 */

#ifdef __powerpc64__

/*
 * The 32-bit exchange_bool is different on powerpc64 because the subf
 * does signed 64-bit arthmatic while the lwarx is 32-bit unsigned
 * (a load word and zero (high 32) form).
 * In powerpc64 register values are 64-bit by default,  including oldval.
 * Net we need to extend sign word the result of lwarx to 64-bit so the
 * 64-bit subtract from gives the expected result and sets the condition
 * correctly.
 */
# define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval) \
({									      \
  unsigned int __tmp;							      \
  __asm __volatile (__ARCH_REL_INSTR "\n"				      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	extsw	%0,%0\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
({									      \
  unsigned long	__tmp;							      \
  __asm __volatile (__ARCH_REL_INSTR "\n"				      \
		    "1:	ldarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stdcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_atomic_exchange_64(mem, value)				      \
    ({									      \
      __typeof (*mem) __val;						      \
      __asm __volatile (__ARCH_REL_INSTR "\n"				      \
			"1:	ldarx	%0,0,%2\n"			      \
			"	stdcx.	%3,0,%2\n"			      \
			"	bne-	1b"				      \
			: "=&r" (__val), "=m" (*mem)			      \
			: "b" (mem), "r" (value), "1" (*mem)		      \
			: "cr0");					      \
      __val;								      \
    })

# define __arch_atomic_exchange_and_add_64(mem, value)			      \
    ({									      \
      __typeof (*mem) __val, __tmp;					      \
      __asm __volatile ("1:	ldarx	%0,0,%3\n"			      \
			"	add	%1,%0,%4\n"			      \
			"	stdcx.	%1,0,%3\n"			      \
			"	bne-	1b"				      \
			: "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
			: "b" (mem), "r" (value), "2" (*mem)		      \
			: "cr0");					      \
      __val;								      \
    })

# define __arch_atomic_decrement_if_positive_64(mem) \
  ({ int __val, __tmp;							      \
     __asm __volatile ("1:	ldarx	%0,0,%3\n"			      \
		       "	cmpdi	0,%0,0\n"			      \
		       "	addi	%1,%0,-1\n"			      \
		       "	ble	2f\n"				      \
		       "	stdcx.	%1,0,%3\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __ARCH_ACQ_INSTR			      \
		       : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		       : "b" (mem), "2" (*mem)				      \
		       : "cr0");					      \
     __val;								      \
  })

#else /* powerpc32 */
# define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval) \
({									      \
  unsigned int __tmp;							      \
  __asm __volatile (__ARCH_REL_INSTR "\n"				      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
  (abort (), 0)

# define __arch_atomic_exchange_64(mem, value) \
    ({ abort (); (*mem) = (value); })
# define __arch_atomic_exchange_and_add_64(mem, value) \
    ({ abort (); (*mem) = (value); })
# define __arch_atomic_decrement_if_positive_64(mem) \
    ({ abort (); (*mem)--; })
#endif

#define __arch_atomic_exchange_32(mem, value)				      \
  ({									      \
    __typeof (*mem) __val;						      \
    __asm __volatile (__ARCH_REL_INSTR "\n"				      \
		      "1:	lwarx	%0,0,%2\n"			      \
		      "		stwcx.	%3,0,%2\n"			      \
		      "		bne-	1b"				      \
		      : "=&r" (__val), "=m" (*mem)			      \
		      : "b" (mem), "r" (value), "1" (*mem)		      \
		      : "cr0");						      \
    __val;								      \
  })

#define __arch_atomic_exchange_and_add_32(mem, value)			      \
  ({									      \
    __typeof (*mem) __val, __tmp;					      \
    __asm __volatile ("1:	lwarx	%0,0,%3\n"			      \
		      "		add	%1,%0,%4\n"			      \
		      "		stwcx.	%1,0,%3\n"			      \
		      "		bne-	1b"				      \
		      : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		      : "b" (mem), "r" (value), "2" (*mem)		      \
		      : "cr0");						      \
    __val;								      \
  })

#define __arch_atomic_decrement_if_positive_32(mem)			      \
  ({ int __val, __tmp;							      \
     __asm __volatile ("1:	lwarx	%0,0,%3\n"			      \
		       "	cmpwi	0,%0,0\n"			      \
		       "	addi	%1,%0,-1\n"			      \
		       "	ble	2f\n"				      \
		       "	stwcx.	%1,0,%3\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __ARCH_ACQ_INSTR			      \
		       : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		       : "b" (mem), "2" (*mem)				      \
		       : "cr0");					      \
     __val;								      \
  })


#define atomic_exchange(mem, value)					      \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_exchange_32 ((mem), (value));		      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_exchange_64 ((mem), (value));		      \
    else 								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_exchange_and_add(mem, value)				      \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_exchange_and_add_32 ((mem), (value));	      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_exchange_and_add_64 ((mem), (value));	      \
    else 								      \
       abort ();							      \
    __result;								      \
  })


/* Decrement *MEM if it is > 0, and return the old value.  */
#define atomic_decrement_if_positive(mem)				      \
  ({ __typeof (*(mem)) __result;					      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_decrement_if_positive_32 (mem);		      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_decrement_if_positive_64 (mem);		      \
    else								      \
       abort ();							      \
    __result;								      \
  })


#define atomic_full_barrier()	__asm ("sync" ::: "memory")
#ifdef __powerpc64__
# define atomic_read_barrier()	__asm ("lwsync" ::: "memory")
#else
# define atomic_read_barrier()	__asm ("sync" ::: "memory")
#endif
#define atomic_write_barrier()	__asm ("eieio" ::: "memory")
