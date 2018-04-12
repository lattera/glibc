#include <bits/types/siginfo_t.h>

#define SA_RESTORER	0x04000000

/* This is the sigaction structure from the Linux 3.2 kernel.  */
struct kernel_sigaction
{
  union
  {
    __sighandler_t _sa_handler;
    void (*_sa_sigaction)(int, siginfo_t *, void *);
  } _u;
#define k_sa_handler _u._sa_handler
  /* The 'struct sigaction' definition in s390 kernel header
     arch/s390/include/uapi/asm/signal.h is used for __NR_rt_sigaction
     on 64 bits and for __NR_sigaction for 31 bits.

     The expected layout for __NR_rt_sigaction for 31 bits is either
     'struct sigaction' from include/linux/signal_types.h or
     'struct compat_sigaction' from include/linux/compat.h.

     So for __NR_rt_sigaction we can use the same layout for both s390x
     and s390.  */
  unsigned long sa_flags;
  void (*sa_restorer)(void);
  sigset_t sa_mask;
};

#define SET_SA_RESTORER(kact, act)             \
  (kact)->sa_restorer = (act)->sa_restorer
#define RESET_SA_RESTORER(act, kact)           \
  (act)->sa_restorer = (kact)->sa_restorer
