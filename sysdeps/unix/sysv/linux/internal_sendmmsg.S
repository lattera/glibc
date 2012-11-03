#include <kernel-features.h>
#include <sys/syscall.h>
#if !defined __NR_sendmmsg && defined __NR_socketcall
# define socket	sendmmsg
# ifndef __ASSUME_SENDMMSG
#  define __socket __internal_sendmmsg
#  define NO_WEAK_ALIAS
# endif
# define NARGS 4
# define NEED_CANCELLATION
# include <socket.S>
# ifdef __ASSUME_SENDMMSG
libc_hidden_def (__sendmmsg)
# endif
#endif
