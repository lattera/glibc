#ifndef _STDLIB_H
#include <stdlib/stdlib.h>

/* Now define the internal interfaces.  */
extern int32_t __random __P ((void));
extern void __srandom __P ((unsigned int __seed));
extern __ptr_t __initstate __P ((unsigned int __seed, __ptr_t __statebuf,
				 size_t __statelen));
extern __ptr_t __setstate __P ((__ptr_t __statebuf));
extern int __random_r __P ((struct random_data *__buf, int32_t *__result));
extern int __srandom_r __P ((unsigned int __seed, struct random_data *__buf));
extern int __initstate_r __P ((unsigned int __seed, __ptr_t __statebuf,
			       size_t __statelen, struct random_data *__buf));
extern int __setstate_r __P ((__ptr_t __statebuf, struct random_data *__buf));
extern int __rand_r __P ((unsigned int *__seed));
extern int __erand48_r __P ((unsigned short int __xsubi[3],
			     struct drand48_data *__buffer, double *__result));
extern int __nrand48_r __P ((unsigned short int __xsubi[3],
			     struct drand48_data *__buffer,
			     long int *__result));
extern int __jrand48_r __P ((unsigned short int __xsubi[3],
			     struct drand48_data *__buffer,
			     long int *__result));
extern int __srand48_r __P ((long int __seedval,
			     struct drand48_data *__buffer));
extern int __seed48_r __P ((unsigned short int __seed16v[3],
			    struct drand48_data *__buffer));
extern int __lcong48_r __P ((unsigned short int __param[7],
			     struct drand48_data *__buffer));

/* Internal function to compute next state of the generator.  */
extern int __drand48_iterate __P ((unsigned short int __xsubi[3],
				   struct drand48_data *__buffer));

extern int __setenv __P ((__const char *__name, __const char *__value,
			  int __replace));
extern void __unsetenv __P ((__const char *__name));
extern int __clearenv __P ((void));
extern char *__canonicalize_file_name __P ((__const char *__name));
extern char *__realpath __P ((__const char *__name, char *__resolved));
extern int __ptsname_r __P ((int __fd, char *__buf, size_t __buflen));
extern int __getpt __P ((void));
#endif
