#include <kernel-features.h>
#include <sys/syscall.h>

/* Do not use the recvmmsg syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_RECVMMSG_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_RECVMMSG_SYSCALL
# undef __NR_recvmmsg
#endif

#if !defined __NR_recvmmsg && defined __NR_socketcall
# define socket	recvmmsg
# ifdef __ASSUME_RECVMMSG_SOCKETCALL
#  define __socket recvmmsg
# else
#  define __socket __internal_recvmmsg
# endif
# define NARGS 5
# define NEED_CANCELLATION
# define NO_WEAK_ALIAS
# include <socket.S>
#endif
