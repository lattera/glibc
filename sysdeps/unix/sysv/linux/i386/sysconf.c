/* Get file-specific information about a file.  Linux version.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
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
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <hp-timing.h>

static long int linux_sysconf (int name);


static long int __attribute__ ((noinline))
handle_i486 (int name)
{
  /* The processor only has a unified level 1 cache of 8k.  */
  switch (name)
    {
    case _SC_LEVEL1_ICACHE_SIZE:
    case _SC_LEVEL1_DCACHE_SIZE:
      return 8 * 1024;

    case _SC_LEVEL1_ICACHE_ASSOC:
    case _SC_LEVEL1_DCACHE_ASSOC:
      // XXX Anybody know this?
      return 0;

    case _SC_LEVEL1_ICACHE_LINESIZE:
    case _SC_LEVEL1_DCACHE_LINESIZE:
      // XXX Anybody know for sure?
      return 16;

    case _SC_LEVEL2_CACHE_SIZE:
    case _SC_LEVEL2_CACHE_ASSOC:
    case _SC_LEVEL2_CACHE_LINESIZE:
    case _SC_LEVEL3_CACHE_SIZE:
    case _SC_LEVEL3_CACHE_ASSOC:
    case _SC_LEVEL3_CACHE_LINESIZE:
    case _SC_LEVEL4_CACHE_SIZE:
    case _SC_LEVEL4_CACHE_ASSOC:
      /* Not available.  */
      break;

    default:
      assert (! "cannot happen");
    }

  return -1;
}


