#ifndef _SYS_SYSCTL_H
#include_next <sys/sysctl.h>

/* Read or write system parameters (Linux specific).  */
extern int __sysctl (int *__name, int __nlen, void *__oldval,
		     size_t *__oldlenp, void *__newval, size_t __newlen);


#endif  /* _SYS_SYSCTL_H */
