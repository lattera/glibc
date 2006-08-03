#ifndef _SYS_MSG_H
#include <sysvipc/sys/msg.h>

extern ssize_t __libc_msgrcv (int msqid, void *msgp, size_t msgsz,
			      long int msgtyp, int msgflg);
extern int __libc_msgsnd (int msqid, const void *msgp, size_t msgsz,
			  int msgflg);

#endif
