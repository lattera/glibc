#ifndef _SYS_KLOG_H
#define _SYS_KLOG_H

/* Control the kernel's logging facility.  This corresponds exactly to
   the kernel's syslog system call, but that name is easily confused
   with the user-level syslog facility, which is something completely
   different.  */
extern int klogctl __P((int type, char *bufp, int len));

#endif /* _SYS_KLOG_H */
