/* Optimized, inlined string functions.  ARM version.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _STRING_H
# error "Never use <bits/string.h> directly; include <string.h> instead."
#endif

/* We must defeat the generic optimized versions of these functions in
   <bits/string2.h> since they don't work on the ARM.  */
#define _HAVE_STRING_ARCH_strcpy 1
#define _HAVE_STRING_ARCH_stpcpy 1

/* We only provide optimizations if GNU CC is used and this is a little
   endian system (the code below does not work on big endian machines).
   With current versions of GCC these optimi\ations produce quite a large
   amount of code so we only enable them if the user specifically asked
   for it.  */
#if !defined __NO_STRING_INLINES && defined __GNUC__ && __GNUC__ >= 2 \
 && !defined __ARMEB__ && defined __USE_STRING_INLINES

/* Copy SRC to DEST.  */
#define strcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (__string2_1bptr_p (src) && strlen (src) + 1 <= 8	      \
		     ? __strcpy_small (dest, src, strlen (src) + 1)	      \
		     : (char *) memcpy (dest, src, strlen (src) + 1))	      \
		  : strcpy (dest, src)))

#define __strcpy_small(dest, src, srclen) \
  (__extension__ ({ char *__dest = (char *) (dest);			      \
		    const char *__src = (const char *) (src);		      \
		    size_t __srclen = (srclen);				      \
		    switch (__srclen)					      \
		      {							      \
		      case 5:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 1:						      \
			*__dest++ = '\0';				      \
			break;						      \
		      case 6:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 2:						      \
			*((unsigned short int *) __dest) =		      \
			     __src[0];					      \
			__dest += 2;					      \
			break;						      \
		      case 7:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 3:						      \
			*((unsigned short int *) __dest) =		      \
			     *((const unsigned short int *) (__src));	      \
			__dest[2] = '\0';				      \
			__dest += 3;					      \
			break;						      \
		      case 8:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
  			__src += 4;					      \
		      case 4:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned short int *) (__src)) |	      \
		             (__src[2] << 16);				      \
			__dest += 4;					      \
			break;						      \
		    }							      \
		  (__dest - __srclen) ; }))

/* Copy SRC to DEST, returning pointer to final NUL byte.  */
#define __stpcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (__string2_1bptr_p (src) && strlen (src) + 1 <= 8	      \
		     ? __stpcpy_small (dest, src, strlen (src) + 1)	      \
		     : ((char *) __mempcpy (dest, src, strlen (src) + 1) - 1))\
		  : __stpcpy (dest, src)))

#define __stpcpy_small(dest, src, srclen) \
  (__extension__ ({ char *__dest = (char *) (dest);			      \
		    const char *__src = (const char *) (src);		      \
		    switch (srclen)					      \
		      {							      \
		      case 5:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 1:						      \
			*__dest++ = '\0';				      \
			break;						      \
		      case 6:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 2:						      \
			*((unsigned short int *) __dest) =		      \
			     __src[0];					      \
			__dest += 2;					      \
			break;						      \
		      case 7:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 3:						      \
			*((unsigned short int *) __dest) =		      \
			     *((const unsigned short int *) (__src));	      \
			__dest[2] = '\0';				      \
			__dest += 3;					      \
			break;						      \
		      case 8:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned long int *) (__src));	      \
			__dest += 4;					      \
			__src += 4;					      \
		      case 4:						      \
			*((unsigned long int *) __dest) =		      \
			     *((const unsigned short int *) (__src)) |	      \
		             (__src[2] << 16);				      \
			__dest += 4;					      \
			break;						      \
		    }							      \
		  __dest; }))

#endif
