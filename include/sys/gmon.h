#ifndef _SYS_GMON_H
#include <gmon/sys/gmon.h>

/* Now define the internal interfaces.  */

/* Write current profiling data to file.  */
extern void __write_profiling (void) __THROW;
extern void write_profiling (void) __THROW;
#endif
