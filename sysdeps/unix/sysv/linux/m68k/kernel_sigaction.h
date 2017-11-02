#ifndef _KERNEL_SIGACTION_H
# define _KERNEL_SIGACTION_H

#include <signal.h>

#define SA_RESTORER 0x04000000

/* This is the sigaction structure from the Linux 3.2 kernel.  */
struct kernel_sigaction
{
  __sighandler_t k_sa_handler;
  sigset_t sa_mask;
  unsigned long sa_flags;
  void (*sa_restorer) (void);
};

#define SET_SA_RESTORER(kact, act)			\
  (kact)->sa_restorer = (act)->sa_restorer
#define RESET_SA_RESTORER(act, kact)			\
  (act)->sa_restorer = (kact)->sa_restorer

#endif
