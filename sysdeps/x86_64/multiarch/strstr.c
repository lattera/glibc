/* strstr with SSE4.2 intrinsics
   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

#include <nmmintrin.h>
#include "varshift.h"

#ifndef STRSTR_SSE42
# define STRSTR_SSE42 __strstr_sse42
#endif

#ifdef USE_AS_STRCASESTR
# include <ctype.h>
# include <locale/localeinfo.h>

# define LOADBYTE(C)		tolower (C)
# define CMPBYTE(C1, C2)	(tolower (C1) == tolower (C2))
#else
# define LOADBYTE(C)		(C)
# define CMPBYTE(C1, C2)	((C1) == (C2))
#endif

/* We use 0xe ordered-compare:
	_SIDD_SBYTE_OPS
	| _SIDD_CMP_EQUAL_ORDER
	| _SIDD_LEAST_SIGNIFICANT
   on pcmpistri to do the scanning and string comparsion requirements of
   sub-string match.  In the scanning phase, we process Cflag and ECX
   index to locate the first fragment match; once the first fragment
   match position has been identified, we do comparison of subsequent
   string fragments until we can conclude false or true match; whe
   n concluding a false match, we may need to repeat scanning process
   from next relevant offset in the target string.

   In the scanning phase we have 4 cases:
   case		ECX	CFlag	ZFlag	SFlag
    1		16	  0	  0	  0
    2a		16	  0	  0	  1
    2b		16	  0	  1	  0
    2c		16	  0	  1	  1

   1. No ordered-comparison match, both 16B fragments are valid, so
      continue to next fragment.
   2. No ordered-comparison match, there is EOS in either fragment,
   2a. Zflg = 0, Sflg = 1, we continue
   2b. Zflg = 1, Sflg = 0, we conclude no match and return.
   2c. Zflg = 1, sflg = 1, lenth determine match or no match

   In the string comparison phase, the 1st fragment match is fixed up
   to produce ECX = 0.  Subsequent fragment compare of nonzero index
   and no match conclude a false match.

   case		ECX	CFlag	ZFlag	SFlag
    3		 X	  1	  0	  0/1
    4a		 0	  1	  0	  0
    4b		 0	  1	  0	  1
    4c		0 < X	  1	  0	  0/1
    5		16	  0	  1	  0

   3. An initial ordered-comparison fragment match, we fix up to do
      subsequent string comparison
   4a. Continuation of fragment comparison of a string compare.
   4b. EOS reached in the reference string, we conclude true match and
       return
   4c. String compare failed if index is nonzero, we need to go back to
       scanning
   5.  failed string compare, go back to scanning
 */

/* Simple replacement of movdqu to address 4KB boundary cross issue.
   If EOS occurs within less than 16B before 4KB boundary, we don't
   cross to next page.  */

static inline __m128i
__m128i_strloadu (const unsigned char * p, __m128i zero)
{
  if (__builtin_expect ((int) ((size_t) p & 0xfff) > 0xff0, 0))
    {
      size_t offset = ((size_t) p & (16 - 1));
      __m128i a = _mm_load_si128 ((__m128i *) (p - offset));
      int bmsk = _mm_movemask_epi8 (_mm_cmpeq_epi8 (a, zero));
      if ((bmsk >> offset) != 0)
	return __m128i_shift_right (a, offset);
    }
  return _mm_loadu_si128 ((__m128i *) p);
}

#if defined USE_AS_STRCASESTR && !defined STRCASESTR_NONASCII

/* Similar to __m128i_strloadu.  Convert to lower case for POSIX/C
   locale and other which have single-byte letters only in the ASCII
   range.  */
static inline __m128i
__m128i_strloadu_tolower (const unsigned char *p, __m128i zero, __m128i uclow,
			  __m128i uchigh, __m128i lcqword)
{
  __m128i frag = __m128i_strloadu (p, zero);

  /* Compare if 'Z' > bytes. Inverted way to get a mask for byte <= 'Z'.  */
  __m128i r2 = _mm_cmpgt_epi8 (uchigh, frag);
  /* Compare if bytes are > 'A' - 1.  */
  __m128i r1 = _mm_cmpgt_epi8 (frag, uclow);
  /* Mask byte == ff if byte(r2) <= 'Z' and byte(r1) > 'A' - 1.  */
  __m128i mask = _mm_and_si128 (r2, r1);
  /* Apply lowercase bit 6 mask for above mask bytes == ff.  */
  return _mm_or_si128 (frag, _mm_and_si128 (mask, lcqword));
}

#endif

