/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

/*
 *	ISO C Standard, Amendment 1, 7.15:
 *	Wide-character classification and mapping utilities  <wctype.h>
 */

#ifndef _WCTYPE_H

#ifndef __need_iswxxx
# define _WCTYPE_H	1

# include <features.h>
# include <bits/types.h>

/* We try to get wint_t from <stddef.h>, but not all GCC versions define it
   there.  So define it ourselves if it remains undefined.  */
# define __need_wint_t
# include <stddef.h>
# ifndef _WINT_T
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
#  define _WINT_T
typedef unsigned int wint_t;
# endif

/* Constant expression of type `wint_t' whose value does not correspond
   to any member of the extended character set.  */
# ifndef WEOF
#  define WEOF (0xffffffffu)
# endif
#endif
#undef __need_iswxxx


/* The following part is also used in the <wcsmbs.h> header when compiled
   in the Unix98 compatibility mode.  */
#ifndef __iswxxx_defined
# define __iswxxx_defined	1

/* Scalar type that can hold values which represent locale-specific
   character classifications.  */
typedef unsigned long int wctype_t;

# ifndef _ISwbit
/* The characteristics are stored always in network byte order (big
   endian).  We define the bit value interpretations here dependent on the
   machine's byte order.  */

#  include <endian.h>
#  if __BYTE_ORDER == __BIG_ENDIAN
#   define _ISwbit(bit)	(1 << (bit))
#  else /* __BYTE_ORDER == __LITTLE_ENDIAN */
#   define _ISwbit(bit)	((bit) < 8 ? 1UL << (bit) << 24 : 1UL << ((bit) + 8))
#  endif

enum
{
  _ISwupper = _ISwbit (0),	/* UPPERCASE.  */
  _ISwlower = _ISwbit (1),	/* lowercase.  */
  _ISwalpha = _ISwbit (2),	/* Alphabetic.  */
  _ISwdigit = _ISwbit (3),	/* Numeric.  */
  _ISwxdigit = _ISwbit (4),	/* Hexadecimal numeric.  */
  _ISwspace = _ISwbit (5),	/* Whitespace.  */
  _ISwprint = _ISwbit (6),	/* Printing.  */
  _ISwgraph = _ISwbit (7),	/* Graphical.  */
  _ISwblank = _ISwbit (8),	/* Blank (usually SPC and TAB).  */
  _ISwcntrl = _ISwbit (9),	/* Control character.  */
  _ISwpunct = _ISwbit (10),	/* Punctuation.  */
  _ISwalnum = _ISwbit (11)	/* Alphanumeric.  */
};
# endif /* Not _ISwbit  */


__BEGIN_DECLS

/*
 * Wide-character classification functions: 7.15.2.1.
 */

/* Test for any wide character for which `iswalpha' or `iswdigit' is
   true.  */
extern int iswalnum __P ((wint_t __wc));

/* Test for any wide character for which `iswupper' or 'iswlower' is
   true, or any wide character that is one of a locale-specific set of
   wide-characters for which none of `iswcntrl', `iswdigit',
   `iswpunct', or `iswspace' is true.  */
extern int iswalpha __P ((wint_t __wc));

/* Test for any control wide character.  */
extern int iswcntrl __P ((wint_t __wc));

/* Test for any wide character that corresponds to a decimal-digit
   character.  */
extern int iswdigit __P ((wint_t __wc));

/* Test for any wide character for which `iswprint' is true and
   `iswspace' is false.  */
extern int iswgraph __P ((wint_t __wc));

/* Test for any wide character that corresponds to a lowercase letter
   or is one of a locale-specific set of wide characters for which
   none of `iswcntrl', `iswdigit', `iswpunct', or `iswspace' is true.  */
extern int iswlower __P ((wint_t __wc));

/* Test for any printing wide character.  */
extern int iswprint __P ((wint_t __wc));

/* Test for any printing wide character that is one of a
   locale-specific et of wide characters for which neither `iswspace'
   nor `iswalnum' is true.  */
extern int iswpunct __P ((wint_t __wc));

