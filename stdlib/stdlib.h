/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
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
 *	ISO C Standard: 4.10 GENERAL UTILITIES	<stdlib.h>
 */

#ifndef	_STDLIB_H

#include <features.h>

/* Get size_t, wchar_t and NULL from <stddef.h>.  */
#define		__need_size_t
#ifndef __need_malloc_and_calloc
# define	__need_wchar_t
# define	__need_NULL
#endif
#include <stddef.h>

__BEGIN_DECLS

#ifndef __need_malloc_and_calloc
#define	_STDLIB_H	1

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

#ifdef __USE_ISOC9X
/* Returned by `lldiv'.  */
__extension__ typedef struct
  {
    long long int quot;		/* Quotient.  */
    long long int rem;		/* Remainder.  */
  } lldiv_t;
#endif


/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */


/* Maximum length of a multibyte character in the current locale.  */
#define	MB_CUR_MAX	(__ctype_get_mb_cur_max ())
extern int __ctype_get_mb_cur_max __P ((void));


/* Convert a string to a floating-point number.  */
extern double atof __P ((__const char *__nptr));
/* Convert a string to an integer.  */
extern int atoi __P ((__const char *__nptr));
/* Convert a string to a long integer.  */
extern long int atol __P ((__const char *__nptr));

#if defined __USE_ISOC9X || (defined __GNUC__ && defined __USE_MISC)
/* These functions will part of the standard C library in ISO C 9X.  */
__extension__ extern long long int atoll __P ((__const char *__nptr));
#endif

/* Convert a string to a floating-point number.  */
extern double strtod __P ((__const char *__restrict __nptr,
			   char **__restrict __endptr));

#ifdef	__USE_ISOC9X
/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float strtof __P ((__const char *__restrict __nptr,
			  char **__restrict __endptr));

extern __long_double_t strtold __P ((__const char *__restrict __nptr,
				     char **__restrict __endptr));
#endif

/* Convert a string to a long integer.  */
extern long int strtol __P ((__const char *__restrict __nptr,
			     char **__restrict __endptr, int __base));
/* Convert a string to an unsigned long integer.  */
extern unsigned long int strtoul __P ((__const char *__restrict __nptr,
				       char **__restrict __endptr,
				       int __base));

#if defined __GNUC__ && defined __USE_BSD
/* Convert a string to a quadword integer.  */
__extension__
extern long long int strtoq __P ((__const char *__restrict __nptr,
				  char **__restrict __endptr, int __base));
/* Convert a string to an unsigned quadword integer.  */
__extension__
extern unsigned long long int strtouq __P ((__const char *__restrict __nptr,
					    char **__restrict __endptr,
					    int __base));
#endif /* GCC and use BSD.  */

#if defined __USE_ISOC9X || (defined __GNUC__ && defined __USE_MISC)
/* These functions will part of the standard C library in ISO C 9X.  */

/* Convert a string to a quadword integer.  */
__extension__
extern long long int strtoll __P ((__const char *__restrict __nptr,
				   char **__restrict __endptr, int __base));
/* Convert a string to an unsigned quadword integer.  */
__extension__
extern unsigned long long int strtoull __P ((__const char *__restrict __nptr,
					     char **__restrict __endptr,
					     int __base));
#endif /* ISO C 9X or GCC and use MISC.  */


#ifdef __USE_GNU
/* The concept of one static locale per category is not very well
   thought out.  Many applications will need to process its data using
   information from several different locales.  Another application is
   the implementation of the internationalization handling in the
   upcoming ISO C++ standard library.  To support this another set of
   the functions using locale data exist which have an additional
   argument.

   Attention: all these functions are *not* standardized in any form.
   This is a proof-of-concept implementation.  */

/* Structure for reentrant locale using functions.  This is an
   (almost) opaque type for the user level programs.  */
# include <xlocale.h>

/* Special versions of the functions above which take the locale to
   use as an additional parameter.  */
extern long int __strtol_l __P ((__const char *__restrict __nptr,
				 char **__restrict __endptr, int __base,
				 __locale_t __loc));

extern unsigned long int __strtoul_l __P ((__const char *__restrict __nptr,
					   char **__restrict __endptr,
					   int __base, __locale_t __loc));

__extension__
extern long long int __strtoll_l __P ((__const char *__restrict __nptr,
				       char **__restrict __endptr, int __base,
				       __locale_t __loc));