/* Calculate Knuth-Morris-Pratt string searching algorithm (or KMP
   algorithm) overlap for a fully populated 16B vector.
   Input parameter: 1st 16Byte loaded from the reference string of a
		    strstr function.
   We don't use KMP algorithm if reference string is less than 16B.  */
static int
__inline__ __attribute__ ((__always_inline__,))
KMP16Bovrlap (__m128i s2)
{
  __m128i b = _mm_unpacklo_epi8 (s2, s2);
  __m128i a = _mm_unpacklo_epi8 (b, b);
  a = _mm_shuffle_epi32 (a, 0);
  b = _mm_srli_si128 (s2, sizeof (char));
  int bmsk = _mm_movemask_epi8 (_mm_cmpeq_epi8 (b, a));

  /* _BitScanForward(&k1, bmsk); */
  int k1;
  __asm ("bsfl %[bmsk], %[k1]" : [k1] "=r" (k1) : [bmsk] "r" (bmsk));
  if (!bmsk)
    return 16;
  else if (bmsk == 0x7fff)
    return 1;
  else if (!k1)
    {
      /* There are al least two distinct chars in s2.  If byte 0 and 1 are
	 idential and the distinct value lies farther down, we can deduce
	 the next byte offset to restart full compare is least no earlier
	 than byte 3.  */
      return 3;
    }
  else
    {
      /* Byte 1 is not degenerated to byte 0.  */
      return k1 + 1;
    }
}

char *
__attribute__ ((section (".text.sse4.2")))
STRSTR_SSE42 (const unsigned char *s1, const unsigned char *s2)
{
#define p1 s1
  const unsigned char *p2 = s2;

#ifndef STRCASESTR_NONASCII
  if (__builtin_expect (p2[0] == '\0', 0))
    return (char *) p1;

  if (__builtin_expect (p1[0] == '\0', 0))
    return NULL;

  /* Check if p1 length is 1 byte long.  */
  if (__builtin_expect (p1[1] == '\0', 0))
    return p2[1] == '\0' && CMPBYTE (p1[0], p2[0]) ? (char *) p1 : NULL;
#endif

#ifdef USE_AS_STRCASESTR
# ifndef STRCASESTR_NONASCII
  if (__builtin_expect (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_NONASCII_CASE)
			!= 0, 0))
    return __strcasestr_sse42_nonascii (s1, s2);

  const __m128i uclow = _mm_set1_epi8 (0x40);
  const __m128i uchigh = _mm_set1_epi8 (0x5b);
  const __m128i lcqword = _mm_set1_epi8 (0x20);
  const __m128i zero = _mm_setzero_si128 ();
#  define strloadu(p) __m128i_strloadu_tolower (p, zero, uclow, uchigh, lcqword)
# else
#  define strloadu __m128i_strloadu_tolower
#  define zero _mm_setzero_si128 ()
# endif
#else
# define strloadu(p) __m128i_strloadu (p, zero)
  const __m128i zero = _mm_setzero_si128 ();
#endif

  /* p1 > 1 byte long.  Load up to 16 bytes of fragment.  */
  __m128i frag1 = strloadu (p1);

  __m128i frag2;
  if (p2[1] != '\0')
    /* p2 is > 1 byte long.  */
    frag2 = strloadu (p2);
  else
    frag2 = _mm_insert_epi8 (zero, LOADBYTE (p2[0]), 0);

  /* Unsigned bytes, equal order, does frag2 has null?  */
  int cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
  int cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
  int cmp = _mm_cmpistri (frag2, frag1, 0x0c);
  int cmp_s = _mm_cmpistrs (frag2, frag1, 0x0c);
  if (cmp_s & cmp_c)
    {
      int bmsk = _mm_movemask_epi8 (_mm_cmpeq_epi8 (frag2, zero));
      int len;
      __asm ("bsfl %[bmsk], %[len]"
	     : [len] "=r" (len) : [bmsk] "r" (bmsk));
      p1 += cmp;
      if ((len + cmp) <= 16)
	return (char *) p1;

      /* Load up to 16 bytes of fragment.  */
      frag1 = strloadu (p1);
      cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
      cmp_s = _mm_cmpistrs (frag2, frag1, 0x0c);
      cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
      cmp = _mm_cmpistri (frag2, frag1, 0x0c);
      if ((len + cmp) <= 16)
	return (char *) p1 + cmp;
    }

  if (cmp_s)
    {
      /* Adjust addr for 16B alginment in ensuing loop.  */
      while (!cmp_z)
	{
	  p1 += cmp;
	  /* Load up to 16 bytes of fragment.  */
	  frag1 = strloadu (p1);
	  cmp = _mm_cmpistri (frag2, frag1, 0x0c);
	  cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
	  cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
	  /* Because s2 < 16 bytes and we adjusted p1 by non-zero cmp
	     once already, this time cmp will be zero and we can exit.  */
	  if ((!cmp) & cmp_c)
	    break;
	}

      if (!cmp_c)
	return NULL;

      /* Since s2 is less than 16 bytes, com_c is definitive
	 determination of full match.  */
      return (char *) p1 + cmp;
    }

  /* General case, s2 is at least 16 bytes or more.
     First, the common case of false-match at first byte of p2.  */
  const unsigned char *pt = NULL;
  int kmp_fwd = 0;
