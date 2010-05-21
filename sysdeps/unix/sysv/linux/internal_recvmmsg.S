#include <kernel-features.h>
#include <sys/syscall.h>
#if !defined __NR_recvmmsg && defined __NR_socketcall
# define socket	recvmmsg
# ifdef __ASSUME_RECVMMSG
#  define __socket recvmmsg
# else
#  define __socket __internal_recvmmsg
# endif
# define NARGS 5
# define NEED_CANCELLATION
# define NO_WEAK_ALIAS
# include <socket.S>
#endif
