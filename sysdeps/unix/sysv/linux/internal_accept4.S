#include <kernel-features.h>
#include <sys/syscall.h>
#if !defined __NR_accept4 && defined __NR_socketcall
# define socket	accept4
# ifdef __ASSUME_ACCEPT4
#  define __socket accept4
# else
#  define __socket __internal_accept4
# endif
# define NARGS 4
# define NEED_CANCELLATION
# define NO_WEAK_ALIAS
# include <socket.S>
#endif
