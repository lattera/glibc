/* Determine the wordsize from the preprocessor defines.  */

#if defined __s390x__
# define __WORDSIZE	64
#else
# define __WORDSIZE	32
#endif
