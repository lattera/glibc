/* Copyright (C) 2004 Free Software Foundation, Inc.
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

#ifndef _STRING_H
# error "Never use <bits/string3.h> directly; include <string.h> instead."
#endif

/* XXX This is temporarily.  We should not redefine any of the symbols
   and instead integrate the error checking into the original
   definitions.  */
#undef memcpy
#undef memmove
#undef memset
#undef strcat
#undef strcpy
#undef strncat
#undef strncpy
#ifdef __USE_GNU
# undef mempcpy
# undef stpcpy
#endif
#ifdef __USE_BSD
# undef bcopy
# undef bzero
#endif


#define memcpy(dest, src, len) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___memcpy_chk (dest, src, len, __bos0 (dest))		\
   : __memcpy_ichk (dest, src, len))
static __inline__ void *
__attribute__ ((__always_inline__))
__memcpy_ichk (void *__restrict __dest, const void *__restrict __src,
	       size_t __len)
{
  return __builtin___memcpy_chk (__dest, __src, __len, __bos0 (__dest));
}


#define memmove(dest, src, len) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___memmove_chk (dest, src, len, __bos0 (dest))		\
   : __memmove_ichk (dest, src, len))
static __inline__ void *
__attribute__ ((__always_inline__))
__memmove_ichk (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len, __bos0 (__dest));
}


#ifdef __USE_GNU
# define mempcpy(dest, src, len) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___mempcpy_chk (dest, src, len, __bos0 (dest))		\
   : __mempcpy_ichk (dest, src, len))
static __inline__ void *
__attribute__ ((__always_inline__))
__mempcpy_ichk (void *__restrict __dest, const void *__restrict __src,
		size_t __len)
{
  return __builtin___mempcpy_chk (__dest, __src, __len, __bos0 (__dest));
}
#endif


#define memset(dest, ch, len) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___memset_chk (dest, ch, len, __bos0 (dest))		\
   : __memset_ichk (dest, ch, len))
static __inline__ void *
__attribute__ ((__always_inline__))
__memset_ichk (void *__dest, int __ch, size_t __len)
{
  return __builtin___memset_chk (__dest, __ch, __len, __bos0 (__dest));
}

#ifdef __USE_BSD
# define bcopy(src, dest, len) ((void) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___memmove_chk (dest, src, len, __bos0 (dest))		\
   : __memmove_ichk (dest, src, len)))
# define bzero(dest, len) ((void) \
  ((__bos0 (dest) != (size_t) -1)					\
   ? __builtin___memset_chk (dest, '\0', len, __bos0 (dest))		\
   : __memset_ichk (dest, '\0', len)))
#endif


#define strcpy(dest, src) \
  ((__bos (dest) != (size_t) -1)					\
   ? __builtin___strcpy_chk (dest, src, __bos (dest))			\
   : __strcpy_ichk (dest, src))
static __inline__ char *
__attribute__ ((__always_inline__))
__strcpy_ichk (char *__restrict __dest, const char *__restrict __src)
{
  return __builtin___strcpy_chk (__dest, __src, __bos (__dest));
}


#ifdef __USE_GNU
# define stpcpy(dest, src) \
  ((__bos (dest) != (size_t) -1)					\
   ? __builtin___stpcpy_chk (dest, src, __bos (dest))			\
   : __stpcpy_ichk (dest, src))
static __inline__ char *
__attribute__ ((__always_inline__))
__stpcpy_ichk (char *__restrict __dest, const char *__restrict __src)
{
  return __builtin___stpcpy_chk (__dest, __src, __bos (__dest));
}
#endif


#define strncpy(dest, src, len) \
  ((__bos (dest) != (size_t) -1)					\
   ? __builtin___strncpy_chk (dest, src, len, __bos (dest))		\
   : __strncpy_ichk (dest, src, len))
static __inline__ char *
__attribute__ ((__always_inline__))
__strncpy_ichk (char *__restrict __dest, const char *__restrict __src,
		size_t __len)
{
  return __builtin___strncpy_chk (__dest, __src, __len, __bos (__dest));
}


#define strcat(dest, src) \
  ((__bos (dest) != (size_t) -1)					\
   ? __builtin___strcat_chk (dest, src, __bos (dest))			\
   : __strcat_ichk (dest, src))
static __inline__ char *
__attribute__ ((__always_inline__))
__strcat_ichk (char *__restrict __dest, const char *__restrict __src)
{
  return __builtin___strcat_chk (__dest, __src, __bos (__dest));
}


#define strncat(dest, src, len) \
  ((__bos (dest) != (size_t) -1)					\
   ? __builtin___strncat_chk (dest, src, len, __bos (dest))		\
   : __strncat_ichk (dest, src, len))
static __inline__ char *
__attribute__ ((__always_inline__))
__strncat_ichk (char *__restrict __dest, const char *__restrict __src,
		size_t __len)
{
  return __builtin___strncat_chk (__dest, __src, __len, __bos (__dest));
}
