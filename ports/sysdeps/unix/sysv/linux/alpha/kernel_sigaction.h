/* This is the sigaction struction from the Linux 2.1.20 kernel.  */

struct old_kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_mask;
	unsigned int sa_flags;
};

/* This is the sigaction structure from the Linux 2.1.68 kernel.  */

struct kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned int sa_flags;
	sigset_t sa_mask;
};