static const struct intel_02_cache_info
{
  unsigned char idx;
  unsigned char assoc;
  unsigned char linesize;
  unsigned char rel_name;
  unsigned int size;
} intel_02_known [] =
  {
#define M(sc) ((sc) - _SC_LEVEL1_ICACHE_SIZE)
    { 0x06,  4, 32, M(_SC_LEVEL1_ICACHE_SIZE),    8192 },
    { 0x08,  4, 32, M(_SC_LEVEL1_ICACHE_SIZE),   16384 },
    { 0x09,  4, 32, M(_SC_LEVEL1_ICACHE_SIZE),   32768 },
    { 0x0a,  2, 32, M(_SC_LEVEL1_DCACHE_SIZE),    8192 },
    { 0x0c,  4, 32, M(_SC_LEVEL1_DCACHE_SIZE),   16384 },
    { 0x0d,  4, 64, M(_SC_LEVEL1_DCACHE_SIZE),   16384 },
    { 0x21,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x22,  4, 64, M(_SC_LEVEL3_CACHE_SIZE),   524288 },
    { 0x23,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  1048576 },
    { 0x25,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  2097152 },
    { 0x29,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  4194304 },
    { 0x2c,  8, 64, M(_SC_LEVEL1_DCACHE_SIZE),   32768 },
    { 0x30,  8, 64, M(_SC_LEVEL1_ICACHE_SIZE),   32768 },
    { 0x39,  4, 64, M(_SC_LEVEL2_CACHE_SIZE),   131072 },
    { 0x3a,  6, 64, M(_SC_LEVEL2_CACHE_SIZE),   196608 },
    { 0x3b,  2, 64, M(_SC_LEVEL2_CACHE_SIZE),   131072 },
    { 0x3c,  4, 64, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x3d,  6, 64, M(_SC_LEVEL2_CACHE_SIZE),   393216 },
    { 0x3e,  4, 64, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x3f,  2, 64, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x41,  4, 32, M(_SC_LEVEL2_CACHE_SIZE),   131072 },
    { 0x42,  4, 32, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x43,  4, 32, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x44,  4, 32, M(_SC_LEVEL2_CACHE_SIZE),  1048576 },
    { 0x45,  4, 32, M(_SC_LEVEL2_CACHE_SIZE),  2097152 },
    { 0x46,  4, 64, M(_SC_LEVEL3_CACHE_SIZE),  4194304 },
    { 0x47,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  8388608 },
    { 0x48, 12, 64, M(_SC_LEVEL2_CACHE_SIZE),  3145728 },
    { 0x49, 16, 64, M(_SC_LEVEL2_CACHE_SIZE),  4194304 },
    { 0x4a, 12, 64, M(_SC_LEVEL3_CACHE_SIZE),  6291456 },
    { 0x4b, 16, 64, M(_SC_LEVEL3_CACHE_SIZE),  8388608 },
    { 0x4c, 12, 64, M(_SC_LEVEL3_CACHE_SIZE), 12582912 },
    { 0x4d, 16, 64, M(_SC_LEVEL3_CACHE_SIZE), 16777216 },
    { 0x4e, 24, 64, M(_SC_LEVEL2_CACHE_SIZE),  6291456 },
    { 0x60,  8, 64, M(_SC_LEVEL1_DCACHE_SIZE),   16384 },
    { 0x66,  4, 64, M(_SC_LEVEL1_DCACHE_SIZE),    8192 },
    { 0x67,  4, 64, M(_SC_LEVEL1_DCACHE_SIZE),   16384 },
    { 0x68,  4, 64, M(_SC_LEVEL1_DCACHE_SIZE),   32768 },
    { 0x78,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),  1048576 },
    { 0x79,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),   131072 },
    { 0x7a,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x7b,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x7c,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),  1048576 },
    { 0x7d,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),  2097152 },
    { 0x7f,  2, 64, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x82,  8, 32, M(_SC_LEVEL2_CACHE_SIZE),   262144 },
    { 0x83,  8, 32, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x84,  8, 32, M(_SC_LEVEL2_CACHE_SIZE),  1048576 },
    { 0x85,  8, 32, M(_SC_LEVEL2_CACHE_SIZE),  2097152 },
    { 0x86,  4, 64, M(_SC_LEVEL2_CACHE_SIZE),   524288 },
    { 0x87,  8, 64, M(_SC_LEVEL2_CACHE_SIZE),  1048576 },
    { 0xd0,  4, 64, M(_SC_LEVEL3_CACHE_SIZE),   524288 },
    { 0xd1,  4, 64, M(_SC_LEVEL3_CACHE_SIZE),  1048576 },
    { 0xd2,  4, 64, M(_SC_LEVEL3_CACHE_SIZE),  2097152 },
    { 0xd6,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  1048576 },
    { 0xd7,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  2097152 },
    { 0xd8,  8, 64, M(_SC_LEVEL3_CACHE_SIZE),  4194304 },
    { 0xdc, 12, 64, M(_SC_LEVEL3_CACHE_SIZE),  2097152 },
    { 0xdd, 12, 64, M(_SC_LEVEL3_CACHE_SIZE),  4194304 },
    { 0xde, 12, 64, M(_SC_LEVEL3_CACHE_SIZE),  8388608 },
    { 0xe2, 16, 64, M(_SC_LEVEL3_CACHE_SIZE),  2097152 },
    { 0xe3, 16, 64, M(_SC_LEVEL3_CACHE_SIZE),  4194304 },
    { 0xe4, 16, 64, M(_SC_LEVEL3_CACHE_SIZE),  8388608 },
    { 0xea, 24, 64, M(_SC_LEVEL3_CACHE_SIZE), 12582912 },
    { 0xeb, 24, 64, M(_SC_LEVEL3_CACHE_SIZE), 18874368 },
    { 0xec, 24, 64, M(_SC_LEVEL3_CACHE_SIZE), 25165824 },
  };
#define nintel_02_known (sizeof (intel_02_known) / sizeof (intel_02_known[0]))


static int
intel_02_known_compare (const void *p1, const void *p2)
{
  const struct intel_02_cache_info *i1;
  const struct intel_02_cache_info *i2;

  i1 = (const struct intel_02_cache_info *) p1;
  i2 = (const struct intel_02_cache_info *) p2;

  if (i1->idx == i2->idx)
    return 0;

  return i1->idx < i2->idx ? -1 : 1;
}


