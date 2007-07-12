/* SETxID functions which only have to change the local thread and
   none of the possible other threads.  */
#include <kernel-features.h>
#include <sysdep.h>

/* If we can use the syscall directly, use it.  */
#if __ASSUME_32BITUIDS > 0 && defined __NR_setresuid32
# define local_seteuid(id) INLINE_SYSCALL (setresuid32, 3, -1, id, -1)
#elif __ASSUME_SETRESUID_SYSCALL > 0
# define local_seteuid(id) INLINE_SYSCALL (setresuid, 3, -1, id, -1)
#else
# define local_seteuid(id) seteuid (id)
#endif


/* If we can use the syscall directly, use it.  */
#if __ASSUME_32BITUIDS > 0 && defined __NR_setresgid32
# define local_setegid(id) INLINE_SYSCALL (setresgid32, 3, -1, id, -1)
#elif __ASSUME_SETRESGID_SYSCALL > 0
# define local_setegid(id) INLINE_SYSCALL (setresgid, 3, -1, id, -1)
#else
# define local_setegid(id) setegid (id)
#endif
