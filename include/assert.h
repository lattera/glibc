#include <assert/assert.h>

extern void __assert_fail_internal (__const char *__assertion,
				    __const char *__file,
				    unsigned int __line,
				    __const char *__function)
     __attribute__ ((__noreturn__)) attribute_hidden;