static long int
__attribute__ ((noinline))
intel_check_word (int name, unsigned int value, bool *has_level_2,
		  bool *no_level_2_or_3)
{
  if ((value & 0x80000000) != 0)
    /* The register value is reserved.  */
    return 0;

  /* Fold the name.  The _SC_ constants are always in the order SIZE,
     ASSOC, LINESIZE.  */
  int folded_rel_name = (M(name) / 3) * 3;

  while (value != 0)
    {
      unsigned int byte = value & 0xff;

      if (byte == 0x40)
	{
	  *no_level_2_or_3 = true;

	  if (folded_rel_name == M(_SC_LEVEL3_CACHE_SIZE))
	    /* No need to look further.  */
	    break;
	}
      else if (byte == 0xff)
	{
	  /* CPUID leaf 0x4 contains all the information.  We need to
	     iterate over it.  */
	  unsigned int eax;
	  unsigned int ebx;
	  unsigned int ecx;
	  unsigned int edx;

	  unsigned int round = 0;
	  while (1)
	    {
	      asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
			    : "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
			    : "0" (4), "2" (round));

	      enum { null = 0, data = 1, inst = 2, uni = 3 } type = eax & 0x1f;
	      if (type == null)
		/* That was the end.  */
		break;

	      unsigned int level = (eax >> 5) & 0x7;

	      if ((level == 1 && type == data
		   && folded_rel_name == M(_SC_LEVEL1_DCACHE_SIZE))
		  || (level == 1 && type == inst
		      && folded_rel_name == M(_SC_LEVEL1_ICACHE_SIZE))
		  || (level == 2 && folded_rel_name == M(_SC_LEVEL2_CACHE_SIZE))
		  || (level == 3 && folded_rel_name == M(_SC_LEVEL3_CACHE_SIZE))
		  || (level == 4 && folded_rel_name == M(_SC_LEVEL4_CACHE_SIZE)))
		{
		  unsigned int offset = M(name) - folded_rel_name;

		  if (offset == 0)
		    /* Cache size.  */
		    return (((ebx >> 22) + 1)
			    * (((ebx >> 12) & 0x3ff) + 1)
			    * ((ebx & 0xfff) + 1)
			    * (ecx + 1));
		  if (offset == 1)
		    return (ebx >> 22) + 1;

		  assert (offset == 2);
		  return (ebx & 0xfff) + 1;
		}

	      ++round;
	    }
	  /* There is no other cache information anywhere else.  */
	  break;
	}
      else
	{
	  if (byte == 0x49 && folded_rel_name == M(_SC_LEVEL3_CACHE_SIZE))
	    {
	      /* Intel reused this value.  For family 15, model 6 it
		 specifies the 3rd level cache.  Otherwise the 2nd
		 level cache.  */
	      unsigned int eax;
	      unsigned int ebx;
	      unsigned int ecx;
	      unsigned int edx;
	      asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
			    : "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
			    : "0" (1));

	      unsigned int family = ((eax >> 20) & 0xff) + ((eax >> 8) & 0xf);
	      unsigned int model = ((((eax >>16) & 0xf) << 4)
				    + ((eax >> 4) & 0xf));
	      if (family == 15 && model == 6)
		{
		  /* The level 3 cache is encoded for this model like
		     the level 2 cache is for other models.  Pretend
		     the caller asked for the level 2 cache.  */
		  name = (_SC_LEVEL2_CACHE_SIZE
			  + (name - _SC_LEVEL3_CACHE_SIZE));
		  folded_rel_name = M(_SC_LEVEL2_CACHE_SIZE);
		}
	    }

	  struct intel_02_cache_info *found;
	  struct intel_02_cache_info search;

	  search.idx = byte;
	  found = bsearch (&search, intel_02_known, nintel_02_known,
			   sizeof (intel_02_known[0]), intel_02_known_compare);
	  if (found != NULL)
	    {
	      if (found->rel_name == folded_rel_name)
		{
		  unsigned int offset = M(name) - folded_rel_name;

		  if (offset == 0)
		    /* Cache size.  */
		    return found->size;
		  if (offset == 1)
		    return found->assoc;

		  assert (offset == 2);
		  return found->linesize;
		}

	      if (found->rel_name == M(_SC_LEVEL2_CACHE_SIZE))
		*has_level_2 = true;
	    }
	}

      /* Next byte for the next round.  */
      value >>= 8;
    }

  /* Nothing found.  */
  return 0;
}


static long int  __attribute__ ((noinline))
handle_intel (int name, unsigned int maxidx)
{
  if (maxidx < 2)
    {
      // XXX Do such processors exist?  When we know we can fill in some
      // values.
      return 0;
    }

  /* OK, we can use the CPUID instruction to get all info about the
     caches.  */
  unsigned int cnt = 0;
  unsigned int max = 1;
  long int result = 0;
  bool no_level_2_or_3 = false;
  bool has_level_2 = false;
  while (cnt++ < max)
    {
      unsigned int eax;
      unsigned int ebx;
      unsigned int ecx;
      unsigned int edx;
      asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
		    : "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
		    : "0" (2));

      /* The low byte of EAX in the first round contain the number of
	 rounds we have to make.  At least one, the one we are already
	 doing.  */
      if (cnt == 1)
	{
	  max = eax & 0xff;
	  eax &= 0xffffff00;
	}

      /* Process the individual registers' value.  */
      result = intel_check_word (name, eax, &has_level_2, &no_level_2_or_3);
      if (result != 0)
	return result;

      result = intel_check_word (name, ebx, &has_level_2, &no_level_2_or_3);
      if (result != 0)
	return result;

      result = intel_check_word (name, ecx, &has_level_2, &no_level_2_or_3);
      if (result != 0)
	return result;

      result = intel_check_word (name, edx, &has_level_2, &no_level_2_or_3);
      if (result != 0)
	return result;
    }

  if (name >= _SC_LEVEL2_CACHE_SIZE && name <= _SC_LEVEL3_CACHE_LINESIZE
      && no_level_2_or_3)
    return -1;

  return 0;
}


