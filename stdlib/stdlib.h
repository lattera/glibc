/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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
extern float strtof __P ((__const char *__nptr, char **__endptr));
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


/* The internal entry points for `strtoX' take an extra flag argument
   saying whether or not to parse locale-dependent number grouping.  */

extern double __strtod_internal (__const char *__nptr,
				 char **__endptr, int __group);
extern float __strtof_internal (__const char *__nptr, char **__endptr,
				int __group);
extern __long_double_t __strtold_internal (__const char *__nptr,
					   char **__endptr, int __group);
extern long int __strtol_internal (__const char *__nptr, char **__endptr,
				   int __base, int __group);
extern unsigned long int __strtoul_internal (__const char *__nptr,
					     char **__endptr, int __base,
					     int __group);
extern long long int __strtoq_internal (__const char *__nptr, char **__endptr,
					int __base, int __group);
extern unsigned long long int __strtouq_internal (__const char *__nptr,
						  char **__endptr, int __base,
						  int __group);

#if defined (__OPTIMIZE__) && __GNUC__ >= 2
/* Define inline functions which call the internal entry points.  */

extern __inline double strtod (__const char *__nptr, char **__endptr)
{ return __strtod_internal (__nptr, __endptr, 0); }
extern __inline long int strtol (__const char *__nptr,
				 char **__endptr, int __base)
{ return __strtol_internal (__nptr, __endptr, __base, 0); }
extern __inline unsigned long int strtoul (__const char *__nptr,
					   char **__endptr, int __base)
{ return __strtoul_internal (__nptr, __endptr, __base, 0); }

#ifdef __USE_GNU
extern __inline float strtof (__const char *__nptr, char **__endptr)
{ return __strtof_internal (__nptr, __endptr, 0); }
extern __inline __long_double_t strtold (__const char *__nptr, char **__endptr)
{ return __strtold_internal (__nptr, __endptr, 0); }
#endif

#ifdef __USE_BSD
extern __inline long long int strtoq (__const char *__nptr, char **__endptr,
				      int __base)
{ return __strtoq_internal (__nptr, __endptr, __base, 0); }
extern __inline unsigned long long int strtouq (__const char *__nptr,
					    char **__endptr, int __base)
{ return __strtouq_internal (__nptr, __endptr, __base, 0); }
#endif

extern __inline double atof (__const char *__nptr)
{ return strtod (__nptr, (char **) NULL); }
extern __inline int atoi (__const char *__nptr)
{ return (int) strtol (__nptr, (char **) NULL, 10); }
extern __inline long int atol (__const char *__nptr)
{ return strtol (__nptr, (char **) NULL, 10); }
#endif /* Optimizing GCC >=2.  */


#ifdef __USE_SVID
/* Convert N to base 64 using the digits "./0-9A-Za-z", least-significant
   digit first.  Returns a pointer to static storage overwritten by the
   next call.  */
extern char *l64a __P ((long int __n));

/* Read a number from a string in base 64 as above.  */
extern long int a64l __P ((const char *));
#endif


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

#ifdef __USE_REENTRANT
/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */

struct random_data
  {
    long int *fptr;		/* Front pointer.  */
    long int *rptr;		/* Rear pointer.  */
    long int *state;		/* Array of state values.  */
    int rand_type;		/* Type of random number generator.  */
    int rand_deg;		/* Degree of random number generator.  */
    int rand_sep;		/* Distance between front and rear.  */
    long int *end_ptr;		/* Pointer behind state table.  */
  };

extern int __random_r __P ((struct random_data *__buf, long int *__result));
extern int __srandom_r __P ((unsigned int __seed, struct random_data *__buf));
extern int __initstate_r __P ((unsigned int __seed, __ptr_t __statebuf,
			       size_t __statelen, struct random_data *__buf));
extern int __setstate_r __P ((__ptr_t __statebuf, struct random_data *__buf));

extern int random_r __P ((struct random_data *__buf, long int *__result));
extern int srandom_r __P ((unsigned int __seed, struct random_data *__buf));
extern int initstate_r __P ((unsigned int __seed, __ptr_t __statebuf,
			     size_t __statelen, struct random_data *__buf));
