/* tile kernel sigaction is similar to generic Linux UAPI one
   and SA_RESTORER is used only for binary compatibility.  */
#define SA_RESTORER 0x04000000
#include <sysdeps/unix/sysv/linux/kernel_sigaction.h>

#define SET_SA_RESTORER(kact, act)             \
  (kact)->sa_restorer = (act)->sa_restorer
#define RESET_SA_RESTORER(act, kact)           \
  (act)->sa_restorer = (kact)->sa_restorer