/* Test for any wide character that corresponds to a locale-specific
   set of wide characters for which none of `iswalnum', `iswgraph', or
   `iswpunct' is true.  */
extern int iswspace __P ((wint_t __wc));

/* Test for any wide character that corresponds to an uppercase letter
   or is one of a locale-specific set of wide character for which none
   of `iswcntrl', `iswdigit', `iswpunct', or `iswspace' is true.  */
extern int iswupper __P ((wint_t __wc));

/* Test for any wide character that corresponds to a hexadecimal-digit
   character equivalent to that performed be the functions described
   in the previous subclause.  */
extern int iswxdigit __P ((wint_t __wc));

/*
 * Extensible wide-character classification functions: 7.15.2.2.
 */

/* Construct value that describes a class of wide characters identified
   by the string argument PROPERTY.  */
extern wctype_t __wctype __P ((__const char *__property));
extern wctype_t wctype __P ((__const char *__property));

/* Determine whether the wide-character WC has the property described by
   DESC.  */
extern int __iswctype __P ((wint_t __wc, wctype_t __desc));
extern int iswctype __P ((wint_t __wc, wctype_t __desc));


/*
 * Wide-character case-mapping functions: 7.15.3.1.
 */

/* Scalar type that can hold values which represent locale-specific
   character mappings.  */
typedef __const __int32_t *wctrans_t;

/* Converts an uppercase letter to the corresponding lowercase letter.  */
extern wint_t towlower __P ((wint_t __wc));

/* Converts an lowercase letter to the corresponding uppercase letter.  */
extern wint_t towupper __P ((wint_t __wc));

/* Map the wide character WC using the mapping described by DESC.  */
extern wint_t __towctrans __P ((wint_t __wc, wctrans_t __desc));


# ifndef __NO_WCTYPE
#  define iswalnum(wc)	__iswctype ((wc), _ISwalnum)
#  define iswalpha(wc)	__iswctype ((wc), _ISwalpha)
#  define iswcntrl(wc)	__iswctype ((wc), _ISwcntrl)
#  define iswdigit(wc)	__iswctype ((wc), _ISwdigit)
#  define iswlower(wc)	__iswctype ((wc), _ISwlower)
#  define iswgraph(wc)	__iswctype ((wc), _ISwgraph)
#  define iswprint(wc)	__iswctype ((wc), _ISwprint)
#  define iswpunct(wc)	__iswctype ((wc), _ISwpunct)
#  define iswspace(wc)	__iswctype ((wc), _ISwspace)
#  define iswupper(wc)	__iswctype ((wc), _ISwupper)
#  define iswxdigit(wc)	__iswctype ((wc), _ISwxdigit)

#  ifdef __USE_GNU
#   define iswblank(wc)	__iswctype ((wc), _ISwblank)
#  endif


/* Pointer to conversion tables.  */
extern __const __int32_t *__ctype_tolower; /* Case conversions.  */
extern __const __int32_t *__ctype_toupper; /* Case conversions.  */

#  define towlower(wc)	__towctrans ((wc), __ctype_tolower)
#  define towupper(wc)	__towctrans ((wc), __ctype_toupper)

# endif /* Not __NO_WCTYPE.  */

__END_DECLS

#endif	/* need iswxxx.  */


/* The remaining definitions and declarations must not appear in the
   <wcsmbs.h> header.  */
#ifdef _WCTYPE_H

/*
 * Extensible wide-character mapping functions: 7.15.3.2.
 */

__BEGIN_DECLS

/* Construct value that describes a mapping between wide characters
   identified by the string argument PROPERTY.  */
extern wctrans_t wctrans __P ((__const char *__property));

/* Map the wide character WC using the mapping described by DESC.  */
extern wint_t towctrans __P ((wint_t __wc, wctrans_t __desc));

# ifdef __USE_GNU
/* Declare the interface to extended locale model.  */
#  include <xlocale.h>

/* Test for any wide character for which `iswalpha' or `iswdigit' is
   true.  */
extern int __iswalnum_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character for which `iswupper' or 'iswlower' is
   true, or any wide character that is one of a locale-specific set of
   wide-characters for which none of `iswcntrl', `iswdigit',
   `iswpunct', or `iswspace' is true.  */