static long int __attribute__ ((noinline))
handle_amd (int name)
{
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
		: "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
		: "0" (0x80000000));

  if (name >= _SC_LEVEL3_CACHE_SIZE)
    return 0;

  unsigned int fn = 0x80000005 + (name >= _SC_LEVEL2_CACHE_SIZE);
  if (eax < fn)
    return 0;

  asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
		: "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
		: "0" (fn));

  if (name < _SC_LEVEL1_DCACHE_SIZE)
    {
      name += _SC_LEVEL1_DCACHE_SIZE - _SC_LEVEL1_ICACHE_SIZE;
      ecx = edx;
    }

  switch (name)
    {
    case _SC_LEVEL1_DCACHE_SIZE:
      return (ecx >> 14) & 0x3fc00;
    case _SC_LEVEL1_DCACHE_ASSOC:
      ecx >>= 16;
      if ((ecx & 0xff) == 0xff)
	/* Fully associative.  */
	return (ecx << 2) & 0x3fc00;
      return ecx & 0xff;
    case _SC_LEVEL1_DCACHE_LINESIZE:
      return ecx & 0xff;
    case _SC_LEVEL2_CACHE_SIZE:
      return (ecx & 0xf000) == 0 ? 0 : (ecx >> 6) & 0x3fffc00;
    case _SC_LEVEL2_CACHE_ASSOC:
      ecx >>= 12;
      switch (ecx & 0xf)
	{
	case 0:
	case 1:
	case 2:
	case 4:
	  return ecx & 0xf;
	case 6:
	  return 8;
	case 8:
	  return 16;
	case 0xf:
	  return (ecx << 6) & 0x3fffc00;
	default:
	  return 0;
	}
    case _SC_LEVEL2_CACHE_LINESIZE:
      return (ecx & 0xf000) == 0 ? 0 : ecx & 0xff;
    default:
      assert (! "cannot happen");
    }
  return -1;
}


static int
i386_i486_test (void)
{
  int eflags;
  int ac;
  asm volatile ("pushfl;\n\t"
		"popl %0;\n\t"
		"movl $0x240000, %1;\n\t"
		"xorl %0, %1;\n\t"
		"pushl %1;\n\t"
		"popfl;\n\t"
		"pushfl;\n\t"
		"popl %1;\n\t"
		"xorl %0, %1;\n\t"
		"pushl %0;\n\t"
		"popfl"
		: "=r" (eflags), "=r" (ac));

  return ac;
}


/* Get the value of the system variable NAME.  */
long int
__sysconf (int name)
{
  /* All the remainder, except the cache information, is handled in
     the generic code.  */
  if (name < _SC_LEVEL1_ICACHE_SIZE || name > _SC_LEVEL4_CACHE_LINESIZE)
    return linux_sysconf (name);

  /* Recognize i386 and compatible.  These don't have any cache on
     board.  */
  int ac = i386_i486_test ();

  if (ac == 0)
    /* This is an i386.  */
    // XXX Is this true for all brands?
    return -1;

  /* Detect i486, the last Intel processor without CPUID.  */
  if ((ac & (1 << 21)) == 0)
    {
      /* No CPUID.  */
      // XXX Fill in info about other brands.  For now only Intel.
      return handle_i486 (name);
    }

  /* Find out what brand of processor.  */
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1"
		: "=a" (eax), "=r" (ebx), "=c" (ecx), "=d" (edx)
		: "0" (0));

  /* This spells out "GenuineIntel".  */
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    return handle_intel (name, eax);

  /* This spells out "AuthenticAMD".  */
  if (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65)
    return handle_amd (name);

  // XXX Fill in more vendors.

  /* CPU not known, we have no information.  */
  return 0;
}

/* Now the generic Linux version.  */
#undef __sysconf
#define __sysconf static linux_sysconf
#include "../sysconf.c"
