#ifndef _SYS_SYSINFO_H
#include_next <sys/sysinfo.h>

/* Now we define the internal interface.  */

/* Return number of configured processors.  */
extern int __get_nprocs_conf (void) __THROW;

/* Return number of available processors.  */
extern int __get_nprocs (void) __THROW;

/* Return number of physical pages of memory in the system.  */
extern int __get_phys_pages (void) __THROW;

/* Return number of available physical pages of memory in the system.  */
extern int __get_avphys_pages (void) __THROW;

#endif /* sys/sysinfo.h */
