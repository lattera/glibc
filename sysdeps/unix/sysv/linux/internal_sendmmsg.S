#include <kernel-features.h>
#include <sys/syscall.h>

/* Do not use the sendmmsg syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL
# undef __NR_sendmmsg
#endif

#if !defined __NR_sendmmsg && defined __NR_socketcall
# define socket	sendmmsg
# ifndef __ASSUME_SENDMMSG_SOCKETCALL
#  define __socket __internal_sendmmsg
#  define NO_WEAK_ALIAS
# endif
# define NARGS 4
# define NEED_CANCELLATION
# include <socket.S>
# ifdef __ASSUME_SENDMMSG_SOCKETCALL
libc_hidden_def (__sendmmsg)
# endif
#endif
