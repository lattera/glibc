/* Get file-specific information about a file.  Linux version.
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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <hp-timing.h>

static long int linux_sysconf (int name);


static long int
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
  unsigned int idx;
  int name;
  long int size;
  long int assoc;
  long int linesize;
} intel_02_known[] =
  {
    { 0x06, _SC_LEVEL1_ICACHE_SIZE, 8192, 4, 32 },
    { 0x08, _SC_LEVEL1_ICACHE_SIZE, 16384, 4, 32 },
    { 0x0a, _SC_LEVEL1_DCACHE_SIZE, 8192, 2, 32 },
    { 0x0c, _SC_LEVEL1_DCACHE_SIZE, 16384, 4, 32 },
    { 0x22, _SC_LEVEL3_CACHE_SIZE, 524288, 4, 64 },
    { 0x23, _SC_LEVEL3_CACHE_SIZE, 1048576, 8, 64 },
    { 0x25, _SC_LEVEL3_CACHE_SIZE, 2097152, 8, 64 },
    { 0x29, _SC_LEVEL3_CACHE_SIZE, 4194304, 8, 64 },
    { 0x2c, _SC_LEVEL1_DCACHE_SIZE, 32768, 8, 64 },
    { 0x30, _SC_LEVEL1_ICACHE_SIZE, 32768, 8, 64 },
    { 0x41, _SC_LEVEL2_CACHE_SIZE, 131072, 4, 32 },
    { 0x42, _SC_LEVEL2_CACHE_SIZE, 262144, 4, 32 },
    { 0x43, _SC_LEVEL2_CACHE_SIZE, 524288, 4, 32 },
    { 0x44, _SC_LEVEL2_CACHE_SIZE, 1048576, 4, 32 },
    { 0x45, _SC_LEVEL2_CACHE_SIZE, 2097152, 4, 32 },
    { 0x60, _SC_LEVEL1_DCACHE_SIZE, 16384, 8, 64 },
    { 0x66, _SC_LEVEL1_DCACHE_SIZE, 8192, 4, 64 },
    { 0x67, _SC_LEVEL1_DCACHE_SIZE, 16384, 4, 64 },
    { 0x68, _SC_LEVEL1_DCACHE_SIZE, 32768, 4, 64 },
    { 0x78, _SC_LEVEL2_CACHE_SIZE, 1048576, 8, 64 },
    { 0x79, _SC_LEVEL2_CACHE_SIZE, 131072, 8, 64 },
    { 0x7a, _SC_LEVEL2_CACHE_SIZE, 262144, 8, 64 },
    { 0x7b, _SC_LEVEL2_CACHE_SIZE, 524288, 8, 64 },
    { 0x7c, _SC_LEVEL2_CACHE_SIZE, 1048576, 8, 64 },
    { 0x7d, _SC_LEVEL2_CACHE_SIZE, 2097152, 8, 64 },
    { 0x82, _SC_LEVEL2_CACHE_SIZE, 262144, 8, 32 },
    { 0x83, _SC_LEVEL2_CACHE_SIZE, 524288, 8, 32 },
    { 0x84, _SC_LEVEL2_CACHE_SIZE, 1048576, 8, 32 },
    { 0x85, _SC_LEVEL2_CACHE_SIZE, 2097152, 8, 32 },
    { 0x86, _SC_LEVEL2_CACHE_SIZE, 524288, 4, 64 },
    { 0x87, _SC_LEVEL2_CACHE_SIZE, 1048576, 8, 64 },
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
intel_check_word (int name, unsigned int value, bool *has_level_2,
		  bool *no_level_2_or_3)
{
  if ((value & 0x80000000) != 0)
    /* The register value is reserved.  */
    return 0;

  /* Fold the name.  The _SC_ constants are always in the order SIZE,
     ASSOC, LINESIZE.  */
  int folded_name = (_SC_LEVEL1_ICACHE_SIZE
		     + ((name - _SC_LEVEL1_ICACHE_SIZE) / 3) * 3);

  while (value != 0)
    {
      unsigned int byte = value & 0xff;

      if (byte == 0x40)
	{
	  *no_level_2_or_3 = true;

	  if (folded_name == _SC_LEVEL3_CACHE_SIZE)
	    /* No need to look further.  */
	    break;
	}
      else
	{
	  struct intel_02_cache_info *found;
	  struct intel_02_cache_info search;

	  search.idx = byte;
	  found = bsearch (&search, intel_02_known, nintel_02_known,
			   sizeof (intel_02_known[0]), intel_02_known_compare);
	  if (found != NULL)
	    {
	      if (found->name == folded_name)
		{
		  unsigned int offset = name - folded_name;

		  if (offset == 0)
		    /* Cache size.  */
		    return found->size;
		  if (offset == 1)
		    return found->assoc;

		  assert (offset == 2);
		  return found->linesize;
		}

	      if (found->name == _SC_LEVEL2_CACHE_SIZE)
		*has_level_2 = true;
	    }
	}

      /* Next byte for the next round.  */
      value >>= 8;
    }

  /* Nothing found.  */
  return 0;
}


static long int
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


static long int
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
  if (name == _SC_CPUTIME || name == _SC_THREAD_CPUTIME)
    {
#if HP_TIMING_AVAIL
      // XXX We can add  here test for machines which cannot support a
      // XXX usable TSC.
      return 200112L;
#else
      return -1;
#endif
    }

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
