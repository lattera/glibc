/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard 4.3: CHARACTER HANDLING	<ctype.h>
 */

#ifndef	_CTYPE_H

#define	_CTYPE_H	1
#include <features.h>

__BEGIN_DECLS

/* These are all the characteristics of characters.  All the
   interdependencies (such as that an alphabetic is an uppercase or a
   lowercase) are here.  If there get to be more than
   (sizeof (unsigned short int) * CHAR_BIT) distinct characteristics,
   many things must be changed that use `unsigned short int's.  */
enum
{
  _ISupper = 1 << 0,		/* UPPERCASE.  */
  _ISlower = 1 << 1,		/* lowercase.  */
  _ISalpha = 1 << 2,	        /* Alphabetic.  */
  _ISdigit = 1 << 3,		/* Numeric.  */
  _ISxdigit = 1 << 4,           /* Hexadecimal numeric.  */
  _ISspace = 1 << 5,		/* Whitespace.  */
  _ISprint = 1 << 6,            /* Printing.  */
  _ISgraph = 1 << 7,	        /* Graphical.  */
  _ISblank = 1 << 8,		/* Blank (usually SPC and TAB).  */
  _IScntrl = 1 << 9,		/* Control character.  */
  _ISpunct = 1 << 10,		/* Punctuation.  */

  /* The following are defined in POSIX.2 as being combinations of the
     classes above.  */
  _ISalnum = _ISalpha | _ISdigit	/* Alphanumeric.  */
};

/* These are defined in localeinfo.c.
   The declarations here must match those in localeinfo.h.

   These point to the second element ([1]) of arrays of size (UCHAR_MAX + 1).
   EOF is -1, so [EOF] is the first element of the original array.
   ANSI requires that the ctype functions work for `unsigned char' values
   and for EOF.  The case conversion arrays are of `short int's rather than
   `unsigned char's because tolower (EOF) must be EOF, which doesn't fit
   into an `unsigned char'.  */
extern __const unsigned short int *__ctype_b;	/* Characteristics.  */
extern __const short int *__ctype_tolower;	/* Case conversions.  */
extern __const short int *__ctype_toupper;	/* Case conversions.  */

#define	__isctype(c, type) \
  (__ctype_b[(int) (c)] & (unsigned short int) type)

#define	__isascii(c)	(((c) & (1 << 7)) == 0)	/* If high bit is set.  */
#define	__toascii(c)	((c) & 0x7f) /* Mask off high bit.  */

#define	__tolower(c)	((int) __ctype_tolower[(int) (c)])
#define	__toupper(c)	((int) __ctype_toupper[(int) (c)])

#define	__exctype(name)	extern int name __P ((int))

/* The following names are all functions:
     int isCHARACTERISTIC(int c);
   which return nonzero iff C has CHARACTERISTIC.
   For the meaning of the characteristic names, see the `enum' above.  */
__exctype (isalnum);
__exctype (isalpha);
__exctype (iscntrl);
__exctype (isdigit);
__exctype (islower);
__exctype (isgraph);
__exctype (isprint);
__exctype (ispunct);
__exctype (isspace);
__exctype (isupper);
__exctype (isxdigit);

#ifdef	__USE_GNU
__exctype (isblank);
#endif


/* Return the lowercase version of C.  */
extern int tolower __P ((int __c));

/* Return the uppercase version of C.  */
extern int toupper __P ((int __c));


#if defined(__USE_SVID) || defined(__USE_MISC)

/* Return nonzero iff C is in the ASCII set
   (i.e., is no more than 7 bits wide).  */
extern int isascii __P ((int __c));

/* Return the part of C that is in the ASCII set
   (i.e., the low-order 7 bits of C).  */
extern int toascii __P ((int __c));

#endif /* Use SVID or use misc.  */

#ifdef	__USE_SVID
/* These are the same as `toupper' and `tolower'.  */
__exctype (_toupper);
__exctype (_tolower);
#endif

#ifndef	__NO_CTYPE
#define	isalnum(c)	__isctype((c), _ISalnum)
#define	isalpha(c)	__isctype((c), _ISalpha)
#define	iscntrl(c)	__isctype((c), _IScntrl)
#define	isdigit(c)	__isctype((c), _ISdigit)
#define	islower(c)	__isctype((c), _ISlower)
#define	isgraph(c)	__isctype((c), _ISgraph)
#define	isprint(c)	__isctype((c), _ISprint)
#define	ispunct(c)	__isctype((c), _ISpunct)
#define	isspace(c)	__isctype((c), _ISspace)
#define	isupper(c)	__isctype((c), _ISupper)
#define	isxdigit(c)	__isctype((c), _ISxdigit)

#ifdef	__USE_GNU
#define	isblank(c)	__isctype((c), _ISblank)
#endif

#define	tolower(c)	__tolower(c)
#define	toupper(c)	__toupper(c)

#if defined(__USE_SVID) || defined(__USE_MISC)
#define	isascii(c)	__isascii(c)
#define	toascii(c)	__toascii(c)
#endif

#endif /* Not __NO_CTYPE.  */

__END_DECLS

#endif /* ctype.h  */
