/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
 *	ANSI Standard: 4.10 GENERAL UTILITIES	<stdlib.h>
 */

#ifndef	_STDLIB_H

#define	_STDLIB_H	1
#include <features.h>

/* Get size_t, wchar_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_wchar_t
#define	__need_NULL
#include <stddef.h>

#define	__need_Emath
#include <errno.h>

__BEGIN_DECLS

/* Returned by `div'.  */
typedef struct
  {
    int quot;			/* Quotient.  */
    int rem;			/* Remainder.  */
  } div_t;

/* Returned by `ldiv'.  */
typedef struct
  {
    long int quot;		/* Quotient.  */
    long int rem;		/* Remainder.  */
  } ldiv_t;


/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */


/* Maximum length of a multibyte character in the current locale.
   This is just one until the fancy locale support is finished.  */
#define	MB_CUR_MAX	1


/* Convert a string to a floating-point number.  */
extern double atof __P ((__const char *__nptr));
/* Convert a string to an integer.  */
extern int atoi __P ((__const char *__nptr));
/* Convert a string to a long integer.  */
extern long int atol __P ((__const char *__nptr));

/* Convert a string to a floating-point number.  */
extern double strtod __P ((__const char *__nptr, char **__endptr));

#ifdef	__USE_GNU
/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float __strtof __P ((__const char *__nptr, char **__endptr));
extern float strtof __P ((__const char *__nptr, char **__endptr));
extern __long_double_t __strtold __P ((__const char *__nptr, char **__endptr));
extern __long_double_t strtold __P ((__const char *__nptr, char **__endptr));
#endif

/* Convert a string to a long integer.  */
extern long int strtol __P ((__const char *__nptr, char **__endptr,
			     int __base));
/* Convert a string to an unsigned long integer.  */
extern unsigned long int strtoul __P ((__const char *__nptr,
				       char **__endptr, int __base));

#if defined (__GNUC__) && defined (__USE_BSD)
/* Convert a string to a quadword integer.  */
extern long long int strtoq __P ((__const char *__nptr, char **__endptr,
				  int __base));
/* Convert a string to an unsigned quadword integer.  */
extern unsigned long long int strtouq __P ((__const char *__nptr,
					    char **__endptr, int __base));
#endif /* GCC and use BSD.  */

#if defined (__OPTIMIZE__) && __GNUC__ >= 2
extern __inline double atof (__const char *__nptr)
{ return strtod(__nptr, (char **) NULL); }
extern __inline int atoi (__const char *__nptr)
{ return (int) strtol (__nptr, (char **) NULL, 10); }
extern __inline long int atol (__const char *__nptr)
{ return strtol (__nptr, (char **) NULL, 10); }
#endif /* Optimizing GCC >=2.  */


/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand __P ((void));
/* Seed the random number generator with the given number.  */
extern void srand __P ((unsigned int __seed));

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and RAND_MAX inclusive.  */
extern long int __random __P ((void));
/* Seed the random number generator with the given number.  */
extern void __srandom __P ((unsigned int __seed));

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern __ptr_t __initstate __P ((unsigned int __seed, __ptr_t __statebuf,
				 size_t __statelen));
/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern __ptr_t __setstate __P ((__ptr_t __statebuf));

#ifdef	__USE_BSD
extern long int random __P ((void));
extern void srandom __P ((unsigned int __seed));
extern __ptr_t initstate __P ((unsigned int __seed, __ptr_t __statebuf,
			       size_t __statelen));
extern __ptr_t setstate __P ((__ptr_t __statebuf));

#if defined (__OPTIMIZE__) && __GNUC__ >= 2
extern __inline long int random (void)
{ return __random(); }
extern __inline void srandom (unsigned int __seed)
{ __srandom(__seed); }
extern __inline __ptr_t initstate (unsigned int __seed,
				   __ptr_t __statebuf, size_t __statelen)
{ return __initstate (__seed, __statebuf, __statelen); }
extern __inline __ptr_t setstate (__ptr_t __statebuf)
{ return __setstate (__statebuf); }
#endif /* Optimizing GCC >=2.  */
#endif /* Use BSD.  */


/* Allocate SIZE bytes of memory.  */
extern __ptr_t malloc __P ((size_t __size));
/* Re-allocate the previously allocated block
   in __ptr_t, making the new block SIZE bytes long.  */
