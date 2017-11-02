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
#ifndef __s390x__
  sigset_t sa_mask;
  unsigned long sa_flags;
  void (*sa_restorer)(void);
#else
  unsigned long sa_flags;
  void (*sa_restorer)(void);
  sigset_t sa_mask;
#endif
};

#define SET_SA_RESTORER(kact, act)             \
  (kact)->sa_restorer = (act)->sa_restorer
#define RESET_SA_RESTORER(act, kact)           \
  (act)->sa_restorer = (kact)->sa_restorer
