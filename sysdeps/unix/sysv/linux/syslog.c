#include <kernel-features.h>

#if __ASSUME_MSG_NOSIGNAL
# define NO_SIGPIPE
# define send_flags MSG_NOSIGNAL
#else
# define send_flags 0
#endif

#include <misc/syslog.c>
