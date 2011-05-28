#include <kernel-features.h>
#include <sys/syscall.h>
#if !defined __NR_sendmmsg && defined __NR_socketcall
# define socket	sendmmsg
# ifdef __ASSUME_SENDMMSG
#  define __socket sendmmsg
# else
#  define __socket __internal_sendmmsg
# endif
# define NARGS 4
# define NEED_CANCELLATION
# define NO_WEAK_ALIAS
# include <socket.S>
#endif
