/* Determine the wordsize from the preprocessor defines.  */

#if defined __s390x__
# define __WORDSIZE	64
#else
# define __WORDSIZE	32
#endif

#if !defined __NO_LONG_DOUBLE_MATH && !defined __LONG_DOUBLE_MATH_OPTIONAL

/* Signal that we didn't used to have a `long double'. The changes all
   the `long double' function variants to be redirects to the double
   functions.  */
# define __LONG_DOUBLE_MATH_OPTIONAL   1
# ifndef __LONG_DOUBLE_128__
#  define __NO_LONG_DOUBLE_MATH        1
# endif
#endif
