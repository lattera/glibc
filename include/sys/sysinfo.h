#ifndef _SYS_SYSINFO_H
#include_next <sys/sysinfo.h>

/* Now we define the internal interface.  */

/* Return number of configured processors.  */
extern int __get_nprocs_conf __P ((void));

/* Return number of available processors.  */
extern int __get_nprocs __P ((void));

/* Return number of physical pages of memory in the system.  */
extern int __get_phys_pages __P ((void));

/* Return number of available physical pages of memory in the system.  */
extern int __get_avphys_pages __P ((void));

#endif /* sys/sysinfo.h */