__extension__
extern unsigned long long int __strtoull_l __P ((__const char *__restrict
						 __nptr,
						 char **__restrict __endptr,
						 int __base,
						 __locale_t __loc));

extern double __strtod_l __P ((__const char *__restrict __nptr,
			       char **__restrict __endptr, __locale_t __loc));

extern float __strtof_l __P ((__const char *__restrict __nptr,
			      char **__restrict __endptr, __locale_t __loc));

extern __long_double_t __strtold_l __P ((__const char *__restrict __nptr,
					 char **__restrict __endptr,
					 __locale_t __loc));
#endif /* GNU */


/* The internal entry points for `strtoX' take an extra flag argument
   saying whether or not to parse locale-dependent number grouping.  */

extern double __strtod_internal __P ((__const char *__restrict __nptr,
				      char **__restrict __endptr,
				      int __group));
extern float __strtof_internal __P ((__const char *__restrict __nptr,
				     char **__restrict __endptr, int __group));
extern __long_double_t __strtold_internal __P ((__const char *
						__restrict __nptr,
						char **__restrict __endptr,
						int __group));
#ifndef __strtol_internal_defined
extern long int __strtol_internal __P ((__const char *__restrict __nptr,
					char **__restrict __endptr,
					int __base, int __group));
# define __strtol_internal_defined	1
#endif
#ifndef __strtoul_internal_defined
extern unsigned long int __strtoul_internal __P ((__const char *
						  __restrict __nptr,
						  char **__restrict __endptr,
						  int __base, int __group));
# define __strtoul_internal_defined	1
#endif
#if defined __GNUC__ || defined __USE_ISOC9X
# ifndef __strtoll_internal_defined
__extension__
extern long long int __strtoll_internal __P ((__const char *__restrict __nptr,
					      char **__restrict __endptr,
					      int __base, int __group));
#  define __strtoll_internal_defined	1
# endif
# ifndef __strtoull_internal_defined
__extension__
extern unsigned long long int __strtoull_internal __P ((__const char *
							__restrict __nptr,
							char **
							__restrict __endptr,
							int __base,
							int __group));
#  define __strtoull_internal_defined	1
# endif
#endif /* GCC */

#if defined __OPTIMIZE__ && !defined __OPTIMIZE_SIZE__ \
    && defined __USE_EXTERN_INLINES
/* Define inline functions which call the internal entry points.  */

extern __inline double
strtod (__const char *__restrict __nptr, char **__restrict __endptr)
{
  return __strtod_internal (__nptr, __endptr, 0);
}
extern __inline long int
strtol (__const char *__restrict __nptr, char **__restrict __endptr,
	int __base)
{
  return __strtol_internal (__nptr, __endptr, __base, 0);
}
extern __inline unsigned long int
strtoul (__const char *__restrict __nptr, char **__restrict __endptr,
	 int __base)
{
  return __strtoul_internal (__nptr, __endptr, __base, 0);
}

# ifdef __USE_ISOC9X
extern __inline float
strtof (__const char *__restrict __nptr, char **__restrict __endptr)
{
  return __strtof_internal (__nptr, __endptr, 0);
}
extern __inline __long_double_t
strtold (__const char *__restrict __nptr, char **__restrict __endptr)
{
  return __strtold_internal (__nptr, __endptr, 0);
}
# endif

# ifdef __USE_BSD
__extension__ extern __inline long long int
strtoq (__const char *__restrict __nptr, char **__restrict __endptr,
	int __base)
{
  return __strtoll_internal (__nptr, __endptr, __base, 0);
}
__extension__ extern __inline unsigned long long int
strtouq (__const char *__restrict __nptr, char **__restrict __endptr,
	 int __base)
{
  return __strtoull_internal (__nptr, __endptr, __base, 0);
}
# endif

# if defined __USE_MISC || defined __USE_ISOC9X
__extension__ extern __inline long long int
strtoll (__const char *__restrict __nptr, char **__restrict __endptr,
	 int __base)
{
  return __strtoll_internal (__nptr, __endptr, __base, 0);
}
__extension__ extern __inline unsigned long long int
strtoull (__const char * __restrict __nptr, char **__restrict __endptr,
	  int __base)
{
  return __strtoull_internal (__nptr, __endptr, __base, 0);
}
# endif

extern __inline double
atof (__const char *__nptr)
{
  return strtod (__nptr, (char **) NULL);
}
extern __inline int
atoi (__const char *__nptr)
{
  return (int) strtol (__nptr, (char **) NULL, 10);
}
extern __inline long int
atol (__const char *__nptr)
{
  return strtol (__nptr, (char **) NULL, 10);
}

