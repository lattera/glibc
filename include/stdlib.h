#ifndef _STDLIB_H

#ifdef __need_malloc_and_calloc
#define __Need_M_And_C
#endif

#include <stdlib/stdlib.h>

/* Now define the internal interfaces.  */
#ifndef __Need_M_And_C
extern int32_t __random (void) __THROW;
extern void __srandom (unsigned int __seed) __THROW;
extern void *__initstate (unsigned int __seed, void *__statebuf,
			  size_t __statelen) __THROW;
extern void *__setstate (void *__statebuf) __THROW;
extern int __random_r (struct random_data *__buf, int32_t *__result) __THROW;
extern int __srandom_r (unsigned int __seed, struct random_data *__buf) __THROW;
extern int __initstate_r (unsigned int __seed, void *__statebuf,
			  size_t __statelen, struct random_data *__buf) __THROW;
extern int __setstate_r (void *__statebuf, struct random_data *__buf) __THROW;
extern int __rand_r (unsigned int *__seed) __THROW;
extern int __erand48_r (unsigned short int __xsubi[3],
			struct drand48_data *__buffer, double *__result) __THROW;
extern int __nrand48_r (unsigned short int __xsubi[3],
			struct drand48_data *__buffer,
			long int *__result) __THROW;
extern int __jrand48_r (unsigned short int __xsubi[3],
			struct drand48_data *__buffer,
			long int *__result) __THROW;
extern int __srand48_r (long int __seedval,
			struct drand48_data *__buffer) __THROW;
extern int __seed48_r (unsigned short int __seed16v[3],
		       struct drand48_data *__buffer) __THROW;
extern int __lcong48_r (unsigned short int __param[7],
			struct drand48_data *__buffer) __THROW;

/* Internal function to compute next state of the generator.  */
extern int __drand48_iterate (unsigned short int __xsubi[3],
			      struct drand48_data *__buffer) __THROW;

extern int __setenv (__const char *__name, __const char *__value,
		     int __replace) __THROW;
extern void __unsetenv (__const char *__name) __THROW;
extern int __clearenv (void) __THROW;
extern char *__canonicalize_file_name (__const char *__name) __THROW;
extern char *__realpath (__const char *__name, char *__resolved) __THROW;
extern int __ptsname_r (int __fd, char *__buf, size_t __buflen) __THROW;
extern int __getpt (void) __THROW;

extern int __add_to_environ (const char *name, const char *value,
			     const char *combines, int replace);
#endif
#undef __Need_M_And_C

#endif  /* include/stdlib.h */
