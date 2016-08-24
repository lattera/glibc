#ifndef _SYS_SYSINFO_H
#include_next <sys/sysinfo.h>

# ifndef _ISOMAC

/* Now we define the internal interface.  */

/* Return number of configured processors.  */
extern int __get_nprocs_conf (void);

/* Return number of available processors.  */
extern int __get_nprocs (void);

/* Return number of physical pages of memory in the system.  */
extern long int __get_phys_pages (void);

/* Return number of available physical pages of memory in the system.  */
extern long int __get_avphys_pages (void);

/* Return maximum number of processes this real user ID can have.  */
extern long int __get_child_max (void);

# endif /* !_ISOMAC */
#endif /* sys/sysinfo.h */
