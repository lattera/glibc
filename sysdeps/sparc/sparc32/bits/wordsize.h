/* Determine the wordsize from the preprocessor defines.  */

#if defined __sparc_v9__ || defined __arch64__ || defined __sparcv9
# define __WORDSIZE	64
#else
# define __WORDSIZE	32
#endif
