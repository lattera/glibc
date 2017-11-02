/* SPARC 'struct __new_sigaction' is similar to generic Linux UAPI with
   a sa_restorer field, even though function is passed as an argument
   to rt_sigaction syscall.  */
#define SA_RESTORER 0x04000000
#include <sysdeps/unix/sysv/linux/kernel_sigaction.h>

#define SET_SA_RESTORER(kact, act)             \
  (kact)->sa_restorer = NULL
#define RESET_SA_RESTORER(act, kact)           \
  (act)->sa_restorer = (kact)->sa_restorer
