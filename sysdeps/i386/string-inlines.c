/* Copyright (C) 1999, 2002, 2003 Free Software Foundation, Inc.
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

/*  <bits/string.h> and <bits/string2.h> declare some extern inline
    functions.  These functions are declared additionally here if
    inlining is not possible.  */

#undef __USE_STRING_INLINES
#define __USE_STRING_INLINES
#define _FORCE_INLINES
#define __STRING_INLINE /* empty */
#define __NO_INLINE__

#include <string.h>
#undef index
#undef rindex

#undef __NO_INLINE__
#include <bits/string.h>
#include <bits/string2.h>

/* Functions which are inlines in i486 but not i386.  */
void *
__memcpy_by2 (void *dest, const void *src, size_t n)
{
  return memcpy (dest, src, n);
}
strong_alias (__memcpy_by2, __memcpy_by4)
strong_alias (__memcpy_by2, __memcpy_g)
strong_alias (__memcpy_by2, __memcpy_g_internal)

void *
__memset_ccn_by2 (void *s, unsigned int c, size_t n)
{
  return memset (s, c & 0xff, n);
}
strong_alias (__memset_ccn_by2, __memset_ccn_by4)

void *
__memset_gcn_by2 (void *s, int c, size_t n)
{
  return memset (s, c, n);
}
strong_alias (__memset_gcn_by2, __memset_gcn_by4)

size_t
__strlen_g (const char *s)
{
  return strlen (s);
}

char *
__strcpy_g (char *d, const char *s)
{
  return strcpy (d, s);
}

char *
__mempcpy_by2 (char *d, const char *s, size_t n)
{
  return mempcpy (d, s, n);
}
strong_alias (__mempcpy_by2, __mempcpy_by4)
strong_alias (__mempcpy_by2, __mempcpy_byn)

char *
__stpcpy_g (char *d, const char *s)
{
  return stpcpy (d, s);
}

char *
__strncpy_by2 (char *d, const char s[], size_t srclen, size_t n)
{
  return strncpy (d, s, n);
}
strong_alias (__strncpy_by2, __strncpy_by4)
strong_alias (__strncpy_by2, __strncpy_byn)

char *
__strncpy_gg (char *d, const char *s, size_t n)
{
  return strncpy (d, s, n);
}

char *
__strcat_c (char *d, const char s[], size_t srclen)
{
  return strcat (d, s);
}

char *
__strcat_g (char *d, const char *s)
{
  return strcat (d, s);
}

char *
__strncat_g (char *d, const char s[], size_t n)
{
  return strncat (d, s, n);
}

int
__strcmp_gg (const char *s1, const char *s2)
{
  return strcmp (s1, s2);
}

int
__strncmp_g (const char *s1, const char *s2, size_t n)
{
  return strncmp (s1, s2, n);
}

char *
__strrchr_c (const char *s, int c)
{
  return strrchr (s, c >> 8);
}

char *
__strrchr_g (const char *s, int c)
{
  return strrchr (s, c);
}

size_t
__strcspn_cg (const char *s, const char reject[], size_t reject_len)
{
  return strcspn (s, reject);
}

size_t
__strcspn_g (const char *s, const char *reject)
{
  return strcspn (s, reject);
}

size_t
__strspn_cg (const char *s, const char accept[], size_t accept_len)
{
  return strspn (s, accept);
}

size_t
__strspn_g (const char *s, const char *accept)
{
  return strspn (s, accept);
}

char *
__strpbrk_cg (const char *s, const char accept[], size_t accept_len)
{
  return strpbrk (s, accept);
}

char *
__strpbrk_g (const char *s, const char *accept)
{
  return strpbrk (s, accept);
}

char *
__strstr_cg (const char *haystack, const char needle[], size_t needle_len)
{
  return strstr (haystack, needle);
}

char *
__strstr_g (const char *haystack, const char needle[])
{
  return strstr (haystack, needle);
}
