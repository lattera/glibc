#ifndef _SIGNAL_H
#if defined __need_sig_atomic_t || defined __need_sigset_t
# include <signal/signal.h>
#else
# include <signal/signal.h>

/* Now define the internal interfaces.  */
extern __sighandler_t __bsd_signal __P ((int __sig, __sighandler_t __handler));
extern int __kill __P ((__pid_t __pid, int __sig));
extern int __sigblock __P ((int __mask));
extern int __sigsetmask __P ((int __mask));
extern int __sigprocmask __P ((int __how,
			       __const sigset_t *__set, sigset_t *__oset));
extern int __sigsuspend __P ((__const sigset_t *__set));
extern int __sigwait __P ((__const sigset_t *__set, int *__sig));
extern int __sigwaitinfo __P ((__const sigset_t *__set, siginfo_t *__info));
extern int __sigtimedwait __P ((__const sigset_t *__set, siginfo_t *__info,
				__const struct timespec *__timeout));
extern int __sigqueue __P ((__pid_t __pid, int __sig,
			    __const union sigval __val));
extern int __sigvec __P ((int __sig, __const struct sigvec *__vec,
			  struct sigvec *__ovec));
extern int __sigreturn __P ((struct sigcontext *__scp));
extern int __sigaltstack __P ((__const struct sigaltstack *__ss,
			       struct sigaltstack *__oss));
#endif
#endif