re_trace:
  while (!cmp_c)
    {
      /* frag1 has null. */
      if (cmp_z)
	return NULL;

      /* frag 1 has no null, advance 16 bytes.  */
      p1 += 16;
      /* Load up to 16 bytes of fragment.  */
      frag1 = strloadu (p1);
      /* Unsigned bytes, equal order, is there a partial match?  */
      cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
      cmp = _mm_cmpistri (frag2, frag1, 0x0c);
      cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
    }

  /* Next, handle initial positive match as first byte of p2.  We have
     a partial fragment match, make full determination until we reached
     end of s2.  */
  if (!cmp)
    {
      if (cmp_z)
	return (char *) p1;

      pt = p1;
      p1 += 16;
      p2 += 16;
      /* Load up to 16 bytes of fragment.  */
      frag2 = strloadu (p2);
    }
  else
    {
      /* Adjust 16B alignment.  */
      p1 += cmp;
      pt = p1;
    }

  /* Load up to 16 bytes of fragment.  */
  frag1 = strloadu (p1);

  /* Unsigned bytes, equal order, does frag2 has null?  */
  cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
  cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
  cmp = _mm_cmpistri (frag2, frag1, 0x0c);
  cmp_s = _mm_cmpistrs (frag2, frag1, 0x0c);
  while (!(cmp | cmp_z | cmp_s))
    {
      p1 += 16;
      p2 += 16;
      /* Load up to 16 bytes of fragment.  */
      frag2 = strloadu (p2);
      /* Load up to 16 bytes of fragment.  */
      frag1 = strloadu (p1);
      /* Unsigned bytes, equal order, does frag2 has null?  */
      cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
      cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
      cmp = _mm_cmpistri (frag2, frag1, 0x0c);
      cmp_s = _mm_cmpistrs (frag2, frag1, 0x0c);
    }

  /* Full determination yielded a false result, retrace s1 to next
     starting position.
     Zflg	1      0      1			0/1
     Sflg	0      1      1			0/1
     cmp	na     0      0			>0
     action   done   done   continue    continue if s2 < s1
	      false  match  retrace s1     else false
   */

  if (cmp_s & !cmp)
    return (char *) pt;
  if (cmp_z)
    {
      if (!cmp_s)
	return NULL;

      /* Handle both zero and sign flag set and s1 is shorter in
	 length.  */
      int bmsk = _mm_movemask_epi8 (_mm_cmpeq_epi8 (zero, frag2));
      int bmsk1 = _mm_movemask_epi8 (_mm_cmpeq_epi8 (zero, frag1));
      int len;
      int len1;
      __asm ("bsfl %[bmsk], %[len]"
	     : [len] "=r" (len) : [bmsk] "r" (bmsk));
      __asm ("bsfl %[bmsk1], %[len1]"
	     : [len1] "=r" (len1) : [bmsk1] "r" (bmsk1));
      if (len >= len1)
	return NULL;
    }
  else if (!cmp)
    return (char *) pt;

  /* Otherwise, we have to retrace and continue.  Default of multiple
     paths that need to retrace from next byte in s1.  */
  p2 = s2;
  frag2 = strloadu (p2);

  if (!kmp_fwd)
    kmp_fwd = KMP16Bovrlap (frag2);

  /* KMP algorithm predicted overlap needs to be corrected for
     partial fragment compare.  */
  p1 = pt + (kmp_fwd > cmp ? cmp : kmp_fwd);

  /* Since s2 is at least 16 bytes long, we're certain there is no
     match.  */
  if (p1[0] == '\0')
    return NULL;

  /* Load up to 16 bytes of fragment.  */
  frag1 = strloadu (p1);

  /* Unsigned bytes, equal order, is there a partial match?  */
  cmp_c = _mm_cmpistrc (frag2, frag1, 0x0c);
  cmp = _mm_cmpistri (frag2, frag1, 0x0c);
  cmp_z = _mm_cmpistrz (frag2, frag1, 0x0c);
  goto re_trace;
}
