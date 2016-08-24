#ifndef _SYS_GMON_H
#include <gmon/sys/gmon.h>

# ifndef _ISOMAC

/* Now define the internal interfaces.  */

/* Write current profiling data to file.  */
extern void __write_profiling (void);
extern void write_profiling (void);

struct __bb;
extern void __bb_init_func (struct __bb *bb);
extern void __bb_exit_func (void);

extern struct gmonparam _gmonparam attribute_hidden;

# endif /* !_ISOMAC */
#endif