# if defined __USE_MISC || defined __USE_ISOC9X
__extension__ extern __inline long long int
atoll (__const char *__nptr)
{
  return strtoll (__nptr, (char **) NULL, 10);
}
# endif
#endif /* Optimizing and Inlining.  */


#if defined __USE_SVID || defined __USE_XOPEN_EXTENDED
/* Convert N to base 64 using the digits "./0-9A-Za-z", least-significant
   digit first.  Returns a pointer to static storage overwritten by the
   next call.  */
extern char *l64a __P ((long int __n));

/* Read a number from a string S in base 64 as above.  */
extern long int a64l __P ((__const char *__s));


# include <sys/types.h>	/* we need int32_t... */

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and RAND_MAX inclusive.  */
extern int32_t random __P ((void));

/* Seed the random number generator with the given number.  */
extern void srandom __P ((unsigned int __seed));

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern __ptr_t initstate __P ((unsigned int __seed, __ptr_t __statebuf,
			       size_t __statelen));

/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern __ptr_t setstate __P ((__ptr_t __statebuf));


# ifdef __USE_MISC
/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */

struct random_data
  {
    int32_t *fptr;		/* Front pointer.  */
    int32_t *rptr;		/* Rear pointer.  */
    int32_t *state;		/* Array of state values.  */
    int rand_type;		/* Type of random number generator.  */
    int rand_deg;		/* Degree of random number generator.  */
    int rand_sep;		/* Distance between front and rear.  */
    int32_t *end_ptr;		/* Pointer behind state table.  */
  };

extern int random_r __P ((struct random_data *__buf, int32_t *__result));

extern int srandom_r __P ((unsigned int __seed, struct random_data *__buf));

extern int initstate_r __P ((unsigned int __seed, __ptr_t __statebuf,
			     size_t __statelen, struct random_data *__buf));

extern int setstate_r __P ((__ptr_t __statebuf, struct random_data *__buf));
# endif	/* Use misc.  */
#endif	/* Use SVID || extended X/Open.  */


/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand __P ((void));
/* Seed the random number generator with the given number.  */
extern void srand __P ((unsigned int __seed));

#ifdef __USE_POSIX
/* Reentrant interface according to POSIX.1.  */
extern int rand_r __P ((unsigned int *__seed));
#endif


#if defined __USE_SVID || defined __USE_XOPEN
/* System V style 48-bit random number generator functions.  */

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern double drand48 __P ((void));
extern double erand48 __P ((unsigned short int __xsubi[3]));

/* Return non-negative, long integer in [0,2^31).  */
extern long int lrand48 __P ((void));
extern long int nrand48 __P ((unsigned short int __xsubi[3]));

/* Return signed, long integers in [-2^31,2^31).  */
extern long int mrand48 __P ((void));
extern long int jrand48 __P ((unsigned short int __xsubi[3]));

/* Seed random number generator.  */
extern void srand48 __P ((long int __seedval));
extern unsigned short int *seed48 __P ((unsigned short int __seed16v[3]));
extern void lcong48 __P ((unsigned short int __param[7]));

/* Data structure for communication with thread safe versions.  */
struct drand48_data
  {
    unsigned short int x[3];	/* Current state.  */
    unsigned short int a[3];	/* Factor in congruential formula.  */
    unsigned short int c;	/* Additive const. in congruential formula.  */
    unsigned short int old_x[3]; /* Old state.  */
    int init;			/* Flag for initializing.  */
  };

# ifdef __USE_MISC
/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern int drand48_r __P ((struct drand48_data *__buffer, double *__result));
extern int erand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, double *__result));

/* Return non-negative, long integer in [0,2^31).  */
extern int lrand48_r __P ((struct drand48_data *__buffer, long int *__result));
extern int nrand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, long int *__result));

/* Return signed, long integers in [-2^31,2^31).  */
extern int mrand48_r __P ((struct drand48_data *__buffer, long int *__result));
extern int jrand48_r __P ((unsigned short int __xsubi[3],
			   struct drand48_data *__buffer, long int *__result));

/* Seed random number generator.  */
extern int srand48_r __P ((long int __seedval, struct drand48_data *__buffer));

extern int seed48_r __P ((unsigned short int __seed16v[3],
			  struct drand48_data *__buffer));

extern int lcong48_r __P ((unsigned short int __param[7],
			   struct drand48_data *__buffer));
