#include <assert/assert.h>

/* This prints an "Assertion failed" message and aborts.
   In installed assert.h this is only conditionally declared,
   so it has to be repeated here.  */
extern void __assert_fail (__const char *__assertion, __const char *__file,
			   unsigned int __line, __const char *__function)
  __THROW __attribute__ ((__noreturn__));

/* Likewise, but prints the error text for ERRNUM.  */
extern void __assert_perror_fail (int __errnum, __const char *__file,
				  unsigned int __line,
				  __const char *__function)
     __THROW __attribute__ ((__noreturn__));

#if !defined NOT_IN_libc || defined IS_IN_rtld
hidden_proto (__assert_fail)
hidden_proto (__assert_perror_fail)
#endif