extern int __iswalpha_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any control wide character.  */
extern int __iswcntrl_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character that corresponds to a decimal-digit
   character.  */
extern int __iswdigit_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character for which `iswprint' is true and
   `iswspace' is false.  */
extern int __iswgraph_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character that corresponds to a lowercase letter
   or is one of a locale-specific set of wide characters for which
   none of `iswcntrl', `iswdigit', `iswpunct', or `iswspace' is true.  */
extern int __iswlower_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any printing wide character.  */
extern int __iswprint_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any printing wide character that is one of a
   locale-specific et of wide characters for which neither `iswspace'
   nor `iswalnum' is true.  */
extern int __iswpunct_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character that corresponds to a locale-specific
   set of wide characters for which none of `iswalnum', `iswgraph', or
   `iswpunct' is true.  */
extern int __iswspace_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character that corresponds to an uppercase letter
   or is one of a locale-specific set of wide character for which none
   of `iswcntrl', `iswdigit', `iswpunct', or `iswspace' is true.  */
extern int __iswupper_l __P ((wint_t __wc, __locale_t __locale));

/* Test for any wide character that corresponds to a hexadecimal-digit
   character equivalent to that performed be the functions described
   in the previous subclause.  */
extern int __iswxdigit_l __P ((wint_t __wc, __locale_t __locale));


/* Construct value that describes a class of wide characters identified
   by the string argument PROPERTY.  */
extern wctype_t __wctype_l __P ((__const char *__property,
				 __locale_t __locale));

/* Determine whether the wide-character WC has the property described by
   DESC.  */
extern int __iswctype_l __P ((wint_t __wc, wctype_t __desc,
			      __locale_t __locale));


/*
 * Wide-character case-mapping functions.
 */

/* Converts an uppercase letter to the corresponding lowercase letter.  */
extern wint_t __towlower_l __P ((wint_t __wc, __locale_t __locale));

/* Converts an lowercase letter to the corresponding uppercase letter.  */
extern wint_t __towupper_l __P ((wint_t __wc, __locale_t __locale));

/* Map the wide character WC using the mapping described by DESC.  */
extern wint_t __towctrans_l __P ((wint_t __wc, wctrans_t __desc,
				  __locale_t __locale));


#  ifndef __NO_WCTYPE
#   define __iswalnum_l(wc, loc) __iswctype_l ((wc), _ISwalnum, (loc))
#   define __iswalpha_l(wc, loc) __iswctype_l ((wc), _ISwalpha, (loc))
#   define __iswcntrl_l(wc, loc) __iswctype_l ((wc), _ISwcntrl, (loc))
#   define __iswdigit_l(wc, loc) __iswctype_l ((wc), _ISwdigit, (loc))
#   define __iswlower_l(wc, loc) __iswctype_l ((wc), _ISwlower, (loc))
#   define __iswgraph_l(wc, loc) __iswctype_l ((wc), _ISwgraph, (loc))
#   define __iswprint_l(wc, loc) __iswctype_l ((wc), _ISwprint, (loc))
#   define __iswpunct_l(wc, loc) __iswctype_l ((wc), _ISwpunct, (loc))
#   define __iswspace_l(wc, loc) __iswctype_l ((wc), _ISwspace, (loc))
#   define __iswupper_l(wc, loc) __iswctype_l ((wc), _ISwupper, (loc))
#   define __iswxdigit_l(wc, loc) __iswctype_l ((wc), _ISwxdigit, (loc))

#   define __iswblank_l(wc, loc) __iswctype_l ((wc), _ISwblank, (loc))

#   define __towlower_l(wc, loc) __towctrans_l ((wc), (loc)->__ctype_tolower, \
						(loc))
#   define __towupper_l(wc, loc) __towctrans_l ((wc), (loc)->__ctype_toupper, \
						(loc))

#  endif /* Not __NO_WCTYPE.  */

# endif /* Use GNU.  */

__END_DECLS

#endif	/* __WCTYPE_H defined.  */

#endif /* wctype.h  */