# endif	/* Use misc.  */
#endif	/* Use SVID or X/Open.  */

#endif /* don't just need malloc and calloc */

#ifndef __malloc_and_calloc_defined
#define __malloc_and_calloc_defined
/* Allocate SIZE bytes of memory.  */
extern __ptr_t malloc __P ((size_t __size));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern __ptr_t calloc __P ((size_t __nmemb, size_t __size));
#endif

#ifndef __need_malloc_and_calloc
/* Re-allocate the previously allocated block
   in __ptr_t, making the new block SIZE bytes long.  */
extern __ptr_t realloc __P ((__ptr_t __ptr, size_t __size));
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __P ((__ptr_t __ptr));

#ifdef	__USE_MISC
/* Free a block.  An alias for `free'.	(Sun Unices).  */
extern void cfree __P ((__ptr_t __ptr));
#endif /* Use misc.  */

#if defined __USE_GNU || defined __USE_BSD || defined __USE_MISC
# include <alloca.h>
#endif /* Use GNU, BSD, or misc.  */

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
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
extern int __on_exit __P ((void (*__func) (int __status, __ptr_t __arg),
			   __ptr_t __arg));
extern int on_exit __P ((void (*__func) (int __status, __ptr_t __arg),
			 __ptr_t __arg));
#endif

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered
   perform stdio cleanup, and terminate program execution with STATUS.  */
extern void exit __P ((int __status)) __attribute__ ((__noreturn__));


/* Return the value of envariable NAME, or NULL if it doesn't exist.  */
extern char *getenv __P ((__const char *__name));

/* This function is similar to the above but returns NULL if the
   programs is running with SUID or SGID enabled.  */
extern char *__secure_getenv __P ((__const char *__name));

#if defined __USE_SVID || defined __USE_XOPEN
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

#ifdef	__USE_MISC
/* The `clearenv' was planned to be added to POSIX.1 but probably
   never made it.  Nevertheless the POSIX.9 standard (POSIX bindings
   for Fortran 77) requires this function.  */
extern int clearenv __P ((void));
#endif


#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED
/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the file name unique.
   Returns TEMPLATE, or a null pointer if it cannot get a unique file name.  */
extern char *mktemp __P ((char *__template));

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing,
   or -1 if it cannot create a uniquely-named file.  */
extern int mkstemp __P ((char *__template));
#endif


/* Execute the given line as a shell command.  */
extern int system __P ((__const char *__command));


#ifdef	__USE_GNU
/* Return a malloc'd string containing the canonical absolute name of the
   named file.  The last file name component need not exist, and may be a
   symlink to a nonexistent file.  */
