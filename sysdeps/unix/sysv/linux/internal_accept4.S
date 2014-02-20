#include <kernel-features.h>
#include <sys/syscall.h>

/* Do not use the accept4 syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_ACCEPT4_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_ACCEPT4_SYSCALL
# undef __NR_accept4
#endif

#if !defined __NR_accept4 && defined __NR_socketcall
# define socket	accept4
# ifdef __ASSUME_ACCEPT4_SOCKETCALL
#  define __socket accept4
# else
#  define __socket __internal_accept4
# endif
# define NARGS 4
# define NEED_CANCELLATION
# define NO_WEAK_ALIAS
# include <socket.S>
#endif