extern int setstate_r __P ((__ptr_t __statebuf, struct random_data *__buf));
#endif	/* __USE_REENTRANT.  */
#endif	/* Use BSD.  */


#ifdef	__USE_SVID
/* System V style 48-bit random number generator functions.  */

/* Data structure for communication with thread safe versions.  */
struct drand48_data
  {
    unsigned short int X[3];	/* Current state.  */
    unsigned short int a[3];	/* Factor in congruential formula.  */
    unsigned short int c;	/* Additive const. in congruential formula.  */
    unsigned short int old_X[3]; /* Old state.  */
    int init;			/* Flag for initializing.  */
  };

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern double drand48 __P ((void));
extern int drand48_r __P ((struct drand48_data *__buffer, double *__result));
extern double erand48 __P ((unsigned short int __xsubi[3]));
extern int erand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, double *__result));
/* Return non-negative, long integer in [0,2^31).  */
extern long lrand48 __P ((void));
extern int lrand48_r __P ((struct drand48_data *__buffer, long *__result));
extern long nrand48 __P ((unsigned short int __xsubi[3]));
extern int nrand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, long *__result));
/* Return signed, long integers in [-2^31,2^31).  */
extern long mrand48 __P ((void));
extern int mrand48_r __P ((struct drand48_data *__buffer, long *__result));
extern long jrand48 __P ((unsigned short int __xsubi[3]));
extern int jrand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, long *__result));
/* Seed random number generator.  */
extern void srand48 __P ((long __seedval));
extern int srand48_r __P ((long __seedval, struct drand48_data *__buffer));
extern unsigned short int *seed48 __P ((unsigned short int __seed16v[3]));
extern int seed48_r __P ((unsigned short int __seed16v[3],
			  struct drand48_data *__buffer));
extern void lcong48 __P ((unsigned short int __param[7]));
extern int lcong48_r __P ((unsigned short int __param[7],
			   struct drand48_data *__buffer));

/* Internal function to compute next state of the generator.  */
extern int __drand48_iterate __P ((unsigned short int __xsubi[3],
				   struct drand48_data *__buffer));
#endif	/* __USE_SVID.  */


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

/* Remove the variable NAME from the environment.  */
extern void unsetenv __P ((__const char *__name));
#endif

/* Execute the given line as a shell command.  */
extern int system __P ((__const char *__command));


/* Shorthand for type of comparison functions.  */
#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (*__compar_fn_t) __P ((__const __ptr_t, __const __ptr_t));
#endif

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


/* Return the absolute value of X.  */
extern int abs __P ((int __x)) __attribute__ ((__const__));
extern long int labs __P ((long int __x)) __attribute__ ((__const__));


/* Return the `div_t' or `ldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern div_t div __P ((int __numer, int __denom)) __attribute__ ((__const__));
extern ldiv_t ldiv __P ((long int __numer, long int __denom)) __attribute__ ((__const__));


#ifdef __USE_SVID
/* Convert floating point numbers to strings.  The returned values are
   valid only until another call to the same function.  */

/* Convert VALUE to a string with NDIGIT digits and return a pointer to
   this.  Set *DECPT with the position of the decimal character and *SIGN
   with the sign of the number.  */
char *ecvt __P ((double __value, int __ndigit, int *__decpt, int *sign));

/* Convert VALUE to a string rounded to NDIGIT decimal digits.  Set *DECPT
   with the position of the decimal character and *SIGN with the sign of
   the number.  */
char *fcvt __P ((double __value, int __ndigit, int *__decpt, int *sign));

/* If possible convert VALUE to a string with NDIGIT significant digits.
   Otherwise use exponential representation.  The resulting string will
   be written to BUF.  */
char *gcvt __P ((double __value, int __ndigit, char *__buf));

/* Reentrant version of the functions above which provide their own
   buffers.  */
int ecvt_r __P ((double __value, int __ndigit, int *__decpt, int *sign,
		 char *__buf, size_t __len));
int fcvt_r __P ((double __value, int __ndigit, int *__decpt, int *sign,
		 char *__buf, size_t __len));
#endif


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
