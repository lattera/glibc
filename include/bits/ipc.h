#include_next <bits/ipc.h>

__BEGIN_DECLS

/* The actual system call: all functions are multiplexed by this.  */
extern int __syscall_ipc (int __call, int __first, int __second,
			  int __third, void *__ptr);

__END_DECLS


/* The codes for the functions to use the multiplexer `__syscall_ipc'.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24