extern char *canonicalize_file_name __P ((__const char *__name));
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Return the canonical absolute name of file NAME.  The last file name
   component need not exist, and may be a symlink to a nonexistent file.
   If RESOLVED is null, the result is malloc'd; otherwise, if the canonical
   name is PATH_MAX chars or more, returns null with `errno' set to
   ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars, returns the
   name in RESOLVED.  */
extern char *realpath __P ((__const char *__name, char *__resolved));
#endif


/* Shorthand for type of comparison functions.  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) __PMT ((__const __ptr_t, __const __ptr_t));

# ifdef	__USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern __ptr_t bsearch __PMT ((__const __ptr_t __key, __const __ptr_t __base,
			       size_t __nmemb, size_t __size,
			       __compar_fn_t __compar));

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort __PMT ((__ptr_t __base, size_t __nmemb, size_t __size,
			  __compar_fn_t __compar));


/* Return the absolute value of X.  */
extern int abs __P ((int __x)) __attribute__ ((__const__));
extern long int labs __P ((long int __x)) __attribute__ ((__const__));
#ifdef __USE_ISOC9X
__extension__ extern long long int llabs __P ((long long int __x))
     __attribute__ ((__const__));
#endif


/* Return the `div_t', `ldiv_t' or `lldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern div_t div __P ((int __numer, int __denom)) __attribute__ ((__const__));
extern ldiv_t ldiv __P ((long int __numer, long int __denom))
     __attribute__ ((__const__));
#ifdef __USE_ISOC9X
__extension__ extern lldiv_t lldiv __P ((long long int __numer,
					 long long int __denom))
     __attribute__ ((__const__));
#endif


#if defined __USE_SVID || defined __USE_XOPEN_EXTENDED
/* Convert floating point numbers to strings.  The returned values are
   valid only until another call to the same function.  */

/* Convert VALUE to a string with NDIGIT digits and return a pointer to
   this.  Set *DECPT with the position of the decimal character and *SIGN
   with the sign of the number.  */
extern char *ecvt __P ((double __value, int __ndigit, int *__decpt,
			int *__sign));

/* Convert VALUE to a string rounded to NDIGIT decimal digits.  Set *DECPT
   with the position of the decimal character and *SIGN with the sign of
   the number.  */
extern char *fcvt __P ((double __value, int __ndigit, int *__decpt,
			int *__sign));

/* If possible convert VALUE to a string with NDIGIT significant digits.
   Otherwise use exponential representation.  The resulting string will
   be written to BUF.  */
extern char *gcvt __P ((double __value, int __ndigit, char *__buf));

/* Long double versions of above functions.  */
extern char *qecvt __P ((__long_double_t __value, int __ndigit, int *__decpt,
			 int *__sign));
extern char *qfcvt __P ((__long_double_t __value, int __ndigit, int *__decpt,
			 int *__sign));
extern char *qgcvt __P ((__long_double_t __value, int __ndigit, char *__buf));


# ifdef __USE_MISC
/* Reentrant version of the functions above which provide their own
   buffers.  */
extern int ecvt_r __P ((double __value, int __ndigit, int *__decpt,
			int *__sign, char *__buf, size_t __len));
extern int fcvt_r __P ((double __value, int __ndigit, int *__decpt,
			int *__sign, char *__buf, size_t __len));

extern int qecvt_r __P ((__long_double_t __value, int __ndigit, int *__decpt,
			 int *__sign, char *__buf, size_t __len));
extern int qfcvt_r __P ((__long_double_t __value, int __ndigit, int *__decpt,
			 int *__sign, char *__buf, size_t __len));
# endif	/* misc */
#endif	/* use MISC || use X/Open Unix */


/* Return the length of the multibyte character
   in S, which is no longer than N.  */
extern int mblen __P ((__const char *__s, size_t __n));
/* Return the length of the given multibyte character,
   putting its `wchar_t' representation in *PWC.  */
extern int mbtowc __P ((wchar_t *__restrict __pwc,
			__const char *__restrict __s, size_t __n));
/* Put the multibyte character represented
   by WCHAR in S, returning its length.  */
extern int wctomb __P ((char *__s, wchar_t __wchar));


/* Convert a multibyte string to a wide char string.  */
extern size_t mbstowcs __P ((wchar_t *__restrict  __pwcs,
			     __const char *__restrict __s, size_t __n));
/* Convert a wide char string to multibyte string.  */
extern size_t wcstombs __P ((char *__restrict __s,
			     __const wchar_t *__restrict __pwcs, size_t __n));


#ifdef __USE_SVID
/* Determine whether the string value of RESPONSE matches the affirmation
   or negative response expression as specified by the LC_MESSAGES category
   in the program's current locale.  Returns 1 if affirmative, 0 if
   negative, and -1 if not matching.  */
extern int rpmatch __P ((__const char *__response));
#endif


#ifdef __USE_XOPEN_EXTENDED
/* Parse comma separated suboption from *OPTIONP and match against
   strings in TOKENS.  If found return index and set *VALUEP to
   optional value introduced by an equal sign.  If the suboption is
   not part of TOKENS return in *VALUEP beginning of unknown
   suboption.  On exit *OPTIONP is set to the beginning of the next
   token or at the terminating NUL character.  */
extern int getsubopt __P ((char **__optionp, __const char *__const *__tokens,
			   char **__valuep));
#endif


#ifdef __USE_XOPEN

/* Setup DES tables according KEY.  */
extern void setkey __P ((__const char *__key));

/* X/Open pseudo terminal handling.  */

/* The next four functions all take a master pseudo-tty fd and
   perform an operation on the associated slave:  */

/* Chown the slave to the calling user.  */
extern int grantpt __P ((int __fd));

/* Release an internal lock so the slave can be opened.
   Call after grantpt().  */
extern int unlockpt __P ((int __fd));

/* Return the pathname of the pseudo terminal slave assoicated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
extern char *ptsname __P ((int __fd));
#endif

#ifdef __USE_GNU
/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
extern int ptsname_r __P ((int __fd, char *__buf, size_t __buflen));

/* Open a master pseudo terminal and return its file descriptor.  */
extern int getpt __P ((void));
#endif

#endif /* don't just need malloc and calloc */
#undef __need_malloc_and_calloc

__END_DECLS

#endif /* stdlib.h  */
