/* This is the sigaction struction from the Linux 2.1.20 kernel.  */

struct old_kernel_sigaction {
	__sighandler_t sa_handler;
	unsigned long sa_mask;
	unsigned int sa_flags;
};
