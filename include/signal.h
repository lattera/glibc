#ifndef _SIGNAL_H
#if defined __need_sig_atomic_t || defined __need_sigset_t
# include <signal/signal.h>
#else
# include <signal/signal.h>

/* Now define the internal interfaces.  */
extern __sighandler_t __bsd_signal (int __sig, __sighandler_t __handler);
extern int __kill (__pid_t __pid, int __sig);
extern int __sigblock (int __mask);
extern int __sigsetmask (int __mask);
extern int __sigprocmask (int __how,
			  __const sigset_t *__set, sigset_t *__oset);
extern int __sigsuspend (__const sigset_t *__set);
extern int __sigwait (__const sigset_t *__set, int *__sig);
extern int __sigwaitinfo (__const sigset_t *__set, siginfo_t *__info);
extern int __sigtimedwait (__const sigset_t *__set, siginfo_t *__info,
			   __const struct timespec *__timeout);
extern int __sigqueue (__pid_t __pid, int __sig,
		       __const union sigval __val);
extern int __sigvec (int __sig, __const struct sigvec *__vec,
		     struct sigvec *__ovec);
extern int __sigreturn (struct sigcontext *__scp);
extern int __sigaltstack (__const struct sigaltstack *__ss,
			  struct sigaltstack *__oss);
extern int __libc_sigaction (int sig, const struct sigaction *act,
			     struct sigaction *oact);

extern int __sigpause (int sig_or_mask, int is_sig);
extern int __default_sigpause (int mask);
extern int __xpg_sigpause (int sig);



/* Allocate real-time signal with highest/lowest available priority.  */
extern int __libc_allocate_rtsig (int __high);
#endif
#endif
