/* Interfaces to control the various kernel daemons.  */

#ifndef _SYS_KDAEMON_H
#define _SYS_KDAEMON_H

/* Start, flush, or tune the kernel's buffer flushing daemon.  */
extern int bdflush (int func, long data);

#endif /* _SYS_KDAEMON_H */