extern __ptr_t realloc __P ((__ptr_t __ptr, size_t __size));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern __ptr_t calloc __P ((size_t __nmemb, size_t __size));
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __P ((__ptr_t __ptr));

#ifdef	__USE_MISC
/* Free a block.  An alias for `free'.	(Sun Unices).  */
extern void cfree __P ((__ptr_t __ptr));
#endif /* Use misc.  */

#if defined(__USE_GNU) || defined(__USE_BSD) || defined(__USE_MISC)
#include <alloca.h>
#endif /* Use GNU, BSD, or misc.  */

#ifdef	__USE_BSD
/* Allocate SIZE bytes on a page boundary.  The storage cannot be freed.  */
extern __ptr_t valloc __P ((size_t __size));
#endif


/* Abort execution and generate a core-dump.  */
extern void abort __P ((void)) __attribute__ ((__noreturn__));


/* Register a function to be called when `exit' is called.  */
extern int atexit __P ((void (*__func) (void)));

#ifdef	__USE_MISC
/* Register a function to be called with the status
   given to `exit' and the given argument.  */
extern int on_exit __P ((void (*__func) (int __status, __ptr_t __arg),
			 __ptr_t __arg));
#endif

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered
   perform stdio cleanup, and terminate program execution with STATUS.  */
extern void exit __P ((int __status)) __attribute__ ((__noreturn__));


/* Return the value of envariable NAME, or NULL if it doesn't exist.  */
extern char *getenv __P ((__const char *__name));

#ifdef	__USE_SVID
/* The SVID says this is in <stdio.h>, but this seems a better place.	*/
/* Put STRING, which is of the form "NAME=VALUE", in the environment.
   If there is no `=', remove NAME from the environment.  */
extern int putenv __P ((__const char *__string));
#endif

#ifdef	__USE_BSD
/* Set NAME to VALUE in the environment.
   If REPLACE is nonzero, overwrite an existing value.  */
extern int setenv __P ((__const char *__name, __const char *__value,
			int __replace));
#endif

/* Execute the given line as a shell command.  */
extern int system __P ((__const char *__command));


/* Shorthand for type of comparison functions.  */
typedef int (*__compar_fn_t) __P ((__const __ptr_t, __const __ptr_t));

#ifdef	__USE_GNU
typedef __compar_fn_t comparison_fn_t;
#endif

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern __ptr_t bsearch __P ((__const __ptr_t __key, __const __ptr_t __base,
			     size_t __nmemb, size_t __size,
			     __compar_fn_t __compar));

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort __P ((__ptr_t __base, size_t __nmemb, size_t __size,
			__compar_fn_t __compar));


#ifndef	__CONSTVALUE
#ifdef	__GNUC__
/* The `const' keyword tells GCC that a function's return value is
   based solely on its arguments, and there are no side-effects.  */
#define	__CONSTVALUE	__const
#else
#define	__CONSTVALUE
#endif /* GCC.  */
#endif /* __CONSTVALUE not defined.  */

/* Return the absolute value of X.  */
extern __CONSTVALUE int abs __P ((int __x));
extern __CONSTVALUE long int labs __P ((long int __x));


/* Return the `div_t' or `ldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern __CONSTVALUE div_t div __P ((int __numer, int __denom));
extern __CONSTVALUE ldiv_t ldiv __P ((long int __numer, long int __denom));


/* Return the length of the multibyte character
   in S, which is no longer than N.  */
extern int mblen __P ((__const char *__s, size_t __n));
/* Return the length of the given multibyte character,
   putting its `wchar_t' representation in *PWC.  */
extern int mbtowc __P ((wchar_t * __pwc, __const char *__s, size_t __n));
/* Put the multibyte character represented
   by WCHAR in S, returning its length.  */
extern int wctomb __P ((char *__s, wchar_t __wchar));

#if defined (__OPTIMIZE__) && __GNUC__ >= 2
extern __inline int mblen (__const char *__s, size_t __n)
{ return mbtowc ((wchar_t *) NULL, __s, __n); }
#endif /* Optimizing GCC >=2.  */


/* Convert a multibyte string to a wide char string.  */
extern size_t mbstowcs __P ((wchar_t * __pwcs, __const char *__s, size_t __n));
/* Convert a wide char string to multibyte string.  */
extern size_t wcstombs __P ((char *__s, __const wchar_t * __pwcs, size_t __n));


__END_DECLS

#endif /* stdlib.h  */
